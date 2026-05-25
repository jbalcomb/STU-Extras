#!/usr/bin/env python3
"""Compare joined procedure evidence using a segment map and procedure ordinal."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any, Dict, List, Tuple


def load_json(path: Path) -> Dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8"))


def write_json(path: Path, payload: Dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")


def procedure_key(record: Dict[str, Any]) -> Tuple[str, int]:
    return (str(record["segment"]), int(record["procedure_ordinal"]))


def mapped_segment_pairs(segment_map: Dict[str, Any], left_database: str, right_database: str) -> List[Dict[str, Any]]:
    left_field = f"{left_database.lower()}_segment"
    right_field = f"{right_database.lower()}_segment"
    pairs: List[Dict[str, Any]] = []
    for pair in segment_map.get("segment_pairs", []):
        left_segment = pair.get("left_segment", pair.get(left_field))
        right_segment = pair.get("right_segment", pair.get(right_field))
        if left_segment is None or right_segment is None:
            raise ValueError(f"Segment pair lacks {left_field}/{right_field}: {pair}")
        normalized = dict(pair)
        normalized["left_segment"] = str(left_segment)
        normalized["right_segment"] = str(right_segment)
        pairs.append(normalized)
    return pairs


def body(record: Dict[str, Any]) -> Dict[str, Any]:
    return dict(record.get("asm_body") or {})


def identity(record: Dict[str, Any]) -> Dict[str, Any]:
    return {
        "database": record.get("database"),
        "name": record.get("name"),
        "segment": record.get("segment"),
        "procedure_ordinal": record.get("procedure_ordinal"),
        "start_ea": record.get("start_ea"),
        "end_ea": record.get("end_ea"),
        "size": record.get("size"),
        "asm_body_name": body(record).get("name"),
    }


def observations(left: Dict[str, Any], right: Dict[str, Any]) -> Dict[str, Any]:
    left_body = body(left)
    right_body = body(right)
    if not left_body or not right_body:
        raise ValueError("Joined procedure evidence lacks asm_body data.")
    fields = [
        "raw_body_sha256",
        "normalized_body_sha256",
        "mnemonic_sequence_sha256",
        "raw_line_count",
        "normalized_line_count",
        "instruction_count",
    ]
    comparable_fields = [field for field in fields if left_body.get(field) is not None and right_body.get(field) is not None]
    return {
        "same_idb_name": left.get("name") == right.get("name"),
        "same_size": left.get("size") == right.get("size"),
        "same_raw_body_hash": (
            left_body.get("raw_body_sha256") == right_body.get("raw_body_sha256")
            if "raw_body_sha256" in comparable_fields
            else None
        ),
        "same_normalized_body_hash": (
            left_body.get("normalized_body_sha256") == right_body.get("normalized_body_sha256")
            if "normalized_body_sha256" in comparable_fields
            else None
        ),
        "same_mnemonic_sequence_hash": (
            left_body.get("mnemonic_sequence_sha256") == right_body.get("mnemonic_sequence_sha256")
            if "mnemonic_sequence_sha256" in comparable_fields
            else None
        ),
        "comparable_body_fields": comparable_fields,
        "matching_body_fields": [field for field in comparable_fields if left_body.get(field) == right_body.get(field)],
        "different_body_fields": [field for field in comparable_fields if left_body.get(field) != right_body.get(field)],
    }


def compare_procedure_evidence(
    left: Dict[str, Any],
    right: Dict[str, Any],
    segment_map: Dict[str, Any],
) -> Dict[str, Any]:
    left_database = str(left.get("database"))
    right_database = str(right.get("database"))
    pairs = mapped_segment_pairs(segment_map, left_database, right_database)
    left_by_key = {procedure_key(record): record for record in left.get("procedures", [])}
    right_by_key = {procedure_key(record): record for record in right.get("procedures", [])}

    procedure_pairs: List[Dict[str, Any]] = []
    for segment_pair in pairs:
        left_segment = str(segment_pair["left_segment"])
        right_segment = str(segment_pair["right_segment"])
        left_ordinals = {ordinal for segment, ordinal in left_by_key if segment == left_segment}
        right_ordinals = {ordinal for segment, ordinal in right_by_key if segment == right_segment}
        if left_ordinals != right_ordinals:
            raise ValueError(
                "Mapped segment procedure ordinal mismatch: "
                f"{left_database}:{left_segment} has {len(left_ordinals)} ordinals, "
                f"{right_database}:{right_segment} has {len(right_ordinals)} ordinals."
            )
        for ordinal in sorted(left_ordinals | right_ordinals):
            left_record = left_by_key.get((left_segment, ordinal))
            right_record = right_by_key.get((right_segment, ordinal))
            procedure_pairs.append(
                {
                    "pairing_rule": "mapped_segment_and_procedure_ordinal",
                    "segment_pair": dict(segment_pair),
                    "procedure_ordinal": ordinal,
                    "left": identity(left_record),
                    "right": identity(right_record),
                    "observations": observations(left_record, right_record),
                }
            )

    return {
        "schema_version": 1,
        "artifact": "procedure_evidence_comparison",
        "left_database": left_database,
        "right_database": right_database,
        "pairing_rule": "mapped_segment_and_procedure_ordinal",
        "summary": {
            "segment_pair_count": len(pairs),
            "procedure_pair_count": len(procedure_pairs),
            "same_normalized_body_hash_count": sum(
                1 for pair in procedure_pairs if pair["observations"]["same_normalized_body_hash"]
            ),
            "different_normalized_body_hash_count": sum(
                1 for pair in procedure_pairs if not pair["observations"]["same_normalized_body_hash"]
            ),
            "same_mnemonic_sequence_hash_count": sum(
                1 for pair in procedure_pairs if pair["observations"]["same_mnemonic_sequence_hash"]
            ),
            "different_name_count": sum(1 for pair in procedure_pairs if not pair["observations"]["same_idb_name"]),
        },
        "segment_pairs": pairs,
        "procedure_pairs": procedure_pairs,
    }


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Compare joined procedure evidence using a segment map.")
    parser.add_argument("--left", type=Path, required=True, help="Left joined procedure evidence JSON.")
    parser.add_argument("--right", type=Path, required=True, help="Right joined procedure evidence JSON.")
    parser.add_argument("--segment-map", type=Path, required=True, help="Segment map JSON.")
    parser.add_argument("--output", type=Path, required=True, help="Output comparison JSON.")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    comparison = compare_procedure_evidence(load_json(args.left), load_json(args.right), load_json(args.segment_map))
    write_json(args.output, comparison)
    summary = comparison["summary"]
    print(f"Wrote {args.output}")
    print(f"Procedure pairs: {summary['procedure_pair_count']}")
    print(f"Same normalized body hash: {summary['same_normalized_body_hash_count']}")
    print(f"Different normalized body hash: {summary['different_normalized_body_hash_count']}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
