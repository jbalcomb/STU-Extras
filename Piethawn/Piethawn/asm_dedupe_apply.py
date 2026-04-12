#!/usr/bin/env python3
"""
Materialize canonical shared ASM files and replace duplicates with references.

This is intentionally conservative:
- default mode is dry-run
- only normalized or raw duplicate groups are considered
- duplicate files are renamed to *.duplicate.json unless --in-place is omitted
"""

from __future__ import annotations

import argparse
import json
import shutil
from pathlib import Path
from typing import Dict, List


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Apply duplicate manifests to split ASM folders.")
    parser.add_argument(
        "--index",
        default="out/asm-dedupe-index.json",
        help="Duplicate index JSON produced by asm_dedupe_index.py",
    )
    parser.add_argument(
        "--common-root",
        default="out/common",
        help="Destination root for canonical shared procedures.",
    )
    parser.add_argument(
        "--manifest",
        default="out/asm-dedupe-apply.json",
        help="Path to the apply manifest to write.",
    )
    parser.add_argument(
        "--in-place",
        action="store_true",
        help="Actually create the common tree and replace duplicate files with reference JSON files.",
    )
    return parser.parse_args()


def eligible_groups(index_data: Dict[str, object]) -> List[Dict[str, object]]:
    groups: List[Dict[str, object]] = []
    for bucket_name in ("exact_raw_groups", "normalized_groups"):
        for group in index_data.get(bucket_name, []):
            groups.append({"bucket": bucket_name, **group})
    return groups


def common_path(common_root: Path, canonical: Dict[str, object]) -> Path:
    return common_root / canonical["library_family"] / canonical["game"] / canonical["segment"] / f'{canonical["proc_name"]}.asm'


def main() -> int:
    args = parse_args()
    common_root = Path(args.common_root)
    manifest_path = Path(args.manifest)
    index_data = json.loads(Path(args.index).read_text(encoding="utf-8"))

    actions: List[Dict[str, object]] = []
    for group in eligible_groups(index_data):
        canonical = group["canonical"]
        canon_path = Path(canonical["absolute_path"])
        shared_path = common_path(common_root, canonical)
        action = {
            "bucket": group["bucket"],
            "group_id": group["group_id"],
            "canonical_source": canonical["absolute_path"],
            "canonical_shared_path": str(shared_path),
            "duplicates": [],
        }

        for member in group["members"]:
            if member["relative_path"] == canonical["relative_path"]:
                continue
            duplicate_path = Path(member["absolute_path"])
            ref_payload = {
                "duplicate_of": canonical["relative_path"],
                "canonical_shared_path": str(shared_path),
                "bucket": group["bucket"],
                "group_id": group["group_id"],
            }
            action["duplicates"].append(
                {
                    "source": member["absolute_path"],
                    "reference_path": str(duplicate_path.with_suffix(".duplicate.json")),
                    "reference_payload": ref_payload,
                }
            )

        if args.in_place:
            shared_path.parent.mkdir(parents=True, exist_ok=True)
            if not shared_path.exists():
                shutil.copyfile(canon_path, shared_path)
            for duplicate in action["duplicates"]:
                duplicate_path = Path(duplicate["source"])
                ref_path = Path(duplicate["reference_path"])
                if duplicate_path.exists():
                    duplicate_path.unlink()
                ref_path.write_text(json.dumps(duplicate["reference_payload"], indent=2), encoding="utf-8")

        actions.append(action)

    manifest_path.parent.mkdir(parents=True, exist_ok=True)
    manifest_path.write_text(json.dumps({"applied": args.in_place, "action_count": len(actions), "actions": actions}, indent=2), encoding="utf-8")
    print(f"Wrote {manifest_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
