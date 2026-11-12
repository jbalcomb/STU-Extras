#!/usr/bin/env python3
"""Restore packed IDA databases from timestamped backups made by idaw.py.

Usage:
    python -B restore_ida_backup.py --idb-dir DIR [--backup-dir DIR] [--idb NAME ...] [--apply]

--idb-dir      folder holding <NAME>.idb (default: %IDA55_IDB_DIR%)
--backup-dir   folder holding <NAME>-YYYYMMDDHHMM.idb backups (default: %IDA55_BACKUP_DIR%, else --idb-dir)
--idb NAME     IDB to restore, by stem or file name; repeatable (default: every <NAME>.idb in --idb-dir)
Without --apply it only prints the plan.
"""

from __future__ import annotations

import argparse
import hashlib
import os
import re
import shutil
from datetime import datetime
from pathlib import Path
from typing import Optional, Sequence


IDB_DIR_ENV = "IDA55_IDB_DIR"
BACKUP_DIR_ENV = "IDA55_BACKUP_DIR"
BACKUP_RE_TEMPLATE = r"^{game}-(\d{{12}})\.idb$"
DERIVED_IDB_RE = re.compile(r"-(\d{12}|damaged-\d{12})\.idb$", re.IGNORECASE)


class RestoreBackupError(RuntimeError):
    pass


def normalize_path(path_value: str | Path) -> Path:
    path_text = str(path_value)
    if os.name != "nt":
        drive_match = re.match(r"^([A-Za-z]):[\\/](.*)$", path_text)
        if drive_match:
            drive = drive_match.group(1).lower()
            rest = drive_match.group(2).replace("\\", "/")
            return Path("/mnt") / drive / rest
    return Path(path_text)


def denormalize_path(path: Path) -> str:
    text = str(path)
    if os.name != "nt":
        match = re.match(r"^/mnt/([A-Za-z])/(.*)$", text)
        if match:
            return f"{match.group(1).upper()}:/" + match.group(2)
    return text


def timestamp(now: Optional[datetime] = None) -> str:
    return (now or datetime.now().astimezone()).strftime("%Y%m%d%H%M")


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for block in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def idb_names(idb_dir: Path, requested: Optional[Sequence[str]]) -> list[str]:
    """Return the IDB stems to restore: the requested ones, else every non-backup .idb in idb_dir. Powered by Claude."""
    if requested:
        return [Path(name).stem if name.lower().endswith(".idb") else name for name in requested]
    names = sorted(
        path.stem
        for path in idb_dir.iterdir()
        if path.is_file() and path.suffix.lower() == ".idb" and not DERIVED_IDB_RE.search(path.name)
    )
    if not names:
        raise RestoreBackupError(f"No .idb files found in {denormalize_path(idb_dir)}")
    return names


def idb_path_for_game(idb_dir: Path, game: str) -> Path:
    return idb_dir / f"{game}.idb"


def backup_candidates(idb_dir: Path, game: str) -> list[Path]:
    pattern = re.compile(BACKUP_RE_TEMPLATE.format(game=re.escape(game)))
    candidates: list[tuple[str, Path]] = []
    for path in idb_dir.iterdir():
        match = pattern.match(path.name)
        if match is None:
            continue
        if not path.is_file() or path.stat().st_size == 0:
            continue
        candidates.append((match.group(1), path))
    return [path for _stamp, path in sorted(candidates, reverse=True)]


def latest_backup(idb_dir: Path, game: str) -> Path:
    candidates = backup_candidates(idb_dir, game)
    if not candidates:
        raise RestoreBackupError(f"No nonzero timestamped backups found for {game} in {denormalize_path(idb_dir)}")
    return candidates[0]


def damaged_copy_path(idb_path: Path, stamp: str) -> Path:
    return idb_path.with_name(f"{idb_path.stem}-damaged-{stamp}{idb_path.suffix}")


