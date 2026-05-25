#!/usr/bin/env python3
"""Copy the standard IDA 5.5 MoM output files into the local input directory."""

from __future__ import annotations

import argparse
import os
import re
import shutil
import sys
from pathlib import Path
from typing import Iterable


DEFAULT_SOURCE_DIR = r"C:\STU_DASM\IDA55_MOM131"
DEFAULT_DEST_DIR = Path("in")
IDA_OUTPUT_FILES = (
    "MAGIC.MAP",
    "MAGIC.ASM",
    "MAGIC.INC",
    "MAGIC.LST",
    "WIZARDS.MAP",
    "WIZARDS.ASM",
    "WIZARDS.INC",
    "WIZARDS.LST",
)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Copy standard MAGIC/WIZARDS IDA 5.5 output files into Piethawn's in/ directory.",
    )
    parser.add_argument("--source", default=DEFAULT_SOURCE_DIR, help=f"Source directory. Default: {DEFAULT_SOURCE_DIR}")
    parser.add_argument("--dest", type=Path, default=DEFAULT_DEST_DIR, help="Destination directory. Default: in")
    parser.add_argument("--dry-run", action="store_true", help="Print planned copies without writing files.")
    parser.add_argument("--no-overwrite", action="store_true", help="Fail if a destination file already exists.")
    parser.add_argument(
        "--ignore-missing",
        action="store_true",
        help="Skip missing source files instead of failing after reporting them.",
    )
    return parser.parse_args()


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


def find_case_insensitive(parent: Path, filename: str) -> Path:
    target_name = filename.casefold()
    if parent.exists():
        for child in parent.iterdir():
            if child.name.casefold() == target_name:
                return child
    return parent / filename


def copy_outputs(
    source_dir: Path,
    dest_dir: Path,
    filenames: Iterable[str],
    dry_run: bool,
    no_overwrite: bool,
    ignore_missing: bool,
) -> int:
    missing = 0
    copied = 0

    if not source_dir.is_dir():
        print(f"Source directory not found: {source_dir}", file=sys.stderr)
        return 1

    if not dry_run:
        dest_dir.mkdir(parents=True, exist_ok=True)

    for filename in filenames:
        source_path = find_case_insensitive(source_dir, filename)
        dest_path = find_case_insensitive(dest_dir, filename)

        if not source_path.exists():
            print(f"missing: {source_path}", file=sys.stderr)
            missing += 1
            continue

        if dest_path.exists() and no_overwrite:
            print(f"exists:  {dest_path}", file=sys.stderr)
            return 1

        print(f"{'would copy' if dry_run else 'copy'}: {source_path} -> {dest_path}")
        if not dry_run:
            shutil.copy2(source_path, dest_path)
            copied += 1

    if missing and not ignore_missing:
        print(f"Failed: {missing} source file(s) missing.", file=sys.stderr)
        return 1

    if dry_run:
        print("Dry run complete.")
    else:
        print(f"Copied {copied} file(s).")

    return 0


def main() -> int:
    args = parse_args()
    repo_root = Path(__file__).resolve().parent.parent
    source_dir = normalize_path(args.source, repo_root)
    dest_dir = normalize_path(args.dest, repo_root)

    return copy_outputs(
        source_dir=source_dir,
        dest_dir=dest_dir,
        filenames=IDA_OUTPUT_FILES,
        dry_run=args.dry_run,
        no_overwrite=args.no_overwrite,
        ignore_missing=args.ignore_missing,
    )


if __name__ == "__main__":
    raise SystemExit(main())
