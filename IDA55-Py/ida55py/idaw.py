#!/usr/bin/env python3
"""Launch IDA Pro 5.5 safely: on a work copy of the IDB, or after backing up the packed IDB.

Usage:
    python -B idaw.py [--ida-exe EXE] [--work-dir DIR | --backup-dir DIR] [--dry-run] -- <IDA arguments, exactly one .idb>

--work-dir DIR    (recommended) Copy the .idb into DIR (new or empty) and run IDA on the copy. The real IDB is
                  never opened, so a hang, kill or crash can only damage the copy; delete DIR to clean up.
                  Scripts that write next to GetIdbPath() write into DIR. No backup is made.
--write-back      With --work-dir, for scripts that change the database: when IDA exits with code 0, the copy is
                  healthy, and the real IDB has not changed since it was copied, back up the real IDB (to
                  --backup-dir) and replace it with the copy. Otherwise the real IDB is left alone and the work
                  directory is kept for inspection.
--backup-dir DIR  Backup folder: for --write-back, or, without --work-dir, the pre-run backup before IDA runs on the
                  real IDB (default: %IDA55_BACKUP_DIR%, else the IDB's own folder).
IDA executable:   --ida-exe, else %IDA55_EXE%, else C:\\STU_DASM\\IDA55\\idaw.exe
Backups are named <stem>-YYYYMMDDHHMM.idb; restore_ida_backup.py reads the same names.
"""

from __future__ import annotations

import argparse
import hashlib
import os
import re
import shutil
import subprocess
import sys
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Optional, Sequence

UNPACKED_DATABASE_TEXT = "IDA has found unpacked version of database"
DEFAULT_IDA_EXE = r"C:\STU_DASM\IDA55\idaw.exe"
IDA_EXE_ENV = "IDA55_EXE"
BACKUP_DIR_ENV = "IDA55_BACKUP_DIR"
RESTORE_SCRIPT = Path(__file__).resolve().parent / "restore_ida_backup.py"
IDA_PROCESS_NAMES = ("idaw.exe", "idag.exe", "idal.exe", "idaq.exe", "ida64.exe", "ida.exe")
UNPACKED_PART_SUFFIXES = (".id0", ".id1", ".nam", ".til")


class IdawError(RuntimeError):
    pass


def parse_args(argv: Optional[Sequence[str]] = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Backup the packed .idb, then launch IDA with the original arguments."
    )
    parser.add_argument("--dry-run", action="store_true", help="Print the backup and IDA command without running IDA.")
    parser.add_argument(
        "--ida-exe",
        help=f"IDA executable to run. Default: %%{IDA_EXE_ENV}%%, else {DEFAULT_IDA_EXE}",
    )
    parser.add_argument(
        "--backup-dir",
        help=f"Directory for the pre-run .idb backup. Default: %%{BACKUP_DIR_ENV}%%, else the IDB's own folder.",
    )
    parser.add_argument(
        "--work-dir",
        help="Run IDA on a copy of the .idb placed in this new or empty directory; the real IDB is never opened.",
    )
    parser.add_argument(
        "--write-back",
        action="store_true",
        help="With --work-dir: after a clean exit and a healthy copy, back up the real IDB and replace it with the copy.",
    )
    parser.add_argument(
        "--allow-running-ida",
        action="store_true",
        help="Launch even if another IDA process is already running. Unsafe; can produce a 0-byte IDB.",
    )
    parser.add_argument(
        "--allow-unpacked-parts",
        action="store_true",
        help="Launch even if .id0/.id1/.nam/.til already sit beside the IDB. Unsafe; the IDB may be mid-recovery.",
    )
    parser.add_argument(
        "ida_args",
        nargs=argparse.REMAINDER,
        help="Arguments to pass through to IDA. Put -- before the IDA arguments if needed.",
    )
    return parser.parse_args(argv)


def normalize_path(path_value: str | Path) -> Path:
    path_text = str(path_value)
    if os.name != "nt":
        drive_match = re.match(r"^([A-Za-z]):[\\/](.*)$", path_text)
        if drive_match:
            drive = drive_match.group(1).lower()
            rest = drive_match.group(2).replace("\\", "/")
            return Path("/mnt") / drive / rest
    return Path(path_text)


def denormalize_for_display(path: Path) -> str:
    text = str(path)
    if os.name != "nt":
        match = re.match(r"^/mnt/([A-Za-z])/(.*)$", text)
        if match:
            return f"{match.group(1).upper()}:\\" + match.group(2).replace("/", "\\")
    return text


