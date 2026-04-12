#!/usr/bin/env python3
"""
Split an IDA Pro 5.5 ASM dump into per-segment folders and per-procedure files.

Current assumptions, based on the supplied samples:
- Procedure blocks are delimited by `proc <name> ...` / `endp <name>`
  or `<name> proc ...` / `<name> endp`.
- A contiguous comment/blank-line preamble immediately above a procedure belongs
  to that procedure.
- If explicit segment directives are present, procedures are written under that
  segment's folder. Otherwise they fall back to a default segment folder.
- Non-procedure lines within a segment are optionally written to `_misc.asm`.
"""

from __future__ import annotations

import argparse
import re
from dataclasses import dataclass, field
from pathlib import Path
from typing import Dict, Iterable, Iterator, List, Optional


PROC_START_PATTERNS = (
    re.compile(r"^\s*proc\s+(?P<name>[A-Za-z_@$?][\w@$?]*)\b", re.IGNORECASE),
    re.compile(r"^\s*(?P<name>[A-Za-z_@$?][\w@$?]*)\b.*\bproc\b", re.IGNORECASE),
)

PROC_END_PATTERNS = (
    re.compile(r"^\s*endp\s+(?P<name>[A-Za-z_@$?][\w@$?]*)\b", re.IGNORECASE),
    re.compile(r"^\s*(?P<name>[A-Za-z_@$?][\w@$?]*)\b.*\bendp\b", re.IGNORECASE),
)

SEGMENT_START_PATTERNS = (
    re.compile(r"^\s*(?P<name>[A-Za-z_@$?][\w@$?]*)\s+segment\b", re.IGNORECASE),
    re.compile(r"^\s*segment\s+(?P<name>[A-Za-z_@$?][\w@$?]*)\b", re.IGNORECASE),
)

SEGMENT_END_PATTERNS = (
    re.compile(r"^\s*(?P<name>[A-Za-z_@$?][\w@$?]*)\s+ends\b", re.IGNORECASE),
    re.compile(r"^\s*ends\s+(?P<name>[A-Za-z_@$?][\w@$?]*)\b", re.IGNORECASE),
)


@dataclass
class ProcBlock:
    name: str
    segment: str
    lines: List[str]


@dataclass
class SegmentBucket:
    name: str
    procedures: List[ProcBlock] = field(default_factory=list)
    misc_lines: List[str] = field(default_factory=list)


def sanitize_name(name: str) -> str:
    sanitized = re.sub(r'[<>:"/\\|?*]', "_", name.strip())
    return sanitized or "_unnamed_"


def detect_name(patterns: Iterable[re.Pattern[str]], line: str) -> Optional[str]:
    for pattern in patterns:
        match = pattern.match(line)
        if match:
            return match.group("name")
    return None


def is_comment_or_blank(line: str) -> bool:
    stripped = line.strip()
    return not stripped or stripped.startswith(";")


def pop_preamble(buffer: List[str]) -> List[str]:
    preamble_reversed: List[str] = []
    while buffer and is_comment_or_blank(buffer[-1]):
        preamble_reversed.append(buffer.pop())
    preamble_reversed.reverse()
    return preamble_reversed


def unique_proc_path(base_dir: Path, proc_name: str) -> Path:
    candidate = base_dir / f"{sanitize_name(proc_name)}.asm"
    if not candidate.exists():
        return candidate

    counter = 2
    while True:
        candidate = base_dir / f"{sanitize_name(proc_name)}__{counter}.asm"
        if not candidate.exists():
            return candidate
        counter += 1


