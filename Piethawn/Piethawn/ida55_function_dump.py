#!/usr/bin/env python3
"""
Convert the line-oriented IDA 5.5 function dump into JSON with byte hashes.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import re
from pathlib import Path
from typing import Dict, List


TOP_RE = re.compile(r"^(?P<key>[A-Za-z0-9_]+)=(?P<value>.*)$")
SEGMENT_RE = re.compile(r"^segment\[(?P<index>\d+)\]\.(?P<field>[A-Za-z0-9_]+)=(?P<value>.*)$")
FUNCTION_RE = re.compile(r"^function\[(?P<findex>\d+)\]\.(?P<field>[A-Za-z0-9_]+)=(?P<value>.*)$")
CHUNK_RE = re.compile(
    r"^function\[(?P<findex>\d+)\]\.chunk\[(?P<cindex>\d+)\]\.(?P<field>[A-Za-z0-9_]+)=(?P<value>.*)$"
)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Convert IDA 5.5 function dump text to JSON with byte hashes.")
    parser.add_argument("dump_file", help="Path to the .ida55-functions.txt dump file.")
    parser.add_argument(
        "-o",
        "--output",
        help="Optional output JSON path. Defaults to <dump stem>.json",
    )
    return parser.parse_args()


def ensure_list_size(items: List[Dict[str, object]], index: int) -> None:
    while len(items) <= index:
        items.append({})


def maybe_int(value: str) -> object:
    text = value.strip()
    if not text:
        return ""
    if text.startswith("0x") or text.endswith("h") or text.endswith("H"):
        try:
            if text.startswith("0x"):
                return int(text, 16)
            return int(text[:-1], 16)
        except ValueError:
            return text
    try:
        return int(text)
    except ValueError:
        return text


def parse_dump(path: Path) -> Dict[str, object]:
    meta: Dict[str, object] = {}
    segments: List[Dict[str, object]] = []
    functions: List[Dict[str, object]] = []

    for line in path.read_text(encoding="utf-8", errors="replace").splitlines():
        match = CHUNK_RE.match(line)
        if match:
            findex = int(match.group("findex"))
            cindex = int(match.group("cindex"))
            ensure_list_size(functions, findex)
            function = functions[findex]
            chunks = function.setdefault("chunks", [])
            assert isinstance(chunks, list)
            ensure_list_size(chunks, cindex)
            chunks[cindex][match.group("field")] = maybe_int(match.group("value"))
            continue

        match = FUNCTION_RE.match(line)
        if match:
            findex = int(match.group("findex"))
            ensure_list_size(functions, findex)
            functions[findex][match.group("field")] = maybe_int(match.group("value"))
            continue

        match = SEGMENT_RE.match(line)
        if match:
            sindex = int(match.group("index"))
            ensure_list_size(segments, sindex)
            segments[sindex][match.group("field")] = maybe_int(match.group("value"))
            continue

        match = TOP_RE.match(line)
        if match:
            meta[match.group("key")] = maybe_int(match.group("value"))

    for function in functions:
        chunks = function.get("chunks", [])
        if not isinstance(chunks, list):
            continue
        all_bytes_hex = "".join(str(chunk.get("bytes", "")) for chunk in chunks)
        function["bytes_hex"] = all_bytes_hex
        function["bytes_sha256"] = hashlib.sha256(bytes.fromhex(all_bytes_hex)).hexdigest() if all_bytes_hex else ""
        function["chunk_bytes_sha256"] = [
            hashlib.sha256(bytes.fromhex(str(chunk.get("bytes", "")))).hexdigest() if chunk.get("bytes") else ""
            for chunk in chunks
        ]

    return {"meta": meta, "segments": segments, "functions": functions}


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
