#!/usr/bin/env python3
"""
compare_mom131_remom.py -- join mom131-source-map's function names (mom131_functions.py, read from the source) to ReMoM's
(remom_name_sync/remom_index.py snapshot) on the procedure id, for WZD and MGC, and plan the renames that make
BinAlgo's source use ReMoM's names (ReMoM wins).

Each program is checked against its own files (mom131_name_sync/programs.py): WIZARDS.EXE and MAGIC.EXE may name
their own functions differently (MAGIC's Unit_Create in src/magic/WORLDGEN.C is ReMoM's Create_Unit_NewGame, WIZARDS'
is Create_Unit), but a name used in a file both programs link (src/wizards/HEAP.C, the shared headers ...) must get
the same new name in both. Renames are applied to the source all at once, so chains and swaps need no temporary names.

Classes per id: same, rename, no-code (ReMoM has only the comment), no-remom (ReMoM has no id), skipped (reconciled);
twin, twin-no-code, twin-no-remom: the other id of a function in a file both programs link (see mark_twins).
Problems (each halts: exit 3) -- fix ReMoM, or add an entry to mom131_name_sync/reconcile.json:
  ambiguous            ReMoM's tags for the id disagree (conflict?), or another id claims its function
  multiple-functions   several ReMoM functions for the id
  invalid-name         not a C identifier
  not-in-source        the mom131 name does not occur in the source's code (a stale function list)
  conflicting-targets  ids disagree about one function (a function in a shared module is one function in both
                       programs), or a name used in a shared file would end up different in WZD and MGC
  duplicate-target     two mom131 functions would get the same name
  collision            the new name is already an identifier in the source's code that is not renamed away
  significance-clash   the new name and another identifier agree in their first 32 characters (Borland -i32)
  library-name         the new name is a C keyword or an identifier of Borland C++ 3.1's own headers (_fstrcpy ...)

    python -B mom131_name_sync/compare_mom131_remom.py
"""

from __future__ import annotations

import argparse
import json
import re
import sys
from collections import Counter, defaultdict
from pathlib import Path
from typing import Dict, List, Optional

if __package__ in (None, ""):
    sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from mom131_name_sync.mom131_functions import DEFAULT_OUT as MOM131_FUNCTIONS, read_rows  # noqa: E402
from mom131_name_sync.mom131_names import LINK_SIGNIFICANT  # noqa: E402
from mom131_name_sync.source_words import WORD, read_text, split_c  # noqa: E402
from remom_name_sync.remom_index import DEFAULT_OUT as REMOM_INDEX, read_index  # noqa: E402

DEFAULT_RECONCILE = Path("mom131_name_sync") / "reconcile.json"
DEFAULT_OUT_DIR = Path("out") / "mom131-name-sync"
PLAN_NAME = "mom131_renames_from_remom.json"
WORDS_NAME = "mom131_code_words.json"
C_NAME = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*$")
ACTIONS = ("skip", "name")
HALT_EXIT_CODE = 3
DEFAULT_BORLAND_INCLUDE = Path(r"C:\STU\devel\mom131-source-map\tools\BORLANDC\INCLUDE")
C_KEYWORDS = set("""asm auto break case cdecl char const continue default do double else enum extern far float for goto
huge if int interrupt long near pascal register return short signed sizeof static struct switch typedef union unsigned
void volatile while _cs _ds _es _ss _AX _BX _CX _DX _SI _DI _BP _SP _AH _AL _BH _BL _CH _CL _DH _DL _FLAGS""".split())

