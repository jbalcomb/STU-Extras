#!/usr/bin/env python3
"""Resolve ReMoM stub/WIP TODO entries to Piethawn ASM files and run Gemini."""

from __future__ import annotations

import argparse
import json
import os
import re
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, Iterable, List, Optional, Sequence


STATUS_SUFFIXES = ("__STUB", "__WIP")
DEFAULT_EXTERNAL_TODO = Path(r"C:\STU\devel\ReMoM\doc\#TODO\stub_wip_todo.md")
TODO_MODULE_RE = re.compile(r"^##\s+(?P<module>.+?)\s*$")
TODO_ITEM_RE = re.compile(r"^-\s+\[[ xX]\]\s+(?P<name>[A-Za-z_]\w*__(?:STUB|WIP))\s*$")


@dataclass(frozen=True)
class TodoEntry:
    order_index: int
    module_name: str
    function_name: str


@dataclass(frozen=True)
class FunctionRecord:
    game: str
    segment: str
    name: str
    status: str
    asm_path: Path
    output_path: Path
    dump_path: Path


@dataclass(frozen=True)
class QueueItem:
    todo_entry: TodoEntry
    candidates: List[FunctionRecord]
    selected: Optional[FunctionRecord]


def record_identity(record: FunctionRecord) -> tuple[str, str, str]:
    return (record.game, record.segment, record.name)


def natural_sort_key(value: str) -> List[object]:
    return [int(part) if part.isdigit() else part.casefold() for part in re.split(r"(\d+)", value)]


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Work through the ReMoM stub/WIP TODO list using Piethawn ASM and the Gemini rasm tooling.",
    )
    parser.add_argument(
        "command",
        nargs="?",
        choices=("list", "next", "run"),
        default="list",
        help="list pending items, run the next pending item, or run a specific function.",
    )
    parser.add_argument("function_name", nargs="?", help="Exact __STUB/__WIP function name for the run command.")
    parser.add_argument("--todo-file", type=Path, default=DEFAULT_EXTERNAL_TODO)
    parser.add_argument("--magic-dump", type=Path, default=Path("MAGIC.idb.ida55-functions.json"))
    parser.add_argument("--wizards-dump", type=Path, default=Path("WIZARDS.idb.ida55-functions.json"))
    parser.add_argument("--out-root", type=Path, default=Path("out"))
    parser.add_argument("--launcher", type=Path, default=Path("Rasm-Gemini.cmd"))
    parser.add_argument("--prompt-file", type=Path, default=Path("RasmAiAgentPrompt.md"))
    parser.add_argument("--output-suffix", default=".c", help="Generated output suffix. Default: .c")
    parser.add_argument("--prefer-game", choices=("MAGIC", "WIZARDS"), default="MAGIC")
    parser.add_argument("--game", choices=("ALL", "MAGIC", "WIZARDS"), default="ALL")
    parser.add_argument("--status", choices=("ALL", "STUB", "WIP"), default="ALL")
    parser.add_argument("--limit", type=int, default=20, help="Maximum rows to print for list. Default: 20")
    parser.add_argument("--force", action="store_true", help="Run even if the output file already exists.")
    parser.add_argument("--dry-run", action="store_true", help="Print what would run without invoking Gemini.")
    parser.add_argument(
        "--model",
        default=None,
        help="Gemini model to use. If omitted, the Gemini CLI default model is used.",
    )
    parser.add_argument("--all-matches", action="store_true", help="Run every matching ASM instead of only the preferred one.")
    return parser.parse_args()


def normalize_path(path_value: Path, root: Path) -> Path:
    return path_value if path_value.is_absolute() else (root / path_value)


def determine_status(function_name: str) -> Optional[str]:
    for suffix in STATUS_SUFFIXES:
        if function_name.endswith(suffix):
            return suffix[2:]
    return None


def parse_todo_markdown(todo_path: Path) -> List[TodoEntry]:
    entries: List[TodoEntry] = []
    current_module = ""
    text = todo_path.read_text(encoding="utf-8", errors="ignore")

    for line in text.splitlines():
        module_match = TODO_MODULE_RE.match(line)
        if module_match:
            current_module = module_match.group("module").strip()
            continue

        item_match = TODO_ITEM_RE.match(line)
        if item_match:
            entries.append(
                TodoEntry(
                    order_index=len(entries),
                    module_name=current_module,
                    function_name=item_match.group("name"),
                )
            )

    return entries


def build_synthetic_todo(function_records: Sequence[FunctionRecord]) -> List[TodoEntry]:
    seen_names = set()
    entries: List[TodoEntry] = []

    for record in sorted(
        function_records,
        key=lambda item: (
            natural_sort_key(item.game),
            natural_sort_key(item.segment),
            natural_sort_key(item.name),
        ),
    ):
        if record.name in seen_names:
            continue
        seen_names.add(record.name)
        entries.append(TodoEntry(len(entries), f"{record.game}/{record.segment}", record.name))

    return entries


