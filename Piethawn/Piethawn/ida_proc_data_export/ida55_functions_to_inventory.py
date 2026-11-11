#!/usr/bin/env python3
"""Build a procedure inventory from an ida55 function-data dump.

The evidence-based name-sync chain historically consumed a dedicated
`ida55_procedure_snapshot.idc` export. That snapshot is a strict subset of the
`ida55_dump_functions.idc` function-data dump the pipeline already produces:
both emit name/segment/start_ea/end_ea/size per function via the same
GetFunctionAttr calls. This adapter derives the same ProcedureInventory from the
function dump so the evidence chain needs no separate IDA export. Powered by Claude.
"""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

if __package__ in (None, ""):
    sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from piethawn_sync.ida_snapshot import ea_sort_key, int_field, parse_snapshot_text, required_field
from piethawn_sync.jsonio import write_json
from piethawn_sync.models import ProcedureInventory, ProcedureRecord


def build_inventory_from_functions_text(text: str, database: str, source_path: Path) -> ProcedureInventory:
    """Convert function-dump text into the same inventory shape as the snapshot path. Powered by Claude."""
    meta, sections = parse_snapshot_text(text)
    functions = sections.get("function", [])

    by_segment: dict[str, list[dict[str, str]]] = {}
    for function in functions:
        by_segment.setdefault(function.get("segment", ""), []).append(function)

    ordinals: dict[int, int] = {}
    for segment_records in by_segment.values():
        ordered = sorted(segment_records, key=lambda item: (ea_sort_key(item.get("start_ea")), item.get("name", "")))
        for ordinal, function in enumerate(ordered, start=1):
            ordinals[id(function)] = ordinal

    records: list[ProcedureRecord] = []
    for index, function in enumerate(
        sorted(functions, key=lambda item: (item.get("segment", ""), ea_sort_key(item.get("start_ea")), item.get("name", ""))),
        start=1,
    ):
        records.append(
            ProcedureRecord(
                database=database,
                name=required_field(function, "name", index),
                segment=required_field(function, "segment", index),
                procedure_ordinal=ordinals[id(function)],
                start_ea=required_field(function, "start_ea", index),
                end_ea=required_field(function, "end_ea", index),
                size=int_field(function, "size", index),
            )
        )

    return ProcedureInventory(
        database=database,
        source_path=str(source_path),
        idb_path=meta.get("idb_path"),
        input_file=meta.get("input_file"),
        function_count=len(records),
        procedures=records,
    )


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Convert an ida55 function-data dump into procedure inventory JSON.")
    parser.add_argument("functions_txt", type=Path, help="Function dump text produced by ida55_dump_functions.idc")
    parser.add_argument("--database", required=True, help="Database label, for example MGC or WZD.")
    parser.add_argument("--output", type=Path, required=True, help="Output procedure inventory JSON path.")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    database = args.database.upper()
    text = args.functions_txt.read_text(encoding="utf-8")
    inventory = build_inventory_from_functions_text(text, database, args.functions_txt)
    write_json(args.output, inventory.to_dict())

    print(f"Wrote {args.output}")
    print(f"Database: {database}")
    print(f"Procedures: {inventory.function_count}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
