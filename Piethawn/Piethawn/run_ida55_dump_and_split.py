#!/usr/bin/env python3
"""
Run the IDA 5.5 file dumps for MAGIC and WIZARDS, then split the ASM into per-procedure files.

  1. ida_dasm_export/ida55_clean_export.py --copy            -> in/{MAGIC,WIZARDS}.{asm,inc,lst,map}
  2. ida_proc_data_export/ida55_export_function_data.py --copy -> in/*.idb.ida55-functions.{txt,json}
  3. ida_dasm_export/ida55_check_clean_asm.py                -> fails on a dirty ASM listing
  4. remove existing out/<GAME>/<segment>/*.asm, then ida55_split_asm.py -> out/MAGIC, out/WIZARDS

Close every IDA window first: both exports run IDA through ida_automation/idaw.py.
--skip-ida runs only steps 3-4, on the files already in in/.
"""

from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path
from typing import List, Sequence

REPO_ROOT = Path(__file__).resolve().parent
GAMES = ("MAGIC", "WIZARDS")
DEFAULT_TIMEOUT_SECONDS = 900


def parse_args(argv: Sequence[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Run the IDA 5.5 ASM and function-data dumps, then split the ASM.")
    parser.add_argument("--skip-ida", action="store_true", help="Do not run IDA; check and split the ASM already in in/.")
    parser.add_argument("--dry-run", action="store_true", help="Print the steps without running IDA, deleting or writing anything.")
    parser.add_argument(
        "--timeout-seconds",
        type=int,
        default=DEFAULT_TIMEOUT_SECONDS,
        help=f"Seconds to wait for each IDA run before killing it. Default: {DEFAULT_TIMEOUT_SECONDS}",
    )
    parser.add_argument("--output-root", type=Path, default=Path("out"), help="Root for the split ASM. Default: out")
    parser.add_argument("--python", default=sys.executable, help="Python interpreter for child scripts. Default: current interpreter")
    return parser.parse_args(argv)


def run_step(command: Sequence[str], dry_run: bool) -> None:
    print("+", " ".join(command), flush=True)
    if not dry_run:
        subprocess.run(command, cwd=REPO_ROOT, check=True)


def ida_export_commands(python: str, timeout_seconds: int, dry_run: bool) -> List[List[str]]:
    """Commands for the two IDA exports, both copying their outputs into in/. Powered by Claude."""
    shared = ["--copy", "--timeout-seconds", str(timeout_seconds), "--python", python] + (["--dry-run"] if dry_run else [])
    return [
        [python, "-B", str(Path("ida_dasm_export") / "ida55_clean_export.py"), "--skip-registry-snapshot", *shared],
        [python, "-B", str(Path("ida_proc_data_export") / "ida55_export_function_data.py"), *shared],
    ]


def split_asm_paths(output_root: Path) -> List[Path]:
    return sorted(path for path in output_root.glob("*/*/*.asm") if path.is_file())


def clear_split_asm_files(output_root: Path) -> int:
    paths = split_asm_paths(output_root)
    for path in paths:
        path.unlink()
    return len(paths)


def main(argv: Sequence[str] | None = None) -> int:
    args = parse_args(argv)
    python = args.python
    asm_paths = [Path("in") / f"{game}.asm" for game in GAMES]

    if not args.skip_ida:
        for command in ida_export_commands(python, args.timeout_seconds, args.dry_run):
            run_step(command, args.dry_run)

    if not args.dry_run:
        missing = [str(path) for path in asm_paths if not (REPO_ROOT / path).is_file()]
        if missing:
            raise FileNotFoundError("Missing ASM dump(s): " + ", ".join(missing))

    run_step([python, "-B", str(Path("ida_dasm_export") / "ida55_check_clean_asm.py"), *map(str, asm_paths)], args.dry_run)

    output_root = REPO_ROOT / args.output_root
    if args.dry_run:
        print(f"would remove {len(split_asm_paths(output_root))} existing split .asm files under {args.output_root}")
    else:
        print(f"removed {clear_split_asm_files(output_root)} existing split .asm files under {args.output_root}")

    for game, asm_path in zip(GAMES, asm_paths):
        run_step([python, "-B", "ida55_split_asm.py", str(asm_path), "--output-dir", str(args.output_root / game)], args.dry_run)

    print("Done." if not args.dry_run else "Dry run complete.")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except subprocess.CalledProcessError as error:
        print(f"ERROR: step failed with exit code {error.returncode}: {' '.join(map(str, error.cmd))}", file=sys.stderr)
        raise SystemExit(error.returncode)
