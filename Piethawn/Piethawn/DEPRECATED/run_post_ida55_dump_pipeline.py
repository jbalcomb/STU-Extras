#!/usr/bin/env python3
"""
Run the Piethawn post-IDA55 pipeline from manually dumped ASM/function text to
the generated MAGIC rename IDC script.

Name sync uses the evidence-based chain: procedure inventories are derived from
the function dumps (no separate IDA procedure snapshot), joined with ASM body
hash evidence, compared against the approved MGC/WZD segment map, and reduced to
a minimal rename handoff that generates ida_proc_name_sync/magic_sync_names_from_wzd.idc.
"""

from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path
from typing import Iterable, List, Sequence


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Run the post-IDA55 pipeline from manual dumps through MAGIC rename IDC generation."
    )
    parser.add_argument("--magic-asm", default=str(Path("in") / "MAGIC.asm"), help="MAGIC ASM dump from IDA Pro 5.5")
    parser.add_argument("--wizards-asm", default=str(Path("in") / "WIZARDS.asm"), help="WIZARDS ASM dump from IDA Pro 5.5")
    parser.add_argument(
        "--magic-functions-txt",
        default=str(Path("in") / "MAGIC.idb.ida55-functions.txt"),
        help="MAGIC function dump text produced by ida_proc_data_export/ida55_dump_functions.idc",
    )
    parser.add_argument(
        "--wizards-functions-txt",
        default=str(Path("in") / "WIZARDS.idb.ida55-functions.txt"),
        help="WIZARDS function dump text produced by ida_proc_data_export/ida55_dump_functions.idc",
    )
    parser.add_argument("--magic-exe", default=str(Path("in") / "MAGIC.EXE"), help="MAGIC executable")
    parser.add_argument("--wizards-exe", default=str(Path("in") / "WIZARDS.EXE"), help="WIZARDS executable")
    parser.add_argument("--output-root", default="out", help="Output root for generated artifacts")
    parser.add_argument(
        "--skip-exe-metadata",
        action="store_true",
        help="Skip exe_header_dump.py and borland_ovrinfo_dump.py",
    )
    parser.add_argument(
        "--skip-asm-analysis",
        action="store_true",
        help="Skip asm_dedupe_index.py and asm_diff_report.py",
    )
    parser.add_argument(
        "--skip-clean-asm-check",
        action="store_true",
        help="Skip ida55_check_clean_asm.py before splitting ASM files",
    )
    parser.add_argument(
        "--skip-clear-split-asm",
        action="store_true",
        help="Do not remove existing split .asm files under output-root before re-splitting",
    )
    parser.add_argument(
        "--python",
        default=sys.executable,
        help="Python interpreter to use for child scripts. Default: current interpreter",
    )
    return parser.parse_args()


def require_files(paths: Iterable[Path]) -> None:
    missing = [str(path) for path in paths if not path.exists()]
    if missing:
        raise FileNotFoundError("Missing required inputs:\n" + "\n".join(f"- {item}" for item in missing))


def resolve_input_path(raw_path: str) -> Path:
    path = Path(raw_path)
    if path.exists():
        return path
    if len(path.parts) >= 2 and path.parts[0].lower() == "in":
        fallback = Path(*path.parts[1:])
        if fallback.exists():
            return fallback
    return path


def run_step(command: Sequence[str]) -> None:
    print("+", " ".join(command))
    subprocess.run(command, check=True)


def split_asm_paths(output_root: Path) -> List[Path]:
    return sorted(path for path in output_root.glob("*/*/*.asm") if path.is_file())


def clear_split_asm_files(output_root: Path) -> int:
    paths = split_asm_paths(output_root)
    for path in paths:
        path.unlink()
    return len(paths)


def split_output_dir(output_root: Path, game: str) -> Path:
    return output_root / game


