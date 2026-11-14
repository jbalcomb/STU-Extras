#!/usr/bin/env python3
"""
mom131_functions.py -- every function defined in BinAlgo's mom131-source-map, read from the source itself, with where
it is in WIZARDS.EXE / MAGIC.EXE: IDA segment, procedure ordinal (the id ReMoM uses, "WZD o050p01"), IDA address,
linker (.MAP) address.

    python -B mom131_name_sync/mom131_functions.py --source C:\\STU\\devel\\mom131-bridge

The source is a fixed, byte-exact recreation, so the source says where everything is:
  src/wizards/MODULES.TSV, src/magic/MODULES.TSV   each module of the program and its code segment (the IDA paragraph:
                                                   SAVELOAD.C 4558 in WIZARDS.EXE, 3c7e in MAGIC.EXE)
  src/ail/AIL.ASM                                  the IDA segment _AIL_ (not in MODULES.TSV; linked as S2FC1 / AIL)
  the module's function definitions                in file order = the procedures of that IDA segment by address
                                                   (Borland C++ and TASM emit code in source order)

IDA only supplies the segment layout and the procedure starts (in/{WIZARDS,MAGIC}.idb.ida55-functions.json). Where a
module's definition count differs from its IDA procedure count, its public functions are placed by the build's .MAP
(work/W/WIZARDS.MAP, work/L/MAGIC.MAP), and its other functions are left without an address. When the .MAP is there,
every public function is also checked against it (column 'check'); a .MAP older than a module is not used.

Output: in/name-snapshots/mom131/mom131_functions.tsv (+ .manifest.json), one row per function per program it is
linked into. Columns:
  exe          WZD (WIZARDS.EXE) or MGC (MAGIC.EXE)
  id           ReMoM's procedure id ("WZD o050p01"); empty where the segment has no id letter
  segment      IDA segment name (seg001, ovr050, _AIL_ ...)
  proc         1-based position of the procedure in its IDA segment
  function     the name in the source
  file, line   the definition
  linkage      public, static (C), proc (assembly)
  ida_address  IDA address of the procedure start (segment:offset)
  map_address  the same place as TLINK's .MAP writes it (resident: IDA paragraph - 1000h; overlay: the stub's)
  ida_name     the procedure's name in the IDA database now
  located      order (n-th definition = n-th procedure), map (placed by the .MAP), between (n-th between two
               .MAP-placed ones = n-th procedure between them), or why it is not placed
  check        map-ok, map-differs (the .MAP puts the public elsewhere), or empty (static, or no .MAP)
"""

from __future__ import annotations

import argparse
import csv
import json
import re
import sys
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Optional, Tuple

if __package__ in (None, ""):
    sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from mom131_name_sync.mom131_names import (DEFAULT_SOURCE, GAMES, IDA_BASE_PARAGRAPH, LINK_SIGNIFICANT, IdaLayout,  # noqa: E402
                                           git_commit, ida_linear, load_aliases, parse_map, segment_id, sha256_of)
from mom131_name_sync.programs import MODULE_LISTS  # noqa: E402
from mom131_name_sync.source_words import read_text, split_c  # noqa: E402

DEFAULT_OUT = Path("in") / "name-snapshots" / "mom131" / "mom131_functions.tsv"
COLUMNS = ["exe", "id", "segment", "proc", "function", "file", "line", "linkage", "ida_address", "map_address",
           "ida_name", "located", "check"]
EXTRA_MODULES = {"src/ail/AIL.ASM": "_AIL_"}  # file -> IDA segment, for code not listed in MODULES.TSV
NOT_FUNCTION_NAMES = set("""if while for switch return sizeof far near huge pascal cdecl interrupt void int char long short
unsigned signed struct union enum const volatile static extern register auto double float _loadds _saveregs _export
_fastcall""".split())
IDENT_PAREN = re.compile(r"([A-Za-z_]\w*)\s*\(")
ASM_PROC = re.compile(r"^\s*([A-Za-z_@$?][\w@$?]*)\s+proc\b", re.IGNORECASE)
PREPROCESSOR = re.compile(r"(?m)^[ \t]*#.*$")


def c_code_only(text: str) -> str:
    """The text with comments, string / char literals and preprocessor lines blanked (newlines kept, so offsets and
    line numbers stay). Powered by Claude."""
    out = [chunk if kind == "code" else re.sub(r"[^\n]", " ", chunk) for kind, chunk in split_c(text)]
    return PREPROCESSOR.sub(lambda m: " " * len(m.group(0)), "".join(out))


