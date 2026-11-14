#!/usr/bin/env python3
"""
compare_remom_wzd.py -- compare ReMoM's procedure names with WIZARDS.idb's and plan the WZD renames (ReMoM wins).

    python -B remom_name_sync/compare_remom_wzd.py [--index in/name-snapshots/remom/remom_index.tsv]
        [--functions in/WIZARDS.idb.ida55-functions.json] [--reconcile remom_name_sync/reconcile.json]
        [--out-dir out/remom-name-sync]

Join: ReMoM id "WZD o076p01" -> IDA segment ovr076, procedure 1 (1-based position in the segment by address, the
same ordinal the MGC rename uses). "s" ids -> segNNN; segments IDA names differently (SOUND, _AIL_) are mapped by
reconcile.json "segment_aliases".

Every WZD row gets one class:
  same            ReMoM and IDA already agree
  rename          IDA gets ReMoM's name
  no-code         ReMoM has the id comment but no function yet (nothing to rename)
  skipped         reconcile.json says skip
  problem kinds   (halt; see PROBLEM_HELP): ambiguous, multiple-functions, no-ida-proc, wrong-letter, invalid-name,
                  duplicate-target, collision, temp-name-taken
Names swapped in a cycle (A<->B) are handled with a temporary name <old>__rnsync_tmp.
IDA procedures (segNNN/ovrNNN/aliased) with no ReMoM id are listed as only-in-ida.

Writes <out-dir>/remom-vs-wzd.report.json and .md, <out-dir>/wzd_renames_from_remom.json (the rename plan, in a safe
order), and, when there are problems, <out-dir>/problems.reconcile-template.json. Exit code 3 when any problem is
not reconciled.
"""

from __future__ import annotations

import argparse
import json
import re
import sys
from collections import Counter, defaultdict
from pathlib import Path
from typing import Dict, List, Optional, Tuple

if __package__ in (None, ""):
    sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from ida_proc_data_export.ida55_functions_to_inventory import build_inventory_from_function_dump_json
from remom_name_sync.remom_index import key_text, read_index

DEFAULT_INDEX = Path("in") / "name-snapshots" / "remom" / "remom_index.tsv"
DEFAULT_FUNCTIONS = Path("in") / "WIZARDS.idb.ida55-functions.json"
DEFAULT_RECONCILE = Path("remom_name_sync") / "reconcile.json"
DEFAULT_OUT_DIR = Path("out") / "remom-name-sync"
PLAN_NAME = "wzd_renames_from_remom.json"
EXE = "WZD"
IDA_NAME = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*$")
SEGMENT_NAME = re.compile(r"^(seg|ovr)0*(\d+)$", re.IGNORECASE)
ALIAS_TARGET = re.compile(r"^([so])0*(\d+)$")
RECONCILE_ACTIONS = ("skip", "name", "accept-letter")

PROBLEM_HELP = {
    "ambiguous": "The scanner tied this id to a C function that another id also claims (a helper between the id "
                 "comment and its function, or a mistyped id). Fix the id comment/placement in ReMoM, or reconcile: "
                 '"skip", or "name" with the right function name.',
    "multiple-functions": "Several C functions were found for this id. Fix ReMoM, or reconcile: \"skip\", or \"name\".",
    "no-ida-proc": "The id names no procedure in WIZARDS.idb (segment missing or proc number too high). Fix the id "
                   "in ReMoM, add a segment_aliases entry if IDA names the segment differently, or reconcile: \"skip\".",
    "wrong-letter": "The id uses s for an overlay or o for a segment; the number matches the other kind. Fix the "
                    'letter in ReMoM, or reconcile: "accept-letter", or "skip".',
    "invalid-name": 'The ReMoM name is not a valid IDA name ([A-Za-z_][A-Za-z0-9_]*). Fix ReMoM, or reconcile: "name" or "skip".',
    "duplicate-target": "Several WZD procedures would get the same name. Fix ReMoM, or reconcile all but one: \"skip\" or \"name\".",
    "collision": "The new name already belongs to another WZD procedure that is not being renamed. Rename that "
                 "procedure in ReMoM too, or reconcile this one: \"skip\" or \"name\".",
    "temp-name-taken": "A rename cycle (names swapped) is broken with a temporary name <old>__rnsync_tmp, but that "
                       'name already exists in WZD. Rename it in IDA, or reconcile one member of the cycle: "skip".',
}
TEMP_SUFFIX = "__rnsync_tmp"


