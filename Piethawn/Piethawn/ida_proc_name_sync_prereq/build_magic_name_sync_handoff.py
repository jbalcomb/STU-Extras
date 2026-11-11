#!/usr/bin/env python3
"""Build the minimal MAGIC procedure-name sync handoff from comparison evidence."""

from __future__ import annotations

import argparse
import sys
from pathlib import Path
from typing import Any, Dict, List, Tuple

if __package__ in (None, ""):
    sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from piethawn_sync.jsonio import load_json, write_json


DEFAULT_COMPARISON = Path("ida_proc_name_sync_prereq/MGC-vs-WZD.procedure-evidence-comparison.json")
DEFAULT_OUTPUT = Path("ida_proc_name_sync/magic_sync_names_from_wzd.json")


def collect_minimal_renames(comparison: Dict[str, Any]) -> List[Dict[str, str]]:
    rows: List[Tuple[Tuple[str, int, str, str], Dict[str, str]]] = []
    seen: Dict[str, str] = {}
    for pair in comparison.get("procedure_pairs", []):
        left = pair.get("left", {})
        right = pair.get("right", {})
        old_name = left.get("name")
        new_name = right.get("name")
        if not old_name or not new_name or old_name == new_name:
            continue
        if old_name in seen:
            if seen[old_name] != new_name:
                raise ValueError(f"Conflicting rename targets for {old_name}: {seen[old_name]} and {new_name}")
            continue
        seen[old_name] = new_name
        rows.append(
            (
                (
                    str(left.get("segment", "")),
                    int(pair.get("procedure_ordinal", 0)),
                    str(old_name),
                    str(new_name),
                ),
                {"old_name": str(old_name), "new_name": str(new_name)},
            )
        )
    return [rename for _, rename in sorted(rows)]


def build_handoff(comparison: Dict[str, Any]) -> Dict[str, Any]:
    return {
        "schema_version": 1,
        "artifact": "magic_proc_name_sync_handoff",
        "renames": collect_minimal_renames(comparison),
    }


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Build minimal MAGIC procedure-name sync handoff JSON.")
    parser.add_argument("--comparison", type=Path, default=DEFAULT_COMPARISON, help="Procedure evidence comparison JSON.")
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT, help="Output minimal rename handoff JSON.")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    print(f"Input comparison: {args.comparison}")
    handoff = build_handoff(load_json(args.comparison))
    write_json(args.output, handoff)
    print(f"Wrote {args.output}")
    print(f"Rename count: {len(handoff['renames'])}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
