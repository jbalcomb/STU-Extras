#!/usr/bin/env python3
"""
Show machine-code and assembly differences for a pair of functions.
"""

from __future__ import annotations

import argparse
import json
from dataclasses import dataclass
from difflib import unified_diff
from pathlib import Path
from typing import Dict, Iterable, List, Sequence, Tuple

from asm_proc_tools import load_proc_record, normalize_asm, unified_normalized_diff


@dataclass
class FunctionRef:
    game: str
    segment: str
    name: str
    asm_path: Path
    dump_path: Path


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Compare a pair of functions by bytes and ASM.")
    parser.add_argument("--left-game", required=True, choices=("MAGIC", "WIZARDS"))
    parser.add_argument("--left-segment", required=True)
    parser.add_argument("--left-name", required=True)
    parser.add_argument("--left-asm", required=True)
    parser.add_argument("--right-game", required=True, choices=("MAGIC", "WIZARDS"))
    parser.add_argument("--right-segment", required=True)
    parser.add_argument("--right-name", required=True)
    parser.add_argument("--right-asm", required=True)
    parser.add_argument("--magic-dump", default="MAGIC.idb.ida55-functions.json")
    parser.add_argument("--wizards-dump", default="WIZARDS.idb.ida55-functions.json")
    parser.add_argument("--output", help="Optional report file path")
    parser.add_argument("--context", type=int, default=3, help="Diff context lines. Default: 3")
    parser.add_argument("--byte-window", type=int, default=16, help="Hex window size around differing bytes. Default: 16")
    return parser.parse_args()


def load_dump(path: Path) -> List[Dict[str, object]]:
    return json.loads(path.read_text(encoding="utf-8")).get("functions", [])


def find_function(functions: Sequence[Dict[str, object]], segment: str, name: str) -> Dict[str, object]:
    matches = [item for item in functions if item.get("segment") == segment and item.get("name") == name]
    if len(matches) != 1:
        raise ValueError(f"Expected exactly one function for {segment}:{name}, found {len(matches)}")
    return matches[0]


def diff_offsets(left_bytes: bytes, right_bytes: bytes) -> List[int]:
    limit = min(len(left_bytes), len(right_bytes))
    offsets = [index for index in range(limit) if left_bytes[index] != right_bytes[index]]
    offsets.extend(range(limit, max(len(left_bytes), len(right_bytes))))
    return offsets


def collapse_offsets(offsets: Sequence[int]) -> List[Tuple[int, int]]:
    if not offsets:
        return []
    ranges: List[Tuple[int, int]] = []
    start = prev = offsets[0]
    for current in offsets[1:]:
        if current == prev + 1:
            prev = current
            continue
        ranges.append((start, prev + 1))
        start = prev = current
    ranges.append((start, prev + 1))
    return ranges


def hex_window(data: bytes, start: int, end: int, radius: int) -> str:
    lo = max(0, start - radius)
    hi = min(len(data), end + radius)
    rows: List[str] = []
    for offset in range(lo, hi, 16):
        chunk = data[offset : min(hi, offset + 16)]
        bytes_hex = " ".join(f"{value:02X}" for value in chunk)
        rows.append(f"{offset:04X}: {bytes_hex}")
    return "\n".join(rows)


def raw_asm_diff(left_text: str, right_text: str, left_label: str, right_label: str, context: int) -> str:
    return "".join(
        unified_diff(
            left_text.splitlines(keepends=True),
            right_text.splitlines(keepends=True),
            fromfile=left_label,
            tofile=right_label,
            n=context,
        )
    )


