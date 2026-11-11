#!/usr/bin/env python3
"""Write a TSV audit for expected exact procedure matches."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any, Dict, List, Optional, Sequence, Tuple


DEFAULT_MAGIC_EVIDENCE = Path("out") / "ida55-procedure-snapshots" / "MGC.procedure-evidence.json"
DEFAULT_WIZARDS_EVIDENCE = Path("out") / "ida55-procedure-snapshots" / "WZD.procedure-evidence.json"
DEFAULT_SEGMENT_MAP = Path("ida_proc_name_sync_prereq") / "MGC-vs-WZD.segment-map.json"
DEFAULT_OUTPUT = Path("ida_proc_name_sync_prereq") / "expected-match-audit.tsv"

Procedure = Dict[str, Any]
ProcedurePair = Tuple[Optional[Procedure], Optional[Procedure], str, str, int]


def parse_args(argv: Optional[Sequence[str]] = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Write an expected-match procedure audit TSV.")
    parser.add_argument("--magic-evidence", default=str(DEFAULT_MAGIC_EVIDENCE))
    parser.add_argument("--wizards-evidence", default=str(DEFAULT_WIZARDS_EVIDENCE))
    parser.add_argument("--segment-map", default=str(DEFAULT_SEGMENT_MAP))
    parser.add_argument("--output", default=str(DEFAULT_OUTPUT))
    return parser.parse_args(argv)


def load_json(path: Path) -> Dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8"))


def load_expected_segment_pairs(path: Path) -> List[Tuple[str, str]]:
    data = load_json(path)
    pairs: List[Tuple[str, str]] = []
    for pair in data.get("segment_pairs", []):
        if pair.get("pairing") != "by_position":
            continue
        if not str(pair.get("relationship", "")).startswith("same_procedure_domain"):
            continue
        pairs.append((str(pair.get("mgc_segment", "")), str(pair.get("wzd_segment", ""))))
    return pairs


def procedures_by_key(evidence: Dict[str, Any]) -> Dict[Tuple[str, int], Procedure]:
    result: Dict[Tuple[str, int], Procedure] = {}
    for procedure in evidence.get("procedures", []):
        result[(str(procedure.get("segment", "")), int(procedure.get("procedure_ordinal", 0)))] = procedure
    return result


def expected_pairs(
    magic_by_key: Dict[Tuple[str, int], Procedure],
    wizards_by_key: Dict[Tuple[str, int], Procedure],
    segment_pairs: Sequence[Tuple[str, str]],
) -> List[ProcedurePair]:
    pairs: List[ProcedurePair] = []
    for magic_segment, wizards_segment in segment_pairs:
        magic_ordinals = {ordinal for segment, ordinal in magic_by_key if segment == magic_segment}
        wizards_ordinals = {ordinal for segment, ordinal in wizards_by_key if segment == wizards_segment}
        for ordinal in sorted(magic_ordinals | wizards_ordinals):
            pairs.append(
                (
                    magic_by_key.get((magic_segment, ordinal)),
                    wizards_by_key.get((wizards_segment, ordinal)),
                    magic_segment,
                    wizards_segment,
                    ordinal,
                )
            )
    return pairs


def body(procedure: Optional[Procedure]) -> Dict[str, Any]:
    if procedure is None:
        return {}
    return dict(procedure.get("asm_body") or {})


def name(procedure: Optional[Procedure]) -> str:
    if procedure is None:
        return ""
    return str(procedure.get("name", ""))


def size(procedure: Optional[Procedure]) -> str:
    if procedure is None:
        return ""
    return str(procedure.get("size", ""))


def body_value(procedure: Optional[Procedure], field: str) -> str:
    return str(body(procedure).get(field, ""))


def match_flag(left: str, right: str) -> str:
    if not left or not right:
        return "missing"
    return "yes" if left == right else "no"


def pair_status(magic: Optional[Procedure], wizards: Optional[Procedure]) -> str:
    if magic is None:
        return "missing_magic"
    if wizards is None:
        return "missing_wizards"
    checks = [
        match_flag(size(magic), size(wizards)),
        match_flag(body_value(magic, "raw_body_sha256"), body_value(wizards, "raw_body_sha256")),
        match_flag(body_value(magic, "normalized_body_sha256"), body_value(wizards, "normalized_body_sha256")),
        match_flag(body_value(magic, "mnemonic_sequence_sha256"), body_value(wizards, "mnemonic_sequence_sha256")),
    ]
    return "match" if all(check == "yes" for check in checks) else "mismatch"


def render_tsv(pairs: Sequence[ProcedurePair]) -> str:
    lines = [
        "\t".join(
            [
                "status",
                "MAGIC",
                "WIZARDS",
                "size_match",
                "raw_match",
                "normalized_match",
                "mnemonic_match",
                "magic_bytes",
                "wizards_bytes",
                "magic_instr",
                "wizards_instr",
            ]
        )
    ]
    for magic, wizards, magic_segment, wizards_segment, ordinal in sorted(pairs, key=sort_key):
        magic_id = f"{magic_segment}p{ordinal}:{name(magic)}"
        wizards_id = f"{wizards_segment}p{ordinal}:{name(wizards)}"
        lines.append(
            "\t".join(
                [
                    pair_status(magic, wizards),
                    magic_id,
                    wizards_id,
                    match_flag(size(magic), size(wizards)),
                    match_flag(body_value(magic, "raw_body_sha256"), body_value(wizards, "raw_body_sha256")),
                    match_flag(body_value(magic, "normalized_body_sha256"), body_value(wizards, "normalized_body_sha256")),
                    match_flag(body_value(magic, "mnemonic_sequence_sha256"), body_value(wizards, "mnemonic_sequence_sha256")),
                    size(magic),
                    size(wizards),
                    body_value(magic, "instruction_count"),
                    body_value(wizards, "instruction_count"),
                ]
            )
        )
    return "\n".join(lines) + "\n"


def sort_key(pair: ProcedurePair) -> Tuple[int, str, str, int]:
    magic, wizards, magic_segment, _wizards_segment, ordinal = pair
    status_rank = 0 if pair_status(magic, wizards) != "match" else 1
    return (status_rank, magic_segment, _wizards_segment, ordinal)


def write_text(path: Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text, encoding="utf-8")


def main(argv: Optional[Sequence[str]] = None) -> int:
    args = parse_args(argv)
    magic_evidence = load_json(Path(args.magic_evidence))
    wizards_evidence = load_json(Path(args.wizards_evidence))
    pairs = expected_pairs(
        procedures_by_key(magic_evidence),
        procedures_by_key(wizards_evidence),
        load_expected_segment_pairs(Path(args.segment_map)),
    )
    output = Path(args.output)
    write_text(output, render_tsv(pairs))
    mismatch_count = sum(1 for pair in pairs if pair_status(pair[0], pair[1]) != "match")
    print(f"Wrote {output} ({len(pairs)} rows, {mismatch_count} mismatches)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