def parse_asm(text: str, default_segment: str) -> Dict[str, SegmentBucket]:
    segments: Dict[str, SegmentBucket] = {}
    current_segment = default_segment
    pending_misc: List[str] = []
    active_proc_name: Optional[str] = None
    active_proc_lines: List[str] = []

    def bucket(name: str) -> SegmentBucket:
        if name not in segments:
            segments[name] = SegmentBucket(name=name)
        return segments[name]

    for raw_line in text.splitlines(keepends=True):
        segment_start = detect_name(SEGMENT_START_PATTERNS, raw_line)
        if active_proc_name is None and segment_start:
            if pending_misc:
                bucket(current_segment).misc_lines.extend(pending_misc)
                pending_misc = []
            current_segment = segment_start
            bucket(current_segment).misc_lines.append(raw_line)
            continue

        segment_end = detect_name(SEGMENT_END_PATTERNS, raw_line)
        if active_proc_name is None and segment_end:
            if pending_misc:
                bucket(current_segment).misc_lines.extend(pending_misc)
                pending_misc = []
            bucket(current_segment).misc_lines.append(raw_line)
            if segment_end.lower() == current_segment.lower():
                current_segment = default_segment
            continue

        if active_proc_name is None:
            proc_name = detect_name(PROC_START_PATTERNS, raw_line)
            if proc_name:
                preamble = pop_preamble(pending_misc)
                active_proc_name = proc_name
                active_proc_lines = [*preamble, raw_line]
                continue
            pending_misc.append(raw_line)
            continue

        active_proc_lines.append(raw_line)
        proc_end_name = detect_name(PROC_END_PATTERNS, raw_line)
        if proc_end_name and proc_end_name.lower() == active_proc_name.lower():
            bucket(current_segment).procedures.append(
                ProcBlock(name=active_proc_name, segment=current_segment, lines=active_proc_lines)
            )
            active_proc_name = None
            active_proc_lines = []

    if active_proc_name is not None:
        bucket(current_segment).procedures.append(
            ProcBlock(name=active_proc_name, segment=current_segment, lines=active_proc_lines)
        )

    if pending_misc:
        bucket(current_segment).misc_lines.extend(pending_misc)

    return segments


def write_output(
    buckets: Dict[str, SegmentBucket],
    output_dir: Path,
    write_misc: bool,
) -> None:
    output_dir.mkdir(parents=True, exist_ok=True)

    for segment_name, segment in buckets.items():
        if not segment.procedures and not (write_misc and "".join(segment.misc_lines).strip()):
            continue

        segment_dir = output_dir / sanitize_name(segment_name)
        segment_dir.mkdir(parents=True, exist_ok=True)

        for proc in segment.procedures:
            proc_path = unique_proc_path(segment_dir, proc.name)
            proc_path.write_text("".join(proc.lines), encoding="utf-8")

        if write_misc and "".join(segment.misc_lines).strip():
            misc_path = segment_dir / "_misc.asm"
            misc_path.write_text("".join(segment.misc_lines), encoding="utf-8")


def summarize(buckets: Dict[str, SegmentBucket]) -> str:
    segment_count = sum(1 for bucket in buckets.values() if bucket.procedures or "".join(bucket.misc_lines).strip())
    proc_count = sum(len(bucket.procedures) for bucket in buckets.values())
    return f"segments={segment_count} procedures={proc_count}"


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Split an IDA Pro 5.5 ASM dump into segment folders and procedure files.")
    parser.add_argument("asm_file", help="Path to the IDA-generated .asm file.")
    parser.add_argument(
        "-o",
        "--output-dir",
        help="Directory to write the split output. Defaults to out/<asm stem>.",
    )
    parser.add_argument(
        "--default-segment",
        default="_no_segment_",
        help="Folder name used when no explicit segment is active. Default: _no_segment_",
    )
    parser.add_argument(
        "--no-misc",
        action="store_true",
        help="Do not write _misc.asm files for non-procedure content.",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    asm_path = Path(args.asm_file)
    output_dir = Path(args.output_dir) if args.output_dir else Path("out") / asm_path.stem
    text = asm_path.read_text(encoding="utf-8", errors="replace")

    buckets = parse_asm(text, default_segment=args.default_segment)
    write_output(buckets, output_dir=output_dir, write_misc=not args.no_misc)
    print(f"Wrote {output_dir} ({summarize(buckets)})")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
