#!/usr/bin/env python3
"""Launch IDA Pro 5.5 after backing up the packed IDB file."""

from __future__ import annotations

import argparse
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
DEFAULT_BACKUP_DIR = "C:\\STU_DASM\\IDA55_MOM131\\"


class IdawError(RuntimeError):
    pass


def parse_args(argv: Optional[Sequence[str]] = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Backup the packed .idb, then launch IDA with the original arguments."
    )
    parser.add_argument("--dry-run", action="store_true", help="Print the backup and IDA command without running IDA.")
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


def backup_path_for_idb(idb_path: Path, timestamp: str) -> Path:
    destination_dir = normalize_path(DEFAULT_BACKUP_DIR)
    return destination_dir / f"{idb_path.stem}-{timestamp}{idb_path.suffix}"


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


def main(argv: Optional[Sequence[str]] = None) -> int:
    args = parse_args(argv)
    ida_args = strip_passthrough_separator(args.ida_args)
    if not ida_args:
        raise IdawError("No IDA arguments were provided.")

    ida_exe = normalize_path(DEFAULT_IDA_EXE)
    idb_path = find_idb_argument(ida_args)
    stamp = timestamp()
    backup_path = backup_path_for_idb(idb_path, stamp)

    log_paths = extract_log_paths(ida_args)
    offsets = log_offsets(log_paths)

    print(f"IDB:    {denormalize_for_display(idb_path)}")
    print(f"Backup: {denormalize_for_display(backup_path)}")
    backup_idb(idb_path, backup_path, args.dry_run)

    command = [str(ida_exe)] + ida_args
    print("+", " ".join(command))
    if args.dry_run:
        return 0

    result = subprocess.run(command)
    if idb_path.stat().st_size == 0:
        raise IdawError(f"IDA left a 0-byte IDB: {denormalize_for_display(idb_path)}")
    if log_contains_unpacked_database_prompt(log_paths, offsets):
        raise IdawError(f"IDA reported unpacked database state for: {denormalize_for_display(idb_path)}")
    return result.returncode


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except IdawError as error:
        print(f"ERROR: {error}", file=sys.stderr)
        raise SystemExit(2)
