#!/usr/bin/env python3
"""Build a normalized procedure inventory from an IDA 5.5 function dump."""

from __future__ import annotations

import argparse
from pathlib import Path

from piethawn_sync.ida_functions import load_inventory
from piethawn_sync.jsonio import write_json


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Build a WZD/MGC procedure inventory artifact.")
    parser.add_argument("function_dump", type=Path, help="IDA function dump JSON, usually in/*.ida55-functions.json")
    parser.add_argument("--database", required=True, help="Database label for this inventory, for example MGC or WZD.")
    parser.add_argument(
        "--output",
        type=Path,
        default=None,
        help="Output JSON path. Defaults to out/wzd-mgc-sync/<database>-procedure-inventory.json.",
    )
    return parser.parse_args()


def default_output_path(database: str) -> Path:
    return Path("out") / "wzd-mgc-sync" / f"{database.lower()}-procedure-inventory.json"


def main() -> int:
    args = parse_args()
    database = args.database.upper()
    inventory = load_inventory(args.function_dump, database)
    output_path = args.output or default_output_path(database)
    write_json(output_path, inventory.to_dict())

    print(f"Wrote {output_path}")
    print(f"Database: {database}")
    print(f"Procedures: {inventory.function_count}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