PROBLEM_HELP = {
    "ambiguous": 'ReMoM\'s tags for this id disagree: "conflict?" = its tags (in .c and .h, on definitions and prototypes) name '
                 'different functions; "definition?"/"prototype?" = another id of the same EXE tags the same function. The '
                 'remom_where column lists every tag (file:line of the tag comment itself). Fix the tags in ReMoM, or reconcile: "skip" or "name".',
    "multiple-functions": 'Several ReMoM functions for this id. Fix ReMoM, or reconcile: "skip" or "name".',
    "invalid-name": 'The new name is not a C identifier. Fix ReMoM, or reconcile: "name" or "skip".',
    "not-in-source": 'The mom131 name is not in the source code (runtime library, or the .MAP is older than the source: rebuild). Reconcile: "skip".',
    "conflicting-targets": 'Ids disagree about one function, or a name in a file both programs link would differ between WZD and MGC. Make ReMoM agree, or reconcile: "skip" or "name".',
    "duplicate-target": 'Two mom131 functions would get the same name. Fix ReMoM, or reconcile all but one: "skip" or "name".',
    "collision": 'The new name is already an identifier in the source that is not renamed away. Reconcile: "name" or "skip".',
    "significance-clash": 'The new name and another identifier agree in their first 32 characters (Borland C++ -i32). Reconcile: "name" or "skip".',
    "library-name": 'The new name is a C keyword or a name in Borland C++ 3.1\'s headers (the build would break). Reconcile: "name" or "skip".',
}


MENTION_PROGRAMS = ("WZD", "MGC", "keep")


def load_reconcile(path: Optional[Path]) -> Dict:
    """{'entries': {id: {'action': 'skip'|'name', 'name'?}}, 'mentions': [{file, name, near, program}]}; raises
    ValueError on a malformed file. A mention entry says which program a doc mention of a name the two programs
    rename differently refers to (apply_mom131_renames.py): the mention of 'name' in 'file' with the text 'near'
    within 120 characters of it; program WZD, MGC, or keep (leave the name). Powered by Claude."""
    if path is None or not path.is_file():
        return {"entries": {}, "mentions": []}
    data = json.loads(path.read_text(encoding="utf-8"))
    entries = data.get("entries", {})
    if not isinstance(entries, dict):
        raise ValueError(f"{path}: 'entries' must be an object keyed by id.")
    for key, entry in entries.items():
        if not re.fullmatch(r"(WZD|MGC) [so]\d{3}p\d{2}", key):
            raise ValueError(f"{path}: entry key {key!r} is not an id like 'WZD o050p01'.")
        action = entry.get("action")
        if action not in ACTIONS:
            raise ValueError(f"{path}: {key}: action must be one of {', '.join(ACTIONS)} (got {action!r}).")
        if action == "name" and not C_NAME.match(str(entry.get("name", ""))):
            raise ValueError(f"{path}: {key}: action 'name' needs a C identifier in 'name'.")
    mentions = data.get("mentions", [])
    for m in mentions:
        if not all(m.get(k) for k in ("file", "name", "near", "program")) or m["program"] not in MENTION_PROGRAMS:
            raise ValueError(f"{path}: mention {m!r} needs file, name, near and program ({', '.join(MENTION_PROGRAMS)}).")
    return {"entries": entries, "mentions": mentions}


def classify(mom131_rows: List[Dict], remom_rows: List[Dict], reconcile: Dict, scope: "SourceScope") -> List[Dict]:
    """One record per mom131 procedure that has an id. Powered by Claude."""
    words = scope.words
    remom: Dict[str, List[Dict]] = defaultdict(list)
    for row in remom_rows:
        remom[row["id"]].append(row)
    entries = reconcile["entries"]
    records = []
    for row in mom131_rows:
        if not row["id"]:
            continue
        rows = remom.get(row["id"], [])
        entry = entries.get(row["id"], {})
        action = entry.get("action", "")
        functions = sorted({r["function"] for r in rows if r["function"]})
        record = {
            "id": row["id"], "exe": row["exe"], "mom131_name": row["function"], "module": row["module"], "line": row.get("line", ""),
            "ida_name": row["ida_name"],
            "remom_name": ", ".join(functions), "remom_where": ", ".join(f"{r['file']}:{r['line']}" for r in rows),
            "new_name": "", "reconciled": action,
        }
        if action == "skip":
            record["class"] = "skipped"
        elif not rows and action != "name":
            record["class"] = "no-remom"
        elif rows and all(r["evidence"] == "comment" for r in rows) and action != "name":
            record["class"] = "no-code"
        elif action != "name" and any(r["evidence"].endswith("?") for r in rows):
            record["class"] = "ambiguous"
        elif action != "name" and len(functions) > 1:
            record["class"] = "multiple-functions"
        else:
            new_name = entry["name"] if action == "name" else functions[0]
            record["new_name"] = new_name
            if not C_NAME.match(new_name):
                record["class"] = "invalid-name"
            elif new_name == row["function"]:
                record["class"] = "same"
            elif not words[row["exe"]].get(row["function"]):
                record["class"] = "not-in-source"
            else:
                record["class"] = "rename"
        records.append(record)
    check_targets(records, scope)
    mark_twins(records)
    return records


