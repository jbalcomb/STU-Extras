"""Build the function-dump JSON (meta, segments, functions with chunks and byte hashes) from
ida55_dump_functions.idc text, on top of the shared key=value parser."""

from __future__ import annotations

import hashlib
from pathlib import Path
from typing import Any, Dict

from .ida_snapshot import convert_int_fields, parse_snapshot_text
from .jsonio import write_json

# Every other field (names, addresses, *_hex colors, bytes) stays a string.
FUNCTION_DUMP_INT_FIELDS = {
    "": {"default_color", "segment_count", "function_count"},
    "segment": {"align", "comb"},
    "function": {
        "size",
        "flags",
        "frame",
        "frsize",
        "frregs",
        "argsize",
        "fpd",
        "funcattr_color",
        "item_color",
        "has_user_color",
        "chunk_count",
        "tail_refqty",
    },
    "function.chunk": {"size"},
}
DEFAULT_COLOR_HEX = "FFFFFFFF"


def sha256_of_hex(bytes_hex: str) -> str:
    return hashlib.sha256(bytes.fromhex(bytes_hex)).hexdigest() if bytes_hex else ""


def build_function_dump(text: str) -> Dict[str, Any]:
    """Parse function-dump text into the JSON payload, adding byte hashes and color flags. Powered by Claude."""
    meta, sections = parse_snapshot_text(text)
    convert_int_fields(meta, sections, FUNCTION_DUMP_INT_FIELDS)

    functions = sections.get("function", [])
    for function in functions:
        if "chunk" in function:
            function["chunks"] = function.pop("chunk")
        chunks = function.get("chunks", [])
        function["bytes_hex"] = "".join(str(chunk.get("bytes", "")) for chunk in chunks)
        function["bytes_sha256"] = sha256_of_hex(function["bytes_hex"])
        function["chunk_bytes_sha256"] = [sha256_of_hex(str(chunk.get("bytes", ""))) for chunk in chunks]
        if "has_user_color" in function:
            function["has_user_color"] = bool(function["has_user_color"])
        if "item_color_hex" in function:
            function["is_default_color"] = str(function["item_color_hex"]).upper() == DEFAULT_COLOR_HEX

    return {"meta": meta, "segments": sections.get("segment", []), "functions": functions}


def read_function_dump(path: Path) -> Dict[str, Any]:
    return build_function_dump(path.read_text(encoding="utf-8", errors="replace"))


def json_path_for_dump(dump_path: Path) -> Path:
    return dump_path.with_suffix(".json")


def write_function_dump_json(dump_path: Path, json_path: Path) -> None:
    """Convert one .ida55-functions.txt into its .json. Powered by Claude."""
    write_json(json_path, read_function_dump(dump_path))
