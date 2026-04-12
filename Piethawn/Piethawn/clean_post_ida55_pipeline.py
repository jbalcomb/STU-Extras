#!/usr/bin/env python3
"""
Remove generated artifacts from the post-IDA55 pipeline while preserving the
manual IDA dump inputs.
"""

from __future__ import annotations

import argparse
import shutil
from pathlib import Path
from typing import Iterable, List


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Clean generated artifacts from the post-IDA55 pipeline.")
    parser.add_argument(
        "--stage",
        choices=("pre-idc", "post-dump"),
        default="pre-idc",
        help=(
            "Cleanup target. pre-idc preserves the final rename IDC files; "
            "post-dump removes all generated outputs after the manual dumps."
        ),
    )
    parser.add_argument(
        "--apply",
        action="store_true",
        help="Actually delete files. Without this flag the script only prints what it would remove.",
    )
    parser.add_argument("--output-root", default="out", help="Generated output root. Default: out")
    return parser.parse_args()


def generated_paths(output_root: Path, stage: str) -> List[Path]:
    paths = [
        Path("MAGIC.exe-header.json"),
        Path("WIZARDS.exe-header.json"),
        Path("MAGIC.borland-ovrinfo.json"),
        Path("WIZARDS.borland-ovrinfo.json"),
        Path("MAGIC.idb.ida55-functions.json"),
        Path("WIZARDS.idb.ida55-functions.json"),
        output_root / "MAGIC",
        output_root / "WIZARDS",
        output_root / "common",
        output_root / "asm-dedupe-index.json",
        output_root / "asm-dedupe-apply.json",
        output_root / "asm-diff-report",
        output_root / "ida55-function-match.json",
        output_root / "Load_SAVE_GAM.pair-diff.md",
        output_root / "Save_SAVE_GAM.pair-diff.md",
    ]
    if stage == "post-dump":
        paths.extend(
            [
                output_root / "ida55-sync",
            ]
        )
    else:
        paths.extend(
            [
                output_root / "ida55-sync" / "magic_equivalents.idc",
                output_root / "ida55-sync" / "wizards_equivalents.idc",
                output_root / "ida55-sync" / "manifest.json",
            ]
        )
    return paths


def existing_paths(paths: Iterable[Path]) -> List[Path]:
    return [path for path in paths if path.exists()]


def remove_path(path: Path) -> None:
    if path.is_dir():
        shutil.rmtree(path)
    else:
        path.unlink()


def main() -> int:
    args = parse_args()
    output_root = Path(args.output_root)
    targets = existing_paths(generated_paths(output_root, args.stage))

    if not targets:
        print("No generated artifacts matched the requested cleanup stage.")
        return 0

    action = "Removing" if args.apply else "Would remove"
    print(f"{action} {len(targets)} paths for stage={args.stage}:")
    for path in targets:
        print(f"- {path}")

    if not args.apply:
        print("Dry run only. Re-run with --apply to delete these paths.")
        return 0

    for path in targets:
        remove_path(path)
    print("Cleanup complete.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
