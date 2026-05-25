#!/usr/bin/env python3
"""Build a structural procedure pair draft from generic IDA inventories."""

from __future__ import annotations

import argparse
from pathlib import Path

from piethawn_sync.jsonio import load_json, write_json
from piethawn_sync.procedure_compare import compare_inventories, load_segment_pairs_from_map


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Build an IDA55 procedure pair draft from inventory JSON files.")
    parser.add_argument("--left", type=Path, required=True, help="Left procedure inventory JSON.")
    parser.add_argument("--right", type=Path, required=True, help="Right procedure inventory JSON.")
    parser.add_argument("--segment-map", type=Path, help="Segment map JSON with a segment_pairs list.")
    parser.add_argument("--output", type=Path, required=True, help="Output procedure pair draft JSON.")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    left_inventory = load_json(args.left)
    right_inventory = load_json(args.right)
    segment_pairs = None
    if args.segment_map:
        segment_pairs = load_segment_pairs_from_map(
            load_json(args.segment_map),
            str(left_inventory.get("database")),
            str(right_inventory.get("database")),
        )
    report = compare_inventories(left_inventory, right_inventory, segment_pairs)
    write_json(args.output, report)

    summary = report["summary"]
    print(f"Wrote {args.output}")
    print(f"Left database: {report['left_database']}")
    print(f"Right database: {report['right_database']}")
    print(f"Segment pairs: {summary['segment_pair_count']}")
    print(f"Procedure pairs: {summary['procedure_pair_count']}")
    print(f"Different-name observations: {summary['different_name_observation_count']}")
    print(f"Evidence differences: {summary['evidence_difference_count']}")
    print(f"Left unpaired: {summary['left_unpaired_count']}")
    print(f"Right unpaired: {summary['right_unpaired_count']}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