def render_report(
    left_ref: FunctionRef,
    right_ref: FunctionRef,
    left_func: Dict[str, object],
    right_func: Dict[str, object],
    left_proc,
    right_proc,
    context: int,
    byte_window_size: int,
) -> str:
    left_bytes = bytes.fromhex(str(left_func.get("bytes_hex", "")))
    right_bytes = bytes.fromhex(str(right_func.get("bytes_hex", "")))
    offsets = diff_offsets(left_bytes, right_bytes)
    ranges = collapse_offsets(offsets)

    lines: List[str] = []
    lines.append(f"# Function Pair Diff: {left_ref.name}")
    lines.append("")
    lines.append("## Summary")
    lines.append(f"- Left: {left_ref.game} {left_ref.segment}:{left_ref.name} {left_func.get('start_ea')}..{left_func.get('end_ea')}")
    lines.append(f"- Right: {right_ref.game} {right_ref.segment}:{right_ref.name} {right_func.get('start_ea')}..{right_func.get('end_ea')}")
    lines.append(f"- Left bytes_sha256: {left_func.get('bytes_sha256')}")
    lines.append(f"- Right bytes_sha256: {right_func.get('bytes_sha256')}")
    lines.append(f"- Left size: {len(left_bytes)}")
    lines.append(f"- Right size: {len(right_bytes)}")
    lines.append(f"- Exact byte match: {'yes' if left_func.get('bytes_sha256') == right_func.get('bytes_sha256') else 'no'}")
    lines.append(f"- Differing byte positions: {len(offsets)}")
    lines.append(f"- Differing byte ranges: {len(ranges)}")
    lines.append("")

    lines.append("## Byte Diff Ranges")
    if not ranges:
        lines.append("- none")
    else:
        for index, (start, end) in enumerate(ranges[:12]):
            lines.append(f"- range {index + 1}: 0x{start:04X}..0x{end - 1:04X} ({end - start} bytes)")
    lines.append("")

    lines.append("## Byte Windows")
    if not ranges:
        lines.append("No differing bytes.")
    else:
        for index, (start, end) in enumerate(ranges[:4]):
            lines.append(f"### Range {index + 1}")
            lines.append("Left")
            lines.append("```text")
            lines.append(hex_window(left_bytes, start, end, byte_window_size))
            lines.append("```")
            lines.append("Right")
            lines.append("```text")
            lines.append(hex_window(right_bytes, start, end, byte_window_size))
            lines.append("```")

    lines.append("")
    lines.append("## Raw ASM Diff")
    lines.append("```diff")
    lines.append(
        raw_asm_diff(
            left_proc.raw_text,
            right_proc.raw_text,
            str(left_ref.asm_path).replace("\\", "/"),
            str(right_ref.asm_path).replace("\\", "/"),
            context,
        )[:40000]
    )
    lines.append("```")

    lines.append("")
    lines.append("## Normalized ASM Diff")
    lines.append("```diff")
    lines.append(unified_normalized_diff(left_proc, right_proc, context=context)[:40000])
    lines.append("```")
    return "\n".join(lines)


def main() -> int:
    args = parse_args()
    left_ref = FunctionRef(
        game=args.left_game,
        segment=args.left_segment,
        name=args.left_name,
        asm_path=Path(args.left_asm),
        dump_path=Path(args.magic_dump if args.left_game == "MAGIC" else args.wizards_dump),
    )
    right_ref = FunctionRef(
        game=args.right_game,
        segment=args.right_segment,
        name=args.right_name,
        asm_path=Path(args.right_asm),
        dump_path=Path(args.magic_dump if args.right_game == "MAGIC" else args.wizards_dump),
    )

    left_func = find_function(load_dump(left_ref.dump_path), left_ref.segment, left_ref.name)
    right_func = find_function(load_dump(right_ref.dump_path), right_ref.segment, right_ref.name)

    root = Path("out")
    left_proc = load_proc_record(root, left_ref.game, left_ref.segment, left_ref.asm_path)
    right_proc = load_proc_record(root, right_ref.game, right_ref.segment, right_ref.asm_path)

    report = render_report(left_ref, right_ref, left_func, right_func, left_proc, right_proc, args.context, args.byte_window)

    if args.output:
        output_path = Path(args.output)
        output_path.parent.mkdir(parents=True, exist_ok=True)
        output_path.write_text(report, encoding="utf-8")
        print(f"Wrote {output_path}")
    else:
        print(report)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
