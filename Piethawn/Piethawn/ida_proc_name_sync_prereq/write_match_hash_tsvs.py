#!/usr/bin/env python3
"""Write simple TSVs for by-position procedure hash matches in known segment pairs."""

from __future__ import annotations

import argparse
import json
from itertools import zip_longest
from pathlib import Path
from typing import Dict, List, Optional, Sequence, Tuple


DEFAULT_MAGIC_FUNCTIONS = Path("in") / "MAGIC.idb.ida55-functions.json"
DEFAULT_WIZARDS_FUNCTIONS = Path("in") / "WIZARDS.idb.ida55-functions.json"
DEFAULT_SEGMENT_MAP = Path("ida_proc_name_sync_prereq") / "MGC-vs-WZD.segment-map.json"
DEFAULT_YAY = Path("ida_proc_name_sync_prereq") / "match-hash-yay.tsv"
DEFAULT_NAY = Path("ida_proc_name_sync_prereq") / "match-hash-nay.tsv"

Function = Dict[str, object]
ProcedurePair = Tuple[Optional[Function], Optional[Function]]


def parse_args(argv: Optional[Sequence[str]] = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Write simple hash-match TSVs for known MGC/WZD segment pairs.")
    parser.add_argument("--magic-functions", default=str(DEFAULT_MAGIC_FUNCTIONS))
    parser.add_argument("--wizards-functions", default=str(DEFAULT_WIZARDS_FUNCTIONS))
    parser.add_argument("--segment-map", default=str(DEFAULT_SEGMENT_MAP))
    parser.add_argument("--yay", default=str(DEFAULT_YAY), help="TSV for paired procedures with matching byte hashes.")
    parser.add_argument("--nay", default=str(DEFAULT_NAY), help="TSV for paired procedures with different byte hashes or missing counterparts.")
    return parser.parse_args(argv)


def load_functions(path: Path) -> List[Function]:
    data = json.loads(path.read_text(encoding="utf-8"))
    functions = data.get("functions", [])
    if not isinstance(functions, list):
        raise ValueError(f"functions must be a list in {path}")
    return functions


def functions_by_segment(functions: Sequence[Function]) -> Dict[str, List[Function]]:
    result: Dict[str, List[Function]] = {}
    for function in functions:
        result.setdefault(str(function.get("segment", "")), []).append(function)
    return result


def load_segment_pairs(path: Path) -> List[Dict[str, object]]:
    data = json.loads(path.read_text(encoding="utf-8"))
    pairs = data.get("segment_pairs", [])
    if not isinstance(pairs, list):
        raise ValueError(f"segment_pairs must be a list in {path}")
    return [pair for pair in pairs if pair.get("pairing") == "by_position" and str(pair.get("relationship", "")).startswith("same_procedure_domain")]


def procedure_name(function: Optional[Function]) -> str:
    if function is None:
        return ""
    return str(function.get("name", ""))


def procedure_hash(function: Optional[Function]) -> str:
    if function is None:
        return ""
    return str(function.get("bytes_sha256", ""))


def pair_functions_by_known_segments(
    magic_by_segment: Dict[str, List[Function]],
    wizards_by_segment: Dict[str, List[Function]],
    segment_pairs: Sequence[Dict[str, object]],
) -> List[ProcedurePair]:
    pairs: List[ProcedurePair] = []
    for segment_pair in segment_pairs:
        magic_segment = str(segment_pair.get("mgc_segment", ""))
        wizards_segment = str(segment_pair.get("wzd_segment", ""))
        magic_functions = magic_by_segment.get(magic_segment, [])
        wizards_functions = wizards_by_segment.get(wizards_segment, [])
        pairs.extend(zip_longest(magic_functions, wizards_functions))
    return pairs


def split_pairs_by_hash(pairs: Sequence[ProcedurePair]) -> Tuple[List[ProcedurePair], List[ProcedurePair]]:
    yay: List[ProcedurePair] = []
    nay: List[ProcedurePair] = []
    for magic_function, wizards_function in pairs:
        if procedure_hash(magic_function) and procedure_hash(magic_function) == procedure_hash(wizards_function):
            yay.append((magic_function, wizards_function))
        else:
            nay.append((magic_function, wizards_function))
    return yay, nay


def render_tsv(pairs: Sequence[ProcedurePair]) -> str:
    lines = ["MAGIC\tWIZARDS"]
    for magic_function, wizards_function in pairs:
        lines.append(f"{procedure_name(magic_function)}\t{procedure_name(wizards_function)}")
    return "\n".join(lines) + "\n"


def write_text(path: Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text, encoding="utf-8")


def main(argv: Optional[Sequence[str]] = None) -> int:
    args = parse_args(argv)
    magic_functions = load_functions(Path(args.magic_functions))
    wizards_functions = load_functions(Path(args.wizards_functions))
    segment_pairs = load_segment_pairs(Path(args.segment_map))
    pairs = pair_functions_by_known_segments(functions_by_segment(magic_functions), functions_by_segment(wizards_functions), segment_pairs)
    yay, nay = split_pairs_by_hash(pairs)

    yay_path = Path(args.yay)
    nay_path = Path(args.nay)
    write_text(yay_path, render_tsv(yay))
    write_text(nay_path, render_tsv(nay))

    print(f"Wrote {yay_path} ({len(yay)} rows)")
    print(f"Wrote {nay_path} ({len(nay)} rows)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
