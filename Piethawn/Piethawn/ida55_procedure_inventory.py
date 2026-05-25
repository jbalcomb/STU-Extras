#!/usr/bin/env python3
"""Convert a generic IDA 5.5 procedure snapshot into inventory JSON."""

from __future__ import annotations

import argparse
from pathlib import Path

from piethawn_sync.ida_snapshot import load_inventory_from_snapshot
from piethawn_sync.jsonio import write_json


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Convert an IDA55 procedure snapshot into inventory JSON.")
    parser.add_argument("snapshot", type=Path, help="Snapshot text produced by ida55_procedure_snapshot.idc")
    parser.add_argument("--database", required=True, help="Database label, for example MGC or WZD.")
    parser.add_argument("--output", type=Path, required=True, help="Output procedure inventory JSON path.")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    database = args.database.upper()
    inventory = load_inventory_from_snapshot(args.snapshot, database)
    write_json(args.output, inventory.to_dict())

    print(f"Wrote {args.output}")
    print(f"Database: {database}")
    print(f"Procedures: {inventory.function_count}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

