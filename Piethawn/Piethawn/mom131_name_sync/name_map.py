#!/usr/bin/env python3
"""
name_map.py -- the function-name map between mom131-source-map and ReMoM, to navigate from either side.

    python -B mom131_name_sync/name_map.py                 (re-takes both function lists first)
    python -B mom131_name_sync/name_map.py --use-existing  (joins the lists already in in/name-snapshots/)

Joins in/name-snapshots/mom131/mom131_functions.tsv (mom131_functions.py) and
in/name-snapshots/remom/remom_functions.tsv (remom_name_sync/remom_functions.py) on program + procedure id.

Output: in/name-snapshots/mom131-remom.name-map.tsv, one row per procedure (a second row when ReMoM has two functions
for one id; mom131 functions without an id get their own rows). Columns:
  exe, id, segment, proc, ida_address, map_address, ida_name   the procedure (in IDA, and in the .MAP's terms)
  mom131_function, mom131_file, mom131_line                    BinAlgo's source
  remom_function, remom_file, remom_line, remom_evidence       ReMoM's source
  remom_via  when the ReMoM columns come from the twin: the other program's id of the same definition (a function
             in a file both programs link has a WZD and an MGC id; ReMoM often tags only one)
  names    same          both name it alike
           different     both have it, named differently
           remom-no-code ReMoM has only the id comment (remom_file / line = the comment)
           mom131-only   no ReMoM id for it, nor for its twin
           remom-only    ReMoM has the id, mom131-source-map has no function there
Sorted by exe, then IDA address (the order of the EXE; overlays after the resident code); sort by either file column
to go from that side.
"""

from __future__ import annotations

import argparse
import sys
from collections import Counter, defaultdict
from pathlib import Path
from typing import Dict, List

if __package__ in (None, ""):
    sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from mom131_name_sync import mom131_functions  # noqa: E402
from mom131_name_sync.mom131_functions import read_rows, write_rows  # noqa: E402
from mom131_name_sync.mom131_names import DEFAULT_SOURCE, ida_linear  # noqa: E402
from remom_name_sync import remom_functions  # noqa: E402
from remom_name_sync.remom_index import DEFAULT_REMOM  # noqa: E402

DEFAULT_OUT = Path("in") / "name-snapshots" / "mom131-remom.name-map.tsv"
COLUMNS = ["exe", "id", "segment", "proc", "ida_address", "map_address", "ida_name",
           "mom131_function", "mom131_file", "mom131_line",
           "remom_function", "remom_file", "remom_line", "remom_evidence", "remom_via", "names"]
PLACE = ["segment", "proc", "ida_address", "map_address", "ida_name"]


def names_class(m: Dict, r: Dict) -> str:
    """The 'names' column for one joined row. Powered by Claude."""
    if m and r:
        if not r["function"]:
            return "remom-no-code"
        return "same" if m["function"] == r["function"] else "different"
    if m:
        return "mom131-only"
    return "remom-only" if r["function"] else "remom-no-code"


def join(mom131_rows: List[Dict], remom_rows: List[Dict]) -> List[Dict]:
    """The map rows. Powered by Claude."""
    remom = defaultdict(list)
    for r in remom_rows:
        remom[(r["exe"], r["id"])].append(r)
    out, seen = [], set()
    for m in mom131_rows:
        key = (m["exe"], m["id"])
        matches = remom.get(key, []) if m["id"] else []
        seen.add(key)
        for r in matches or [None]:
            out.append(row(m, r))
    for key, rs in remom.items():
        if key not in seen:
            for r in rs:
                out.append(row(None, r))
    fill_from_twins(out)
    out.sort(key=lambda x: (x["exe"], x["ida_address"] == "", ida_linear(x["ida_address"]) if x["ida_address"] else 0,
                            x["mom131_file"], int(x["mom131_line"] or 0)))
    return out


REMOM_FIELDS = ("remom_function", "remom_file", "remom_line", "remom_evidence")


def fill_from_twins(rows: List[Dict]) -> None:
    """A row with no ReMoM id whose definition (mom131 file + line) is also the other program's procedure takes that
    twin's ReMoM columns (its function, or its comment-only id), with remom_via = the twin's id. Powered by Claude."""
    by_def = defaultdict(list)
    for r in rows:
        if r["mom131_file"]:
            by_def[(r["mom131_file"], str(r["mom131_line"]))].append(r)
    for group in by_def.values():
        for r in group:
            if r["remom_function"]:
                continue
            if r["remom_file"]:  # its own id comment, no code: keep it
                continue
            twins = [t for t in group if t["exe"] != r["exe"] and t["remom_file"] and not t.get("remom_via")]
            if not twins:
                continue
            twin = sorted(twins, key=lambda t: not t["remom_function"])[0]
            r.update({k: twin[k] for k in REMOM_FIELDS})
            r["remom_via"] = twin["id"]
            r["names"] = twin["names"] if not twin["remom_function"] else (
                "same" if r["mom131_function"] == twin["remom_function"] else "different")


def row(m, r) -> Dict:
    """One map row from a mom131 row and/or a ReMoM row. Powered by Claude."""
    src = m or r
    out = {"exe": src["exe"], "id": src["id"]}
    for field in PLACE:
        out[field] = (m or {}).get(field) or (r or {}).get(field) or ""
    out.update(mom131_function=m["function"] if m else "", mom131_file=m["file"] if m else "",
               mom131_line=m["line"] if m else "", remom_function=r["function"] if r else "",
               remom_file=r["file"] if r else "", remom_line=r["line"] if r else "",
               remom_evidence=r["evidence"] if r else "", remom_via="", names=names_class(m, r))
    return out


def main(argv=None) -> int:
    """Command-line entry point. Powered by Claude."""
    p = argparse.ArgumentParser(description="Function-name map mom131-source-map <-> ReMoM.")
    p.add_argument("--source", type=Path, default=Path(DEFAULT_SOURCE), help=f"mom131-source-map checkout. Default: {DEFAULT_SOURCE}")
    p.add_argument("--remom", default=DEFAULT_REMOM, help=f"ReMoM checkout. Default: {DEFAULT_REMOM}")
    p.add_argument("--use-existing", action="store_true", help="Join the function lists already written.")
    p.add_argument("--out", type=Path, default=DEFAULT_OUT)
    args = p.parse_args(argv)
    repo_root = Path(__file__).resolve().parent.parent
    m_path, r_path = repo_root / mom131_functions.DEFAULT_OUT, repo_root / remom_functions.DEFAULT_OUT
    if not args.use_existing:
        mom131_functions.take_snapshot(args.source, m_path, repo_root)
        remom_functions.take_snapshot(args.remom, r_path, repo_root)
    rows = join(read_rows(m_path), read_rows(r_path))
    write_rows(rows, repo_root / args.out, COLUMNS)
    counts = Counter((r["exe"], r["names"]) for r in rows)
    print(f"{args.out}: {len(rows)} rows")
    for (exe, names), n in sorted(counts.items()):
        print(f"  {exe} {names:14} {n}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
