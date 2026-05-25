#!/usr/bin/env python3
"""Restore packed IDA databases from timestamped backups."""

from __future__ import annotations

import argparse
import hashlib
import os
import re
import shutil
from datetime import datetime
from pathlib import Path
from typing import Optional, Sequence


DEFAULT_IDB_DIR = r"C:\STU_DASM\IDA55_MOM131"
GAME_NAMES = ("MAGIC", "WIZARDS")
BACKUP_RE_TEMPLATE = r"^{game}-(\d{{12}})\.idb$"


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


def game_list(game: str) -> list[str]:
    if game == "BOTH":
        return list(GAME_NAMES)
    return [game]


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
) -> list[str]:
    backup_path = latest_backup(idb_dir, game)
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
    parser.add_argument("--game", choices=("MAGIC", "WIZARDS", "BOTH"), default="BOTH")
    parser.add_argument("--idb-dir", default=DEFAULT_IDB_DIR)
    parser.add_argument("--apply", action="store_true", help="Actually restore. Without this, only prints the plan.")
    parser.add_argument(
        "--allow-overwrite-nonzero",
        action="store_true",
        help="Allow restoring over a nonzero current IDB after first making a damaged-current copy.",
    )
    return parser.parse_args(argv)


def main(argv: Optional[Sequence[str]] = None) -> int:
    args = parse_args(argv)
    idb_dir = normalize_path(args.idb_dir)
    stamp = timestamp()
    for game in game_list(args.game):
        for line in restore_game(
            idb_dir,
            game,
            apply=args.apply,
            allow_overwrite_nonzero=args.allow_overwrite_nonzero,
            stamp=stamp,
        ):
            print(line)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