NAMED = ("rename", "same")
UNNAMED = ("no-remom", "no-code")


def mark_twins(records: List[Dict]) -> None:
    """A function in a file both programs link is ONE definition with a WZD and an MGC id. When one id is unnamed
    (no-remom, no-code), it takes its twin's outcome, so every function counts once:
      twin            the twin is named (rename / same): renamed with it (new_name, remom_name from the twin)
      twin-no-code    the twin's id has only a ReMoM comment
      twin-no-remom   neither id is in ReMoM (the MGC side of the pair)
    Powered by Claude."""
    by_def: Dict[tuple, List[Dict]] = defaultdict(list)
    for r in records:
        if r.get("line"):
            by_def[(r["module"], str(r["line"]))].append(r)
    for group in by_def.values():
        if len(group) != 2:
            continue
        for r, twin in ((group[0], group[1]), (group[1], group[0])):
            if r["class"] not in UNNAMED:
                continue
            if twin["class"] in NAMED:
                r.update({"class": "twin", "new_name": twin["new_name"], "remom_name": twin["remom_name"],
                          "remom_where": f"via {twin['id']}: {twin['remom_where']}"})
            elif r["class"] == "no-remom" and twin["class"] == "no-code":
                r.update({"class": "twin-no-code", "remom_where": f"via {twin['id']}: {twin['remom_where']}"})
            elif r["class"] == twin["class"] and r["exe"] == "MGC":
                r.update({"class": "twin-" + r["class"], "remom_where": f"same function as {twin['id']}"})


class SourceScope:
    """Identifiers of a checkout by program. words[exe]: code identifiers in the program's files; own_words[exe]: in
    the files only that program links; shared_words: in files both link; module_programs: file -> {programs}.
    Powered by Claude."""

    def __init__(self, words: Dict[str, Counter], own_words: Dict[str, Counter], shared_words: Counter,
                 module_programs: Dict[str, set], reserved: Optional[set] = None):
        self.words, self.own_words, self.shared_words, self.module_programs = words, own_words, shared_words, module_programs
        self.reserved = set(reserved or ()) | C_KEYWORDS

    def applies_to(self, record: Dict) -> tuple:
        """The programs a rename of this procedure changes: both for a function defined in a shared module, else
        its own program. Powered by Claude."""
        programs = self.module_programs.get(record["module"], set())
        return tuple(sorted(programs)) if len(programs) > 1 else (record["exe"],)