def timestamp(now: Optional[datetime] = None) -> str:
    value = now or datetime.now().astimezone()
    return value.strftime("%Y%m%d%H%M")


def strip_passthrough_separator(args: Sequence[str]) -> List[str]:
    if args and args[0] == "--":
        return list(args[1:])
    return list(args)


def find_idb_argument(ida_args: Sequence[str]) -> Path:
    candidates = [arg for arg in ida_args if arg.lower().endswith(".idb")]
    if len(candidates) != 1:
        raise IdawError(f"Expected exactly one .idb argument, found {len(candidates)}.")
    return normalize_path(candidates[0])


def resolve_ida_exe(cli_value: Optional[str]) -> Path:
    """Pick the IDA executable: --ida-exe, then the IDA55_EXE variable, then the built-in path. Powered by Claude."""
    return normalize_path(cli_value or os.environ.get(IDA_EXE_ENV) or DEFAULT_IDA_EXE)


def resolve_backup_dir(cli_value: Optional[str], idb_path: Path) -> Path:
    """Pick the backup folder: --backup-dir, then the IDA55_BACKUP_DIR variable, then the IDB's folder. Powered by Claude."""
    value = cli_value or os.environ.get(BACKUP_DIR_ENV)
    return normalize_path(value) if value else idb_path.parent


def backup_path_for_idb(idb_path: Path, timestamp: str, backup_dir: Path) -> Path:
    """Return <backup_dir>/<stem>-<timestamp>.idb. Powered by Claude."""
    return backup_dir / f"{idb_path.stem}-{timestamp}{idb_path.suffix}"


def backup_idb(idb_path: Path, backup_path: Path, dry_run: bool = False) -> None:
    if not idb_path.exists():
        raise FileNotFoundError(f"IDB not found: {denormalize_for_display(idb_path)}")
    if not idb_path.is_file():
        raise IdawError(f"IDB is not a file: {denormalize_for_display(idb_path)}")
    if idb_path.stat().st_size == 0:
        raise IdawError(f"Refusing to launch IDA with a 0-byte IDB: {denormalize_for_display(idb_path)}")
    if backup_path.exists():
        raise FileExistsError(f"Backup already exists: {denormalize_for_display(backup_path)}")
    if dry_run:
        return
    backup_path.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(idb_path, backup_path)
    if backup_path.stat().st_size != idb_path.stat().st_size:
        raise IdawError(
            "IDB backup size mismatch: "
            f"{denormalize_for_display(idb_path)} -> {denormalize_for_display(backup_path)}"
        )


def extract_log_paths(ida_args: Sequence[str]) -> List[Path]:
    paths: List[Path] = []
    for index, arg in enumerate(ida_args):
        if arg == "-L" and index + 1 < len(ida_args):
            paths.append(normalize_path(ida_args[index + 1]))
        elif arg.startswith("-L") and len(arg) > 2:
            paths.append(normalize_path(arg[2:]))
    return paths


def log_offsets(log_paths: Sequence[Path]) -> Dict[Path, int]:
    offsets: Dict[Path, int] = {}
    for path in log_paths:
        offsets[path] = path.stat().st_size if path.exists() else 0
    return offsets


def read_log_since(path: Path, offset: int) -> str:
    if not path.exists():
        return ""
    with path.open("rb") as handle:
        handle.seek(min(offset, path.stat().st_size))
        return handle.read().decode("utf-8", errors="replace")


def log_contains_unpacked_database_prompt(log_paths: Sequence[Path], offsets: Optional[Dict[Path, int]] = None) -> bool:
    offsets = offsets or {}
    for path in log_paths:
        text = read_log_since(path, offsets.get(path, 0))
        if UNPACKED_DATABASE_TEXT in text:
            return True
    return False


def probe_process_list() -> str:
    """Return the raw OS process listing, or fail closed if it cannot be read. Powered by Claude."""
    command = ["tasklist", "/FO", "CSV", "/NH"] if os.name == "nt" else ["ps", "-eo", "comm="]
    try:
        completed = subprocess.run(command, capture_output=True, text=True, timeout=60)
    except (OSError, subprocess.SubprocessError) as error:
        raise IdawError(
            f"Could not list running processes to check for a live IDA session: {error}. "
            "Close all IDA windows and retry, or pass --allow-running-ida if you are certain none are open."
        ) from error
    if completed.returncode != 0:
        raise IdawError(
            f"Process listing command failed with exit code {completed.returncode}. "
            "Close all IDA windows and retry, or pass --allow-running-ida if you are certain none are open."
        )
    return completed.stdout


