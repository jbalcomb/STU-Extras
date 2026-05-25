#!/usr/bin/env python3
"""Build procedure body evidence JSON from an IDA55 ASM export."""

from __future__ import annotations

import argparse
from pathlib import Path

from piethawn_sync.asm_evidence import build_asm_evidence_from_text
from piethawn_sync.jsonio import write_json


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Build procedure body hash evidence from an IDA55 ASM export.")
    parser.add_argument("asm_file", type=Path, help="IDA-generated ASM export.")
    parser.add_argument("--database", required=True, help="Database label, for example MGC or WZD.")
    parser.add_argument("--output", type=Path, required=True, help="Output ASM procedure evidence JSON.")
    parser.add_argument(
        "--default-segment",
        default="_no_segment_",
        help="Segment name used if the ASM export contains procedure blocks outside a segment.",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    database = args.database.upper()
    text = args.asm_file.read_text(encoding="utf-8", errors="replace")
    evidence = build_asm_evidence_from_text(
        text,
        database=database,
        source_path=str(args.asm_file),
        default_segment=args.default_segment,
    )
    write_json(args.output, evidence)

    print(f"Wrote {args.output}")
    print(f"Database: {database}")
    print(f"ASM procedures: {evidence['procedure_count']}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
