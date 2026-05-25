"""Normalize IDA 5.5 function dump JSON into sync inventories."""

from __future__ import annotations

from collections import defaultdict
from pathlib import Path
from typing import Any, DefaultDict, Dict, Iterable, List

from .jsonio import load_json
from .models import ProcedureInventory, ProcedureRecord


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


def text_field(item: Dict[str, Any], field_name: str, index: int) -> str:
    value = item.get(field_name)
    if value is None or str(value) == "":
        raise ValueError(f"Function #{index} is missing required field: {field_name}")
    return str(value)


def int_field(item: Dict[str, Any], field_name: str, index: int) -> int:
    value = item.get(field_name)
    if value is None:
        raise ValueError(f"Function #{index} is missing required field: {field_name}")
    try:
        return int(value)
    except (TypeError, ValueError) as error:
        raise ValueError(f"Function #{index} has non-integer field: {field_name}") from error


def sorted_segment_functions(functions: Iterable[Dict[str, Any]]) -> List[Dict[str, Any]]:
    return sorted(
        functions,
        key=lambda item: (
            str(item.get("segment", "")),
            ea_sort_key(item.get("start_ea")),
            str(item.get("name", "")),
        ),
    )


def build_inventory(function_dump: Dict[str, Any], database: str, source_path: Path) -> ProcedureInventory:
    raw_functions = function_dump.get("functions", [])
    if not isinstance(raw_functions, list):
        raise ValueError("Function dump field 'functions' must be a list.")

    by_segment: DefaultDict[str, List[Dict[str, Any]]] = defaultdict(list)
    for item in raw_functions:
        if not isinstance(item, dict):
            raise ValueError("Each function record must be an object.")
        by_segment[str(item.get("segment", ""))].append(item)

    ordinals: Dict[int, int] = {}
    for segment_functions in by_segment.values():
        for ordinal, item in enumerate(sorted_segment_functions(segment_functions), start=1):
            ordinals[id(item)] = ordinal

    procedures: List[ProcedureRecord] = []
    for index, item in enumerate(sorted_segment_functions(raw_functions), start=1):
        chunk_hashes = item.get("chunk_bytes_sha256", [])
        if chunk_hashes is None:
            chunk_hashes = []
        if not isinstance(chunk_hashes, list):
            raise ValueError(f"Function #{index} has non-list field: chunk_bytes_sha256")

        procedures.append(
            ProcedureRecord(
                database=database,
                name=text_field(item, "name", index),
                segment=text_field(item, "segment", index),
                procedure_ordinal=ordinals[id(item)],
                start_ea=text_field(item, "start_ea", index),
                end_ea=text_field(item, "end_ea", index),
                size=int_field(item, "size", index),
                bytes_sha256=str(item["bytes_sha256"]) if item.get("bytes_sha256") else None,
                chunk_bytes_sha256=[str(value) for value in chunk_hashes],
            )
        )

    meta = function_dump.get("meta", {})
    if not isinstance(meta, dict):
        meta = {}

    return ProcedureInventory(
        database=database,
        source_path=str(source_path),
        idb_path=str(meta["idb_path"]) if meta.get("idb_path") else None,
        input_file=str(meta["input_file"]) if meta.get("input_file") else None,
        function_count=len(procedures),
        procedures=procedures,
    )


def load_inventory(function_dump_path: Path, database: str) -> ProcedureInventory:
    return build_inventory(load_json(function_dump_path), database, function_dump_path)
