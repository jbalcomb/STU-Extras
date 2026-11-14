#!/usr/bin/env python3
"""Orchestrate clean IDA Pro 5.5 exports for MAGIC and WIZARDS, running IDA on a work copy of each IDB.

For each game:
  1. idaw.py copies <idb-dir>\\<GAME>.idb into a new work folder (<work-root>\\<GAME>-clean-<timestamp>) and runs
     ida55_clean_export.idc on the copy. The real IDB is never opened by IDA. (The IDC changes export settings and
     expands hidden areas while it works; on a copy those changes are thrown away.)
  2. The IDC writes <GAME>.map/.asm/.inc/.lst plus settings dumps next to the copy.
  3. The four exports are copied into <out-dir> (default in\\); the settings dumps go to the log folder.
  4. The work folder is deleted on success (--keep-work keeps it). If IDA hangs, is killed or damages the copy,
     the real IDB is unchanged: delete the work folder and re-run.
"""

from __future__ import annotations

import argparse
import os
import re
import shutil
import subprocess
import sys
from dataclasses import dataclass
from datetime import datetime
from pathlib import Path
from typing import Iterable, List, Optional, Sequence


DEFAULT_IDB_DIR = r"C:\STU_DASM\IDA55_MOM131"
DEFAULT_IDA_EXE = r"C:\STU_DASM\IDA55\idaw.exe"
# Shared IDA launcher, relative to the Piethawn repo root (STU-Extras/Piethawn/Piethawn -> STU-Extras/IDA55-Py).
DEFAULT_IDAW_WRAPPER = Path("..") / ".." / "IDA55-Py" / "ida55py" / "idaw.py"
DEFAULT_GAMES = ("MAGIC", "WIZARDS")
OUTPUT_SUFFIXES = (".map", ".asm", ".inc", ".lst")
SETTINGS_DUMP_SUFFIXES = (".ida55-clean-export-before.txt", ".ida55-clean-export-active.txt", ".ida55-clean-export-after.txt")
REGISTRY_KEY = r"HKCU\Software\Hex-Rays\IDA"
DEFAULT_OUT_DIR = Path("in")
DEFAULT_WORK_ROOT = Path("out") / "ida55-work"
DEFAULT_LOG_DIR = Path("out") / "ida55-clean-export-logs"
DEFAULT_TIMEOUT_SECONDS = 900  # WIZARDS takes over 2 minutes to generate and re-pack.


@dataclass(frozen=True)
class ExportJob:
    game: str
    idb_path: Path


@dataclass
class RunSummary:
    checks: List[str]
    actions: List[str]
    skipped: List[str]
    unproven: List[str]

    def add_check(self, message: str) -> None:
        self.checks.append(message)

    def add_action(self, message: str) -> None:
        self.actions.append(message)

    def add_skipped(self, message: str) -> None:
        self.skipped.append(message)

    def add_unproven(self, message: str) -> None:
        self.unproven.append(message)


def make_summary() -> RunSummary:
    return RunSummary(checks=[], actions=[], skipped=[], unproven=[])


