#!/usr/bin/env python3
"""
Extract Borland overlay metadata from a DOS EXE into JSON.

This targets the Borland overlay manager's EXEINFO block, which contains:
- a SEGTABLE_entry array
- the original executable name
- a 4-byte FILE_DATE structure
"""

from __future__ import annotations

import argparse
import json
import re
import struct
import sys
from pathlib import Path
from typing import Dict, List, Optional, Sequence, Tuple


MZ_HEADER_FORMAT = "<14H4H2H10HI"
MZ_HEADER_SIZE = struct.calcsize(MZ_HEADER_FORMAT)
SEGTABLE_ENTRY_SIZE = 8
KNOWN_SEGMENT_FLAGS = {
    0x0000: ["DATA"],
    0x0001: ["CODE"],
    0x0003: ["CODE", "OVERLAY"],
    0x0004: ["VIRTUAL"],
}
EXE_NAME_RE = re.compile(rb"([A-Za-z0-9_.-]{1,64}\.exe)\x00{1,4}", re.IGNORECASE)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Extract Borland OVRINFO metadata from DOS EXE files.")
    parser.add_argument("exe_files", nargs="+", help="One or more EXE files to inspect.")
    parser.add_argument(
        "-o",
        "--output",
        help="Optional output JSON path. Only valid when processing a single EXE.",
    )
    return parser.parse_args()


def format_hex(value: int, width: int = 4) -> str:
    return f"0x{value:0{width}X}"


def parse_mz_header(data: bytes) -> Dict[str, int]:
    if len(data) < MZ_HEADER_SIZE:
        raise ValueError("file is too small to contain a DOS MZ header")

    values = struct.unpack_from(MZ_HEADER_FORMAT, data, 0)
    header = {
        "e_magic": values[0],
        "e_cparhdr": values[4],
    }
    if header["e_magic"] != 0x5A4D:
        raise ValueError("file does not start with an MZ signature")
    return header


def is_plausible_date(day: int, month: int, year: int) -> bool:
    return 1 <= day <= 31 and 1 <= month <= 12 and 1980 <= year <= 2099


def find_exe_name_and_date(data: bytes) -> Tuple[int, str, Dict[str, object]]:
    matches: List[Tuple[int, str, Dict[str, object]]] = []
    for match in EXE_NAME_RE.finditer(data):
        name_offset = match.start(1)
        name_text = match.group(1).decode("ascii")
        name_end = match.start(1) + len(match.group(1))
        for padding in range(1, 5):
            date_offset = name_end + padding
            if date_offset + 4 > len(data):
                continue
            day = data[date_offset]
            month = data[date_offset + 1]
            year = struct.unpack_from("<H", data, date_offset + 2)[0]
            if not is_plausible_date(day, month, year):
                continue
            matches.append(
                (
                    name_offset,
                    name_text,
                    {
                        "offset": date_offset,
                        "offset_hex": format_hex(date_offset, 8),
                        "day": day,
                        "month": month,
                        "year": year,
                        "iso_date": f"{year:04d}-{month:02d}-{day:02d}",
                        "name_padding_bytes": padding,
                        "raw_bytes_hex": data[date_offset : date_offset + 4].hex().upper(),
                    },
                )
            )

    if not matches:
        raise ValueError("could not find a Borland __EXENAME__/__EXEDATE__ block")

    return max(matches, key=lambda item: item[0])


def unpack_segtable_entry(data: bytes, offset: int) -> Tuple[int, int, int, int]:
    return struct.unpack_from("<4H", data, offset)


def is_known_flag(flag: int) -> bool:
    return flag in KNOWN_SEGMENT_FLAGS


def find_segtable_start(data: bytes, exename_offset: int) -> int:
    candidates: List[int] = []
    alignment = exename_offset % SEGTABLE_ENTRY_SIZE
    for offset in range(alignment, exename_offset, SEGTABLE_ENTRY_SIZE):
        remaining = exename_offset - offset
        if remaining < SEGTABLE_ENTRY_SIZE or remaining % SEGTABLE_ENTRY_SIZE != 0:
            continue
        seg_base, seg_size, seg_flags, _ = unpack_segtable_entry(data, offset)
        if seg_base != 0:
            continue
        if seg_size == 0:
            continue
        if not is_known_flag(seg_flags):
            continue
        candidates.append(offset)

    if not candidates:
        raise ValueError("could not identify the start of the Borland SEGTABLE")

    return max(candidates)


