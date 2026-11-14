#!/usr/bin/env python3
"""
remom_functions.py -- every ReMoM function that carries a procedure id ("// WZD o050p01"), with its file and line and
where the id is in WIZARDS.EXE / MAGIC.EXE: IDA segment, procedure ordinal, IDA address, linker (.MAP) address.

    python -B remom_name_sync/remom_functions.py --remom C:\\STU\\devel\\ReMoM

The ids, functions, files and lines come from remom_index.py (scan of ReMoM's id comments). The id names its place:
'o' + number = IDA segment ovrNNN, 's' + number = segNNN (or an alias from remom_name_sync/reconcile.json:
WZD s037 = _AIL_, s038 = SOUND; MGC s035 = _AIL_, s036 = SOUND), pNN = the NN-th procedure of that segment by address in
in/{WIZARDS,MAGIC}.idb.ida55-functions.json.

Output: in/name-snapshots/remom/remom_functions.tsv (+ .manifest.json). Columns:
  exe, id, segment, proc, function, file, line, evidence (as remom_index.tsv: definition, prototype, comment = no code
  yet; '?' = another id claims the same function), ida_address, map_address, ida_name, located (empty, or why the id
  has no procedure in IDA).
"""

from __future__ import annotations

import argparse
import json
import sys
from datetime import datetime
from pathlib import Path
from typing import Dict, List

if __package__ in (None, ""):
    sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from mom131_name_sync.mom131_functions import Layout, write_rows  # noqa: E402
from mom131_name_sync.mom131_names import GAMES, ida_linear, load_aliases, sha256_of  # noqa: E402
from remom_name_sync.remom_index import DEFAULT_REMOM, git_commit, index_rows, load_same_as, scan_remom  # noqa: E402

DEFAULT_OUT = Path("in") / "name-snapshots" / "remom" / "remom_functions.tsv"
COLUMNS = ["exe", "id", "segment", "proc", "function", "file", "line", "evidence", "ida_address", "map_address",
           "ida_name", "located"]


def ida_segment_name(exe: str, letter: str, number: int, aliases: Dict[str, str]) -> str:
    """The IDA segment an id's segment part names. Powered by Claude."""
    wanted = "%s%03d" % (letter, number)
    for name, alias in aliases.get(exe, {}).items():
        if alias == wanted:
            return name
    return ("ovr" if letter == "o" else "seg") + "%03d" % number


def place_rows(index: List[Dict], layouts: Dict[str, Layout], aliases: Dict[str, str]) -> List[Dict]:
    """remom_index rows -> remom_functions rows. Powered by Claude."""
    rows = []
    for r in index:
        exe = r["exe"]
        segment = ida_segment_name(exe, r["letter"], int(r["segment"]), aliases)
        row = {"exe": exe, "id": r["id"], "segment": segment, "proc": int(r["proc"]), "function": r["function"],
               "file": r["file"], "line": r["line"], "evidence": r["evidence"], "ida_address": "", "map_address": "",
               "ida_name": "", "located": ""}
        layout = layouts.get(exe)
        procs = layout.procs.get(segment, []) if layout else []
        if layout is None:
            row["located"] = "no IDA export"
        elif segment not in layout.segments:
            row["located"] = f"no IDA segment {segment}"
        elif not 1 <= int(r["proc"]) <= len(procs):
            row["located"] = f"{segment} has {len(procs)} procedures"
        else:
            f = procs[int(r["proc"]) - 1]
            linear = ida_linear(f["start_ea"])
            row.update(ida_address=layout.address(segment, linear), map_address=layout.map_address(segment, linear),
                       ida_name=f["name"])
        rows.append(row)
    return rows


def take_snapshot(remom: str, out_path: Path, repo_root: Path) -> List[Dict]:
    """Scan ReMoM, place each id, write remom_functions.tsv + manifest; return the rows. Powered by Claude."""
    aliases = load_aliases(repo_root / "remom_name_sync" / "reconcile.json")
    layouts, exports = {}, {}
    for exe, (_, functions_rel, _) in GAMES.items():
        path = repo_root / functions_rel
        if path.is_file():
            layouts[exe] = Layout(json.loads(path.read_text(encoding="utf-8")))
            exports[exe] = {"ida_functions": str(path), "ida_functions_sha256": sha256_of(path)}
    rows = place_rows(index_rows(scan_remom(str(remom), None, load_same_as(repo_root / "remom_name_sync" / "reconcile.json"))), layouts, aliases)
    write_rows(rows, out_path, COLUMNS)
    manifest = {"remom": str(remom), "commit": git_commit(remom),
                "taken": datetime.now().astimezone().isoformat(timespec="seconds"), "ida": exports,
                "rows": len(rows), "not_placed": sum(1 for r in rows if r["located"])}
    Path(str(out_path) + ".manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    return rows


def main(argv=None) -> int:
    """Command-line entry point. Powered by Claude."""
    p = argparse.ArgumentParser(description="Every ReMoM function with a procedure id, with its IDA / .MAP place.")
    p.add_argument("--remom", default=DEFAULT_REMOM, help=f"ReMoM checkout. Default: {DEFAULT_REMOM}")
    p.add_argument("--out", type=Path, default=DEFAULT_OUT)
    args = p.parse_args(argv)
    repo_root = Path(__file__).resolve().parent.parent
    rows = take_snapshot(args.remom, repo_root / args.out, repo_root)
    print(f"{args.out}: {len(rows)} rows, {sum(1 for r in rows if r['located'])} without an IDA procedure")
    return 0


if __name__ == "__main__":
    sys.exit(main())
