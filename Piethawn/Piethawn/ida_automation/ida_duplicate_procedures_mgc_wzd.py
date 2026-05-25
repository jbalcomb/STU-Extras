#!/usr/bin/env python3
"""Expand duplicate-segment FWV rows into duplicate-procedure FWV rows."""

from __future__ import annotations

import argparse
import json
import re
from pathlib import Path
from typing import Any, Dict, List, Optional, Sequence, Tuple


DEFAULT_SEGMENTS = Path("ida_automation") / "ida_duplicate_segments_mgc_wzd.fwv"
DEFAULT_MAGIC_EVIDENCE = Path("out") / "ida55-procedure-snapshots" / "MGC.procedure-evidence.json"
DEFAULT_WIZARDS_EVIDENCE = Path("out") / "ida55-procedure-snapshots" / "WZD.procedure-evidence.json"
DEFAULT_OUTPUT = Path("ida_automation") / "ida_duplicate_procedures_mgc_wzd.fwv"

Procedure = Dict[str, Any]
SegmentPair = Tuple[str, str]
ProcedurePair = Tuple[Optional[Procedure], Optional[Procedure], str, str, int]
SEGMENT_NUMBER_RE = re.compile(r"^seg(\d{3})$")


def parse_args(argv: Optional[Sequence[str]] = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Expand duplicate segment pairs into procedure ordinal pairs.")
    parser.add_argument("--segments", default=str(DEFAULT_SEGMENTS))
    parser.add_argument("--magic-evidence", default=str(DEFAULT_MAGIC_EVIDENCE))
    parser.add_argument("--wizards-evidence", default=str(DEFAULT_WIZARDS_EVIDENCE))
    parser.add_argument("--output", default=str(DEFAULT_OUTPUT))
    return parser.parse_args(argv)


def read_segment_pairs(path: Path) -> List[SegmentPair]:
    lines = path.read_text(encoding="utf-8-sig").splitlines()
    pairs: List[SegmentPair] = []
    for line_number, line in enumerate(lines, 1):
        stripped = line.strip()
        if not stripped:
            continue
        columns = stripped.split()
        if line_number == 1 and columns == ["MAGIC", "WIZARDS"]:
            continue
        if len(columns) != 2:
            raise ValueError(f"{path}:{line_number}: expected two columns, got {len(columns)}")
        pairs.append((columns[0], columns[1]))
    return pairs


def load_procedures(path: Path) -> List[Procedure]:
    data = json.loads(path.read_text(encoding="utf-8"))
    procedures = data.get("procedures", [])
    if not isinstance(procedures, list):
        raise ValueError(f"{path}: procedures must be a list")
    return procedures


def procedures_by_segment(procedures: Sequence[Procedure]) -> Dict[str, List[Procedure]]:
    result: Dict[str, List[Procedure]] = {}
    for procedure in procedures:
        result.setdefault(str(procedure.get("segment", "")), []).append(procedure)
    for segment_procedures in result.values():
        segment_procedures.sort(key=lambda procedure: int(procedure.get("procedure_ordinal", 0)))
    return result


def expected_procedure_pairs(
    magic_by_segment: Dict[str, List[Procedure]],
    wizards_by_segment: Dict[str, List[Procedure]],
    segment_pairs: Sequence[SegmentPair],
) -> List[ProcedurePair]:
    pairs: List[ProcedurePair] = []
    for magic_segment, wizards_segment in segment_pairs:
        magic_procedures = magic_by_segment.get(magic_segment, [])
        wizards_procedures = wizards_by_segment.get(wizards_segment, [])
        max_count = max(len(magic_procedures), len(wizards_procedures))
        for index in range(max_count):
            ordinal = index + 1
            magic_procedure = magic_procedures[index] if index < len(magic_procedures) else None
            wizards_procedure = wizards_procedures[index] if index < len(wizards_procedures) else None
            pairs.append((magic_procedure, wizards_procedure, magic_segment, wizards_segment, ordinal))
    return pairs


def procedure_segment_id(segment: str) -> str:
    match = SEGMENT_NUMBER_RE.match(segment)
    if match is None:
        return segment
    return f"s{match.group(1)}"


def procedure_id(procedure: Optional[Procedure], segment: str, ordinal: int) -> str:
    name = "" if procedure is None else str(procedure.get("name", ""))
    segment_id = procedure_segment_id(segment)
    if not name:
        return f"{segment_id}p{ordinal:03d}:missing"
    return f"{segment_id}p{ordinal:03d}:{name}"


def render_fwv(pairs: Sequence[ProcedurePair]) -> str:
    rows = [
        (
            procedure_id(magic, magic_segment, ordinal),
            procedure_id(wizards, wizards_segment, ordinal),
        )
        for magic, wizards, magic_segment, wizards_segment, ordinal in pairs
    ]
    left_width = max([len("MAGIC")] + [len(left) for left, _right in rows])
    lines = [f"{'MAGIC':<{left_width}}  WIZARDS"]
    for left, right in rows:
        lines.append(f"{left:<{left_width}}  {right}")
    return "\n".join(lines) + "\n"


def write_text(path: Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text, encoding="utf-8")


def main(argv: Optional[Sequence[str]] = None) -> int:
    args = parse_args(argv)
    segment_pairs = read_segment_pairs(Path(args.segments))
    magic_by_segment = procedures_by_segment(load_procedures(Path(args.magic_evidence)))
    wizards_by_segment = procedures_by_segment(load_procedures(Path(args.wizards_evidence)))
    procedure_pairs = expected_procedure_pairs(magic_by_segment, wizards_by_segment, segment_pairs)
    output = Path(args.output)
    write_text(output, render_fwv(procedure_pairs))
    print(f"Wrote {output} ({len(procedure_pairs)} rows)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
