#!/usr/bin/env python3
"""Randomly pick a pending STUB/WIP TODO entry and run it through Gemini."""

from __future__ import annotations

import argparse
import random
import sys
from pathlib import Path
from typing import Optional

import rasm_todo_gemini as todo_gemini


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Randomly select one pending STUB/WIP TODO function and submit it to Gemini.",
    )
    parser.add_argument("--todo-file", type=Path, default=todo_gemini.DEFAULT_EXTERNAL_TODO)
    parser.add_argument("--magic-dump", type=Path, default=Path("MAGIC.idb.ida55-functions.json"))
    parser.add_argument("--wizards-dump", type=Path, default=Path("WIZARDS.idb.ida55-functions.json"))
    parser.add_argument("--out-root", type=Path, default=Path("out"))
    parser.add_argument("--launcher", type=Path, default=Path("Rasm-Gemini.cmd"))
    parser.add_argument("--prompt-file", type=Path, default=Path("RasmAiAgentPrompt.md"))
    parser.add_argument("--output-suffix", default=".c", help="Generated output suffix. Default: .c")
    parser.add_argument("--prefer-game", choices=("MAGIC", "WIZARDS"), default="MAGIC")
    parser.add_argument("--game", choices=("ALL", "MAGIC", "WIZARDS"), default="ALL")
    parser.add_argument("--status", choices=("ALL", "STUB", "WIP"), default="ALL")
    parser.add_argument("--force", action="store_true", help="Allow rerunning items that already have output.")
    parser.add_argument("--dry-run", action="store_true", help="Print the chosen item and command without invoking Gemini.")
    parser.add_argument(
        "--model",
        default=None,
        help="Gemini model to use. If omitted, the Gemini CLI default model is used.",
    )
    parser.add_argument("--seed", type=int, help="Optional RNG seed for reproducible selection.")
    return parser.parse_args()


def choose_random_pending_item(args: argparse.Namespace, repo_root: Path) -> Optional[todo_gemini.QueueItem]:
    args.todo_file = todo_gemini.normalize_path(args.todo_file, repo_root)
    args.magic_dump = todo_gemini.normalize_path(args.magic_dump, repo_root)
    args.wizards_dump = todo_gemini.normalize_path(args.wizards_dump, repo_root)
    args.out_root = todo_gemini.normalize_path(args.out_root, repo_root)
    args.launcher = todo_gemini.normalize_path(args.launcher, repo_root)
    args.prompt_file = todo_gemini.normalize_path(args.prompt_file, repo_root)

    output_suffix = args.output_suffix if args.output_suffix.startswith(".") else f".{args.output_suffix}"
    function_records = todo_gemini.load_function_records(args.magic_dump, "MAGIC", args.out_root, output_suffix)
    function_records.extend(todo_gemini.load_function_records(args.wizards_dump, "WIZARDS", args.out_root, output_suffix))
    function_records = todo_gemini.merge_function_records(
        function_records,
        todo_gemini.discover_asm_records(args.out_root, output_suffix),
    )
    function_records = todo_gemini.filter_records(function_records, args.game, args.status)

    if args.todo_file.exists():
        todo_entries = todo_gemini.parse_todo_markdown(args.todo_file)
    else:
        todo_entries = todo_gemini.build_synthetic_todo(function_records)

    queue = todo_gemini.build_queue(todo_entries, function_records, args.prefer_game)
    pending_items = [item for item in queue if todo_gemini.item_is_pending(item, args.force)]

    if not pending_items:
        return None

    rng = random.Random(args.seed)
    return rng.choice(pending_items)


def main() -> int:
    args = parse_args()
    repo_root = Path(__file__).resolve().parent
    queue_item = choose_random_pending_item(args, repo_root)

    if queue_item is None:
        print("No pending TODO entries matched the current filters.")
        return 0

    selected = queue_item.selected
    if selected is None:
        print("Random selection did not resolve to an ASM file.", file=sys.stderr)
        return 1

    print(f"Random TODO: {queue_item.todo_entry.module_name} :: {queue_item.todo_entry.function_name}")
    if args.seed is not None:
        print(f"Seed: {args.seed}")

    return todo_gemini.run_launcher(selected, args.launcher, args.prompt_file, args.model, args.dry_run, repo_root)


if __name__ == "__main__":
    raise SystemExit(main())