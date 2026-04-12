#!/usr/bin/env python3
"""
Convert the line-oriented IDA 5.5 function color dump into JSON.
"""

from __future__ import annotations

import argparse
import json
import re
from pathlib import Path
from typing import Dict, List


TOP_RE = re.compile(r"^(?P<key>[A-Za-z0-9_]+)=(?P<value>.*)$")
FUNCTION_RE = re.compile(r"^function\[(?P<index>\d+)\]\.(?P<field>[A-Za-z0-9_]+)=(?P<value>.*)$")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Convert IDA 5.5 function color dump text to JSON.")
    parser.add_argument("dump_file", help="Path to the .ida55-function-colors.txt dump file.")
    parser.add_argument("-o", "--output", help="Optional output JSON path. Defaults to <dump stem>.json")
    return parser.parse_args()


def maybe_int(value: str) -> object:
    text = value.strip()
    if not text:
        return ""
    try:
        return int(text)
    except ValueError:
        return text


def ensure_list_size(items: List[Dict[str, object]], index: int) -> None:
    while len(items) <= index:
        items.append({})


def parse_dump(path: Path) -> Dict[str, object]:
    meta: Dict[str, object] = {}
    functions: List[Dict[str, object]] = []
    for line in path.read_text(encoding="utf-8", errors="replace").splitlines():
        match = FUNCTION_RE.match(line)
        if match:
            index = int(match.group("index"))
            ensure_list_size(functions, index)
            functions[index][match.group("field")] = maybe_int(match.group("value"))
            continue
        match = TOP_RE.match(line)
        if match:
            meta[match.group("key")] = maybe_int(match.group("value"))

    for item in functions:
        item["has_user_color"] = bool(int(item.get("has_user_color", 0)))
        item["is_default_color"] = str(item.get("item_color_hex", "")).upper() == "FFFFFFFF"

    return {"meta": meta, "functions": functions}


def default_output_path(input_path: Path) -> Path:
    return input_path.with_suffix(".json")


def main() -> int:
    args = parse_args()
    input_path = Path(args.dump_file)
    output_path = Path(args.output) if args.output else default_output_path(input_path)
    payload = parse_dump(input_path)
    output_path.write_text(json.dumps(payload, indent=2), encoding="utf-8")
    print(f"Wrote {output_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
