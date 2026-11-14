#!/usr/bin/env python3
"""
mom131_names.py -- snapshot of the function names in BinAlgo's mom131-source-map, keyed by the same procedure id ReMoM
uses ("WZD o050p01"), from the TLINK .MAP a build writes and the IDA function export.

    python -B mom131_name_sync/mom131_names.py --source C:\\STU\\devel\\mom131-bridge

The .MAP's "Publics by Value" give 'ssss:oooo _Name'. Each public converts to an IDA address:
  resident code   MAP ssss:oooo -> IDA linear (ssss + 0x1000) * 16 + oooo, inside a segNNN (or _AIL_, SOUND ...)
  overlay code    MAP ssss is a STUBSEG: IDA stubNNN starts at paragraph ssss + 0x1000, and the code is in ovrNNN at
                  the same offset oooo
A public that lands on an IDA function start names that procedure; its id is the segment (s = segNNN, o = ovrNNN, or a
segment_aliases / MGC_segment_aliases entry from remom_name_sync/reconcile.json) and its 1-based position in the segment by
address -- the ordinal Part A and Part B use. Publics that land elsewhere (data, the runtime's internals) are counted.

Output: in/name-snapshots/mom131/{WZD,MGC}.names.tsv, columns: exe, id, ida_segment, ida_start, ida_name, public,
function (the public without the C underscore), module (the source file, relative to the checkout), map_address.
<out>.manifest.json records the checkout, its git commit, the .MAP and IDA export sha256 and the counts.
"""

from __future__ import annotations

import argparse
import csv
import hashlib
import json
import os
import re
import subprocess
import sys
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Optional, Tuple

if __package__ in (None, ""):
    sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

DEFAULT_SOURCE = r"C:\STU\devel\mom131-bridge"
DEFAULT_OUT_DIR = Path("in") / "name-snapshots" / "mom131"
DEFAULT_RECONCILE = Path("remom_name_sync") / "reconcile.json"
GAMES = {  # exe -> (MAP path under the checkout, IDA export, source folder)
    "WZD": (Path("work") / "W" / "WIZARDS.MAP", Path("in") / "WIZARDS.idb.ida55-functions.json", "src/wizards"),
    "MGC": (Path("work") / "L" / "MAGIC.MAP", Path("in") / "MAGIC.idb.ida55-functions.json", "src/magic"),
}
COLUMNS = ["exe", "id", "ida_segment", "ida_start", "ida_name", "public", "function", "module", "map_address"]
IDA_BASE_PARAGRAPH = 0x1000
OBJECT_ALIASES = {"S2FC1": "AIL"}  # build/build.py links src/ail/AIL.ASM into WIZARDS.EXE as S2FC1.OBJ
LINK_SIGNIFICANT = 32  # Borland C++ 3.1 keeps 32 characters of an identifier (-i32); TLINK adds the C underscore

PUBLIC_LINE = re.compile(r"^\s*([0-9A-F]{4}):([0-9A-F]{4})\s+(?:idle\s+|Abs\s+)?(\S+)\s*$")
SEGMENT_LINE = re.compile(r"^\s*([0-9A-F]{5})H\s+([0-9A-F]{5})H\s+([0-9A-F]{5})H\s+(\S+)\s+(\S+)\s*$")


def ida_linear(address: str) -> int:
    """IDA 'SSSS:OOOO' (or IDA 5.5's '0:000LLLLL' linear form) -> linear address. Powered by Claude."""
    seg, off = address.split(":")
    if int(seg, 16) == 0 and len(off) > 4:
        return int(off, 16)
    return int(seg, 16) * 16 + int(off, 16)