def load_reconcile(path: Optional[Path]) -> Dict:
    """Load and validate reconcile.json; a missing file means no aliases and no entries. Powered by Claude."""
    if path is None or not path.exists():
        return {"segment_aliases": {}, "entries": {}}
    data = json.loads(path.read_text(encoding="utf-8"))
    aliases = data.get("segment_aliases", {}) or {}
    entries = data.get("entries", {}) or {}
    errors = []
    for segment, target in aliases.items():
        if not ALIAS_TARGET.match(str(target)):
            errors.append(f"segment_aliases.{segment}: {target!r} is not sNNN or oNNN")
    for key, entry in entries.items():
        action = entry.get("action") if isinstance(entry, dict) else None
        if action not in RECONCILE_ACTIONS:
            errors.append(f"entries.{key}: action must be one of {RECONCILE_ACTIONS}, got {action!r}")
        elif action == "name" and not IDA_NAME.match(str(entry.get("name", ""))):
            errors.append(f"entries.{key}: action 'name' needs a valid \"name\"")
    if errors:
        raise ValueError(f"Invalid {path}:\n  " + "\n  ".join(errors))
    return {"segment_aliases": aliases, "entries": entries}


def ida_procedures(functions_payload: Dict, aliases: Dict[str, str]) -> Dict[Tuple[str, int, int], Dict]:
    """{(letter, segment number, proc ordinal): {name, segment, start_ea}} for segNNN/ovrNNN/aliased segments. Powered by Claude."""
    inventory = build_inventory_from_function_dump_json(functions_payload, EXE, Path("functions.json"))
    out = {}
    for proc in inventory.procedures:
        m = SEGMENT_NAME.match(proc.segment)
        if m:
            letter, number = ("s" if m.group(1).lower() == "seg" else "o"), int(m.group(2))
        elif proc.segment in aliases:
            a = ALIAS_TARGET.match(aliases[proc.segment])
            letter, number = a.group(1), int(a.group(2))
        else:
            continue
        out[(letter, number, proc.procedure_ordinal)] = {"name": proc.name, "segment": proc.segment, "start_ea": proc.start_ea}
    return out


def classify(index_rows: List[Dict], ida: Dict, reconcile: Dict) -> List[Dict]:
    """One record per WZD id: class, ReMoM and IDA names, target name, source location. Powered by Claude."""
    by_id: Dict[str, List[Dict]] = defaultdict(list)
    for row in index_rows:
        if row["exe"] == EXE:
            by_id[row["id"]].append(row)
    entries = reconcile["entries"]
    records = []
    for remom_id in sorted(by_id):
        rows = by_id[remom_id]
        first = rows[0]
        key = (first["letter"], first["segment"], first["proc"])
        other = ("o" if first["letter"] == "s" else "s", first["segment"], first["proc"])
        entry = entries.get(remom_id, {})
        action = entry.get("action")
        functions = sorted({r["function"] for r in rows if r["function"]})
        record = {
            "id": remom_id,
            "remom_name": ", ".join(functions),
            "remom_where": ", ".join(f"{r['file']}:{r['line']}" for r in rows),
            "evidence": ", ".join(sorted({r["evidence"] for r in rows})),
            "ida_segment": "", "ida_name": "", "start_ea": "", "new_name": "", "reconciled": action or "",
        }
        target = ida.get(key)
        if target is None and action == "accept-letter":
            target = ida.get(other)
        if target:
            record.update(ida_segment=target["segment"], ida_name=target["name"], start_ea=target["start_ea"])

        if action == "skip":
            record["class"] = "skipped"
        elif all(r["evidence"] == "comment" for r in rows) and action != "name":
            record["class"] = "no-code"
        elif target is None:
            record["class"] = "wrong-letter" if other in ida else "no-ida-proc"
            if other in ida:
                record.update(ida_segment=ida[other]["segment"], ida_name=ida[other]["name"])
        elif action != "name" and any(r["evidence"].endswith("?") for r in rows):
            record["class"] = "ambiguous"
        elif action != "name" and len(functions) > 1:
            record["class"] = "multiple-functions"
        else:
            new_name = entry["name"] if action == "name" else functions[0]
            record["new_name"] = new_name
            if not IDA_NAME.match(new_name):
                record["class"] = "invalid-name"
            elif new_name == target["name"]:
                record["class"] = "same"
            else:
                record["class"] = "rename"
        records.append(record)
    check_targets(records, ida)
    return records


