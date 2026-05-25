#!/usr/bin/env python3
"""Bucket procedure evidence comparison results for review."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any, Dict, List


BUCKET_LABELS = {
    "same_normalized_hash_different_name": "Same normalized body hash, different name",
    "same_mnemonic_hash_different_normalized_hash": "Same mnemonic hash, different normalized hash",
    "same_name_different_normalized_hash": "Same name, different normalized body hash",
    "different_name_different_normalized_hash": "Different name, different normalized body hash",
}


def load_json(path: Path) -> Dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8"))


def write_json(path: Path, payload: Dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")


def pair_summary(pair: Dict[str, Any]) -> Dict[str, Any]:
    left = pair["left"]
    right = pair["right"]
    observations = pair["observations"]
    return {
        "segment_pair_id": pair.get("segment_pair", {}).get("pair_id"),
        "left_segment": left.get("segment"),
        "right_segment": right.get("segment"),
        "procedure_ordinal": pair.get("procedure_ordinal"),
        "left_name": left.get("name"),
        "right_name": right.get("name"),
        "left_start_ea": left.get("start_ea"),
        "right_start_ea": right.get("start_ea"),
        "same_size": observations.get("same_size"),
        "same_normalized_body_hash": observations.get("same_normalized_body_hash"),
        "same_mnemonic_sequence_hash": observations.get("same_mnemonic_sequence_hash"),
        "different_body_fields": list(observations.get("different_body_fields", [])),
    }


def bucket_name(pair: Dict[str, Any]) -> str:
    observations = pair["observations"]
    same_name = observations.get("same_idb_name")
    same_normalized = observations.get("same_normalized_body_hash")
    same_mnemonic = observations.get("same_mnemonic_sequence_hash")
    if same_normalized and not same_name:
        return "same_normalized_hash_different_name"
    if same_mnemonic and not same_normalized:
        return "same_mnemonic_hash_different_normalized_hash"
    if same_name and not same_normalized:
        return "same_name_different_normalized_hash"
    if not same_name and not same_normalized:
        return "different_name_different_normalized_hash"
    return "same_name_same_normalized_hash"


def bucket_comparison(comparison: Dict[str, Any]) -> Dict[str, Any]:
    bucket_order = [
        "same_normalized_hash_different_name",
        "same_mnemonic_hash_different_normalized_hash",
        "same_name_different_normalized_hash",
        "different_name_different_normalized_hash",
        "same_name_same_normalized_hash",
    ]
    buckets: Dict[str, List[Dict[str, Any]]] = {name: [] for name in bucket_order}

    for pair in comparison.get("procedure_pairs", []):
        buckets.setdefault(bucket_name(pair), []).append(pair_summary(pair))

    bucket_counts = {name: len(buckets.get(name, [])) for name in bucket_order}
    return {
        "schema_version": 1,
        "artifact": "procedure_comparison_review_buckets",
        "source_artifact": comparison.get("artifact"),
        "left_database": comparison.get("left_database"),
        "right_database": comparison.get("right_database"),
        "pairing_rule": comparison.get("pairing_rule"),
        "source_summary": comparison.get("summary", {}),
        "bucket_counts": bucket_counts,
        "buckets": buckets,
    }


def format_pair(item: Dict[str, Any]) -> str:
    return (
        f"- `{item.get('left_segment')}:{item.get('procedure_ordinal')}` "
        f"`{item.get('left_name')}` -> `{item.get('right_name')}` "
        f"({item.get('left_start_ea')} -> {item.get('right_start_ea')})"
    )


def render_markdown(report: Dict[str, Any], limit_per_bucket: int) -> str:
    lines = [
        "# Procedure Comparison Review Buckets",
        "",
        f"Left database: `{report.get('left_database')}`",
        f"Right database: `{report.get('right_database')}`",
        f"Pairing rule: `{report.get('pairing_rule')}`",
        "",
        "## Counts",
        "",
    ]
    for bucket, count in report["bucket_counts"].items():
        if count == 0:
            continue
        label = BUCKET_LABELS.get(bucket, bucket.replace("_", " "))
        lines.append(f"- `{bucket}`: {count} - {label}")

    lines.extend(["", "## Review Buckets", ""])
    for bucket, items in report["buckets"].items():
        if not items:
            continue
        label = BUCKET_LABELS.get(bucket, bucket.replace("_", " "))
        lines.extend([f"### {label}", "", f"Count: {len(items)}", ""])
        for item in items[:limit_per_bucket]:
            lines.append(format_pair(item))
        if len(items) > limit_per_bucket:
            lines.append(f"- ... {len(items) - limit_per_bucket} more")
        lines.append("")
    return "\n".join(lines).rstrip() + "\n"


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Bucket a procedure evidence comparison into review categories.")
    parser.add_argument("comparison", type=Path, help="Procedure evidence comparison JSON.")
    parser.add_argument("--json-output", type=Path, required=True, help="Output bucketed JSON report.")
    parser.add_argument("--markdown-output", type=Path, required=True, help="Output bucketed Markdown report.")
    parser.add_argument("--limit-per-bucket", type=int, default=40, help="Maximum rows per Markdown bucket.")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    report = bucket_comparison(load_json(args.comparison))
    write_json(args.json_output, report)
    args.markdown_output.parent.mkdir(parents=True, exist_ok=True)
    args.markdown_output.write_text(render_markdown(report, args.limit_per_bucket), encoding="utf-8")
    print(f"Wrote {args.json_output}")
    print(f"Wrote {args.markdown_output}")
    for bucket, count in report["bucket_counts"].items():
        if count == 0:
            continue
        print(f"{bucket}: {count}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
