#!/usr/bin/env python3
"""Orchestrate clean IDA Pro 5.5 exports for MAGIC and WIZARDS."""

from __future__ import annotations

import argparse
import os
import re
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable, List, Optional, Sequence


DEFAULT_IDB_DIR = r"C:\STU_DASM\IDA55_MOM131"
DEFAULT_IDAW_WRAPPER = Path("ida_automation") / "idaw.py"
DEFAULT_GAMES = ("MAGIC", "WIZARDS")
OUTPUT_SUFFIXES = (".map", ".asm", ".inc", ".lst")
REGISTRY_KEY = r"HKCU\Software\Hex-Rays\IDA"
DEFAULT_LOG_DIR = Path("out") / "ida55-clean-export-logs"
DEFAULT_TIMEOUT_SECONDS = 120


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
    parser.add_argument("--copy", action="store_true", help="Copy exported MAP/ASM/INC/LST files into in/ after export.")
    parser.add_argument("--post-process", action="store_true", help="Run run_post_ida55_dump_pipeline.py after copy/export.")
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
    wrapper_path = repo_root / DEFAULT_IDAW_WRAPPER
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


def build_ida_command(python: str, idaw_wrapper: Path, script_path: Path, idb_path: Path, log_path: Optional[Path] = None) -> List[str]:
    command = [python, "-B", str(idaw_wrapper), "--", "-A"]
    if log_path is not None:
        command.append(f"-L{log_path}")
    command.extend([f"-S{script_path}", str(idb_path)])
    return command


def build_registry_export_command(output_path: Path) -> List[str]:
    return ["reg", "export", REGISTRY_KEY, str(output_path), "/y"]


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
                transcript_handle.write(f"\n[timeout]\nProcess exceeded {timeout_seconds} seconds; terminating.\n")
                transcript_handle.flush()
            process.terminate()
            try:
                process.wait(timeout=10)
            except subprocess.TimeoutExpired:
                if transcript_handle is not None:
                    transcript_handle.write("Terminate did not finish within 10 seconds; killing.\n")
                    transcript_handle.flush()
                process.kill()
                process.wait()
            raise TimeoutError(f"Command timed out after {timeout_seconds} seconds: {' '.join(command)}") from error
        except KeyboardInterrupt:
            if transcript_handle is not None:
                transcript_handle.write("\n[interrupted]\nKeyboardInterrupt received; terminating child process.\n")
                transcript_handle.flush()
            process.terminate()
            try:
                process.wait(timeout=10)
            except subprocess.TimeoutExpired:
                process.kill()
                process.wait()
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
) -> None:
    if not dry_run:
        log_dir.mkdir(parents=True, exist_ok=True)
    for job in jobs:
        if not job.idb_path.exists():
            if dry_run:
                summary.add_skipped(f"{job.game}: IDB missing during dry-run: {denormalize_for_display(job.idb_path)}")
                print(f"would require IDB: {job.idb_path}")
                continue
            raise FileNotFoundError(f"IDB not found for {job.game}: {job.idb_path}")
        summary.add_check(f"{job.game}: IDB exists: {denormalize_for_display(job.idb_path)}")
        ida_log_path = log_dir / f"{job.game}.ida.log"
        transcript_path = log_dir / f"{job.game}.process.log"
        command = build_ida_command(python, idaw_wrapper, script_path, job.idb_path, None if no_ida_log else ida_log_path)
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
            raise RuntimeError(f"IDA export failed for {job.game} with exit code {result}.")


def run_copy(project_root: Path, script_dir: Path, python: str, dry_run: bool) -> None:
    command = [python, str(script_dir / "ida55_copy_outputs.py")]
    if dry_run:
        command.append("--dry-run")
    result = run_command(command, project_root, dry_run)
    if result != 0:
        raise RuntimeError(f"Copy step failed with exit code {result}.")


def run_post_process(project_root: Path, python: str, dry_run: bool) -> None:
    command = [python, "-B", str(project_root / "run_post_ida55_dump_pipeline.py")]
    result = run_command(command, project_root, dry_run)
    if result != 0:
        raise RuntimeError(f"Post-processing failed with exit code {result}.")


def print_plan(python: str, idaw_wrapper: Path, script_path: Path, jobs: Sequence[ExportJob]) -> None:
    print(f"IDAW wrapper:   {python} -B {denormalize_for_display(idaw_wrapper)}")
    print(f"IDC script:     {denormalize_for_display(script_path)}")
    for job in jobs:
        print(f"{job.game} IDB:      {denormalize_for_display(job.idb_path)}")
        for output_path in expected_outputs_for_idb(job.idb_path):
            print(f"  expects:     {denormalize_for_display(output_path)}")


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
        idaw_wrapper = resolve_idaw_wrapper(repo_root)
        summary.add_check(f"IDAW wrapper resolved: {denormalize_for_display(idaw_wrapper)}")
        check_readable_file(script_path, "IDC script", summary)

        jobs = build_jobs(games, idb_dir)
        print_plan(args.python, idaw_wrapper, script_path, jobs)

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
        )

        if not args.skip_verify and not args.dry_run:
            for job in jobs:
                verify_files(expected_outputs_for_idb(job.idb_path))
                summary.add_check(f"{job.game}: expected output files exist and are non-empty.")
        elif args.dry_run:
            summary.add_skipped("Output verification skipped because --dry-run is set.")
            summary.add_unproven("No MAP/ASM/INC/LST files were created or verified.")
        else:
            summary.add_skipped("Output verification disabled by --skip-verify.")

        if not args.skip_registry_snapshot:
            snapshot_registry(repo_root, "after", args.dry_run, args.allow_registry_snapshot_failure)
            if not args.dry_run:
                summary.add_action("Registry snapshot after export completed.")

        if args.copy:
            run_copy(repo_root, script_dir, args.python, args.dry_run)
            if args.dry_run:
                summary.add_skipped("Copy step command printed but not executed because --dry-run is set.")
            else:
                summary.add_action("Copy step completed.")
        else:
            summary.add_skipped("Copy step not requested; pass --copy to enable it.")

        if args.post_process:
            run_post_process(repo_root, args.python, args.dry_run)
            if args.dry_run:
                summary.add_skipped("Post-processing command printed but not executed because --dry-run is set.")
            else:
                summary.add_action("Post-processing completed.")
        else:
            summary.add_skipped("Post-processing not requested; pass --post-process to enable it.")

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
