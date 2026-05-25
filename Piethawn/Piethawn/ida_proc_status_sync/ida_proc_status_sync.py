#!/usr/bin/env python3
"""Compatibility entrypoint: build a status plan, then generate status IDC scripts."""

from __future__ import annotations

import argparse
import sys
from pathlib import Path
from typing import Optional, Sequence

if __package__ in (None, ""):
    sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from ida_proc_status_sync.generate_status_idc import write_status_idc
from ida_proc_status_sync.status_plan import (
    DEFAULT_COMPLETED_COLOR_HEX,
    DEFAULT_OUTPUT,
    build_status_plan_from_files,
    write_json,
)


def parse_args(argv: Optional[Sequence[str]] = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Build completion-status sync plan and IDC scripts.")
    parser.add_argument("--magic-functions", default=str(Path("in") / "MAGIC.idb.ida55-functions.json"))
    parser.add_argument("--wizards-functions", default=str(Path("in") / "WIZARDS.idb.ida55-functions.json"))
    parser.add_argument("--completed-color-hex", default=DEFAULT_COMPLETED_COLOR_HEX)
    parser.add_argument("--output", default=str(DEFAULT_OUTPUT))
    parser.add_argument("--magic-idc", default=str(Path("ida_proc_status_sync") / "mgc_proc_status_sync.idc"))
    parser.add_argument("--wizards-idc", default=str(Path("ida_proc_status_sync") / "wzd_proc_status_sync.idc"))
    return parser.parse_args(argv)


def main(argv: Optional[Sequence[str]] = None) -> int:
    args = parse_args(argv)
    plan = build_status_plan_from_files(
        Path(args.magic_functions),
        Path(args.wizards_functions),
        args.completed_color_hex,
    )
    output_path = Path(args.output)
    write_json(output_path, plan)
    write_status_idc(plan, Path(args.magic_idc), Path(args.wizards_idc))
    print(f"Wrote {output_path}")
    print(f"Wrote {args.magic_idc}")
    print(f"Wrote {args.wizards_idc}")
    print(f"Completion color: {plan['summary']['completed_color_hex']}")
    print(f"Join key: {plan['summary']['join_key']}")
    print(f"MAGIC updates: {plan['summary']['sync_update_count_by_game']['MAGIC']}")
    print(f"WIZARDS updates: {plan['summary']['sync_update_count_by_game']['WIZARDS']}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
