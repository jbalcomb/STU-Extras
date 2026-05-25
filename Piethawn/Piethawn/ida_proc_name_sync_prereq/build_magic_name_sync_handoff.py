#!/usr/bin/env python3
"""Build the minimal MAGIC procedure-name sync handoff from comparison evidence."""

from __future__ import annotations

import argparse
import json
import shutil
from pathlib import Path
from typing import Any, Dict, List, Tuple


DEFAULT_COMPARISON = Path("ida_proc_name_sync_prereq/MGC-vs-WZD.procedure-evidence-comparison.json")
DEFAULT_OUTPUT = Path("ida_proc_name_sync_prereq/magic_sync_names_from_wzd.json")
DEFAULT_FINAL_COPY = Path("ida_proc_name_sync/magic_sync_names_from_wzd.json")


def load_json(path: Path) -> Dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8"))


def write_json(path: Path, payload: Dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")


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
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT, help="Output prereq minimal rename handoff JSON.")
    parser.add_argument("--final-copy", type=Path, default=DEFAULT_FINAL_COPY, help="Copy of the handoff for operational scripts.")
    parser.add_argument("--no-final-copy", action="store_true", help="Do not copy the handoff to the operational directory.")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    handoff = build_handoff(load_json(args.comparison))
    write_json(args.output, handoff)
    print(f"Wrote {args.output}")
    if not args.no_final_copy:
        args.final_copy.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(args.output, args.final_copy)
        print(f"Copied {args.final_copy}")
    print(f"Rename count: {len(handoff['renames'])}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