def load_function_records(dump_path: Path, game: str, out_root: Path, output_suffix: str) -> List[FunctionRecord]:
    data = json.loads(dump_path.read_text(encoding="utf-8"))
    function_records: List[FunctionRecord] = []

    for item in data.get("functions", []):
        function_name = str(item.get("name", "")).strip()
        status = determine_status(function_name)
        if status is None:
            continue

        segment_name = str(item.get("segment", "")).strip()
        asm_path = out_root / game / segment_name / f"{function_name}.asm"
        output_path = asm_path.with_suffix(output_suffix)
        function_records.append(
            FunctionRecord(
                game=game,
                segment=segment_name,
                name=function_name,
                status=status,
                asm_path=asm_path,
                output_path=output_path,
                dump_path=dump_path,
            )
        )

    return function_records


def discover_asm_records(out_root: Path, output_suffix: str) -> List[FunctionRecord]:
    function_records: List[FunctionRecord] = []

    for asm_path in out_root.glob("*/*/*.asm"):
        relative_path = asm_path.relative_to(out_root)
        if len(relative_path.parts) != 3:
            continue

        game = relative_path.parts[0]
        segment = relative_path.parts[1]
        function_name = asm_path.stem
        status = determine_status(function_name)
        if status is None:
            continue

        function_records.append(
            FunctionRecord(
                game=game,
                segment=segment,
                name=function_name,
                status=status,
                asm_path=asm_path,
                output_path=asm_path.with_suffix(output_suffix),
                dump_path=Path(""),
            )
        )

    return function_records


def merge_function_records(*record_groups: Sequence[FunctionRecord]) -> List[FunctionRecord]:
    merged: Dict[tuple[str, str, str], FunctionRecord] = {}
    for record_group in record_groups:
        for record in record_group:
            merged.setdefault(record_identity(record), record)
    return list(merged.values())


def index_by_name(function_records: Iterable[FunctionRecord]) -> Dict[str, List[FunctionRecord]]:
    index: Dict[str, List[FunctionRecord]] = {}
    for record in function_records:
        index.setdefault(record.name, []).append(record)
    return index


def filter_records(
    function_records: Sequence[FunctionRecord],
    game_filter: str,
    status_filter: str,
) -> List[FunctionRecord]:
    filtered: List[FunctionRecord] = []
    for record in function_records:
        if game_filter != "ALL" and record.game != game_filter:
            continue
        if status_filter != "ALL" and record.status != status_filter:
            continue
        filtered.append(record)
    return filtered


def choose_record(candidates: Sequence[FunctionRecord], prefer_game: str) -> Optional[FunctionRecord]:
    if not candidates:
        return None

    return sorted(
        candidates,
        key=lambda item: (
            0 if item.asm_path.exists() else 1,
            0 if item.game == prefer_game else 1,
            natural_sort_key(item.game),
            natural_sort_key(item.segment),
            natural_sort_key(item.name),
        ),
    )[0]


def build_queue(todo_entries: Sequence[TodoEntry], function_records: Sequence[FunctionRecord], prefer_game: str) -> List[QueueItem]:
    records_by_name = index_by_name(function_records)
    queue: List[QueueItem] = []

    for todo_entry in todo_entries:
        candidates = records_by_name.get(todo_entry.function_name, [])
        queue.append(
            QueueItem(
                todo_entry=todo_entry,
                candidates=sorted(
                    candidates,
                    key=lambda item: (
                        natural_sort_key(item.game),
                        natural_sort_key(item.segment),
                        natural_sort_key(item.name),
                    ),
                ),
                selected=choose_record(candidates, prefer_game),
            )
        )

    return queue


def item_is_pending(queue_item: QueueItem, force: bool) -> bool:
    if queue_item.selected is None:
        return False
    if not queue_item.selected.asm_path.exists():
        return False
    if force:
        return True
    return not queue_item.selected.output_path.exists()


def format_item(queue_item: QueueItem, force: bool) -> str:
    selected = queue_item.selected
    if selected is None:
        return f"missing-match  {queue_item.todo_entry.module_name:20}  {queue_item.todo_entry.function_name}"

    if not selected.asm_path.exists():
        state = "missing-asm"
    elif selected.output_path.exists() and not force:
        state = "done"
    else:
        state = "pending"

    note = ""
    if len(queue_item.candidates) > 1:
        note = f"  matches={len(queue_item.candidates)}"

    return (
        f"{state:12}  {queue_item.todo_entry.module_name:20}  {queue_item.todo_entry.function_name:40}  "
        f"{selected.game}/{selected.segment}{note}"
    )