def c_definitions(text: str) -> List[Tuple[str, int, str]]:
    """[(name, line, 'public'|'static')] of the function definitions in C text, in file order: a top-level '{' whose
    declaration has a parameter list and no '=' before it. Powered by Claude."""
    code = c_code_only(text)
    found, depth, start = [], 0, 0
    for i, ch in enumerate(code):
        if ch == "{":
            if depth == 0:
                decl = code[start:i]
                if "(" in decl and "=" not in decl.split("(")[0]:
                    for m in IDENT_PAREN.finditer(decl):
                        if m.group(1) not in NOT_FUNCTION_NAMES:
                            line = code.count("\n", 0, start + m.start(1)) + 1
                            static = re.search(r"\bstatic\b", decl[:m.start()]) is not None
                            found.append((m.group(1), line, "static" if static else "public"))
                            break
            depth += 1
        elif ch == "}":
            depth -= 1
            if depth == 0:
                start = i + 1
        elif ch == ";" and depth == 0:
            start = i + 1
    return found


def asm_definitions(text: str) -> List[Tuple[str, int, str]]:
    """[(name, line, 'proc')] of the PROC definitions in assembly text, in file order. Powered by Claude."""
    found = []
    for number, line in enumerate(text.splitlines(), 1):
        m = ASM_PROC.match(line.split(";")[0])
        if m:
            found.append((m.group(1), number, "proc"))
    return found


def definitions(path: Path) -> List[Tuple[str, int, str]]:
    """The function definitions of a C or assembly file. Powered by Claude."""
    text = read_text(path)
    return asm_definitions(text) if path.suffix.upper() == ".ASM" else c_definitions(text)


def modules(source: Path, exe: str) -> List[Tuple[str, str]]:
    """[(checkout-relative file, IDA segment key)] of one program: MODULES.TSV's paragraph (as 'para:XXXX') plus the
    EXTRA_MODULES segment names. Powered by Claude."""
    listing = source / MODULE_LISTS[exe]
    out = []
    for line in listing.read_text(encoding="latin-1").splitlines():
        if not line.strip() or line.startswith("#"):
            continue
        paragraph, name = [part.strip() for part in line.split("\t")[:2]]
        path = (listing.parent / name).resolve()
        out.append((path.relative_to(source.resolve()).as_posix(), "para:" + paragraph.lower()))
    for rel, segment in EXTRA_MODULES.items():
        if (source / rel).is_file():
            out.append((rel, segment))
    return out


class Layout:
    """IDA segments (with their paragraph bases) and procedures of one program. Powered by Claude."""

    def __init__(self, payload: Dict):
        self.segments: Dict[str, Dict] = {}
        for s in payload.get("segments", []):
            sel = s["start"].split(":")[0]
            start = ida_linear(s["start"])
            base = int(sel, 16) if int(sel, 16) else start // 16
            self.segments[s["name"]] = {"name": s["name"], "start": start, "end": ida_linear(s["end"]), "base": base}
        self.procs: Dict[str, List[Dict]] = {}
        for f in payload.get("functions", []):
            self.procs.setdefault(f["segment"], []).append(f)
        for items in self.procs.values():
            items.sort(key=lambda f: ida_linear(f["start_ea"]))

    def segment_for(self, key: str) -> Optional[str]:
        """'para:4558' or a segment name -> the IDA segment name. Powered by Claude."""
        if not key.startswith("para:"):
            return key if key in self.segments else None
        paragraph = int(key[5:], 16)
        hits = [s["name"] for s in self.segments.values() if s["base"] == paragraph]
        return hits[0] if len(hits) == 1 else None

    def address(self, segment: str, linear: int) -> str:
        """IDA 'SSSS:OOOO' of a linear address in a segment. Powered by Claude."""
        base = self.segments[segment]["base"]
        return "%04X:%04X" % (base, linear - base * 16)

    def map_address(self, segment: str, linear: int) -> str:
        """The .MAP's 'SSSS:OOOO' for an IDA address: resident (paragraph - 1000h), overlay (the stub's paragraph -
        1000h, offset in the overlay). Powered by Claude."""
        seg = self.segments[segment]
        if segment.startswith("ovr"):
            stub = self.segments.get("stub" + segment[3:])
            if stub is None:
                return ""
            return "%04X:%04X" % (stub["base"] - IDA_BASE_PARAGRAPH, linear - seg["start"])
        return "%04X:%04X" % (seg["base"] - IDA_BASE_PARAGRAPH, linear - seg["base"] * 16)