def ensure_restore_allowed(idb_path: Path, allow_overwrite_nonzero: bool) -> None:
    if not idb_path.exists():
        return
    if idb_path.stat().st_size == 0:
        return
    if allow_overwrite_nonzero:
        return
    raise RestoreBackupError(
        "Refusing to overwrite nonzero current IDB without --allow-overwrite-nonzero: "
        f"{denormalize_path(idb_path)}"
    )


def restore_game(
    idb_dir: Path,
    game: str,
    *,
    apply: bool,
    allow_overwrite_nonzero: bool,
    stamp: str,
    backup_dir: Optional[Path] = None,
) -> list[str]:
    """Restore one IDB from its newest nonzero backup (backups looked up in backup_dir, else idb_dir). Powered by Claude."""
    backup_path = latest_backup(backup_dir or idb_dir, game)
    idb_path = idb_path_for_game(idb_dir, game)
    damaged_path = damaged_copy_path(idb_path, stamp)
    lines = [
        f"game={game}",
        f"current={denormalize_path(idb_path)}",
        f"backup={denormalize_path(backup_path)}",
        f"damaged_copy={denormalize_path(damaged_path) if idb_path.exists() else ''}",
        f"backup_size={backup_path.stat().st_size}",
        f"apply={1 if apply else 0}",
    ]

    ensure_restore_allowed(idb_path, allow_overwrite_nonzero)

    if not apply:
        lines.append("status=dry-run")
        return lines

    if backup_path.stat().st_size == 0:
        raise RestoreBackupError(f"Refusing to restore from 0-byte backup: {denormalize_path(backup_path)}")
    if damaged_path.exists():
        raise RestoreBackupError(f"Damaged-copy path already exists: {denormalize_path(damaged_path)}")
    if idb_path.exists():
        shutil.copy2(idb_path, damaged_path)
        lines.append(f"damaged_copy_size={damaged_path.stat().st_size}")

    shutil.copy2(backup_path, idb_path)
    if idb_path.stat().st_size != backup_path.stat().st_size:
        raise RestoreBackupError("Restored IDB size does not match selected backup.")
    if sha256_file(idb_path) != sha256_file(backup_path):
        raise RestoreBackupError("Restored IDB hash does not match selected backup.")

    lines.append(f"restored_size={idb_path.stat().st_size}")
    lines.append("status=restored")
    return lines


def parse_args(argv: Optional[Sequence[str]] = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Restore IDA .idb files from timestamped idaw.py backups.")
    parser.add_argument(
        "--idb",
        "--game",
        dest="names",
        action="append",
        help="IDB to restore, by stem (MAGIC) or file name (MAGIC.idb). Repeatable. Default: every .idb in --idb-dir.",
    )
    parser.add_argument("--idb-dir", default=os.environ.get(IDB_DIR_ENV), help=f"Folder holding the IDBs. Default: %%{IDB_DIR_ENV}%%")
    parser.add_argument(
        "--backup-dir",
        default=os.environ.get(BACKUP_DIR_ENV),
        help=f"Folder holding the timestamped backups. Default: %%{BACKUP_DIR_ENV}%%, else --idb-dir.",
    )
    parser.add_argument("--apply", action="store_true", help="Actually restore. Without this, only prints the plan.")
    parser.add_argument(
        "--allow-overwrite-nonzero",
        action="store_true",
        help="Allow restoring over a nonzero current IDB after first making a damaged-current copy.",
    )
    return parser.parse_args(argv)


def main(argv: Optional[Sequence[str]] = None) -> int:
    args = parse_args(argv)
    if not args.idb_dir:
        raise SystemExit(f"error: --idb-dir is required (or set {IDB_DIR_ENV})")
    idb_dir = normalize_path(args.idb_dir)
    backup_dir = normalize_path(args.backup_dir) if args.backup_dir else idb_dir
    stamp = timestamp()
    for name in idb_names(idb_dir, args.names):
        for line in restore_game(
            idb_dir,
            name,
            apply=args.apply,
            allow_overwrite_nonzero=args.allow_overwrite_nonzero,
            stamp=stamp,
            backup_dir=backup_dir,
        ):
            print(line)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