def parse_args(argv: Optional[Sequence[str]] = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Run clean IDA 5.5 exports for MAGIC/WIZARDS IDBs.")
    parser.add_argument("--idb-dir", default=DEFAULT_IDB_DIR, help=f"Directory containing MAGIC/WIZARDS IDBs. Default: {DEFAULT_IDB_DIR}")
    parser.add_argument("--ida-exe", default=DEFAULT_IDA_EXE, help=f"IDA executable passed to idaw.py. Default: {DEFAULT_IDA_EXE}")
    parser.add_argument("--script", type=Path, default=Path("ida55_clean_export.idc"), help="IDC export script.")
    parser.add_argument("--game", choices=DEFAULT_GAMES, action="append", help="Game to export. May be specified twice.")
    parser.add_argument("--dry-run", action="store_true", help="Print planned actions without running IDA or child scripts.")
    parser.add_argument("--skip-registry-snapshot", action="store_true", help="Do not export before/after registry snapshots.")
    parser.add_argument("--allow-registry-snapshot-failure", action="store_true", help="Continue if reg export fails.")
    parser.add_argument("--skip-verify", action="store_true", help="Do not verify expected exported files after IDA runs.")
    parser.add_argument("--log-dir", type=Path, default=DEFAULT_LOG_DIR, help=f"Directory for IDA/process logs. Default: {DEFAULT_LOG_DIR}")
    parser.add_argument("--no-ida-log", action="store_true", help="Do not pass IDA's -L logfile option.")
    parser.add_argument("--timeout-seconds", type=int, default=DEFAULT_TIMEOUT_SECONDS, help=f"Seconds to wait for each IDA run. Use 0 to disable. Default: {DEFAULT_TIMEOUT_SECONDS}")
    parser.add_argument("--no-startup-enter", action="store_true", help="Do not send an initial Enter to IDA after launch.")
    parser.add_argument("--out-dir", type=Path, default=DEFAULT_OUT_DIR, help=f"Where MAP/ASM/INC/LST are copied. Default: {DEFAULT_OUT_DIR}")
    parser.add_argument("--work-root", type=Path, default=DEFAULT_WORK_ROOT, help=f"Parent of the per-run work folders. Default: {DEFAULT_WORK_ROOT}")
    parser.add_argument("--keep-work", action="store_true", help="Keep the work folder (IDB copy and raw outputs) after a successful export.")
    parser.add_argument("--copy", action="store_true", help=argparse.SUPPRESS)  # Old flag: outputs now always go to --out-dir.
    parser.add_argument("--python", default=sys.executable, help="Python interpreter for child scripts.")
    return parser.parse_args(argv)


def normalize_path(path_value: str | Path, repo_root: Path) -> Path:
    path_text = str(path_value)
    if os.name != "nt":
        drive_match = re.match(r"^([A-Za-z]):[\\/](.*)$", path_text)
        if drive_match:
            drive = drive_match.group(1).lower()
            rest = drive_match.group(2).replace("\\", "/")
            return Path("/mnt") / drive / rest

    path = Path(path_text)
    return path if path.is_absolute() else repo_root / path


def denormalize_for_display(path: Path) -> str:
    text = str(path)
    if os.name != "nt":
        match = re.match(r"^/mnt/([A-Za-z])/(.*)$", text)
        if match:
            return f"{match.group(1).upper()}:\\" + match.group(2).replace("/", "\\")
    return text


def resolve_idaw_wrapper(repo_root: Path) -> Path:
    """Locate the shared IDA55-Py idaw.py launcher relative to the Piethawn repo root. Powered by Claude."""
    wrapper_path = (repo_root / DEFAULT_IDAW_WRAPPER).resolve()
    if not wrapper_path.exists():
        raise FileNotFoundError(f"IDAW wrapper not found: {wrapper_path}")
    if not wrapper_path.is_file():
        raise FileNotFoundError(f"IDAW wrapper is not a file: {wrapper_path}")
    return wrapper_path

def build_jobs(games: Sequence[str], idb_dir: Path) -> List[ExportJob]:
    return [ExportJob(game=game, idb_path=idb_dir / f"{game}.idb") for game in games]


def expected_outputs_for_idb(idb_path: Path) -> List[Path]:
    base = idb_path.with_suffix("")
    return [base.with_suffix(suffix) for suffix in OUTPUT_SUFFIXES]


def work_dir_for_job(work_root: Path, game: str, stamp: str) -> Path:
    """Return the per-run work folder <work_root>/<GAME>-clean-<stamp>. Powered by Claude."""
    return work_root / f"{game}-clean-{stamp}"


def work_idb_for_job(work_root: Path, job: "ExportJob", stamp: str) -> Path:
    """Return where idaw.py puts the IDB copy for this job. Powered by Claude."""
    return work_dir_for_job(work_root, job.game, stamp) / job.idb_path.name


def copy_file_atomically(source: Path, destination: Path) -> None:
    """Copy source to destination through a temporary file, so a failed copy never leaves a partial file. Powered by Claude."""
    temp_path = destination.with_name(destination.name + ".tmp")
    shutil.copy2(source, temp_path)
    os.replace(temp_path, destination)


def collect_outputs(work_idb: Path, out_dir: Path, log_dir: Path, game: str, dry_run: bool) -> List[Path]:
    """Copy MAP/ASM/INC/LST from the work folder into out_dir, and the settings dumps into log_dir. Powered by Claude."""
    copied: List[Path] = []
    for source in expected_outputs_for_idb(work_idb):
        destination = out_dir / source.name
        print(f"{'would copy' if dry_run else 'copy'} {denormalize_for_display(source)} -> {denormalize_for_display(destination)}")
        if not dry_run:
            copy_file_atomically(source, destination)
        copied.append(destination)
    base = work_idb.with_suffix("")
    for suffix in SETTINGS_DUMP_SUFFIXES:
        source = Path(str(base) + suffix)
        if not dry_run and source.exists():
            shutil.copy2(source, log_dir / f"{game}{suffix}")
    return copied


def build_ida_command(
    python: str,
    idaw_wrapper: Path,
    script_path: Path,
    idb_path: Path,
    log_path: Optional[Path] = None,
    *,
    ida_exe: Optional[Path] = None,
    backup_dir: Optional[Path] = None,
    work_dir: Optional[Path] = None,
) -> List[str]:
    """Build the idaw.py command line; idaw.py options go before '--', IDA's after. Powered by Claude."""
    command = [python, "-B", str(idaw_wrapper)]
    if ida_exe is not None:
        command.extend(["--ida-exe", str(ida_exe)])
    if backup_dir is not None:
        command.extend(["--backup-dir", str(backup_dir)])
    if work_dir is not None:
        command.extend(["--work-dir", str(work_dir)])
    command.extend(["--", "-A"])
    if log_path is not None:
        command.append(f"-L{log_path}")
    command.extend([f"-S{script_path}", str(idb_path)])
    return command


def build_registry_export_command(output_path: Path) -> List[str]:
    return ["reg", "export", REGISTRY_KEY, str(output_path), "/y"]


def terminate_process_tree(process: subprocess.Popen) -> None:
    """Kill a child process plus any grandchildren (idaw.py and the idaw.exe it started). Powered by Claude."""
    if os.name == "nt":
        subprocess.run(["taskkill", "/T", "/F", "/PID", str(process.pid)], capture_output=True, check=False)
    else:
        process.kill()
    try:
        process.wait(timeout=30)
    except subprocess.TimeoutExpired:
        process.kill()
        process.wait()


def run_command(
    command: Sequence[str],
    cwd: Path,
    dry_run: bool,
    transcript_path: Optional[Path] = None,
    timeout_seconds: Optional[int] = None,
    startup_enter: bool = False,
) -> int:
    print("+", " ".join(command))
    if dry_run:
        return 0
    transcript_handle = None
    try:
        if transcript_path is not None:
            transcript_path.parent.mkdir(parents=True, exist_ok=True)
            transcript_handle = transcript_path.open("w", encoding="utf-8", errors="replace")
            transcript_handle.write("command=" + " ".join(command) + "\n")
            transcript_handle.write(f"timeout_seconds={timeout_seconds if timeout_seconds else 'disabled'}\n")
            transcript_handle.write(f"startup_enter={startup_enter}\n")
            transcript_handle.write("\n[process output]\n")
            transcript_handle.flush()

        process = subprocess.Popen(
            command,
            cwd=cwd,
            stdin=subprocess.PIPE if startup_enter else None,
            stdout=transcript_handle or None,
            stderr=subprocess.STDOUT if transcript_handle is not None else None,
            text=True,
        )

        if startup_enter and process.stdin is not None:
            process.stdin.write("\n")
            process.stdin.flush()
            process.stdin.close()

        try:
            return_code = process.wait(timeout=timeout_seconds if timeout_seconds and timeout_seconds > 0 else None)
        except subprocess.TimeoutExpired as error:
            if transcript_handle is not None:
                transcript_handle.write(f"\n[timeout]\nProcess exceeded {timeout_seconds} seconds; killing the process tree.\n")
                transcript_handle.flush()
            terminate_process_tree(process)
            raise TimeoutError(
                f"Command timed out after {timeout_seconds} seconds (only the work copy is at risk; "
                f"delete its work folder and re-run, or raise --timeout-seconds): {' '.join(command)}"
            ) from error
        except KeyboardInterrupt:
            if transcript_handle is not None:
                transcript_handle.write("\n[interrupted]\nKeyboardInterrupt received; killing the process tree.\n")
                transcript_handle.flush()
            terminate_process_tree(process)
            raise

        if transcript_handle is not None:
            transcript_handle.write(f"\n[returncode]\n{return_code}\n")
            transcript_handle.flush()
        return return_code
    finally:
        if transcript_handle is not None:
            transcript_handle.close()


def tail_text(path: Path, line_count: int = 40) -> str:
    if not path.exists():
        return ""
    lines = path.read_text(encoding="utf-8", errors="replace").splitlines()
    return "\n".join(lines[-line_count:])


def add_log_tail(summary: RunSummary, label: str, path: Path) -> None:
    tail = tail_text(path)
    display_path = denormalize_for_display(path)
    if tail:
        summary.add_unproven(f"{label} tail from {display_path}:\n{tail}")
    else:
        summary.add_unproven(f"{label} was not created or was empty: {display_path}")


def check_readable_file(path: Path, label: str, summary: RunSummary) -> None:
    if not path.exists():
        raise FileNotFoundError(f"{label} not found: {path}")
    if not path.is_file():
        raise FileNotFoundError(f"{label} is not a file: {path}")
    summary.add_check(f"{label} exists: {denormalize_for_display(path)}")


def verify_files(paths: Iterable[Path]) -> None:
    missing: List[str] = []
    empty: List[str] = []
    for path in paths:
        if not path.exists():
            missing.append(str(path))
        elif path.stat().st_size == 0:
            empty.append(str(path))

    if missing or empty:
        details = []
        if missing:
            details.append("Missing files:\n" + "\n".join(f"- {path}" for path in missing))
        if empty:
            details.append("Empty files:\n" + "\n".join(f"- {path}" for path in empty))
        raise RuntimeError("\n".join(details))


def snapshot_registry(repo_root: Path, label: str, dry_run: bool, allow_failure: bool) -> int:
    output_path = repo_root / f"ida55-reg-{label}.reg"
    command = build_registry_export_command(output_path)
    result = run_command(command, repo_root, dry_run)
    if result != 0 and not allow_failure:
        raise RuntimeError(f"Registry snapshot failed for {label} with exit code {result}.")
    return result


def run_exports(
    python: str,
    idaw_wrapper: Path,
    script_path: Path,
    jobs: Sequence[ExportJob],
    repo_root: Path,
    dry_run: bool,
    summary: RunSummary,
    log_dir: Path,
    no_ida_log: bool,
    timeout_seconds: int,
    startup_enter: bool,
    ida_exe: Optional[Path] = None,
    work_root: Optional[Path] = None,
    stamp: str = "",
    out_dir: Optional[Path] = None,
    skip_verify: bool = False,
    keep_work: bool = False,
) -> None:
    """Export each job on a work copy, then collect its outputs and remove the work folder. Powered by Claude."""
    work_root = work_root or DEFAULT_WORK_ROOT
    out_dir = out_dir or DEFAULT_OUT_DIR
    if not dry_run:
        log_dir.mkdir(parents=True, exist_ok=True)
        out_dir.mkdir(parents=True, exist_ok=True)
    for job in jobs:
        work_dir = work_dir_for_job(work_root, job.game, stamp)
        work_idb = work_idb_for_job(work_root, job, stamp)
        if not job.idb_path.exists():
            if dry_run:
                summary.add_skipped(f"{job.game}: IDB missing during dry-run: {denormalize_for_display(job.idb_path)}")
                print(f"would require IDB: {job.idb_path}")
                continue
            raise FileNotFoundError(f"IDB not found for {job.game}: {job.idb_path}")
        summary.add_check(f"{job.game}: IDB exists: {denormalize_for_display(job.idb_path)}")
        ida_log_path = log_dir / f"{job.game}.ida.log"
        transcript_path = log_dir / f"{job.game}.process.log"
        command = build_ida_command(
            python,
            idaw_wrapper,
            script_path,
            job.idb_path,
            None if no_ida_log else ida_log_path,
            ida_exe=ida_exe,
            work_dir=work_dir,
        )
        print(f"{job.game} work:     {denormalize_for_display(work_dir)}")
        result = run_command(
            command,
            repo_root,
            dry_run,
            None if dry_run else transcript_path,
            timeout_seconds=timeout_seconds,
            startup_enter=startup_enter,
        )
        if dry_run:
            summary.add_skipped(f"{job.game}: IDA was not launched because --dry-run is set.")
            summary.add_unproven(f"{job.game}: IDC syntax, IDA exit code, settings restore, and generated files are untested.")
            if not no_ida_log:
                summary.add_skipped(f"{job.game}: IDA log would be written to {denormalize_for_display(ida_log_path)}.")
            summary.add_skipped(f"{job.game}: process transcript would be written to {denormalize_for_display(transcript_path)}.")
            if startup_enter:
                summary.add_skipped(f"{job.game}: wrapper would send an initial Enter to IDA after launch.")
            if timeout_seconds > 0:
                summary.add_skipped(f"{job.game}: wrapper would time out IDA after {timeout_seconds} seconds.")
        else:
            summary.add_action(f"{job.game}: IDA export command completed with exit code {result}.")
            if not no_ida_log:
                summary.add_action(f"{job.game}: IDA log path: {denormalize_for_display(ida_log_path)}")
            summary.add_action(f"{job.game}: process transcript path: {denormalize_for_display(transcript_path)}")
        if result != 0:
            if not no_ida_log:
                add_log_tail(summary, f"{job.game}: IDA log", ida_log_path)
            add_log_tail(summary, f"{job.game}: process transcript", transcript_path)
            raise RuntimeError(
                f"IDA export failed for {job.game} with exit code {result}. "
                f"The real IDB is unchanged; delete the work folder: {denormalize_for_display(work_dir)}"
            )

        if dry_run:
            collect_outputs(work_idb, out_dir, log_dir, job.game, dry_run=True)
            continue
        if not skip_verify:
            verify_files(expected_outputs_for_idb(work_idb))
            summary.add_check(f"{job.game}: MAP/ASM/INC/LST exist and are non-empty in the work folder.")
        copied = collect_outputs(work_idb, out_dir, log_dir, job.game, dry_run=False)
        summary.add_action(f"{job.game}: copied {', '.join(path.name for path in copied)} into {denormalize_for_display(out_dir)}")
        if keep_work:
            summary.add_action(f"{job.game}: kept work folder {denormalize_for_display(work_dir)}")
        else:
            shutil.rmtree(work_dir)
            summary.add_action(f"{job.game}: removed work folder {denormalize_for_display(work_dir)}")


def print_plan(python: str, idaw_wrapper: Path, script_path: Path, jobs: Sequence[ExportJob], out_dir: Path = DEFAULT_OUT_DIR) -> None:
    print(f"IDAW wrapper:   {python} -B {denormalize_for_display(idaw_wrapper)}")
    print(f"IDC script:     {denormalize_for_display(script_path)}")
    for job in jobs:
        print(f"{job.game} IDB:      {denormalize_for_display(job.idb_path)}  (read only; IDA runs on a work copy)")
        for output_path in expected_outputs_for_idb(job.idb_path):
            print(f"  writes:      {denormalize_for_display(out_dir / output_path.name)}")


def print_summary(summary: RunSummary, dry_run: bool) -> None:
    print("")
    print("Summary")
    print(f"Mode: {'dry-run' if dry_run else 'execute'}")

    if summary.checks:
        print("Worked/checked:")
        for item in summary.checks:
            print(f"- {item}")

    if summary.actions:
        print("Actions completed:")
        for item in summary.actions:
            print(f"- {item}")

    if summary.skipped:
        print("Skipped:")
        for item in summary.skipped:
            print(f"- {item}")

    if summary.unproven:
        print("Not proven by this run:")
        for item in summary.unproven:
            print(f"- {item}")


def main(argv: Optional[Sequence[str]] = None) -> int:
    args = parse_args(argv)
    summary = make_summary()
    script_dir = Path(__file__).resolve().parent
    repo_root = script_dir.parent

    try:
        games = args.game if args.game else list(DEFAULT_GAMES)
        idb_dir = normalize_path(args.idb_dir, repo_root)
        script_path = normalize_path(args.script, script_dir)
        log_dir = normalize_path(args.log_dir, repo_root)
        out_dir = normalize_path(args.out_dir, repo_root)
        work_root = normalize_path(args.work_root, repo_root)
        stamp = datetime.now().strftime("%Y%m%d%H%M%S")
        idaw_wrapper = resolve_idaw_wrapper(repo_root)
        summary.add_check(f"IDAW wrapper resolved: {denormalize_for_display(idaw_wrapper)}")
        check_readable_file(script_path, "IDC script", summary)

        jobs = build_jobs(games, idb_dir)
        print_plan(args.python, idaw_wrapper, script_path, jobs, out_dir)

        if not args.skip_registry_snapshot:
            snapshot_registry(repo_root, "before", args.dry_run, args.allow_registry_snapshot_failure)
            if args.dry_run:
                summary.add_skipped("Registry snapshot commands were not executed because --dry-run is set.")
            else:
                summary.add_action("Registry snapshot before export completed.")
        else:
            summary.add_skipped("Registry snapshots disabled by --skip-registry-snapshot.")

        run_exports(
            args.python,
            idaw_wrapper,
            script_path,
            jobs,
            repo_root,
            args.dry_run,
            summary,
            log_dir,
            args.no_ida_log,
            args.timeout_seconds,
            not args.no_startup_enter,
            ida_exe=normalize_path(args.ida_exe, repo_root),
            work_root=work_root,
            stamp=stamp,
            out_dir=out_dir,
            skip_verify=args.skip_verify,
            keep_work=args.keep_work,
        )

        if args.dry_run:
            summary.add_skipped("Output verification and copy skipped because --dry-run is set.")
            summary.add_unproven("No MAP/ASM/INC/LST files were created or verified.")
        elif args.skip_verify:
            summary.add_skipped("Output verification disabled by --skip-verify.")

        if not args.skip_registry_snapshot:
            snapshot_registry(repo_root, "after", args.dry_run, args.allow_registry_snapshot_failure)
            if not args.dry_run:
                summary.add_action("Registry snapshot after export completed.")

        print_summary(summary, args.dry_run)
        return 0
    except Exception as error:
        summary.add_unproven(f"Run stopped before completion: {error}")
        print_summary(summary, args.dry_run)
        raise
    except KeyboardInterrupt:
        summary.add_unproven("Run interrupted by Ctrl-C.")
        print_summary(summary, args.dry_run)
        return 130


if __name__ == "__main__":
    raise SystemExit(main())
