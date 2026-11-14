#!/usr/bin/env python3
"""
apply_mom131_renames.py -- apply compare_mom131_remom.py's plan (out/mom131-name-sync/mom131_renames_from_remom.json)
to a mom131-source-map checkout: every git-tracked text file, all renames at once, whole words only.

  source files (.C .H .ASM .INC .MAC)  code and comments are renamed, string/char literals never (their bytes are in
                                       the EXE); a file gets the renames of the program(s) that link it
                                       (mom131_name_sync/programs.py); assembly '_Name' follows 'Name'
  other text files (docs, tools, re/,  renamed where the name means one thing. A name that two programs rename
  index.html ...)                      differently (or one renames and the other keeps) is resolved per mention:
                                       re/magic_names.tsv, re/current_MAGIC/ = MGC; re/names.tsv,
                                       re/current_WIZARDS/ = WZD; else the nearest IDA address within 80 characters
                                       that is one program's definition of it ("Unit_Create (4358:95d5)",
                                       ["a0ec:0000","Unit_Create"]); else a 'mentions' entry of
                                       mom131_name_sync/reconcile.json; else MAGIC / WIZARDS in the file name, or
                                       named alone on the line. A mention none of these decides is left and listed
                                       with file:line.

Refuses a checkout with uncommitted changes (--allow-dirty overrides) and a plan with problems still in the report.
Afterwards build the checkout (build/build.py) and require both EXEs identical: names are not in the EXE bytes.

    python -B mom131_name_sync/apply_mom131_renames.py --source C:\\STU\\devel\\mom131-bridge
"""

from __future__ import annotations

import argparse
import csv
import json
import re
import subprocess
import sys
from collections import Counter
from pathlib import Path
from typing import Dict, List, Set, Tuple

if __package__ in (None, ""):
    sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from mom131_name_sync.mom131_functions import DEFAULT_OUT as MOM131_FUNCTIONS  # noqa: E402
from mom131_name_sync.compare_mom131_remom import DEFAULT_OUT_DIR, DEFAULT_RECONCILE, PLAN_NAME, load_reconcile, mappings_from_plan, source_scope  # noqa: E402
from mom131_name_sync.mom131_names import DEFAULT_SOURCE  # noqa: E402
from mom131_name_sync.programs import rel  # noqa: E402
from mom131_name_sync.source_words import WORD, read_text, rename_text, rename_word, tracked_text_files  # noqa: E402


class ApplyError(RuntimeError):
    pass


def merged_mapping(programs: Set[str], mappings: Dict[str, Dict[str, str]]) -> Tuple[Dict[str, str], Set[str]]:
    """The renames for a file linked into these programs, and the names its programs rename differently (left as
    they are: the comparison allows them in a shared file's comments only; in code the build would fail).
    Powered by Claude."""
    merged: Dict[str, str] = {}
    disagree: Set[str] = set()
    for program in sorted(programs):
        for old, new in mappings.get(program, {}).items():
            if merged.get(old, new) != new:
                disagree.add(old)
            merged[old] = new
    return {k: v for k, v in merged.items() if k not in disagree}, disagree


def unscoped_mapping(mappings: Dict[str, Dict[str, str]], own_words: Dict[str, Counter]) -> Tuple[Dict[str, str], List[str]]:
    """Renames for files no program links (docs ...): only names with one meaning. Returns (mapping, names left).
    Powered by Claude."""
    merged: Dict[str, str] = {}
    left: Set[str] = set()
    for old in sorted({o for m in mappings.values() for o in m}):
        targets = set()
        for program in mappings:
            if old in mappings[program]:
                targets.add(mappings[program][old])
            elif own_words[program].get(old):
                targets.add(old)
        if len(targets) == 1:
            merged[old] = targets.pop()
        else:
            left.add(old)
    return merged, sorted(left)


PATH_PROGRAM = (("re/magic_names.tsv", "MGC"), ("re/current_MAGIC/", "MGC"), ("re/names.tsv", "WZD"),
                ("re/current_WIZARDS/", "WZD"))