def find_running_ida_processes() -> List[str]:
    """Return process-listing rows that look like a running IDA instance. Powered by Claude."""
    matches: List[str] = []
    for line in probe_process_list().splitlines():
        lowered = line.lower()
        if any(name in lowered for name in IDA_PROCESS_NAMES):
            matches.append(line.strip())
    return matches


def find_unpacked_parts(idb_path: Path) -> List[Path]:
    """Return the .id0/.id1/.nam/.til component files sitting beside an IDB. Powered by Claude."""
    base = idb_path.with_suffix("")
    return [candidate for candidate in (base.with_suffix(suffix) for suffix in UNPACKED_PART_SUFFIXES) if candidate.exists()]


def preflight_checks(idb_path: Path, allow_running_ida: bool, allow_unpacked_parts: bool) -> None:
    """Refuse to launch IDA when a concurrent session or a half-unpacked database is detected. Powered by Claude."""
    if allow_running_ida:
        print("PREFLIGHT: running-IDA check SKIPPED (--allow-running-ida).")
    else:
        running = find_running_ida_processes()
        if running:
            listing = "\n".join(f"    {row}" for row in running)
            raise IdawError(
                "Refusing to launch: another IDA process is already running.\n"
                f"{listing}\n"
                "  Two IDA sessions against the same database directory can leave a 0-byte .idb.\n"
                "  Close every IDA window, confirm with: tasklist | findstr /I ida\n"
                "  Then re-run. Use --allow-running-ida only if you know what you are doing."
            )
        print("PREFLIGHT: no other IDA process is running.")

    if allow_unpacked_parts:
        print("PREFLIGHT: unpacked-parts check SKIPPED (--allow-unpacked-parts).")
    else:
        parts = find_unpacked_parts(idb_path)
        if parts:
            listing = "\n".join(f"    {denormalize_for_display(part)}" for part in parts)
            raise IdawError(
                "Refusing to launch: unpacked database parts already sit beside the IDB.\n"
                f"{listing}\n"
                "  This means the database is open elsewhere, or a previous session died without re-packing.\n"
                "  Resolve that first; launching now risks losing whichever copy is newer.\n"
                "  Use --allow-unpacked-parts only if you know what you are doing."
            )
        print("PREFLIGHT: no unpacked database parts beside the IDB.")


def print_recovery_guidance(idb_path: Path, backup_path: Path, reason: str) -> None:
    """Explain, without changing anything, how to manually repair a damaged IDB. Powered by Claude."""
    idb_display = denormalize_for_display(idb_path)
    backup_display = denormalize_for_display(backup_path)
    parts = find_unpacked_parts(idb_path)
    restore_command = (
        f'python -B "{denormalize_for_display(RESTORE_SCRIPT)}"'
        f' --idb-dir "{denormalize_for_display(idb_path.parent)}"'
        f' --backup-dir "{denormalize_for_display(backup_path.parent)}"'
        f" --idb {idb_path.stem}"
    )

    lines = [
        "",
        "=" * 78,
        f"DATABASE PROBLEM: {reason}",
        "=" * 78,
        f"  IDB:              {idb_display}",
        f"  Pre-run backup:   {backup_display}",
    ]
    if parts:
        lines.append("  Unpacked parts still on disk (these ARE the database, in expanded form):")
        lines.extend(f"    {denormalize_for_display(part)}" for part in parts)
    else:
        lines.append("  Unpacked parts: none found.")

    lines += [
        "",
        "  NOTHING HAS BEEN RESTORED OR DELETED. Recovery is left to you on purpose.",
        "",
        "  DO THIS FIRST -- stop running pipeline commands, and preserve the evidence:",
        "    1. Close every IDA window.",
        "    2. Copy the IDB, the pre-run backup, and any .id0/.id1/.nam/.til listed above",
        "       into a new, empty directory before touching them.",
        "",
        "  THEN PICK ONE RECOVERY PATH:",
        "",
        "  A. Rebuild from the unpacked parts (keeps the most recent state):",
        f"       Open {idb_display} in the IDA GUI.",
        "       Answer YES to 'IDA has found unpacked version of database'.",
        "       Then File > Save to re-pack the .idb.",
        "",
        "  B. Restore the packed pre-run backup (discards anything done in the failed session):",
        "       Move the .id0/.id1/.nam/.til aside first, or IDA will offer the unpacked copy.",
        f"       Preview:  {restore_command}",
        f"       Apply:    {restore_command} --apply",
        "       It keeps a -damaged- copy of the current file and verifies the restore by sha256.",
        "",
        "  Verify when done: the .idb is non-zero and no .id0/.id1 remain beside it.",
        "=" * 78,
        "",
    ]
    for line in lines:
        print(line, file=sys.stderr)


