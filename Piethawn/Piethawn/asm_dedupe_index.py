#!/usr/bin/env python3
"""
Index split ASM procedures across MAGIC and WIZARDS and group duplicates.
"""

from __future__ import annotations

import argparse
from collections import defaultdict
from itertools import combinations
from pathlib import Path
from typing import DefaultDict, Dict, List, Sequence, Tuple

from asm_proc_tools import (
    ProcRecord,
    candidate_similarity,
    choose_canonical,
    diff_category,
    load_all_proc_records,
    write_json,
)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Build duplicate and similarity indexes for split ASM procedures.")
    parser.add_argument(
        "--root",
        default="out",
        help="Root folder containing MAGIC/ and WIZARDS/ split ASM output. Default: out",
    )
    parser.add_argument(
        "--output",
        default="out/asm-dedupe-index.json",
        help="JSON output manifest path.",
    )
    parser.add_argument(
        "--similarity-threshold",
        type=float,
        default=0.92,
        help="Threshold for near-match reporting. Default: 0.92",
    )
    return parser.parse_args()


def summarize_records(records: Sequence[ProcRecord]) -> Dict[str, object]:
    by_game: DefaultDict[str, int] = defaultdict(int)
    by_family: DefaultDict[str, int] = defaultdict(int)
    for record in records:
        by_game[record.game] += 1
        by_family[record.library_family] += 1
    return {
        "procedure_count": len(records),
        "by_game": dict(sorted(by_game.items())),
        "by_library_family": dict(sorted(by_family.items())),
    }


def group_by_hash(records: Sequence[ProcRecord], attr: str) -> List[Dict[str, object]]:
    buckets: DefaultDict[str, List[ProcRecord]] = defaultdict(list)
    for record in records:
        buckets[getattr(record, attr)].append(record)

    groups: List[Dict[str, object]] = []
    for key, members in sorted(buckets.items()):
        if len(members) < 2:
            continue
        canonical = choose_canonical(members)
        groups.append(
            {
                "group_id": f"{attr}:{key}",
                "match_type": attr,
                "canonical": canonical,
                "members": sorted(members, key=lambda item: item.relative_path),
                "games": sorted({item.game for item in members}),
                "library_families": sorted({item.library_family for item in members}),
            }
        )
    return groups


def pair_key(left: ProcRecord, right: ProcRecord) -> Tuple[str, str]:
    return tuple(sorted((left.relative_path, right.relative_path)))


def build_near_matches(records: Sequence[ProcRecord], threshold: float) -> List[Dict[str, object]]:
    pairs: List[Dict[str, object]] = []
    seen: set[Tuple[str, str]] = set()
    by_name: DefaultDict[str, List[ProcRecord]] = defaultdict(list)
    by_shape: DefaultDict[Tuple[str, str, int], List[ProcRecord]] = defaultdict(list)
    for record in records:
        by_name[record.proc_name.lower()].append(record)
        by_shape[(record.library_family, record.segment, record.instruction_count)].append(record)

    candidate_groups: List[Sequence[ProcRecord]] = list(by_name.values()) + [
        group for group in by_shape.values() if 1 < len(group) <= 12
    ]
    for group in candidate_groups:
        if len(group) < 2:
            continue
        for left, right in combinations(group, 2):
            if left.game == right.game:
                continue
            key = pair_key(left, right)
            if key in seen:
                continue
            seen.add(key)

            if left.raw_sha256 == right.raw_sha256 or left.normalized_sha256 == right.normalized_sha256:
                continue

            score = candidate_similarity(left, right)
            if score < threshold:
                continue

            category = diff_category(left, right)
            pairs.append(
                {
                    "left": left,
                    "right": right,
                    "similarity": round(score, 6),
                    "category": category,
                    "same_proc_name": left.proc_name.lower() == right.proc_name.lower(),
                    "same_segment": left.segment == right.segment,
                    "same_library_family": left.library_family == right.library_family,
                }
            )

    pairs.sort(key=lambda item: (-item["similarity"], item["left"].relative_path, item["right"].relative_path))
    return pairs


def main() -> int:
    args = parse_args()
    root = Path(args.root)
    records = load_all_proc_records(root)

    payload = {
        "summary": summarize_records(records),
        "exact_raw_groups": group_by_hash(records, "raw_sha256"),
        "normalized_groups": group_by_hash(records, "normalized_sha256"),
        "mnemonic_groups": group_by_hash(records, "mnemonic_sha256"),
        "near_matches": build_near_matches(records, args.similarity_threshold),
    }

    output_path = Path(args.output)
    output_path.parent.mkdir(parents=True, exist_ok=True)
    write_json(output_path, payload)
    print(f"Wrote {output_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