ADDRESS = re.compile(r"\b([0-9A-Fa-f]{4}:[0-9A-Fa-f]{4})\b")
ADDRESS_WINDOW = 80
MENTION_WINDOW = 120  # a reconcile 'mentions' entry's 'near' text must be this close to the mention
LINE_PROGRAM = (("MGC", re.compile(r"\bMAGIC")), ("WZD", re.compile(r"\bWIZARDS")))


def definition_addresses(functions_tsv: Path, steps: List[Dict], names: Set[str]) -> Dict[str, Dict[str, str]]:
    """{old name: {IDA address (lower case): program}} for the names: the renamed ids' addresses from the plan, and a
    program's own unrenamed function of that name. functions_tsv: mom131_functions.tsv taken before the renames.
    Powered by Claude."""
    with open(functions_tsv, newline="", encoding="utf-8") as f:
        rows = list(csv.DictReader(f, delimiter="\t"))
    by_id = {(r["exe"], r["id"]): r for r in rows if r["id"]}
    out: Dict[str, Dict[str, str]] = {n: {} for n in names}
    for step in steps:
        if step["old_name"] in names:
            for rid in step["ids"]:
                row = by_id.get((rid.split()[0], rid))
                if row and row["ida_address"]:
                    out[step["old_name"]][row["ida_address"].lower()] = row["exe"]
    for r in rows:
        if r["function"] in names and r["ida_address"]:
            out[r["function"]].setdefault(r["ida_address"].lower(), r["exe"])
    return out


def mention_program(rel_path: str, text: str, start: int, end: int, addresses: Dict[str, str],
                    mentions: List[Dict] = ()) -> str:
    """Which program one mention of a two-meaning name refers to ('WZD', 'MGC', 'keep'), or ''. Powered by Claude."""
    for prefix, program in PATH_PROGRAM:
        if rel_path.startswith(prefix):
            return program
    best = None
    lo, hi = max(0, start - ADDRESS_WINDOW), min(len(text), end + ADDRESS_WINDOW)
    for m in ADDRESS.finditer(text, lo, hi):
        program = addresses.get(m.group(1).lower())
        if program:
            distance = start - m.end() if m.end() <= start else m.start() - end
            if best is None or distance < best[0]:
                best = (distance, program)
    if best:
        return best[1]
    line_start, line_end = text.rfind("\n", 0, start) + 1, text.find("\n", end)
    line = text[line_start:line_end if line_end != -1 else len(text)]
    name = text[start:end]
    around = text[max(0, start - MENTION_WINDOW):end + MENTION_WINDOW]
    for m in mentions:
        if m["file"] == rel_path and m["name"] == name and m["near"] in around:
            return m["program"]
    file_name = rel_path.rsplit("/", 1)[-1].upper()
    named = {program for program, word in (("MGC", "MAGIC"), ("WZD", "WIZARDS")) if word in file_name}
    if len(named) == 1:
        return named.pop()
    named = {program for program, pattern in LINE_PROGRAM if pattern.search(line)}
    return named.pop() if len(named) == 1 else ""


def rename_doc(rel_path: str, text: str, mapping: Dict[str, str], left: List[str], mappings: Dict[str, Dict[str, str]],
               addresses: Dict[str, Dict[str, str]], mentions: List[Dict] = ()) -> Tuple[str, int, List[Tuple[str, str]]]:
    """One pass over a file no program links: names with one meaning by mapping, each mention of a two-meaning name
    (left) by the program it refers to -- decided on the original text. Returns (text, count, [(name, file:line) not
    decided]). Powered by Claude."""
    two = set(left)
    count, undecided = 0, []

    def sub(m):
        nonlocal count
        word = m.group(0)
        if word in two:
            program = mention_program(rel_path, text, m.start(), m.end(), addresses.get(word, {}), mentions)
            if not program:
                undecided.append((word, f"{rel_path}:{text.count(chr(10), 0, m.start()) + 1}"))
            new = mappings.get(program, {}).get(word, word) if program in ("WZD", "MGC") else word
        else:
            new = rename_word(word, mapping)
        count += new != word
        return new

    return WORD.sub(sub, text), count, undecided


def dirty(source: Path) -> List[str]:
    """Uncommitted changes of tracked files in the checkout. Powered by Claude."""
    out = subprocess.run(["git", "-C", str(source), "status", "--porcelain", "--untracked-files=no"],
                         capture_output=True, text=True, check=True).stdout
    return [line for line in out.splitlines() if line.strip()]