def check_targets(records: List[Dict], scope: SourceScope) -> None:
    """Mark conflicting-targets, duplicate-target, collision and significance-clash. Powered by Claude."""
    opinions: Dict[tuple, set] = defaultdict(set)  # (program, mom131 name) -> the names ids want ('same' keeps it)
    for r in records:
        if r["class"] in ("rename", "same"):
            for program in scope.applies_to(r):
                opinions[(program, r["mom131_name"])].add(r["new_name"])
    for r in records:
        if r["class"] in ("rename", "same"):
            wanted = set().union(*(opinions[(p, r["mom131_name"])] for p in scope.applies_to(r)))
            if len(wanted) > 1:
                r["class"] = "conflicting-targets"
                r["conflict"] = ", ".join(sorted(wanted))

    def mappings_now() -> Dict[str, Dict[str, str]]:
        """{program: {old: new}} of the records still classed rename. Powered by Claude."""
        m: Dict[str, Dict[str, str]] = {exe: {} for exe in scope.words}
        for r in records:
            if r["class"] == "rename":
                for program in scope.applies_to(r):
                    m[program][r["mom131_name"]] = r["new_name"]
        return m

    mappings = mappings_now()
    for r in records:  # a name in a shared file must end up the same for both programs
        if r["class"] != "rename" or not scope.shared_words.get(r["mom131_name"]):
            continue
        targets = {}
        for program in scope.words:
            if r["mom131_name"] in mappings[program]:
                targets[program] = mappings[program][r["mom131_name"]]
            elif scope.own_words[program].get(r["mom131_name"]):  # the program has its own, unrenamed one
                targets[program] = r["mom131_name"]
        if len(set(targets.values())) > 1:
            r["class"] = "conflicting-targets"
            r["conflict"] = ", ".join(f"{exe}: {t}" for exe, t in sorted(targets.items()))
    for exe, words in scope.words.items():
        mapping = mappings_now()[exe]
        renames = [r for r in records if r["class"] == "rename" and exe in scope.applies_to(r)]
        by_target: Dict[str, set] = defaultdict(set)
        for old, new in mapping.items():
            by_target[new].add(old)
        for r in renames:
            if r["new_name"] in scope.reserved:
                r["class"] = "library-name"
            elif len(by_target[r["new_name"]]) > 1:
                r["class"] = "duplicate-target"
            elif words.get(r["new_name"]) and r["new_name"] not in mapping:
                r["class"] = "collision"
        renames = [r for r in renames if r["class"] == "rename"]
        final = (set(words) - set(mapping)) | set(mapping.values())
        by_prefix: Dict[str, set] = defaultdict(set)
        for name in final:
            if len(name) >= LINK_SIGNIFICANT:
                by_prefix[name[:LINK_SIGNIFICANT]].add(name)
        for r in renames:
            clash = by_prefix.get(r["new_name"][:LINK_SIGNIFICANT], set()) - {r["new_name"]}
            if clash:
                r["class"] = "significance-clash"
                r["conflict"] = ", ".join(sorted(clash))


def plan(records: List[Dict], scope: SourceScope) -> List[Dict]:
    """The rename plan: one step per mom131 function, with the programs it changes and the ids that ask for it.
    Powered by Claude."""
    steps: Dict[tuple, Dict] = {}
    for r in records:
        if r["class"] == "rename":
            programs = scope.applies_to(r)
            step = steps.setdefault((programs, r["mom131_name"]), {"programs": list(programs), "old_name": r["mom131_name"],
                                                                    "new_name": r["new_name"], "ids": [], "module": r["module"]})
            step["ids"].append(r["id"])
    return [steps[k] for k in sorted(steps, key=lambda k: (k[1], k[0]))]


def mappings_from_plan(steps: List[Dict]) -> Dict[str, Dict[str, str]]:
    """{program: {old: new}} from plan steps. Powered by Claude."""
    m: Dict[str, Dict[str, str]] = defaultdict(dict)
    for step in steps:
        for program in step["programs"]:
            m[program][step["old_name"]] = step["new_name"]
    return dict(m)


def problems_of(records: List[Dict]) -> List[Dict]:
    """Records whose class is a problem kind. Powered by Claude."""
    return [r for r in records if r["class"] in PROBLEM_HELP]


def stale_reconcile_entries(records: List[Dict], reconcile: Dict) -> List[str]:
    """Reconcile entries for ids no mom131 procedure has. Powered by Claude."""
    ids = {r["id"] for r in records}
    return sorted(k for k in reconcile["entries"] if k not in ids)


def reconcile_template(problems: List[Dict]) -> Dict:
    """A reconcile.json skeleton with one entry per problem ("action": "?"). Powered by Claude."""
    return {"entries": {
        p["id"]: {"action": "?", "problem": p["class"], "mom131": p["mom131_name"], "remom": p["remom_name"],
                  "where": p["remom_where"], **({"conflict": p["conflict"]} if p.get("conflict") else {})}
        for p in problems
    }}