def classify_entry_flags(flags: int) -> List[str]:
    return KNOWN_SEGMENT_FLAGS.get(flags, [f"UNKNOWN_{flags}"])


def parse_segtable_entries(data: bytes, start: int, end: int, load_module_offset: int) -> List[Dict[str, object]]:
    entries: List[Dict[str, object]] = []
    for index, offset in enumerate(range(start, end, SEGTABLE_ENTRY_SIZE)):
        seg_base, seg_size, seg_flags, min_alloc = unpack_segtable_entry(data, offset)
        file_offset = load_module_offset + seg_base * 16
        entry: Dict[str, object] = {
            "index": index,
            "record_offset": offset,
            "record_offset_hex": format_hex(offset, 8),
            "segment_base_paragraph": seg_base,
            "segment_base_paragraph_hex": format_hex(seg_base),
            "segment_base_linear": seg_base * 16,
            "segment_base_linear_hex": format_hex(seg_base * 16, 8),
            "size_paragraphs": seg_size,
            "size_paragraphs_hex": format_hex(seg_size),
            "flags": seg_flags,
            "flags_hex": format_hex(seg_flags),
            "flag_names": classify_entry_flags(seg_flags),
            "min_alloc_paragraphs": min_alloc,
            "min_alloc_paragraphs_hex": format_hex(min_alloc),
        }
        if "VIRTUAL" not in entry["flag_names"]:
            entry["file_offset"] = file_offset
            entry["file_offset_hex"] = format_hex(file_offset, 8)
            if seg_size != 0xFFFF:
                size_bytes = seg_size * 16
                entry["size_bytes"] = size_bytes
                entry["size_bytes_hex"] = format_hex(size_bytes, 8)
        entries.append(entry)
    return entries


def summarize_entries(entries: Sequence[Dict[str, object]]) -> Dict[str, object]:
    overlay_indices = [entry["index"] for entry in entries if "OVERLAY" in entry["flag_names"]]
    virtual_indices = [entry["index"] for entry in entries if "VIRTUAL" in entry["flag_names"]]
    code_indices = [entry["index"] for entry in entries if "CODE" in entry["flag_names"]]
    data_indices = [entry["index"] for entry in entries if entry["flag_names"] == ["DATA"]]
    return {
        "entry_count": len(entries),
        "overlay_entry_count": len(overlay_indices),
        "virtual_entry_count": len(virtual_indices),
        "code_entry_count": len(code_indices),
        "data_entry_count": len(data_indices),
        "overlay_entry_indices": overlay_indices,
        "virtual_entry_indices": virtual_indices,
    }


def inspect_exe(path: Path) -> Dict[str, object]:
    data = path.read_bytes()
    header = parse_mz_header(data)
    load_module_offset = header["e_cparhdr"] * 16

    exename_offset, exename, exedate = find_exe_name_and_date(data)
    segtable_start = find_segtable_start(data, exename_offset)
    entries = parse_segtable_entries(data, segtable_start, exename_offset, load_module_offset)

    return {
        "input_file": str(path),
        "file_name": path.name,
        "file_size_bytes": len(data),
        "borland_ovrinfo": {
            "segtable": {
                "offset": segtable_start,
                "offset_hex": format_hex(segtable_start, 8),
                "entry_size_bytes": SEGTABLE_ENTRY_SIZE,
                "entry_count": len(entries),
                "entries": entries,
            },
            "exe_name": {
                "value": exename,
                "offset": exename_offset,
                "offset_hex": format_hex(exename_offset, 8),
            },
            "exe_date": exedate,
            "summary": summarize_entries(entries),
        },
    }


def default_output_path(exe_path: Path) -> Path:
    return exe_path.with_name(f"{exe_path.stem}.borland-ovrinfo.json")


def write_report(report: Dict[str, object], output_path: Path) -> None:
    output_path.write_text(json.dumps(report, indent=2), encoding="utf-8")


def main() -> int:
    args = parse_args()
    if args.output and len(args.exe_files) != 1:
        print("--output may only be used with a single EXE file", file=sys.stderr)
        return 2

    for exe_name in args.exe_files:
        exe_path = Path(exe_name)
        report = inspect_exe(exe_path)
        output_path = Path(args.output) if args.output else default_output_path(exe_path)
        write_report(report, output_path)
        print(f"Wrote {output_path}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