def apply_plan(source: Path, plan: Dict, dry_run: bool = False, functions_tsv: Path = None,
               reconcile: Path = None) -> Dict:
    """Rename across the checkout; returns {files: {path: count}, left_in_docs: [(name, file:line)], renames: n}.
    functions_tsv: mom131_functions.tsv of the checkout before the renames (the addresses that tell a two-meaning
    name's mentions apart). Powered by Claude."""
    steps = plan["renames"]
    mappings = mappings_from_plan(steps)
    scope = source_scope(source, None)
    docs_mapping, left = unscoped_mapping(mappings, scope.own_words)
    functions_tsv = functions_tsv or Path(__file__).resolve().parent.parent / MOM131_FUNCTIONS
    addresses = definition_addresses(functions_tsv, steps, set(left)) if left and functions_tsv.is_file() else {}
    mentions = load_reconcile(reconcile or Path(__file__).resolve().parent.parent / DEFAULT_RECONCILE)["mentions"]
    changed: Dict[str, int] = {}
    left_in: Set[Tuple[str, str]] = set()
    undecided: List[Tuple[str, str]] = []
    for path in tracked_text_files(source):
        name = rel(source, path)
        programs = scope.module_programs.get(name)
        if programs:
            mapping, disagree = merged_mapping(programs, mappings)
            left_in.update((n, name) for n in disagree if n in read_text(path))
        else:
            mapping = docs_mapping
        text = read_text(path)
        if programs:
            new_text, count = rename_text(path, text, mapping) if mapping else (text, 0)
        else:
            new_text, count, open_ = rename_doc(name, text, mapping, left, mappings, addresses, mentions)
            undecided += open_
        if count:
            changed[name] = count
            if not dry_run:
                path.write_bytes(new_text.encode("latin-1"))
    return {"files": changed, "left_in_docs": undecided, "left_in_shared": sorted(left_in), "renames": len(steps)}


def main(argv=None) -> int:
    """Command-line entry point. Powered by Claude."""
    repo_root = Path(__file__).resolve().parent.parent
    p = argparse.ArgumentParser(description="Apply the mom131 rename plan to a mom131-source-map checkout.")
    p.add_argument("--source", type=Path, default=Path(DEFAULT_SOURCE))
    p.add_argument("--plan", type=Path, default=repo_root / DEFAULT_OUT_DIR / PLAN_NAME)
    p.add_argument("--allow-dirty", action="store_true", help="Apply on top of uncommitted changes.")
    p.add_argument("--allow-problems", action="store_true", help="Apply the plan although the last report had problems (testing only).")
    p.add_argument("--dry-run", action="store_true", help="Count the changes, write nothing.")
    args = p.parse_args(argv)
    plan = json.loads(args.plan.read_text(encoding="utf-8"))
    template = args.plan.parent / "problems.reconcile-template.json"
    if template.exists() and not args.allow_problems:
        raise ApplyError(f"The last comparison had problems ({template}); reconcile them and compare again first.")
    if plan.get("source") and Path(plan["source"]).resolve() != args.source.resolve():
        raise ApplyError(f"The plan was made for {plan['source']}, not {args.source}.")
    changes = dirty(args.source)
    if changes and not args.allow_dirty and not args.dry_run:
        raise ApplyError(f"{args.source} has uncommitted changes ({len(changes)} file(s)); commit or stash them, or pass --allow-dirty.")
    result = apply_plan(args.source, plan, args.dry_run)
    total = sum(result["files"].values())
    print(f"{'Would rename' if args.dry_run else 'Renamed'} {total} occurrence(s) of {result['renames']} function(s) in {len(result['files'])} file(s).")
    for old, where in result["left_in_shared"]:
        print(f"Left {old} in {where} (a shared file; WZD and MGC rename it differently).")
    for old, where in result["left_in_docs"]:
        print(f"Left {old} at {where} (WZD and MGC name it differently; the mention does not say which).")
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except (ApplyError, ValueError, FileNotFoundError) as error:
        print(f"ERROR: {error}", file=sys.stderr)
        sys.exit(1)