def parse_map(path: Path) -> Tuple[List[Tuple[int, int, str]], List[Dict]]:
    """(publics [(paragraph, offset, name)] from 'Publics by Value', segments [{start, length, segment, cls}] from the
    segment list at the top (' Start  Stop  Length Name  Class', linear addresses; /s and /m maps both have it).
    Powered by Claude."""
    publics, segments, section = [], [], None
    for line in path.read_text(encoding="latin-1").splitlines():
        if "Publics by Value" in line:
            section = "value"
            continue
        if "Publics by Name" in line:
            section = "name"
            continue
        if "Detailed map of segments" in line or line.startswith("Program entry point"):
            section = "other"
            continue
        if section == "value":
            m = PUBLIC_LINE.match(line)
            if m:
                publics.append((int(m.group(1), 16), int(m.group(2), 16), m.group(3)))
        elif section is None:
            m = SEGMENT_LINE.match(line)
            if m:
                segments.append({"start": int(m.group(1), 16), "length": int(m.group(3), 16),
                                 "segment": m.group(4), "cls": m.group(5)})
    if not publics:
        raise ValueError(f"{path}: no 'Publics by Value' entries found; is this a TLINK .MAP with /m?")
    return publics, segments


class IdaLayout:
    """IDA segments and function starts from an ida55-functions.json payload. Powered by Claude."""

    def __init__(self, payload: Dict):
        self.segments = []
        for s in payload.get("segments", []):
            self.segments.append({"name": s["name"], "start": ida_linear(s["start"]), "end": ida_linear(s["end"])})
        self.by_name = {s["name"]: s for s in self.segments}
        self.stub_by_paragraph = {s["start"] // 16: s for s in self.segments if s["name"].startswith("stub") and s["start"] % 16 == 0}
        per_segment: Dict[str, List[Tuple[int, Dict]]] = {}
        for f in payload.get("functions", []):
            per_segment.setdefault(f["segment"], []).append((ida_linear(f["start_ea"]), f))
        self.functions: Dict[int, Tuple[str, int, Dict]] = {}  # linear start -> (segment, ordinal, function)
        for seg, items in per_segment.items():
            for ordinal, (linear, f) in enumerate(sorted(items, key=lambda x: x[0]), 1):
                self.functions[linear] = (seg, ordinal, f)

    def convert(self, paragraph: int, offset: int) -> Tuple[Optional[int], str]:
        """MAP paragraph:offset -> (IDA linear address or None, 'overlay'/'resident'/reason). Powered by Claude."""
        stub = self.stub_by_paragraph.get(paragraph + IDA_BASE_PARAGRAPH)
        if stub is not None:
            ovr = self.by_name.get("ovr" + stub["name"][4:])
            if ovr is None:
                return None, f"no ovr segment for {stub['name']}"
            return ovr["start"] + offset, "overlay"
        return (paragraph + IDA_BASE_PARAGRAPH) * 16 + offset, "resident"


def segment_id(exe: str, ida_segment: str, aliases: Dict[str, str]) -> Optional[Tuple[str, int]]:
    """IDA segment -> (letter, number) of the ReMoM id, or None. Powered by Claude."""
    m = re.fullmatch(r"(seg|ovr)(\d+)", ida_segment)
    if m:
        return ("s" if m.group(1) == "seg" else "o", int(m.group(2)))
    alias = aliases.get(exe, {}).get(ida_segment)
    if alias:
        return (alias[0], int(alias[1:]))
    return None


def code_segment_for(segments: List[Dict], paragraph: int, offset: int) -> str:
    """The .MAP code segment (e.g. SAVELOAD_TEXT) that holds paragraph:offset: the overlay's STUBSEG at that
    paragraph, else the CODE segment containing the address. Powered by Claude."""
    linear = paragraph * 16 + offset
    for s in segments:
        if s["cls"] == "STUBSEG" and s["start"] == paragraph * 16 and s["segment"].endswith("_TEXT"):
            return s["segment"]
    for s in segments:
        if s["cls"] == "CODE" and s["start"] <= linear < s["start"] + s["length"]:
            return s["segment"]
    return ""


def module_path(segment: str, files: Dict[str, str], program_files) -> str:
    """Code segment NAME_TEXT -> the program's source file NAME.C / NAME.ASM (checkout-relative), or ''. Powered by Claude."""
    stem = (segment[:-5] if segment.endswith("_TEXT") else segment).strip("_")
    stem = OBJECT_ALIASES.get(stem.upper(), stem)
    candidates = [path for path in program_files if Path(path).stem.upper() == stem.upper()
                  and Path(path).suffix.upper() in (".C", ".ASM")]
    if len(candidates) == 1:
        return candidates[0]
    for ext in (".C", ".ASM"):
        if stem.upper() + ext in files:
            return files[stem.upper() + ext]
    return ""


def source_files(source: Path) -> Dict[str, str]:
    """Upper-case file name -> path relative to the checkout, for the files under src/. Powered by Claude."""
    found = {}
    for folder, _, names in os.walk(source / "src"):
        for name in names:
            rel = os.path.relpath(os.path.join(folder, name), source).replace(os.sep, "/")
            found.setdefault(name.upper(), rel)
    return found


def full_name(name: str, words) -> str:
    """A public cut to 32 characters -> the one longer identifier in the source that starts with it (else the
    name as is). Powered by Claude."""
    if len(name) != LINK_SIGNIFICANT or name in words:
        return name
    longer = sorted(w for w in words if len(w) > LINK_SIGNIFICANT and w.startswith(name))
    return longer[0] if len(longer) == 1 else name


def build_rows(exe: str, map_path: Path, payload: Dict, source: Path, aliases: Dict[str, str], words=None,
               program_files=()) -> Tuple[List[Dict], Dict]:
    """Snapshot rows for one EXE plus counts. words: the source's code identifiers (for names TLINK cut short).
    Powered by Claude."""
    publics, segments = parse_map(map_path)
    layout = IdaLayout(payload)
    files = source_files(source) if source.is_dir() else {}
    words = words or ()
    rows, counts = [], {"publics": len(publics), "procedures": 0, "not_a_function_start": 0, "no_id": 0}
    seen = set()
    for paragraph, offset, public in publics:
        linear, how = layout.convert(paragraph, offset)
        hit = layout.functions.get(linear) if linear is not None else None
        if hit is None:
            counts["not_a_function_start"] += 1
            continue
        ida_segment, ordinal, f = hit
        if linear in seen:  # two publics at one address (an alias label): keep the first
            continue
        seen.add(linear)
        sid = segment_id(exe, ida_segment, aliases)
        if sid is None:
            counts["no_id"] += 1
        module = module_path(code_segment_for(segments, paragraph, offset), files, program_files)
        rows.append({
            "exe": exe,
            "id": "%s %s%03dp%02d" % (exe, sid[0], sid[1], ordinal) if sid else "",
            "ida_segment": ida_segment, "ida_start": f["start_ea"], "ida_name": f["name"], "public": public,
            "function": full_name(public[1:] if public.startswith("_") else public, words),
            "module": module, "map_address": "%04X:%04X" % (paragraph, offset),
        })
        counts["procedures"] += 1
    rows.sort(key=lambda r: (r["id"] == "", r["id"], r["ida_start"]))
    return rows, counts


def write_rows(rows: List[Dict], out_path: Path) -> None:
    """Write the snapshot TSV (tab-separated, header row, LF). Powered by Claude."""
    out_path.parent.mkdir(parents=True, exist_ok=True)
    with open(out_path, "w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, fieldnames=COLUMNS, delimiter="\t", lineterminator="\n")
        w.writeheader()
        w.writerows(rows)


def read_rows(path: Path) -> List[Dict]:
    """Read a snapshot TSV back. Powered by Claude."""
    with open(path, newline="", encoding="utf-8") as f:
        return list(csv.DictReader(f, delimiter="\t"))


def sha256_of(path: Path) -> str:
    """sha256 hex digest of a file. Powered by Claude."""
    return hashlib.sha256(path.read_bytes()).hexdigest()


def git_commit(root: Path) -> str:
    """HEAD commit of a checkout plus '-dirty' with uncommitted changes, or '' without git. Powered by Claude."""
    try:
        head = subprocess.run(["git", "-C", str(root), "rev-parse", "HEAD"], capture_output=True, text=True, check=True).stdout.strip()
        dirty = subprocess.run(["git", "-C", str(root), "status", "--porcelain", "--", "src"], capture_output=True, text=True, check=True).stdout.strip()
        return head + ("-dirty" if dirty else "")
    except (OSError, subprocess.CalledProcessError):
        return ""


def load_aliases(path: Path) -> Dict[str, Dict[str, str]]:
    """{exe: {IDA segment: sNNN}} from the ReMoM reconcile file: 'segment_aliases' for WZD (WZD-only tools read it as
    it is), 'MGC_segment_aliases' for MGC; empty when the file does not exist. Powered by Claude."""
    if not path.is_file():
        return {"WZD": {}, "MGC": {}}
    data = json.loads(path.read_text(encoding="utf-8"))
    return {"WZD": dict(data.get("segment_aliases", {})), "MGC": dict(data.get("MGC_segment_aliases", {}))}


def take_snapshot(source: Path, out_dir: Path, repo_root: Path, reconcile: Path = DEFAULT_RECONCILE,
                  exes=("WZD", "MGC")) -> Dict[str, List[Dict]]:
    """Write {exe}.names.tsv + manifest for each EXE; return {exe: rows}. Powered by Claude."""
    from mom131_name_sync.programs import program_files as programs_of
    from mom131_name_sync.source_words import code_words
    aliases = load_aliases(repo_root / reconcile)
    by_program = programs_of(source)
    result = {}
    for exe in exes:
        map_rel, functions_rel, _ = GAMES[exe]
        map_path, functions = source / map_rel, repo_root / functions_rel
        if not map_path.is_file():
            raise FileNotFoundError(f"{map_path} does not exist; build the checkout first (build/build.py).")
        payload = json.loads(functions.read_text(encoding="utf-8"))
        words = code_words(source, [source / f for f in sorted(by_program[exe])])
        rows, counts = build_rows(exe, map_path, payload, source, aliases, words, by_program[exe])
        out = out_dir / f"{exe}.names.tsv"
        write_rows(rows, out)
        manifest = {
            "source": str(source), "commit": git_commit(source), "map": str(map_path), "map_sha256": sha256_of(map_path),
            "ida_functions": str(functions), "ida_functions_sha256": sha256_of(functions),
            "taken": datetime.now().astimezone().isoformat(timespec="seconds"), "counts": counts,
        }
        Path(str(out) + ".manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
        result[exe] = rows
    return result


def main(argv=None) -> int:
    """Command-line entry point. Powered by Claude."""
    p = argparse.ArgumentParser(description="Snapshot mom131-source-map function names keyed by ReMoM procedure id.")
    p.add_argument("--source", type=Path, default=Path(DEFAULT_SOURCE), help=f"Built checkout. Default: {DEFAULT_SOURCE}")
    p.add_argument("--out-dir", type=Path, default=DEFAULT_OUT_DIR)
    p.add_argument("--exe", choices=sorted(GAMES), action="append", help="Only this EXE (repeatable). Default: both.")
    args = p.parse_args(argv)
    repo_root = Path(__file__).resolve().parent.parent
    result = take_snapshot(args.source, repo_root / args.out_dir, repo_root, exes=tuple(args.exe or ("WZD", "MGC")))
    for exe, rows in result.items():
        manifest = json.loads((repo_root / args.out_dir / f"{exe}.names.tsv.manifest.json").read_text(encoding="utf-8"))
        print(f"{exe}: {len(rows)} procedures named; counts {manifest['counts']}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