def main() -> int:
    args = parse_args()
    python = args.python
    output_root = Path(args.output_root)
    magic_asm = resolve_input_path(args.magic_asm)
    wizards_asm = resolve_input_path(args.wizards_asm)
    magic_functions_txt = resolve_input_path(args.magic_functions_txt)
    wizards_functions_txt = resolve_input_path(args.wizards_functions_txt)
    magic_exe = resolve_input_path(args.magic_exe)
    wizards_exe = resolve_input_path(args.wizards_exe)

    require_files([magic_asm, wizards_asm, magic_functions_txt, wizards_functions_txt])
    require_files([Path("ida_proc_name_sync_prereq") / "MGC-vs-WZD.segment-map.json"])
    if not args.skip_exe_metadata:
        require_files([magic_exe, wizards_exe])

    output_root.mkdir(parents=True, exist_ok=True)

    if not args.skip_exe_metadata:
        run_step([python, "exe_header_dump.py", str(magic_exe), str(wizards_exe)])
        run_step([python, "borland_ovrinfo_dump.py", str(magic_exe), str(wizards_exe)])

    if not args.skip_clean_asm_check:
        run_step([python, str(Path("ida_dasm_export") / "ida55_check_clean_asm.py"), str(magic_asm), str(wizards_asm)])

    if not args.skip_clear_split_asm:
        removed_count = clear_split_asm_files(output_root)
        print(f"Removed {removed_count} existing split ASM files from {output_root}.")

    run_step([python, "ida55_split_asm.py", str(magic_asm), "--output-dir", str(split_output_dir(output_root, "MAGIC"))])
    run_step([python, "ida55_split_asm.py", str(wizards_asm), "--output-dir", str(split_output_dir(output_root, "WIZARDS"))])

    if not args.skip_asm_analysis:
        run_step([python, "asm_dedupe_index.py", "--root", str(output_root), "--output", str(output_root / "asm-dedupe-index.json")])
        run_step(
            [
                python,
                "asm_diff_report.py",
                "--root",
                str(output_root),
                "--index",
                str(output_root / "asm-dedupe-index.json"),
                "--output-dir",
                str(output_root / "asm-diff-report"),
            ]
        )

    # Evidence-based MAGIC name sync. Inventories come from the function dumps via
    # the adapter, so no separate IDA procedure-snapshot export is required.
    snapshots_dir = output_root / "ida55-procedure-snapshots"
    prereq_dir = Path("ida_proc_name_sync_prereq")
    name_sync_dir = Path("ida_proc_name_sync")

    mgc_inventory = snapshots_dir / "MGC.procedure-inventory.json"
    wzd_inventory = snapshots_dir / "WZD.procedure-inventory.json"
    mgc_body_evidence = snapshots_dir / "MGC.asm-body-evidence.json"
    wzd_body_evidence = snapshots_dir / "WZD.asm-body-evidence.json"
    mgc_evidence = snapshots_dir / "MGC.procedure-evidence.json"
    wzd_evidence = snapshots_dir / "WZD.procedure-evidence.json"
    segment_map = prereq_dir / "MGC-vs-WZD.segment-map.json"
    comparison = prereq_dir / "MGC-vs-WZD.procedure-evidence-comparison.json"
    handoff = prereq_dir / "magic_sync_names_from_wzd.json"
    handoff_final = name_sync_dir / "magic_sync_names_from_wzd.json"
    rename_idc = name_sync_dir / "magic_sync_names_from_wzd.idc"

    functions_to_inventory = Path("ida_proc_data_export") / "ida55_functions_to_inventory.py"
    run_step([python, str(functions_to_inventory), str(magic_functions_txt), "--database", "MGC", "--output", str(mgc_inventory)])
    run_step([python, str(functions_to_inventory), str(wizards_functions_txt), "--database", "WZD", "--output", str(wzd_inventory)])

    run_step([python, "ida55_asm_body_evidence.py", str(magic_asm), "--database", "MGC", "--output", str(mgc_body_evidence)])
    run_step([python, "ida55_asm_body_evidence.py", str(wizards_asm), "--database", "WZD", "--output", str(wzd_body_evidence)])

    run_step([python, "ida55_join_procedure_evidence.py", "--inventory", str(mgc_inventory), "--asm-body-evidence", str(mgc_body_evidence), "--output", str(mgc_evidence)])
    run_step([python, "ida55_join_procedure_evidence.py", "--inventory", str(wzd_inventory), "--asm-body-evidence", str(wzd_body_evidence), "--output", str(wzd_evidence)])

    run_step([python, str(prereq_dir / "compare_procedure_evidence.py"), "--left", str(mgc_evidence), "--right", str(wzd_evidence), "--segment-map", str(segment_map), "--output", str(comparison)])

    run_step([python, str(prereq_dir / "build_magic_name_sync_handoff.py"), "--comparison", str(comparison), "--output", str(handoff), "--final-copy", str(handoff_final)])

    run_step([python, str(name_sync_dir / "generate_magic_name_sync.py"), "--renames", str(handoff_final), "--output-idc", str(rename_idc)])

    print("Pipeline complete.")
    print(f"MAGIC name-sync IDC: {rename_idc}")
    print(
        "If it has a non-zero rename count, apply it inside MAGIC.idb (via the IDA GUI, "
        "or by passing it as the --script to ida_dasm_export/ida55_clean_export.py)."
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