def write_markdown(path: Path, counts: Counter, steps: List[Dict], problems: List[Dict], stale: List[str]) -> None:
    """Human-readable report. Powered by Claude."""
    lines = ["# mom131-source-map vs ReMoM (ReMoM wins)", "", "| Class | Count |", "|---|---|"]
    lines += [f"| {k} | {v} |" for k, v in sorted(counts.items())]
    lines += ["", "One row per procedure id. A function in a file both programs link has a WZD and an MGC id; the twin-* "
              "rows are the second id of such a function (twin: renamed with the other id), so no-code and no-remom "
              "count functions."]
    lines += ["", f"## Problems ({len(problems)})", ""]
    lines += [f"- `{p['id']}` **{p['class']}**: mom131 `{p['mom131_name']}` -> ReMoM `{p['remom_name']}`"
              + (f" (conflict: {p['conflict']})" if p.get("conflict") else "") + f" at {p['remom_where']}" for p in problems]
    if stale:
        lines += ["", "## Stale reconcile entries", ""] + [f"- `{s}`" for s in stale]
    lines += ["", f"## Renames ({len(steps)})", "", "| programs | mom131 | ReMoM | ids | module |", "|---|---|---|---|---|"]
    lines += [f"| {'+'.join(s['programs'])} | `{s['old_name']}` | `{s['new_name']}` | {', '.join(s['ids'])} | {s['module']} |" for s in steps]
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


SYSTEM_INCLUDE = re.compile(r'^\s*#\s*include\s*[<"]([^>"]+)[>"]', re.MULTILINE)


def included_headers(source: Path, include_dir: Path) -> List[Path]:
    """The Borland headers the checkout's sources include (#include <...>), with the headers those include, found in
    include_dir (case-insensitive). Powered by Claude."""
    by_name = {p.relative_to(include_dir).as_posix().upper(): p for p in include_dir.rglob("*") if p.is_file()}
    todo = []
    for path in (source / "src").rglob("*"):
        if path.suffix.upper() in (".C", ".H"):
            todo += re.findall(r'^\s*#\s*include\s*<([^>]+)>', read_text(path), re.MULTILINE)
    found: Dict[str, Path] = {}
    while todo:
        key = todo.pop().replace("\\", "/").upper()
        if key in found or key not in by_name:
            continue
        found[key] = by_name[key]
        todo += SYSTEM_INCLUDE.findall(read_text(by_name[key]))
    return sorted(found.values())


def borland_names(include_dir: Optional[Path], source: Optional[Path] = None) -> set:
    """The identifiers in the code (not comments or strings) of the Borland C++ headers the source includes -- all
    headers when source is None; empty when the folder is missing. A name only in a comment, or only in a header no
    module includes (DIR in DIRENT.H), cannot break the build. Powered by Claude."""
    names: set = set()
    if not (include_dir and include_dir.is_dir()):
        return names
    headers = included_headers(source, include_dir) if source else [p for p in include_dir.rglob("*") if p.suffix.upper() == ".H"]
    for path in headers:
        for kind, chunk in split_c(read_text(path)):
            if kind == "code":
                names |= set(WORD.findall(chunk))
    return names


def source_scope(source: Path, borland_include: Optional[Path] = DEFAULT_BORLAND_INCLUDE) -> SourceScope:
    """Scan a checkout's identifiers by program. Powered by Claude."""
    from mom131_name_sync.programs import file_programs, program_files
    from mom131_name_sync.source_words import code_words
    files = program_files(source)
    by_file = file_programs(files)
    words = {exe: code_words(source, [source / f for f in sorted(names)]) for exe, names in files.items()}
    own = {exe: code_words(source, [source / f for f in sorted(names) if len(by_file[f]) == 1]) for exe, names in files.items()}
    shared = code_words(source, [source / f for f, programs in sorted(by_file.items()) if len(programs) > 1])
    reserved = borland_names(borland_include, source)
    if borland_include and not reserved:
        print(f"WARNING: no Borland headers in {borland_include}; library names are not checked.", file=sys.stderr)
    return SourceScope(words, own, shared, by_file, reserved)


