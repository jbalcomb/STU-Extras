#!/usr/bin/env python3
"""
Normalize a split ASM procedure file for duplicate analysis.
"""

from __future__ import annotations

import argparse
from pathlib import Path

from asm_proc_tools import normalize_asm


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Normalize an ASM procedure file.")
    parser.add_argument("asm_file", help="Procedure .asm file to normalize.")
    parser.add_argument("--strict", action="store_true", help="Strip comments and decimal literals more aggressively.")
    parser.add_argument(
        "--mode",
        choices=("normalized", "mnemonic", "both"),
        default="normalized",
        help="Which normalized view to print.",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    text = Path(args.asm_file).read_text(encoding="utf-8", errors="replace")
    normalized_text, mnemonic_text = normalize_asm(text, strict=args.strict)
    if args.mode in {"normalized", "both"}:
        print(normalized_text)
    if args.mode == "both":
        print("\n--- mnemonic ---")
    if args.mode in {"mnemonic", "both"}:
        print(mnemonic_text)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