def print_queue(queue: Sequence[QueueItem], limit: int, force: bool) -> int:
    visible_items = list(queue[:limit]) if limit > 0 else list(queue)
    pending_count = sum(1 for item in queue if item_is_pending(item, force))
    print(f"Queue entries: {len(queue)}")
    print(f"Pending entries: {pending_count}")
    print("")
    for queue_item in visible_items:
        print(format_item(queue_item, force))

    if limit > 0 and len(queue) > limit:
        print("")
        print(f"Showing first {limit} entries.")

    return 0


def run_launcher(
    record: FunctionRecord,
    launcher_path: Path,
    prompt_file: Path,
    model_name: Optional[str],
    dry_run: bool,
    repo_root: Path,
) -> int:
    command = ["cmd", "/c", str(launcher_path), str(record.asm_path), str(record.output_path)]
    env = os.environ.copy()

    if prompt_file.exists():
        env["GEMINI_SYSTEM_MD"] = str(prompt_file)

    if model_name:
        env["RASM_GEMINI_MODEL"] = model_name
    else:
        env.pop("RASM_GEMINI_MODEL", None)

    print(f"ASM:    {record.asm_path}")
    print(f"Output: {record.output_path}")
    print(f"Game:   {record.game}/{record.segment}")
    if prompt_file.exists():
        print(f"Prompt: {prompt_file}")
    if model_name:
        print(f"Model:  {model_name}")
    print(f"Cmd:    {' '.join(command)}")

    if dry_run:
        return 0

    completed = subprocess.run(command, cwd=repo_root, env=env)
    return completed.returncode


def run_named_items(
    function_name: str,
    function_records: Sequence[FunctionRecord],
    args: argparse.Namespace,
    repo_root: Path,
) -> int:
    matching_records = [record for record in function_records if record.name == function_name]
    if not matching_records:
        print(f"No ASM entry matched {function_name}.", file=sys.stderr)
        return 1

    records_to_run = matching_records if args.all_matches else [choose_record(matching_records, args.prefer_game)]
    exit_code = 0

    for record in records_to_run:
        if record is None:
            continue
        if not record.asm_path.exists():
            print(f"Missing ASM file: {record.asm_path}", file=sys.stderr)
            exit_code = 1
            continue
        if record.output_path.exists() and not args.force:
            print(f"Skipping existing output: {record.output_path}")
            continue

        result = run_launcher(record, args.launcher, args.prompt_file, args.model, args.dry_run, repo_root)
        if result != 0:
            return result

    return exit_code


def run_next_item(queue: Sequence[QueueItem], args: argparse.Namespace, repo_root: Path) -> int:
    for queue_item in queue:
        if not item_is_pending(queue_item, args.force):
            continue
        selected = queue_item.selected
        if selected is None:
            continue
        print(f"Next TODO: {queue_item.todo_entry.module_name} :: {queue_item.todo_entry.function_name}")
        return run_launcher(selected, args.launcher, args.prompt_file, args.model, args.dry_run, repo_root)

    print("No pending TODO entries matched the current filters.")
    return 0


def main() -> int:
    args = parse_args()
    repo_root = Path(__file__).resolve().parent

    args.todo_file = normalize_path(args.todo_file, repo_root)
    args.magic_dump = normalize_path(args.magic_dump, repo_root)
    args.wizards_dump = normalize_path(args.wizards_dump, repo_root)
    args.out_root = normalize_path(args.out_root, repo_root)
    args.launcher = normalize_path(args.launcher, repo_root)
    args.prompt_file = normalize_path(args.prompt_file, repo_root)

    output_suffix = args.output_suffix if args.output_suffix.startswith(".") else f".{args.output_suffix}"
    function_records = load_function_records(args.magic_dump, "MAGIC", args.out_root, output_suffix)
    function_records.extend(load_function_records(args.wizards_dump, "WIZARDS", args.out_root, output_suffix))
    function_records = merge_function_records(function_records, discover_asm_records(args.out_root, output_suffix))
    function_records = filter_records(function_records, args.game, args.status)

    if args.command == "run":
        if not args.function_name:
            print("The run command requires a function name.", file=sys.stderr)
            return 1
        return run_named_items(args.function_name, function_records, args, repo_root)

    if args.todo_file.exists():
        todo_entries = parse_todo_markdown(args.todo_file)
    else:
        todo_entries = build_synthetic_todo(function_records)

    queue = build_queue(todo_entries, function_records, args.prefer_game)

    if args.command == "next":
        return run_next_item(queue, args, repo_root)

    return print_queue(queue, args.limit, args.force)


if __name__ == "__main__":
    raise SystemExit(main())