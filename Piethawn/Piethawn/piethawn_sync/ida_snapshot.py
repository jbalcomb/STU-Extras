"""Parse IDA 5.5 key=value export text (meta lines plus section[index].field records).

Keys may nest: function[0].chunk[1].bytes puts a "chunk" list inside each "function" record.
All values stay strings; convert_int_fields() turns an explicit list of fields into integers.
"""

from __future__ import annotations

import re
from typing import Any, Dict, List, Mapping, Set, Tuple

KEY_PART_RE = re.compile(r"^(?P<name>[A-Za-z_][A-Za-z0-9_]*)(?:\[(?P<index>\d+)\])?$")


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


def split_indexed_key(key: str) -> List[Tuple[str, int]] | None:
    """Split "a[0].b[1].c" into [("a", 0), ("b", 1)] plus field "c"; None for a plain meta key. Powered by Claude."""
    parts = key.split(".")
    if len(parts) < 2:
        return None
    path: List[Tuple[str, int]] = []
    for part in parts[:-1]:
        match = KEY_PART_RE.match(part)
        if not match or match.group("index") is None:
            return None
        path.append((match.group("name"), int(match.group("index"))))
    field_match = KEY_PART_RE.match(parts[-1])
    if not field_match or field_match.group("index") is not None:
        return None
    return path


def finalize_records(records: Dict[int, Dict[str, Any]]) -> List[Dict[str, Any]]:
    """Turn {index: record} maps into index-ordered lists, recursing into nested sections. Powered by Claude."""
    ordered: List[Dict[str, Any]] = []
    for index in sorted(records):
        record = records[index]
        for name, value in list(record.items()):
            if isinstance(value, dict):
                record[name] = finalize_records(value)
        ordered.append(record)
    return ordered


def parse_snapshot_text(text: str) -> Tuple[Dict[str, str], Dict[str, List[Dict[str, Any]]]]:
    meta: Dict[str, str] = {}
    sections: Dict[str, Dict[int, Dict[str, Any]]] = {}

    for line_number, raw_line in enumerate(text.splitlines(), start=1):
        line = raw_line.strip()
        if not line:
            continue
        if "=" not in line:
            raise ValueError(f"Line {line_number} is not key=value: {raw_line}")
        key, value = line.split("=", 1)
        path = split_indexed_key(key)
        if path is None:
            meta[key] = value
            continue

        container = sections
        record: Dict[str, Any] = {}
        for depth, (name, index) in enumerate(path):
            record = container.setdefault(name, {}).setdefault(index, {})
            if depth < len(path) - 1:
                container = record
        record[key.rsplit(".", 1)[1]] = value

    ordered_sections = {name: finalize_records(records) for name, records in sections.items()}
    return meta, ordered_sections


def to_int(value: str, label: str) -> int:
    try:
        return int(value)
    except ValueError as error:
        raise ValueError(f"{label} is not an integer: {value!r}") from error


def convert_int_fields(
    meta: Dict[str, Any],
    sections: Dict[str, List[Dict[str, Any]]],
    int_fields: Mapping[str, Set[str]],
) -> None:
    """Convert the listed fields to int in place; keys are "" for meta, else dotted section paths ("function.chunk"). Powered by Claude."""
    for field in int_fields.get("", set()):
        if field in meta:
            meta[field] = to_int(meta[field], field)

    def walk(records: List[Dict[str, Any]], path: str) -> None:
        fields = int_fields.get(path, set())
        for index, record in enumerate(records):
            for field, value in record.items():
                if isinstance(value, list):
                    walk(value, f"{path}.{field}")
                elif field in fields:
                    record[field] = to_int(value, f"{path}[{index}].{field}")

    for name, records in sections.items():
        walk(records, name)


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
