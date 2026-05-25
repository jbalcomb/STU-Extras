#!/usr/bin/env python3
"""Build procedure body hash evidence from an IDA55 ASM export."""

from __future__ import annotations

import argparse
import hashlib
import json
import re
from pathlib import Path
from typing import Any, Dict, Iterable, List, Optional


PROC_START_RE = re.compile(
    r"^\s*(?:proc\s+(?P<proc_first>[A-Za-z_@$?][\w@$?]*)|(?P<name_first>[A-Za-z_@$?][\w@$?]*)\s+proc\b)",
    re.IGNORECASE,
)
PROC_END_RE = re.compile(
    r"^\s*(?:endp\s+(?P<proc_first>[A-Za-z_@$?][\w@$?]*)|(?P<name_first>[A-Za-z_@$?][\w@$?]*)\s+endp\b)",
    re.IGNORECASE,
)
SEGMENT_START_RE = re.compile(
    r"^\s*(?:segment\s+(?P<segment_first>[A-Za-z_@$?][\w@$?]*)|(?P<name_first>[A-Za-z_@$?][\w@$?]*)\s+segment\b)",
    re.IGNORECASE,
)
SEGMENT_END_RE = re.compile(
    r"^\s*(?:ends\s+(?P<segment_first>[A-Za-z_@$?][\w@$?]*)|(?P<name_first>[A-Za-z_@$?][\w@$?]*)\s+ends\b)",
    re.IGNORECASE,
)
COMMENT_RE = re.compile(r"\s*;.*$")
WHITESPACE_RE = re.compile(r"\s+")
AUTO_LABEL_RE = re.compile(r"\b(?:loc|sub|off|byte|word|dword|unk|asc|algn)_[0-9A-Fa-f]+\b")
LOCAL_LABEL_RE = re.compile(r"@@[A-Za-z0-9_.$?]+")
HEX_RE = re.compile(r"\b(?:0x[0-9A-Fa-f]+|[0-9A-Fa-f]+h)\b")
DEC_RE = re.compile(r"\b\d+\b")


def matched_name(match: Optional[re.Match[str]]) -> Optional[str]:
    if match is None:
        return None
    groups = match.groupdict()
    return groups.get("proc_first") or groups.get("segment_first") or groups.get("name_first")


def sha256_text(text: str) -> str:
    return hashlib.sha256(text.encode("utf-8")).hexdigest()


def normalized_body_lines(lines: Iterable[str]) -> List[str]:
    output: List[str] = []
    for raw_line in lines:
        stripped = COMMENT_RE.sub("", raw_line).strip()
        if not stripped:
            continue
        if PROC_START_RE.match(stripped) or PROC_END_RE.match(stripped):
            continue
        lowered = stripped.lower()
        if lowered.startswith(("public ", "extrn ", "assume ")):
            continue
        lowered = LOCAL_LABEL_RE.sub("@@local", lowered)
        lowered = AUTO_LABEL_RE.sub("auto_label", lowered)
        lowered = HEX_RE.sub("hex", lowered)
        lowered = DEC_RE.sub("dec", lowered)
        lowered = WHITESPACE_RE.sub(" ", lowered)
        output.append(lowered)
    return output


def mnemonic_lines(lines: Iterable[str]) -> List[str]:
    output: List[str] = []
    for line in normalized_body_lines(lines):
        if line.endswith(":"):
            continue
        mnemonic = line.split(None, 1)[0]
        if mnemonic in {"db", "dw", "dd", "align", "segment", "ends"}:
            continue
        output.append(mnemonic)
    return output


def build_evidence_record(database: str, segment: str, ordinal: int, name: str, lines: List[str]) -> Dict[str, Any]:
    raw_text = "".join(lines)
    normalized = "\n".join(normalized_body_lines(lines))
    mnemonics = "\n".join(mnemonic_lines(lines))
    return {
        "database": database,
        "segment": segment,
        "procedure_ordinal": ordinal,
        "name": name,
        "raw_body_sha256": sha256_text(raw_text),
        "normalized_body_sha256": sha256_text(normalized),
        "mnemonic_sequence_sha256": sha256_text(mnemonics),
        "raw_line_count": len(raw_text.splitlines()),
        "normalized_line_count": len(normalized.splitlines()) if normalized else 0,
        "instruction_count": len(mnemonics.splitlines()) if mnemonics else 0,
    }


def build_asm_body_evidence(text: str, database: str, source_path: str, default_segment: str) -> Dict[str, Any]:
    current_segment = default_segment
    ordinals_by_segment: Dict[str, int] = {}
    procedures: List[Dict[str, Any]] = []
    active_name: Optional[str] = None
    active_segment: Optional[str] = None
    active_lines: List[str] = []

    for raw_line in text.splitlines(keepends=True):
        if active_name is None:
            segment_name = matched_name(SEGMENT_START_RE.match(raw_line))
            if segment_name:
                current_segment = segment_name
                continue
            segment_end_name = matched_name(SEGMENT_END_RE.match(raw_line))
            if segment_end_name and segment_end_name.lower() == current_segment.lower():
                current_segment = default_segment
                continue
            proc_name = matched_name(PROC_START_RE.match(raw_line))
            if proc_name:
                active_name = proc_name
                active_segment = current_segment
                active_lines = [raw_line]
                continue
        else:
            active_lines.append(raw_line)
            end_name = matched_name(PROC_END_RE.match(raw_line))
            if end_name and end_name.lower() == active_name.lower():
                segment = active_segment or default_segment
                ordinal = ordinals_by_segment.get(segment, 0) + 1
                ordinals_by_segment[segment] = ordinal
                procedures.append(build_evidence_record(database, segment, ordinal, active_name, active_lines))
                active_name = None
                active_segment = None
                active_lines = []

    if active_name is not None:
        segment = active_segment or default_segment
        ordinal = ordinals_by_segment.get(segment, 0) + 1
        procedures.append(build_evidence_record(database, segment, ordinal, active_name, active_lines))

    return {
        "schema_version": 1,
        "artifact": "asm_body_evidence",
        "database": database,
        "source_path": source_path,
        "procedure_count": len(procedures),
        "procedures": procedures,
    }


def write_json(path: Path, payload: Dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Build procedure body hash evidence from an IDA55 ASM export.")
    parser.add_argument("asm_file", type=Path, help="IDA-generated ASM export.")
    parser.add_argument("--database", required=True, help="Database label, for example MGC or WZD.")
    parser.add_argument("--output", type=Path, required=True, help="Output ASM body evidence JSON.")
    parser.add_argument("--default-segment", default="_no_segment_", help="Fallback segment name.")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    database = args.database.upper()
    text = args.asm_file.read_text(encoding="utf-8", errors="replace")
    evidence = build_asm_body_evidence(text, database, str(args.asm_file), args.default_segment)
    write_json(args.output, evidence)
    print(f"Wrote {args.output}")
    print(f"Database: {database}")
    print(f"ASM body procedures: {evidence['procedure_count']}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
