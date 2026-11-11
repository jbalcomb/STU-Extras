#!/usr/bin/env python3
"""
Build the WZD -> MGC rename IDC from the IDA 5.5 dumps already in in/ (no IDA run).

  1. ida_proc_data_export/ida55_functions_to_inventory.py   -> out/ida55-procedure-snapshots/{MGC,WZD}.procedure-inventory.json
  2. ida55_asm_body_evidence.py                              -> {MGC,WZD}.asm-body-evidence.json
  3. ida55_join_procedure_evidence.py                        -> {MGC,WZD}.procedure-evidence.json
  4. ida_proc_name_sync_prereq/compare_procedure_evidence.py -> ida_proc_name_sync_prereq/MGC-vs-WZD.procedure-evidence-comparison.json
  5. ida_proc_name_sync_prereq/build_magic_name_sync_handoff.py -> ida_proc_name_sync/magic_sync_names_from_wzd.json
  6. ida_proc_name_sync/generate_magic_name_sync.py          -> ida_proc_name_sync/magic_sync_names_from_wzd.idc

Refresh in/ first with run_ida55_dump_and_split.py. Apply the IDC to MAGIC.idb in the IDA GUI.
"""

from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path
from typing import List, Sequence

REPO_ROOT = Path(__file__).resolve().parent
SEGMENT_MAP = Path("ida_proc_name_sync_prereq") / "MGC-vs-WZD.segment-map.json"
COMPARISON = Path("ida_proc_name_sync_prereq") / "MGC-vs-WZD.procedure-evidence-comparison.json"
HANDOFF = Path("ida_proc_name_sync") / "magic_sync_names_from_wzd.json"
RENAME_IDC = Path("ida_proc_name_sync") / "magic_sync_names_from_wzd.idc"
SNAPSHOTS = Path("out") / "ida55-procedure-snapshots"
DATABASES = (("MGC", "MAGIC"), ("WZD", "WIZARDS"))


def parse_args(argv: Sequence[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Build the WZD -> MGC rename IDC from the dumps in in/.")
    parser.add_argument("--dry-run", action="store_true", help="Print the steps without running them.")
    parser.add_argument("--python", default=sys.executable, help="Python interpreter for child scripts. Default: current interpreter")
    return parser.parse_args(argv)


def required_inputs() -> List[Path]:
    paths = [SEGMENT_MAP]
    for _, game in DATABASES:
        paths += [Path("in") / f"{game}.asm", Path("in") / f"{game}.idb.ida55-functions.txt"]
    return paths


def rename_chain_commands(python: str) -> List[List[str]]:
    """The six chain steps in order; every input and output path is explicit. Powered by Claude."""
    commands: List[List[str]] = []
    for database, game in DATABASES:
        inventory = SNAPSHOTS / f"{database}.procedure-inventory.json"
        body = SNAPSHOTS / f"{database}.asm-body-evidence.json"
        evidence = SNAPSHOTS / f"{database}.procedure-evidence.json"
        commands += [
            [python, "-B", str(Path("ida_proc_data_export") / "ida55_functions_to_inventory.py"),
             str(Path("in") / f"{game}.idb.ida55-functions.txt"), "--database", database, "--output", str(inventory)],
            [python, "-B", "ida55_asm_body_evidence.py", str(Path("in") / f"{game}.asm"), "--database", database, "--output", str(body)],
            [python, "-B", "ida55_join_procedure_evidence.py", "--inventory", str(inventory), "--asm-body-evidence", str(body), "--output", str(evidence)],
        ]
    commands += [
        [python, "-B", str(Path("ida_proc_name_sync_prereq") / "compare_procedure_evidence.py"),
         "--left", str(SNAPSHOTS / "MGC.procedure-evidence.json"), "--right", str(SNAPSHOTS / "WZD.procedure-evidence.json"),
         "--segment-map", str(SEGMENT_MAP), "--output", str(COMPARISON)],
        [python, "-B", str(Path("ida_proc_name_sync_prereq") / "build_magic_name_sync_handoff.py"), "--comparison", str(COMPARISON), "--output", str(HANDOFF)],
        [python, "-B", str(Path("ida_proc_name_sync") / "generate_magic_name_sync.py"), "--renames", str(HANDOFF), "--output-idc", str(RENAME_IDC)],
    ]
    return commands


def main(argv: Sequence[str] | None = None) -> int:
    args = parse_args(argv)
    if not args.dry_run:
        missing = [str(path) for path in required_inputs() if not (REPO_ROOT / path).is_file()]
        if missing:
            raise FileNotFoundError("Missing inputs: " + ", ".join(missing))

    for command in rename_chain_commands(args.python):
        print("+", " ".join(command), flush=True)
        if not args.dry_run:
            subprocess.run(command, cwd=REPO_ROOT, check=True)

    print(f"Rename IDC: {RENAME_IDC}" if not args.dry_run else "Dry run complete.")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except subprocess.CalledProcessError as error:
        print(f"ERROR: step failed with exit code {error.returncode}: {' '.join(map(str, error.cmd))}", file=sys.stderr)
        raise SystemExit(error.returncode)