def prepare_work_copy(idb_path: Path, work_dir: Path, dry_run: bool = False) -> Path:
    """Copy the packed IDB into a new or empty work directory and return the copy's path. Powered by Claude."""
    if not idb_path.is_file():
        raise FileNotFoundError(f"IDB not found: {denormalize_for_display(idb_path)}")
    if idb_path.stat().st_size == 0:
        raise IdawError(f"Refusing to copy a 0-byte IDB: {denormalize_for_display(idb_path)}")
    if work_dir.exists() and any(work_dir.iterdir()):
        raise IdawError(f"Work directory is not empty: {denormalize_for_display(work_dir)}")
    work_idb = work_dir / idb_path.name
    if dry_run:
        return work_idb
    work_dir.mkdir(parents=True, exist_ok=True)
    shutil.copy2(idb_path, work_idb)
    if work_idb.stat().st_size != idb_path.stat().st_size:
        raise IdawError(
            f"Work copy size mismatch: {denormalize_for_display(idb_path)} -> {denormalize_for_display(work_idb)}"
        )
    return work_idb


def replace_idb_argument(ida_args: Sequence[str], new_idb: Path) -> List[str]:
    """Return the IDA arguments with the single .idb argument replaced by new_idb. Powered by Claude."""
    return [str(new_idb) if arg.lower().endswith(".idb") else arg for arg in ida_args]


def work_copy_problem(work_idb: Path, log_paths: Sequence[Path], offsets: Dict[Path, int]) -> Optional[str]:
    """Describe what is wrong with the work copy after IDA exits, or None if it looks healthy. Powered by Claude."""
    if not work_idb.exists() or work_idb.stat().st_size == 0:
        return "IDA left a missing or 0-byte work copy"
    if log_contains_unpacked_database_prompt(log_paths, offsets):
        return "IDA reported unpacked database state for the work copy"
    if find_unpacked_parts(work_idb):
        return "IDA exited without re-packing the work copy; unpacked parts remain"
    return None


def file_fingerprint(path: Path) -> tuple:
    """Size and modification time, used to detect that a file changed. Powered by Claude."""
    stat = path.stat()
    return (stat.st_size, stat.st_mtime_ns)


def sha256_of(path: Path) -> str:
    """sha256 of a file, read in 1 MiB blocks. Powered by Claude."""
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for block in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def write_back(idb_path: Path, work_idb: Path, original_fingerprint: tuple, backup_path: Path, allow_running_ida: bool) -> None:
    """Replace the real IDB with the healthy work copy: refuse if the real IDB changed since it was copied, if IDA is
    running, or if unpacked parts appeared beside it; back the real IDB up first; replace through a temporary file
    in the same folder; verify by sha256. Powered by Claude."""
    if file_fingerprint(idb_path) != original_fingerprint:
        raise IdawError(
            f"Not writing back: the real IDB changed while IDA worked on the copy: {denormalize_for_display(idb_path)}"
        )
    if not allow_running_ida and find_running_ida_processes():
        raise IdawError("Not writing back: an IDA process is running; it may have the real IDB open.")
    if find_unpacked_parts(idb_path):
        raise IdawError(
            f"Not writing back: unpacked parts appeared beside the real IDB (open elsewhere?): {denormalize_for_display(idb_path)}"
        )
    backup_idb(idb_path, backup_path)
    print(f"Backup: {denormalize_for_display(backup_path)}")
    temp_path = idb_path.with_name(idb_path.name + ".write-back.tmp")
    shutil.copy2(work_idb, temp_path)
    os.replace(temp_path, idb_path)
    if sha256_of(idb_path) != sha256_of(work_idb):
        raise IdawError(f"Write-back verification failed (sha256 differs): {denormalize_for_display(idb_path)}")
    print(f"Written back: {denormalize_for_display(work_idb)} -> {denormalize_for_display(idb_path)}")