def check_targets(records: List[Dict], ida: Dict) -> None:
    """Mark duplicate targets, collisions with names that stay, and rename cycles. Powered by Claude."""
    renames = [r for r in records if r["class"] == "rename"]
    by_target = defaultdict(list)
    for r in renames:
        by_target[r["new_name"]].append(r)
    for target, group in by_target.items():
        if len(group) > 1:
            for r in group:
                r["class"] = "duplicate-target"
    renames = [r for r in renames if r["class"] == "rename"]
    old_names = {r["ida_name"]: r for r in renames}
    final_names = Counter()
    renamed_away = set(old_names)
    for proc in ida.values():
        if proc["name"] not in renamed_away:
            final_names[proc["name"]] += 1
    for r in renames:
        if final_names.get(r["new_name"]):
            r["class"] = "collision"


def order_renames(records: List[Dict], ida: Dict) -> List[Dict]:
    """Rename steps in a safe order: a procedure that frees a name is renamed before the one that takes it. A cycle
    (names swapped) is broken by first moving one member to <old>__rnsync_tmp; if that name already exists the member
    is marked temp-name-taken instead. Returns steps {id, old_name, new_name}. Powered by Claude."""
    existing = {p["name"] for p in ida.values()}
    while True:
        renames = [r for r in records if r["class"] == "rename"]
        old_names = {r["ida_name"]: r for r in renames}
        state: Dict[str, int] = {}
        steps: List[Dict] = []
        current_name: Dict[str, str] = {}
        blocked = None

        def visit(r):
            nonlocal blocked
            if state.get(r["id"]) == 2 or blocked:
                return
            if state.get(r["id"]) == 1:  # back at a procedure still in progress: a cycle; free its name first
                temp = r["ida_name"] + TEMP_SUFFIX
                if temp in existing:
                    blocked = r
                    return
                steps.append({"id": r["id"], "old_name": r["ida_name"], "new_name": temp})
                current_name[r["id"]] = temp
                return
            state[r["id"]] = 1
            blocker = old_names.get(r["new_name"])
            if blocker is not None and blocker["id"] != r["id"]:
                visit(blocker)
            if blocked:
                return
            state[r["id"]] = 2
            steps.append({"id": r["id"], "old_name": current_name.get(r["id"], r["ida_name"]), "new_name": r["new_name"]})

        for r in sorted(renames, key=lambda x: x["id"]):
            visit(r)
        if not blocked:
            return steps
        blocked["class"] = "temp-name-taken"


def only_in_ida(records: List[Dict], ida: Dict) -> List[Dict]:
    """IDA procedures that no ReMoM WZD id points at. Powered by Claude."""
    claimed = {(r["ida_segment"], r["ida_name"]) for r in records if r["ida_segment"]}
    return [
        {"segment": p["segment"], "name": p["name"], "start_ea": p["start_ea"], "id": key_text((EXE,) + k)}
        for k, p in sorted(ida.items())
        if (p["segment"], p["name"]) not in claimed
    ]


def stale_reconcile_entries(records: List[Dict], reconcile: Dict) -> List[str]:
    """reconcile.json entries for ids that no longer exist in the ReMoM snapshot. Powered by Claude."""
    ids = {r["id"] for r in records}
    return sorted(k for k in reconcile["entries"] if k not in ids)


def reconcile_template(problems: List[Dict]) -> Dict:
    """A reconcile.json "entries" skeleton for the problems, for the user to fill in. Powered by Claude."""
    return {
        "entries": {
            p["id"]: {
                "action": "?",
                "problem": p["class"],
                "remom_name": p["remom_name"],
                "ida_name": p["ida_name"],
                "where": p["remom_where"],
                "options": PROBLEM_HELP[p["class"]],
            }
            for p in problems
        }
    }


