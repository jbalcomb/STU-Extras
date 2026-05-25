"""Parse generic IDA 5.5 procedure snapshot files."""

from __future__ import annotations

import re
from pathlib import Path
from typing import Any, Dict, List, Tuple

from .models import ProcedureInventory, ProcedureRecord

INDEXED_RE = re.compile(r"^(?P<section>[A-Za-z_]+)\[(?P<index>\d+)\]\.(?P<field>[A-Za-z_]+)$")


def ea_sort_key(value: object) -> int:
    text = str(value or "")
    if ":" in text:
        text = text.split(":", 1)[1]
    if text.lower().startswith("0x"):
        text = text[2:]
    try:
        return int(text, 16)
    except ValueError:
        return 0


def parse_snapshot_text(text: str) -> Tuple[Dict[str, str], Dict[str, List[Dict[str, str]]]]:
    meta: Dict[str, str] = {}
    sections: Dict[str, Dict[int, Dict[str, str]]] = {}

    for line_number, raw_line in enumerate(text.splitlines(), start=1):
        line = raw_line.strip()
        if not line:
            continue
        if "=" not in line:
            raise ValueError(f"Line {line_number} is not key=value: {raw_line}")
        key, value = line.split("=", 1)
        match = INDEXED_RE.match(key)
        if match:
            section = match.group("section")
            index = int(match.group("index"))
            field = match.group("field")
            sections.setdefault(section, {}).setdefault(index, {})[field] = value
        else:
            meta[key] = value

    ordered_sections = {
        section: [records[index] for index in sorted(records)]
        for section, records in sections.items()
    }
    return meta, ordered_sections


def required_field(record: Dict[str, str], field_name: str, record_index: int) -> str:
    value = record.get(field_name)
    if value is None or value == "":
        raise ValueError(f"Procedure #{record_index} is missing required field: {field_name}")
    return value


def int_field(record: Dict[str, str], field_name: str, record_index: int) -> int:
    value = required_field(record, field_name, record_index)
    try:
        return int(value)
    except ValueError as error:
        raise ValueError(f"Procedure #{record_index} has non-integer field: {field_name}") from error


def build_inventory_from_snapshot_text(text: str, database: str, source_path: Path) -> ProcedureInventory:
    meta, sections = parse_snapshot_text(text)
    procedures = sections.get("procedure", [])
    by_segment: Dict[str, List[Dict[str, str]]] = {}
    for procedure in procedures:
        by_segment.setdefault(procedure.get("segment", ""), []).append(procedure)

    ordinals: Dict[int, int] = {}
    for segment_records in by_segment.values():
        ordered = sorted(segment_records, key=lambda item: (ea_sort_key(item.get("start_ea")), item.get("name", "")))
        for ordinal, procedure in enumerate(ordered, start=1):
            ordinals[id(procedure)] = ordinal

    records: List[ProcedureRecord] = []
    for index, procedure in enumerate(
        sorted(procedures, key=lambda item: (item.get("segment", ""), ea_sort_key(item.get("start_ea")), item.get("name", ""))),
        start=1,
    ):
        records.append(
            ProcedureRecord(
                database=database,
                name=required_field(procedure, "name", index),
                segment=required_field(procedure, "segment", index),
                procedure_ordinal=ordinals[id(procedure)],
                start_ea=required_field(procedure, "start_ea", index),
                end_ea=required_field(procedure, "end_ea", index),
                size=int_field(procedure, "size", index),
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


def load_inventory_from_snapshot(snapshot_path: Path, database: str) -> ProcedureInventory:
    return build_inventory_from_snapshot_text(snapshot_path.read_text(encoding="utf-8"), database, snapshot_path)
