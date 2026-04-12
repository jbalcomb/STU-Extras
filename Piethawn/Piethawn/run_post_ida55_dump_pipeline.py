#!/usr/bin/env python3
"""
Run the Piethawn post-IDA55 pipeline from manually dumped ASM/function text to
the generated MAGIC rename IDC script.
"""

from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path
from typing import Iterable, List, Sequence, Tuple


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Run the post-IDA55 pipeline from manual dumps through MAGIC rename IDC generation."
    )
    parser.add_argument("--magic-asm", default=str(Path("in") / "MAGIC.asm"), help="MAGIC ASM dump from IDA Pro 5.5")
    parser.add_argument("--wizards-asm", default=str(Path("in") / "WIZARDS.asm"), help="WIZARDS ASM dump from IDA Pro 5.5")
    parser.add_argument(
        "--magic-functions-txt",
        default=str(Path("in") / "MAGIC.idb.ida55-functions.txt"),
        help="MAGIC function dump text produced by ida55_dump_functions.idc",
    )
    parser.add_argument(
        "--wizards-functions-txt",
        default=str(Path("in") / "WIZARDS.idb.ida55-functions.txt"),
        help="WIZARDS function dump text produced by ida55_dump_functions.idc",
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
        "--skip-sample-pair-diffs",
        action="store_true",
        help="Skip Save_SAVE_GAM and Load_SAVE_GAM pair diff reports",
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


def split_output_dir(output_root: Path, game: str) -> Path:
    return output_root / game


def function_json_path(functions_txt: Path) -> Path:
    return functions_txt.with_suffix(".json")


def sample_pair_jobs(output_root: Path) -> List[Tuple[str, List[str], Path, Path]]:
    return [
        (
            "Save_SAVE_GAM",
            [
                "--left-game",
                "MAGIC",
                "--left-segment",
                "ovr048",
                "--left-name",
                "Save_SAVE_GAM",
                "--left-asm",
                str(output_root / "MAGIC" / "ovr048" / "Save_SAVE_GAM.asm"),
                "--right-game",
                "WIZARDS",
                "--right-segment",
                "ovr050",
                "--right-name",
                "Save_SAVE_GAM",
                "--right-asm",
                str(output_root / "WIZARDS" / "ovr050" / "Save_SAVE_GAM.asm"),
                "--output",
                str(output_root / "Save_SAVE_GAM.pair-diff.md"),
            ],
            output_root / "MAGIC" / "ovr048" / "Save_SAVE_GAM.asm",
            output_root / "WIZARDS" / "ovr050" / "Save_SAVE_GAM.asm",
        ),
        (
            "Load_SAVE_GAM",
            [
                "--left-game",
                "MAGIC",
                "--left-segment",
                "ovr048",
                "--left-name",
                "Load_SAVE_GAM",
                "--left-asm",
                str(output_root / "MAGIC" / "ovr048" / "Load_SAVE_GAM.asm"),
                "--right-game",
                "WIZARDS",
                "--right-segment",
                "ovr050",
                "--right-name",
                "Load_SAVE_GAM",
                "--right-asm",
                str(output_root / "WIZARDS" / "ovr050" / "Load_SAVE_GAM.asm"),
                "--output",
                str(output_root / "Load_SAVE_GAM.pair-diff.md"),
            ],
            output_root / "MAGIC" / "ovr048" / "Load_SAVE_GAM.asm",
            output_root / "WIZARDS" / "ovr050" / "Load_SAVE_GAM.asm",
        ),
    ]


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
    magic_functions_json = function_json_path(magic_functions_txt)
    wizards_functions_json = function_json_path(wizards_functions_txt)

    require_files([magic_asm, wizards_asm, magic_functions_txt, wizards_functions_txt])
    if not args.skip_exe_metadata:
        require_files([magic_exe, wizards_exe])

    output_root.mkdir(parents=True, exist_ok=True)

    if not args.skip_exe_metadata:
        run_step([python, "exe_header_dump.py", str(magic_exe), str(wizards_exe)])
        run_step([python, "borland_ovrinfo_dump.py", str(magic_exe), str(wizards_exe)])

    run_step([python, "ida55_split_asm.py", str(magic_asm), "--output-dir", str(split_output_dir(output_root, "MAGIC"))])
    run_step([python, "ida55_split_asm.py", str(wizards_asm), "--output-dir", str(split_output_dir(output_root, "WIZARDS"))])

    run_step([python, "ida55_function_dump.py", str(magic_functions_txt), "--output", str(magic_functions_json)])
    run_step([python, "ida55_function_dump.py", str(wizards_functions_txt), "--output", str(wizards_functions_json)])

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

    run_step(
        [
            python,
            "ida55_match_functions.py",
            "--magic",
            str(magic_functions_json),
            "--wizards",
            str(wizards_functions_json),
            "--output",
            str(output_root / "ida55-function-match.json"),
        ]
    )
    run_step(
        [
            python,
            "ida55_generate_magic_rename_idc.py",
            "--match",
            str(output_root / "ida55-function-match.json"),
            "--output",
            str(output_root / "ida55-sync" / "magic_rename_to_wizards.idc"),
            "--manifest",
            str(output_root / "ida55-sync" / "magic_rename_to_wizards.json"),
        ]
    )

    if not args.skip_sample_pair_diffs:
        for _, extra_args, left_asm, right_asm in sample_pair_jobs(output_root):
            if left_asm.exists() and right_asm.exists():
                run_step(
                    [
                        python,
                        "ida55_function_pair_diff.py",
                        "--magic-dump",
                        str(magic_functions_json),
                        "--wizards-dump",
                        str(wizards_functions_json),
                        *extra_args,
                    ]
                )
            else:
                print(f"! Skipping sample pair diff because input ASM is missing: {left_asm} or {right_asm}")

    print("Pipeline complete.")
    print(f"Run this next inside MAGIC.idb: {output_root / 'ida55-sync' / 'magic_rename_to_wizards.idc'}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
