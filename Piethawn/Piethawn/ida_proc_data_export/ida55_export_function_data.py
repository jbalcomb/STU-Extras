#!/usr/bin/env python3
"""Export IDA 5.5 function data dumps through the shared IDA launcher."""

from __future__ import annotations

import argparse
import os
import re
import shutil
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import List, Optional, Sequence


DEFAULT_IDB_DIR = r"C:\STU_DASM\IDA55_MOM131"
DEFAULT_IDAW_WRAPPER = Path("ida_automation") / "idaw.py"
DEFAULT_GAMES = ("MAGIC", "WIZARDS")
DEFAULT_LOG_DIR = Path("out") / "ida55-function-data-export-logs"
DEFAULT_TIMEOUT_SECONDS = 0


@dataclass(frozen=True)
class FunctionDataExportJob:
    game: str
    idb_path: Path


def parse_args(argv: Optional[Sequence[str]] = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Run IDA 5.5 function data exports for MAGIC/WIZARDS IDBs.")
    parser.add_argument("--idb-dir", default=DEFAULT_IDB_DIR, help=f"Directory containing MAGIC/WIZARDS IDBs. Default: {DEFAULT_IDB_DIR}")
    parser.add_argument("--script", type=Path, default=Path("ida55_dump_functions.idc"), help="IDC function dump script.")
    parser.add_argument("--game", choices=DEFAULT_GAMES, action="append", help="Game to export. May be specified twice.")
    parser.add_argument("--dry-run", action="store_true", help="Print planned actions without running IDA or copying files.")
    parser.add_argument("--skip-verify", action="store_true", help="Do not verify expected dump files after IDA runs.")
    parser.add_argument("--copy", action="store_true", help="Copy generated function dumps into in/ after export.")
    parser.add_argument("--log-dir", type=Path, default=DEFAULT_LOG_DIR, help=f"Directory for IDA/process logs. Default: {DEFAULT_LOG_DIR}")
    parser.add_argument("--no-ida-log", action="store_true", help="Do not pass IDA's -L logfile option.")
    parser.add_argument("--timeout-seconds", type=int, default=DEFAULT_TIMEOUT_SECONDS, help=f"Seconds to wait for each IDA run. Use 0 to disable. Default: {DEFAULT_TIMEOUT_SECONDS}")
    parser.add_argument("--python", default=sys.executable, help="Python interpreter for child scripts.")
    return parser.parse_args(argv)


def normalize_path(path_value: str | Path, base_dir: Path) -> Path:
    path_text = str(path_value)
    if os.name != "nt":
        drive_match = re.match(r"^([A-Za-z]):[\\/](.*)$", path_text)
        if drive_match:
            drive = drive_match.group(1).lower()
            rest = drive_match.group(2).replace("\\", "/")
            return Path("/mnt") / drive / rest

    path = Path(path_text)
    return path if path.is_absolute() else base_dir / path


def denormalize_for_display(path: Path) -> str:
    text = str(path)
    if os.name != "nt":
        match = re.match(r"^/mnt/([A-Za-z])/(.*)$", text)
        if match:
            return f"{match.group(1).upper()}:\\" + match.group(2).replace("/", "\\")
    return text


def resolve_repo_root() -> Path:
    return Path(__file__).resolve().parent.parent


def resolve_idaw_wrapper(repo_root: Path) -> Path:
    wrapper = repo_root / DEFAULT_IDAW_WRAPPER
    if not wrapper.is_file():
        raise FileNotFoundError(f"IDAW wrapper not found: {wrapper}")
    return wrapper


def build_jobs(games: Sequence[str], idb_dir: Path) -> List[FunctionDataExportJob]:
    return [FunctionDataExportJob(game=game, idb_path=idb_dir / f"{game}.idb") for game in games]


def expected_dump_for_idb(idb_path: Path) -> Path:
    return Path(str(idb_path) + ".ida55-functions.txt")


def repo_dump_path(repo_root: Path, idb_path: Path) -> Path:
    return repo_root / "in" / expected_dump_for_idb(idb_path).name


def build_ida_command(python: str, idaw_wrapper: Path, script_path: Path, idb_path: Path, log_path: Optional[Path] = None) -> List[str]:
    command = [python, "-B", str(idaw_wrapper), "--", "-A"]
    if log_path is not None:
        command.append(f"-L{log_path}")
    command.extend([f"-S{script_path}", str(idb_path)])
    return command


def run_command(command: Sequence[str], cwd: Path, dry_run: bool, timeout_seconds: int) -> int:
    print("+", " ".join(command))
    if dry_run:
        return 0
    return subprocess.run(command, cwd=cwd, timeout=timeout_seconds if timeout_seconds > 0 else None).returncode


def verify_file(path: Path, label: str) -> None:
    if not path.exists():
        raise FileNotFoundError(f"{label} not found: {denormalize_for_display(path)}")
    if path.stat().st_size == 0:
        raise RuntimeError(f"{label} is empty: {denormalize_for_display(path)}")


def copy_dump_to_repo(repo_root: Path, dump_path: Path, dry_run: bool) -> Path:
    destination = repo_root / "in" / dump_path.name
    print(f"copy {denormalize_for_display(dump_path)} -> {denormalize_for_display(destination)}")
    if not dry_run:
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(dump_path, destination)
    return destination


def main(argv: Optional[Sequence[str]] = None) -> int:
    args = parse_args(argv)
    repo_root = resolve_repo_root()
    script_dir = Path(__file__).resolve().parent
    idaw_wrapper = resolve_idaw_wrapper(repo_root)
    idb_dir = normalize_path(args.idb_dir, repo_root)
    script_path = normalize_path(args.script, script_dir)
    log_dir = normalize_path(args.log_dir, repo_root)
    games = args.game if args.game else list(DEFAULT_GAMES)

    verify_file(script_path, "IDC script")
    jobs = build_jobs(games, idb_dir)

    print(f"IDAW wrapper: {args.python} -B {denormalize_for_display(idaw_wrapper)}")
    print(f"IDC script:   {denormalize_for_display(script_path)}")

    for job in jobs:
        verify_file(job.idb_path, f"{job.game} IDB")
        dump_path = expected_dump_for_idb(job.idb_path)
        ida_log = None if args.no_ida_log else log_dir / f"{job.game}.ida.log"
        print(f"{job.game} IDB:    {denormalize_for_display(job.idb_path)}")
        print(f"  expects: {denormalize_for_display(dump_path)}")
        command = build_ida_command(args.python, idaw_wrapper, script_path, job.idb_path, ida_log)
        result = run_command(command, repo_root, args.dry_run, args.timeout_seconds)
        if result != 0:
            raise RuntimeError(f"IDA function data export failed for {job.game} with exit code {result}.")
        if not args.skip_verify and not args.dry_run:
            verify_file(dump_path, f"{job.game} function dump")
        if args.copy:
            if not args.skip_verify and not args.dry_run:
                verify_file(dump_path, f"{job.game} function dump")
            copy_dump_to_repo(repo_root, dump_path, args.dry_run)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
