#!/usr/bin/env python3
"""
Produce normalized diffs for matched split ASM procedures.
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Dict, List

from asm_proc_tools import (
    ProcRecord,
    diff_category,
    load_all_proc_records,
    unified_normalized_diff,
    write_json,
)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Generate normalized diff reports for duplicate candidates.")
    parser.add_argument("--root", default="out", help="Root folder containing MAGIC/ and WIZARDS/. Default: out")
    parser.add_argument(
        "--index",
        default="out/asm-dedupe-index.json",
        help="Duplicate index JSON produced by asm_dedupe_index.py",
    )
    parser.add_argument(
        "--output-dir",
        default="out/asm-diff-report",
        help="Directory where diff reports should be written.",
    )
    parser.add_argument(
        "--limit",
        type=int,
        default=250,
        help="Maximum number of pair reports to emit. Default: 250",
    )
    return parser.parse_args()


def build_record_map(records: List[ProcRecord]) -> Dict[str, ProcRecord]:
    return {record.relative_path: record for record in records}


def pair_items(index_data: Dict[str, object]) -> List[Dict[str, object]]:
    pairs: List[Dict[str, object]] = []
    for bucket_name in ("near_matches",):
        pairs.extend(index_data.get(bucket_name, []))

    for bucket_name in ("normalized_groups", "mnemonic_groups"):
        for group in index_data.get(bucket_name, []):
            canonical = group["canonical"]
            for member in group["members"]:
                if member["relative_path"] == canonical["relative_path"]:
                    continue
                pairs.append(
                    {
                        "left": canonical,
                        "right": member,
                        "category": bucket_name[:-1],
                        "similarity": 1.0,
                    }
                )
    return pairs


def safe_pair_name(left: ProcRecord, right: ProcRecord) -> str:
    label = f"{left.game}__{left.segment}__{left.proc_name}__VS__{right.game}__{right.segment}__{right.proc_name}"
    return "".join(ch if ch.isalnum() or ch in "._-" else "_" for ch in label)


def main() -> int:
    args = parse_args()
    root = Path(args.root)
    output_dir = Path(args.output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)

    records = load_all_proc_records(root)
    record_map = build_record_map(records)
    index_data = json.loads(Path(args.index).read_text(encoding="utf-8"))

    manifest: List[Dict[str, object]] = []
    for pair in pair_items(index_data)[: args.limit]:
        left = record_map[pair["left"]["relative_path"]]
        right = record_map[pair["right"]["relative_path"]]
        report_name = safe_pair_name(left, right) + ".diff"
        diff_text = unified_normalized_diff(left, right)
        report_path = output_dir / report_name
        report_path.write_text(diff_text or "# no normalized diff\n", encoding="utf-8")
        manifest.append(
            {
                "left": left,
                "right": right,
                "report_path": str(report_path),
                "category": diff_category(left, right),
                "similarity": pair.get("similarity", 1.0),
            }
        )

    write_json(output_dir / "manifest.json", {"pair_count": len(manifest), "pairs": manifest})
    print(f"Wrote {output_dir}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