def write_markdown(path: Path, counts: Counter, records: List[Dict], plan: List[Dict], problems: List[Dict], stale: List[str]) -> None:
    """Human-readable report. Powered by Claude."""
    lines = ["# ReMoM vs WIZARDS.idb procedure names", "", "| Class | Count |", "|---|---|"]
    lines += [f"| {k} | {v} |" for k, v in sorted(counts.items())]
    if problems:
        lines += ["", "## Problems (halt)", "", "| Id | Problem | ReMoM | IDA | Where |", "|---|---|---|---|---|"]
        lines += [f"| {p['id']} | {p['class']} | {p['remom_name']} | {p['ida_name']} | {p['remom_where']} |" for p in problems]
    if stale:
        lines += ["", "## Stale reconcile.json entries", ""] + [f"- {s}" for s in stale]
    lines += ["", "## Rename steps (IDA name -> ReMoM name), in apply order", "", "| Id | From | To |", "|---|---|---|"]
    lines += [f"| {r['id']} | {r['old_name']} | {r['new_name']} |" for r in plan]
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def compare(index_path: Path, functions_path: Path, reconcile_path: Optional[Path], out_dir: Path) -> Dict:
    """Run the comparison, write the report, plan and (if needed) reconcile template; return a summary. Powered by Claude."""
    reconcile = load_reconcile(reconcile_path)
    ida = ida_procedures(json.loads(functions_path.read_text(encoding="utf-8")), reconcile["segment_aliases"])
    records = classify(read_index(index_path), ida, reconcile)
    plan = order_renames(records, ida)
    problems = [r for r in records if r["class"] in PROBLEM_HELP]
    extra = only_in_ida(records, ida)
    stale = stale_reconcile_entries(records, reconcile)
    counts = Counter(r["class"] for r in records)
    counts["only-in-ida"] = len(extra)

    out_dir.mkdir(parents=True, exist_ok=True)
    report = {"index": str(index_path), "functions": str(functions_path), "reconcile": str(reconcile_path or ""),
              "counts": dict(counts), "problems": problems, "stale_reconcile_entries": stale,
              "records": records, "only_in_ida": extra}
    (out_dir / "remom-vs-wzd.report.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    write_markdown(out_dir / "remom-vs-wzd.report.md", counts, records, plan, problems, stale)
    plan_payload = {"schema_version": 1, "artifact": "wzd_proc_name_sync_from_remom", "renames": plan}
    (out_dir / PLAN_NAME).write_text(json.dumps(plan_payload, indent=2) + "\n", encoding="utf-8")
    template_path = out_dir / "problems.reconcile-template.json"
    if problems:
        template_path.write_text(json.dumps(reconcile_template(problems), indent=2) + "\n", encoding="utf-8")
    elif template_path.exists():
        template_path.unlink()
    return {"counts": counts, "problems": problems, "plan": plan, "stale": stale, "out_dir": out_dir}


def print_summary(result: Dict) -> None:
    """Console summary, with each problem and its reconcile options. Powered by Claude."""
    print("ReMoM vs WIZARDS.idb: " + ", ".join(f"{k}={v}" for k, v in sorted(result["counts"].items())))
    for stale in result["stale"]:
        print(f"WARNING: reconcile.json entry for an id no longer in ReMoM: {stale}")
    if result["problems"]:
        print(f"\nHALT: {len(result['problems'])} problem(s) must be fixed in ReMoM or reconciled first:", file=sys.stderr)
        for kind in sorted({p["class"] for p in result["problems"]}):
            group = [p for p in result["problems"] if p["class"] == kind]
            print(f"\n  {kind} ({len(group)}): {PROBLEM_HELP[kind]}", file=sys.stderr)
            for p in group:
                print(f"    {p['id']}  ReMoM={p['remom_name'] or '-'}  IDA={p['ida_name'] or '-'}  at {p['remom_where']}", file=sys.stderr)
        print(f"\n  Template: {result['out_dir'] / 'problems.reconcile-template.json'}  (copy entries into "
              f"{DEFAULT_RECONCILE}, set each action)", file=sys.stderr)


def main(argv=None) -> int:
    """Command-line entry point. Powered by Claude."""
    p = argparse.ArgumentParser(description="Compare ReMoM names with WIZARDS.idb and plan the WZD renames.")
    p.add_argument("--index", type=Path, default=DEFAULT_INDEX)
    p.add_argument("--functions", type=Path, default=DEFAULT_FUNCTIONS)
    p.add_argument("--reconcile", type=Path, default=DEFAULT_RECONCILE)
    p.add_argument("--out-dir", type=Path, default=DEFAULT_OUT_DIR)
    args = p.parse_args(argv)
    result = compare(args.index, args.functions, args.reconcile, args.out_dir)
    print_summary(result)
    print(f"Report: {args.out_dir / 'remom-vs-wzd.report.md'}")
    return 3 if result["problems"] else 0


if __name__ == "__main__":
    sys.exit(main())