def compare(mom131_functions: Path, remom_index: Path, reconcile_path: Optional[Path], out_dir: Path, source: Path,
            borland_include: Optional[Path] = DEFAULT_BORLAND_INCLUDE) -> Dict:
    """Run the comparison and write the report, the plan and the problem template. Powered by Claude."""
    scope = source_scope(source, borland_include)
    reconcile = load_reconcile(reconcile_path)
    if not mom131_functions.is_file():
        raise FileNotFoundError(f"{mom131_functions} does not exist; run mom131_name_sync/mom131_functions.py first.")
    mom131_rows = [dict(r, module=r["file"]) for r in read_rows(mom131_functions) if r["id"]]
    records = classify(mom131_rows, read_index(remom_index), reconcile, scope)
    steps = plan(records, scope)
    problems = problems_of(records)
    stale = stale_reconcile_entries(records, reconcile)
    counts = Counter(r["class"] for r in records)
    out_dir.mkdir(parents=True, exist_ok=True)
    (out_dir / "mom131-vs-remom.report.json").write_text(json.dumps({"counts": counts, "records": records, "stale_reconcile": stale}, indent=2) + "\n", encoding="utf-8")
    (out_dir / PLAN_NAME).write_text(json.dumps({"schema_version": 1, "source": str(source), "renames": steps}, indent=2) + "\n", encoding="utf-8")
    write_markdown(out_dir / "mom131-vs-remom.report.md", counts, steps, problems, stale)
    template = out_dir / "problems.reconcile-template.json"
    if problems:
        template.write_text(json.dumps(reconcile_template(problems), indent=2) + "\n", encoding="utf-8")
    elif template.exists():
        template.unlink()
    return {"counts": counts, "plan": steps, "problems": problems, "stale": stale, "out_dir": out_dir}


def print_summary(result: Dict) -> None:
    """Counts, then every problem with its options. Powered by Claude."""
    print("mom131 vs ReMoM: " + ", ".join(f"{k}={v}" for k, v in sorted(result["counts"].items())))
    print(f"Rename plan: {len(result['plan'])} function(s)")
    for s in result["stale"]:
        print(f"  stale reconcile entry (no such id): {s}", file=sys.stderr)
    problems = result["problems"]
    if not problems:
        return
    print(f"\nHALT: {len(problems)} problem(s) must be fixed in ReMoM or reconciled first:", file=sys.stderr)
    by_kind: Dict[str, List[Dict]] = defaultdict(list)
    for p in problems:
        by_kind[p["class"]].append(p)
    for kind, items in sorted(by_kind.items()):
        print(f"\n  {kind} ({len(items)}): {PROBLEM_HELP[kind]}", file=sys.stderr)
        for p in items:
            extra = f"  conflict: {p['conflict']}" if p.get("conflict") else ""
            print(f"    {p['id']}  mom131={p['mom131_name']}  ReMoM={p['remom_name']}{extra}  at {p['remom_where']}", file=sys.stderr)
    print(f"\n  Template: {result['out_dir'] / 'problems.reconcile-template.json'}  (copy entries into {DEFAULT_RECONCILE}, set each action)", file=sys.stderr)


def main(argv=None) -> int:
    """Command-line entry point. Powered by Claude."""
    from mom131_name_sync.mom131_names import DEFAULT_SOURCE
    repo_root = Path(__file__).resolve().parent.parent
    p = argparse.ArgumentParser(description="Compare mom131-source-map names with ReMoM's and plan the renames.")
    p.add_argument("--source", type=Path, default=Path(DEFAULT_SOURCE))
    p.add_argument("--mom131-functions", type=Path, default=repo_root / MOM131_FUNCTIONS)
    p.add_argument("--remom-index", type=Path, default=repo_root / REMOM_INDEX)
    p.add_argument("--reconcile", type=Path, default=repo_root / DEFAULT_RECONCILE)
    p.add_argument("--out-dir", type=Path, default=repo_root / DEFAULT_OUT_DIR)
    p.add_argument("--borland-include", type=Path, default=DEFAULT_BORLAND_INCLUDE)
    args = p.parse_args(argv)
    result = compare(args.mom131_functions, args.remom_index, args.reconcile, args.out_dir, args.source, args.borland_include)
    print_summary(result)
    return HALT_EXIT_CODE if result["problems"] else 0


if __name__ == "__main__":
    sys.exit(main())
