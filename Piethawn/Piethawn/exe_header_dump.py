#!/usr/bin/env python3
"""
Extract standard DOS MZ executable header information to JSON.

The script focuses on the canonical DOS EXE header fields and also emits a few
derived values that are useful when inspecting Borland-era executables.
"""

from __future__ import annotations

import argparse
import json
import struct
import sys
from pathlib import Path
from typing import Dict, List, Sequence


MZ_HEADER_FORMAT = "<14H4H2H10HI"
MZ_HEADER_FIELDS = (
    "e_magic",
    "e_cblp",
    "e_cp",
    "e_crlc",
    "e_cparhdr",
    "e_minalloc",
    "e_maxalloc",
    "e_ss",
    "e_sp",
    "e_csum",
    "e_ip",
    "e_cs",
    "e_lfarlc",
    "e_ovno",
    "e_res_0",
    "e_res_1",
    "e_res_2",
    "e_res_3",
    "e_oemid",
    "e_oeminfo",
    "e_res2_0",
    "e_res2_1",
    "e_res2_2",
    "e_res2_3",
    "e_res2_4",
    "e_res2_5",
    "e_res2_6",
    "e_res2_7",
    "e_res2_8",
    "e_res2_9",
    "e_lfanew",
)
MZ_HEADER_SIZE = struct.calcsize(MZ_HEADER_FORMAT)
SECONDARY_SIGNATURES = {b"PE\x00\x00": "PE", b"NE": "NE", b"LE": "LE", b"LX": "LX"}


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Extract DOS EXE header information to JSON.")
    parser.add_argument("exe_files", nargs="+", help="One or more EXE files to inspect.")
    parser.add_argument(
        "-o",
        "--output",
        help="Optional output JSON path. Only valid when processing a single EXE.",
    )
    return parser.parse_args()


def decode_magic(value: int) -> str:
    return value.to_bytes(2, "little").decode("ascii", errors="replace")


def format_hex(value: int, width: int = 4) -> str:
    return f"0x{value:0{width}X}"


def compute_file_size_from_header(e_cp: int, e_cblp: int) -> int:
    if e_cp == 0:
        return 0
    if e_cblp == 0:
        return e_cp * 512
    return (e_cp - 1) * 512 + e_cblp


def parse_secondary_header(data: bytes, offset: int) -> Dict[str, object]:
    if offset < 0 or offset >= len(data):
        return {
            "offset": offset,
            "offset_hex": format_hex(offset, 8),
            "present": False,
            "reason": "offset is outside the file",
        }

    probe = data[offset : offset + 4]
    kind = None
    for signature, name in SECONDARY_SIGNATURES.items():
        if probe.startswith(signature):
            kind = name
            break

    result: Dict[str, object] = {
        "offset": offset,
        "offset_hex": format_hex(offset, 8),
        "present": kind is not None,
        "signature_bytes_hex": probe.hex().upper(),
    }
    if kind is None:
        result["reason"] = "offset does not point to a recognized PE/NE/LE/LX header"
    else:
        result["kind"] = kind
    return result


def parse_relocations(data: bytes, table_offset: int, count: int) -> List[Dict[str, object]]:
    relocations: List[Dict[str, object]] = []
    for index in range(count):
        entry_offset = table_offset + index * 4
        if entry_offset + 4 > len(data):
            relocations.append(
                {
                    "index": index,
                    "file_offset": entry_offset,
                    "file_offset_hex": format_hex(entry_offset, 8),
                    "truncated": True,
                }
            )
            break

        reloc_offset, reloc_segment = struct.unpack_from("<HH", data, entry_offset)
        linear_address = reloc_segment * 16 + reloc_offset
        relocations.append(
            {
                "index": index,
                "file_offset": entry_offset,
                "file_offset_hex": format_hex(entry_offset, 8),
                "offset": reloc_offset,
                "offset_hex": format_hex(reloc_offset),
                "segment": reloc_segment,
                "segment_hex": format_hex(reloc_segment),
                "linear_address": linear_address,
                "linear_address_hex": format_hex(linear_address, 8),
            }
        )
    return relocations


def inspect_exe(path: Path) -> Dict[str, object]:
    data = path.read_bytes()
    if len(data) < MZ_HEADER_SIZE:
        raise ValueError(f"{path} is too small to contain a DOS EXE header")

    values = struct.unpack_from(MZ_HEADER_FORMAT, data, 0)
    header = dict(zip(MZ_HEADER_FIELDS, values))

    if decode_magic(header["e_magic"]) != "MZ":
        raise ValueError(f"{path} does not start with an MZ signature")

    header_size_bytes = header["e_cparhdr"] * 16
    header_report = {
        "magic_ascii": decode_magic(header["e_magic"]),
        "magic_hex": format_hex(header["e_magic"]),
        "bytes_in_last_page": header["e_cblp"],
        "pages_in_file": header["e_cp"],
        "relocation_count": header["e_crlc"],
        "header_paragraphs": header["e_cparhdr"],
        "minimum_extra_paragraphs": header["e_minalloc"],
        "maximum_extra_paragraphs": header["e_maxalloc"],
        "initial_ss": header["e_ss"],
        "initial_sp": header["e_sp"],
        "checksum": header["e_csum"],
        "initial_ip": header["e_ip"],
        "initial_cs": header["e_cs"],
        "relocation_table_offset": header["e_lfarlc"],
        "overlay_number": header["e_ovno"],
        "reserved_words": [header[f"e_res_{index}"] for index in range(4)],
        "oem_id": header["e_oemid"],
        "oem_info": header["e_oeminfo"],
        "reserved2_words": [header[f"e_res2_{index}"] for index in range(10)],
        "new_header_offset": header["e_lfanew"],
        "raw_fields": {
            name: {
                "value": value,
                "hex": format_hex(value, 8 if name == "e_lfanew" else 4),
            }
            for name, value in header.items()
        },
    }

    file_size_from_header = compute_file_size_from_header(header["e_cp"], header["e_cblp"])
    load_module_offset = header_size_bytes
    load_module_size = max(0, len(data) - load_module_offset)
    relocations = parse_relocations(data, header["e_lfarlc"], header["e_crlc"])

    return {
        "input_file": str(path),
        "file_name": path.name,
        "file_size_bytes": len(data),
        "dos_mz_header": header_report,
        "derived": {
            "header_size_bytes": header_size_bytes,
            "header_size_hex": format_hex(header_size_bytes, 8),
            "file_size_from_header_bytes": file_size_from_header,
            "file_size_from_header_hex": format_hex(file_size_from_header, 8),
            "file_size_matches_header": len(data) == file_size_from_header,
            "load_module_offset": load_module_offset,
            "load_module_offset_hex": format_hex(load_module_offset, 8),
            "load_module_size_bytes": load_module_size,
            "load_module_size_hex": format_hex(load_module_size, 8),
            "entry_point_relative_offset": header["e_cs"] * 16 + header["e_ip"],
            "entry_point_relative_offset_hex": format_hex(header["e_cs"] * 16 + header["e_ip"], 8),
            "stack_pointer_linear": header["e_ss"] * 16 + header["e_sp"],
            "stack_pointer_linear_hex": format_hex(header["e_ss"] * 16 + header["e_sp"], 8),
        },
        "secondary_header_probe": parse_secondary_header(data, header["e_lfanew"]),
        "relocation_table": relocations,
    }


def default_output_path(exe_path: Path) -> Path:
    return exe_path.with_name(f"{exe_path.stem}.exe-header.json")


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
