#!/usr/bin/env python3
"""Combine IDB procedure metadata with ASM body evidence."""

from __future__ import annotations

import argparse
from pathlib import Path

from piethawn_sync.asm_evidence import combine_inventory_with_asm_evidence
from piethawn_sync.jsonio import load_json, write_json


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Combine IDA55 procedure inventory JSON with ASM evidence JSON.")
    parser.add_argument("--inventory", type=Path, required=True, help="Procedure inventory JSON.")
    parser.add_argument("--asm-evidence", type=Path, required=True, help="ASM procedure evidence JSON.")
    parser.add_argument("--output", type=Path, required=True, help="Output combined procedure evidence JSON.")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    combined = combine_inventory_with_asm_evidence(
        load_json(args.inventory),
        load_json(args.asm_evidence),
    )
    write_json(args.output, combined)

    summary = combined["summary"]
    print(f"Wrote {args.output}")
    print(f"Database: {combined['database']}")
    print(f"Enriched procedures: {summary['enriched_procedure_count']}")
    print(f"Missing evidence: {summary['missing_evidence_count']}")
    print(f"Unused evidence: {summary['unused_evidence_count']}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