def map_publics(map_path: Optional[Path], ida: IdaLayout) -> Dict[str, int]:
    """{public name without the C underscore, cut to 32: IDA linear address} from a .MAP, or {} without one.
    Powered by Claude."""
    if map_path is None or not map_path.is_file():
        return {}
    publics, _ = parse_map(map_path)
    out = {}
    for paragraph, offset, name in publics:
        linear, _ = ida.convert(paragraph, offset)
        if linear is not None:
            out.setdefault(name[1:] if name.startswith("_") else name, linear)
    return out


def public_address(publics: Dict[str, int], name: str) -> Optional[int]:
    """A function's address from the .MAP publics: the full name (TASM keeps long names), else cut to 32 characters
    (Borland C++ -i32); assembly names may differ in case. Powered by Claude."""
    for key in (name, name[:LINK_SIGNIFICANT]):
        if key in publics:
            return publics[key]
    lower = {k.lower(): v for k, v in publics.items()}
    for key in (name, name[:LINK_SIGNIFICANT]):
        if key.lower() in lower:
            return lower[key.lower()]
    return None


def fill_between_anchors(defs: List[Tuple[str, int, str]], procs: List[Dict], publics: Dict[str, int],
                         by_start: Dict[int, Tuple[int, Dict]]) -> Dict[int, int]:
    """{definition index: proc ordinal} for the definitions between two .MAP-placed ones (or a module end) where the
    definitions and the procedures in that gap are equally many: the n-th of the gap is the n-th procedure.
    Powered by Claude."""
    anchors = [(-1, 0)]
    for index, (name, _, linkage) in enumerate(defs):
        mapped = public_address(publics, name) if linkage != "static" else None
        if mapped in by_start and by_start[mapped][0] > anchors[-1][1]:
            anchors.append((index, by_start[mapped][0]))
    anchors.append((len(defs), len(procs) + 1))
    out = {}
    for (d0, p0), (d1, p1) in zip(anchors, anchors[1:]):
        if d1 - d0 == p1 - p0:
            for k in range(1, d1 - d0):
                out[d0 + k] = p0 + k
    return out


def program_rows(exe: str, source: Path, payload: Dict, aliases: Dict[str, str], map_path: Optional[Path]) -> Tuple[List[Dict], Dict]:
    """Rows for one program plus counts. Powered by Claude."""
    layout, ida = Layout(payload), IdaLayout(payload)
    publics = map_publics(map_path, ida)
    rows, counts = [], {"modules": 0, "functions": 0, "by_order": 0, "by_map": 0, "by_between": 0, "not_placed": 0,
                        "map_ok": 0, "map_differs": 0, "modules_count_differs": []}
    used_segments: Dict[str, str] = {}
    for rel, key in modules(source, exe):
        counts["modules"] += 1
        defs = definitions(source / rel)
        segment = layout.segment_for(key)
        procs = layout.procs.get(segment, []) if segment else []
        if segment and segment in used_segments:
            raise ValueError(f"{exe}: {rel} and {used_segments[segment]} both map to IDA segment {segment}")
        if segment:
            used_segments[segment] = rel
        sid = segment_id(exe, segment, aliases) if segment else None
        by_start = {ida_linear(f["start_ea"]): (n, f) for n, f in enumerate(procs, 1)}
        same_count = segment is not None and len(defs) == len(procs)
        between: Dict[int, int] = {}
        if segment and not same_count:
            counts["modules_count_differs"].append(f"{rel} ({segment}: source {len(defs)}, IDA {len(procs)})")
            between = fill_between_anchors(defs, procs, publics, by_start)
        for index, (name, line, linkage) in enumerate(defs):
            row = {"exe": exe, "id": "", "segment": segment or "", "proc": "", "function": name, "file": rel,
                   "line": line, "linkage": linkage, "ida_address": "", "map_address": "", "ida_name": "",
                   "located": "", "check": ""}
            counts["functions"] += 1
            mapped = public_address(publics, name) if linkage != "static" else None
            hit = None
            if segment is None:
                row["located"] = f"no IDA segment for {key}"
            elif same_count:
                hit, row["located"] = (index + 1, procs[index]), "order"
            elif mapped in by_start:
                hit, row["located"] = by_start[mapped], "map"
            elif index in between:
                hit, row["located"] = (between[index], procs[between[index] - 1]), "between"
            else:
                row["located"] = f"count differs (source {len(defs)}, IDA {len(procs)})"
            if hit:
                ordinal, f = hit
                linear = ida_linear(f["start_ea"])
                row.update(proc=ordinal, ida_address=layout.address(segment, linear),
                           map_address=layout.map_address(segment, linear), ida_name=f["name"],
                           id="%s %s%03dp%02d" % (exe, sid[0], sid[1], ordinal) if sid else "")
                counts["by_" + row["located"]] += 1
                if mapped is not None:
                    row["check"] = "map-ok" if mapped == linear else "map-differs"
                    counts["map_ok" if mapped == linear else "map_differs"] += 1
            else:
                counts["not_placed"] += 1
            rows.append(row)
    rows.sort(key=lambda r: (r["segment"] == "", r["id"] == "", r["id"] or r["file"], int(r["proc"] or 0), int(r["line"])))
    return rows, counts


