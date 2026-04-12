#!/usr/bin/env python3
"""Run Gemini on a named ASM function outside the STUB/WIP todo flow."""

from __future__ import annotations

import argparse
import sys
from pathlib import Path
from typing import List, Optional

import rasm_todo_gemini as todo_gemini


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Resolve a named function to an ASM file and submit it to Gemini.",
    )
    parser.add_argument("function_name", help="Exact ASM procedure name, for example UU_VGA_WndDrawRect")
    parser.add_argument("--game", choices=("ALL", "MAGIC", "WIZARDS"), default="ALL")
    parser.add_argument("--segment", help="Optional exact segment filter, for example seg014")
    parser.add_argument("--out-root", type=Path, default=Path("out"))
    parser.add_argument("--launcher", type=Path, default=Path("Rasm-Gemini.cmd"))
    parser.add_argument("--prompt-file", type=Path, default=Path("RasmAiAgentPrompt.md"))
    parser.add_argument("--output-suffix", default=".c", help="Generated output suffix. Default: .c")
    parser.add_argument("--prefer-game", choices=("MAGIC", "WIZARDS"), default="WIZARDS")
    parser.add_argument("--force", action="store_true", help="Allow rerunning even if the output file already exists.")
    parser.add_argument("--dry-run", action="store_true", help="Print the resolved command without invoking Gemini.")
    parser.add_argument(
        "--model",
        default=None,
        help="Gemini model to use. If omitted, the Gemini CLI default model is used.",
    )
    parser.add_argument("--list-matches", action="store_true", help="List all matching ASM files and exit.")
    return parser.parse_args()


def scan_named_records(out_root: Path, function_name: str, output_suffix: str) -> List[todo_gemini.FunctionRecord]:
    records: List[todo_gemini.FunctionRecord] = []
    for asm_path in out_root.glob("*/*/*.asm"):
        if asm_path.stem != function_name:
            continue

        relative_path = asm_path.relative_to(out_root)
        if len(relative_path.parts) != 3:
            continue

        game = relative_path.parts[0]
        segment = relative_path.parts[1]
        records.append(
            todo_gemini.FunctionRecord(
                game=game,
                segment=segment,
                name=function_name,
                status=todo_gemini.determine_status(function_name) or "NAMED",
                asm_path=asm_path,
                output_path=asm_path.with_suffix(output_suffix),
                dump_path=Path(""),
            )
        )

    return records


def filter_named_records(
    records: List[todo_gemini.FunctionRecord],
    game_filter: str,
    segment_filter: Optional[str],
) -> List[todo_gemini.FunctionRecord]:
    filtered: List[todo_gemini.FunctionRecord] = []
    for record in records:
        if game_filter != "ALL" and record.game != game_filter:
            continue
        if segment_filter and record.segment != segment_filter:
            continue
        filtered.append(record)
    return filtered


def choose_named_record(records: List[todo_gemini.FunctionRecord], prefer_game: str) -> Optional[todo_gemini.FunctionRecord]:
    if not records:
        return None

    return sorted(
        records,
        key=lambda item: (
            0 if item.game == prefer_game else 1,
            todo_gemini.natural_sort_key(item.game),
            todo_gemini.natural_sort_key(item.segment),
        ),
    )[0]


def main() -> int:
    args = parse_args()
    repo_root = Path(__file__).resolve().parent

    args.out_root = todo_gemini.normalize_path(args.out_root, repo_root)
    args.launcher = todo_gemini.normalize_path(args.launcher, repo_root)
    args.prompt_file = todo_gemini.normalize_path(args.prompt_file, repo_root)

    output_suffix = args.output_suffix if args.output_suffix.startswith(".") else f".{args.output_suffix}"
    records = scan_named_records(args.out_root, args.function_name, output_suffix)
    records = filter_named_records(records, args.game, args.segment)

    if not records:
        print(f"No ASM entry matched {args.function_name}.", file=sys.stderr)
        return 1

    if args.list_matches:
        for record in sorted(records, key=lambda item: (item.game, item.segment, item.asm_path.name)):
            print(f"{record.game}/{record.segment}  {record.asm_path}")
        return 0

    record = choose_named_record(records, args.prefer_game)
    if record is None:
        print(f"No ASM entry matched {args.function_name}.", file=sys.stderr)
        return 1

    if record.output_path.exists() and not args.force:
        print(f"Skipping existing output: {record.output_path}")
        return 0

    return todo_gemini.run_launcher(record, args.launcher, args.prompt_file, args.model, args.dry_run, repo_root)


if __name__ == "__main__":
    raise SystemExit(main())