def run_on_work_copy(args: argparse.Namespace, ida_exe: Path, idb_path: Path, ida_args: Sequence[str]) -> int:
    """Run IDA on a copy of the IDB in --work-dir; the real IDB is only read, never opened by IDA. With --write-back,
    a healthy copy replaces the real IDB afterwards. Powered by Claude."""
    work_dir = normalize_path(args.work_dir)
    print(f"IDB:    {denormalize_for_display(idb_path)}  ({'replaced only by --write-back' if args.write_back else 'read only'}; IDA runs on the work copy)")
    preflight_checks(idb_path, args.allow_running_ida, args.allow_unpacked_parts)
    original_fingerprint = file_fingerprint(idb_path) if idb_path.exists() else None
    work_idb = prepare_work_copy(idb_path, work_dir, args.dry_run)
    print(f"Work:   {denormalize_for_display(work_idb)}")
    backup_path = backup_path_for_idb(idb_path, timestamp(), resolve_backup_dir(args.backup_dir, idb_path))
    if args.write_back:
        print(f"Write-back backup: {denormalize_for_display(backup_path)}")

    work_args = replace_idb_argument(ida_args, work_idb)
    log_paths = extract_log_paths(work_args)
    offsets = log_offsets(log_paths)
    command = [str(ida_exe)] + work_args
    print("+", " ".join(command))
    if args.dry_run:
        return 0

    result = subprocess.run(command)

    problem = work_copy_problem(work_idb, log_paths, offsets)
    if problem:
        print(
            f"\nWORK COPY DAMAGED: {problem}.\n"
            f"  The real IDB was not opened and is unchanged: {denormalize_for_display(idb_path)}\n"
            f"  Outputs in the work directory may be incomplete. Delete it: {denormalize_for_display(work_dir)}\n",
            file=sys.stderr,
        )
        raise IdawError(f"{problem}: {denormalize_for_display(work_idb)}")
    if args.write_back:
        if result.returncode != 0:
            print(
                f"\nNOT WRITTEN BACK: IDA exited with code {result.returncode}.\n"
                f"  The real IDB is unchanged: {denormalize_for_display(idb_path)}\n"
                f"  The work copy is kept for inspection: {denormalize_for_display(work_idb)}\n",
                file=sys.stderr,
            )
            return result.returncode
        write_back(idb_path, work_idb, original_fingerprint, backup_path, args.allow_running_ida)
    return result.returncode


def main(argv: Optional[Sequence[str]] = None) -> int:
    args = parse_args(argv)
    ida_args = strip_passthrough_separator(args.ida_args)
    if not ida_args:
        raise IdawError("No IDA arguments were provided.")

    ida_exe = resolve_ida_exe(args.ida_exe)
    idb_path = find_idb_argument(ida_args)
    if args.write_back and not args.work_dir:
        raise IdawError("--write-back needs --work-dir.")
    if args.work_dir:
        if args.backup_dir and not args.write_back:
            raise IdawError("--work-dir and --backup-dir cannot be used together without --write-back; a read-only work copy needs no backup.")
        return run_on_work_copy(args, ida_exe, idb_path, ida_args)

    stamp = timestamp()
    backup_path = backup_path_for_idb(idb_path, stamp, resolve_backup_dir(args.backup_dir, idb_path))

    log_paths = extract_log_paths(ida_args)
    offsets = log_offsets(log_paths)

    print(f"IDB:    {denormalize_for_display(idb_path)}")
    print(f"Backup: {denormalize_for_display(backup_path)}")

    preflight_checks(idb_path, args.allow_running_ida, args.allow_unpacked_parts)

    backup_idb(idb_path, backup_path, args.dry_run)

    command = [str(ida_exe)] + ida_args
    print("+", " ".join(command))
    if args.dry_run:
        return 0

    result = subprocess.run(command)

    if idb_path.stat().st_size == 0:
        print_recovery_guidance(idb_path, backup_path, "IDA left a 0-byte IDB.")
        raise IdawError(f"IDA left a 0-byte IDB: {denormalize_for_display(idb_path)}")
    if log_contains_unpacked_database_prompt(log_paths, offsets):
        print_recovery_guidance(idb_path, backup_path, "IDA reported unpacked database state.")
        raise IdawError(f"IDA reported unpacked database state for: {denormalize_for_display(idb_path)}")

    leftover_parts = find_unpacked_parts(idb_path)
    if leftover_parts:
        print_recovery_guidance(
            idb_path,
            backup_path,
            "IDA exited without re-packing the database; unpacked parts remain.",
        )
        raise IdawError(f"IDA left unpacked database parts beside: {denormalize_for_display(idb_path)}")

    return result.returncode


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except IdawError as error:
        print(f"ERROR: {error}", file=sys.stderr)
        raise SystemExit(2)