def map_is_stale(source: Path, exe: str, map_path: Path) -> bool:
    """True when the .MAP is not from this source: a public C function the source defines is not among its publics
    (renamed or reverted since the build). File times cannot tell: build/build.py dates its output 1995-03-03.
    Powered by Claude."""
    publics, _ = parse_map(map_path)
    names = {n[1:] if n.startswith("_") else n for _, _, n in publics}
    for rel, _ in modules(source, exe):
        if not rel.upper().endswith(".C"):
            continue
        for name, _, linkage in definitions(source / rel):
            if linkage == "public" and name not in names and name[:LINK_SIGNIFICANT] not in names:
                return True
    return False


def write_rows(rows: List[Dict], out_path: Path, columns=COLUMNS) -> None:
    """Write a TSV (tab-separated, header row, LF). Powered by Claude."""
    out_path.parent.mkdir(parents=True, exist_ok=True)
    with open(out_path, "w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, fieldnames=columns, delimiter="\t", lineterminator="\n")
        w.writeheader()
        w.writerows(rows)


def read_rows(path: Path) -> List[Dict]:
    """Read a TSV back as row dicts. Powered by Claude."""
    with open(path, newline="", encoding="utf-8") as f:
        return list(csv.DictReader(f, delimiter="\t"))


def take_snapshot(source: Path, out_path: Path, repo_root: Path, exes=("WZD", "MGC")) -> Tuple[List[Dict], Dict]:
    """Write mom131_functions.tsv + manifest; return (rows, manifest). Powered by Claude."""
    aliases = load_aliases(repo_root / "remom_name_sync" / "reconcile.json")
    rows, manifest = [], {"source": str(source), "commit": git_commit(source),
                          "taken": datetime.now().astimezone().isoformat(timespec="seconds"), "programs": {}}
    for exe in exes:
        map_rel, functions_rel, _ = GAMES[exe]
        functions, map_path = repo_root / functions_rel, source / map_rel
        payload = json.loads(functions.read_text(encoding="utf-8"))
        stale = map_path.is_file() and map_is_stale(source, exe, map_path)
        usable = map_path if map_path.is_file() and not stale else None
        exe_rows, counts = program_rows(exe, source, payload, aliases, usable)
        rows += exe_rows
        manifest["programs"][exe] = {
            "ida_functions": str(functions), "ida_functions_sha256": sha256_of(functions),
            "map": str(map_path) if usable else "", "map_sha256": sha256_of(map_path) if usable else "",
            "map_stale": bool(stale),
            "counts": counts,
        }
    write_rows(rows, out_path)
    Path(str(out_path) + ".manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    return rows, manifest


def main(argv=None) -> int:
    """Command-line entry point. Powered by Claude."""
    p = argparse.ArgumentParser(description="Every function of mom131-source-map with its IDA / .MAP place and ReMoM id.")
    p.add_argument("--source", type=Path, default=Path(DEFAULT_SOURCE), help=f"Checkout. Default: {DEFAULT_SOURCE}")
    p.add_argument("--out", type=Path, default=DEFAULT_OUT)
    args = p.parse_args(argv)
    repo_root = Path(__file__).resolve().parent.parent
    rows, manifest = take_snapshot(args.source, repo_root / args.out, repo_root)
    for exe, info in manifest["programs"].items():
        print(f"{exe}: {info['counts']}")
    print(f"{args.out}: {len(rows)} rows")
    return 0


if __name__ == "__main__":
    sys.exit(main())
