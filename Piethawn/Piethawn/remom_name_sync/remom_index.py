#!/usr/bin/env python3
"""
remom_index.py -- snapshot of ReMoM's procedure names: every id comment ("// WZD s161p04") in ReMoM's C sources,
with the C function that reconstructs it, its file and line.

Copied from ReMoM-Mod tools/mom_patch_diff (remom_index.py + scan_remom()/key()/key_text() from address_table.py,
commit f08ee954) so Piethawn owns it. Standard library only. Deliberate changes since: lines inside /* */ block
comments are ignored (ids and code pasted into notes), and a WZD and an MGC id comment stacked above one function both
name it (ReMoM-Mod's keeps only the last), so the output is no longer byte-identical to ReMoM-Mod's.

    python -B remom_name_sync/remom_index.py --remom C:\\STU\\devel\\ReMoM --out in/name-snapshots/remom/remom_index.tsv

Columns: exe (WZD, MGC), id, letter (s or o, as written), segment (number), proc, function, file, line, evidence.
evidence: definition (the next function definition after the comment), prototype (a prototype after the comment names
the function), comment (an id with no code yet); a trailing "?" means another id of the same EXE claims the same
function. A <out>.manifest.json records the ReMoM checkout, its git commit and the time.
"""

from __future__ import annotations

import argparse
import csv
import json
import os
import re
import subprocess
import sys
from collections import defaultdict
from datetime import datetime
from pathlib import Path

COLUMNS = ["exe", "id", "letter", "segment", "proc", "function", "file", "line", "evidence"]
SOURCE_FOLDERS = ("MoM", "MoX", "src", "STU")
DEFAULT_REMOM = r"C:\STU\devel\ReMoM"
DEFAULT_OUT = Path("in") / "name-snapshots" / "remom" / "remom_index.tsv"

ID_COMMENT = re.compile(r"^\s*//\s*(WZD|MGC)\s+([so])0*(\d+)p0*(\d+)\b")
DEFINITION = re.compile(r"^[A-Za-z_][\w \t\*]*?\b([A-Za-z_]\w*)\s*\([^;]*\)\s*(\{.*)?$")
SAME_AS = re.compile(r"^\s*//\s*===\s*//\s*(WZD|MGC)\s+([so])0*(\d+)p0*(\d+)\b")   # "// === // WZD s01p07 ...": same code
MALFORMED_ID = re.compile(r"^\s*//\s*(WZD|MGC)\s+[so]\d+[a-oq-z]\d+\b"   # "// WZD s14o03": a mistyped id
                          r"|^//\s*[so]\d+p\d+\b")                           # "// s13p16" at column 0: no WZD/MGC
PREFIX_COMMENT = re.compile(r"^/\*.*?\*/\s*(?=[A-Za-z_])")                       # "/* HACK */  int16_t F(void)"
PROTOTYPE = re.compile(r"^[A-Za-z_][\w \t\*]*?\b([A-Za-z_]\w*)\s*\([^;]*\)\s*;\s*(//.*|/\*.*)?$")


def key(exe, kind, seg, proc):
    """(exe, letter, segment number, proc number). Powered by Claude."""
    return (exe, kind, int(seg), int(proc))


def key_text(k):
    """'WZD o076p01' form of a key. Powered by Claude."""
    return "%s %s%03dp%02d" % k


def block_comment_lines(lines):
    """[bool per line]: True when the line starts inside a /* */ block comment, or its first non-blank text opens one.
    Such lines are notes: an id comment or function definition there is not real. "//" comments and "..." / '...'
    literals are honoured so a "/*" inside them does not open a block. Powered by Claude."""
    mask, inside = [], False
    for line in lines:
        text = line.lstrip()
        mask.append(inside or (text.startswith("/*") and not PREFIX_COMMENT.match(text)))
        i, quote = 0, None
        while i < len(line):
            two = line[i:i + 2]
            if inside:
                if two == "*/":
                    inside, i = False, i + 2
                    continue
            elif quote:
                if line[i] == "\\":
                    i += 2
                    continue
                if line[i] == quote:
                    quote = None
            elif two == "//":
                break
            elif two == "/*":
                inside, i = True, i + 2
                continue
            elif line[i] in "\"'":
                quote = line[i]
            i += 1
    return mask


def load_same_as(path: Path = None):
    """{id key: id key} from the 'same_as' object of remom_name_sync/reconcile.json ("MGC o050p03": "WZD o160p01"):
    procedures whose code ReMoM already has under another id, as a "// === // WZD ..." line in ReMoM would say.
    Raises ValueError on a malformed entry. Powered by Claude."""
    path = path or Path(__file__).resolve().parent / "reconcile.json"
    if not path.is_file():
        return {}
    pairs = {}
    for a, b in json.loads(path.read_text(encoding="utf-8")).get("same_as", {}).items():
        ka, kb = (re.fullmatch(r"(WZD|MGC) ([so])(\d{3})p(\d{2})", s) for s in (a, str(b)))
        if not (ka and kb):
            raise ValueError(f"{path}: same_as {a!r}: {b!r}: both must be ids like 'WZD o160p01'")
        pairs[key(*ka.groups())] = key(*kb.groups())
    return pairs


SIGNATURE_START = re.compile(r"^[A-Za-z_][\w \t\*]*?\b([A-Za-z_]\w*)\s*\(")


def signature_at(lines, i):
    """(name, kind, last line index) when lines[i] starts a top-level function signature: kind "definition" (a "{"
    follows) or "prototype" (it ends with ";"); a signature may span lines. None otherwise. Powered by Claude."""
    if not SIGNATURE_START.match(lines[i]) or "=" in lines[i].split("(")[0]:
        return None
    text, j = lines[i], i
    while text.count("(") > text.count(")") and j + 1 < len(lines) and j - i < 8:
        j += 1
        text += " " + lines[j].strip()
    text = re.sub(r"//.*$", "", text).rstrip()
    proto = PROTOTYPE.match(text)
    if proto:
        return proto.group(1), "prototype", j
    d = DEFINITION.match(text)
    if not d:
        return None
    following = next((l for l in lines[j + 1:j + 4] if l.strip()), "")
    if text.endswith("{") or following.startswith("{"):
        return d.group(1), "definition", j
    return None


def scan_remom(root, malformed=None, extra_same_as=None, misplaced=None):
    """{key: [(function, file, line, evidence)]} from ReMoM's id comments ("// WZD s161p04").

    An id comment is a tag for the function signature directly below it: only comments ("//" lines, /* */ blocks),
    blank lines and preprocessor lines may come between. Areas of code are marked with BEGIN/END notes, not with the
    tag. Id comments of different EXEs stacked above one signature all name it; a second id of the same EXE in one
    stack replaces the first (which then has only its comment). Tags are read in the .c and the .h files, on
    definitions and on prototypes; every tag of one id must name the same function.

    evidence: "definition"  a tag on the function's definition (file, line of the definition)
              "prototype"   tags only on prototypes; the definition is elsewhere (file, line of the definition)
              "same-as"     "// === // WZD s01p07 ..." under the id comment (or a reconcile.json same_as entry): the
                            procedure is the same code as that id, whose function it takes
              "comment"     an id with no code yet (file, line of the comment)
              "conflict?"   the id's tags name different functions: one row per tag
              "...?"        the function is also claimed by another id of the same EXE
    A tag with code or data (not a signature) after it binds nothing: alone, its id is "comment" (no code yet); when
    another tag of the id binds, it is appended to 'misplaced' as (file, line, id, function) -- it marks an area of
    code, which BEGIN/END notes are for. Any "?" is a problem for the compare tools. Mistyped id comments
    ("// WZD s14o03") are appended to 'malformed' as (file, line, text). Powered by Claude.
    """
    definitions = {}                                    # function name -> (file, line), every definition in the .c files
    sites = defaultdict(list)                           # key -> [(function, file, line, kind)]: tags on signatures
    loose, comments, same_as = defaultdict(list), {}, {}
    for top in SOURCE_FOLDERS:
        for folder, _, names in os.walk(os.path.join(root, top)):
            for name in sorted(names):
                if not name.endswith((".c", ".h")):
                    continue
                path = os.path.join(folder, name)
                with open(path, encoding="utf-8", errors="replace") as f:
                    lines = f.read().splitlines()
                rel = os.path.relpath(path, root).replace(os.sep, "/")
                in_block = block_comment_lines(lines)
                pending = {}                            # exe -> (key, line): the tags waiting for their signature
                listing = {}                            # exe -> line of the last tag in a run of consecutive tags
                last_id = None
                skip_to = -1
                for i, raw in enumerate(lines):
                    number = i + 1
                    if i <= skip_to or in_block[i]:
                        continue
                    line = PREFIX_COMMENT.sub("", raw) if raw.startswith("/*") else raw
                    text = line.strip()
                    if not text or text.startswith("#"):
                        continue
                    if text.startswith("//"):
                        same = SAME_AS.match(line)
                        if same and last_id:
                            same_as[last_id] = key(*same.groups())
                            if pending.get(last_id[0], (None,))[0] == last_id:
                                del pending[last_id[0]]
                            continue
                        m = ID_COMMENT.match(line)
                        if m:
                            k = key(*m.groups())
                            comments.setdefault(k, (rel, number))
                            last_id = k
                            # tags of one EXE on consecutive lines are a list (an index table), not a tag: none binds
                            if pending.get(k[0], (None, -1))[1] == number - 1 or listing.get(k[0]) == number - 1:
                                pending.pop(k[0], None)
                                listing[k[0]] = number
                            else:
                                pending[k[0]] = (k, number)
                        elif MALFORMED_ID.match(line) and malformed is not None:
                            malformed.append((rel, number, text))
                        continue
                    last_id = None
                    sig = signature_at(lines[:i] + [line] + lines[i + 1:], i) if not line[0].isspace() else None
                    if sig:
                        fn, kind, skip_to = sig
                        if kind == "definition" and rel.endswith(".c"):
                            definitions.setdefault(fn, (rel, number))
                        for k, at in pending.values():
                            sites[k].append((fn, rel, number, kind, at))
                    else:
                        for k, at in pending.values():
                            loose[k].append((rel, at))
                    pending = {}
    found = {}
    for k in comments:
        fns = sorted({s[0] for s in sites.get(k, [])})
        if loose.get(k) and fns and misplaced is not None:   # another tag binds: this one marks an area, not the function
            misplaced.extend((f, n, key_text(k), fns[0]) for f, n in loose[k])
        if len(fns) > 1:
            found[k] = [(s[0], s[1], s[4], "conflict?") for s in sites[k]]   # where each tag is
        elif fns:
            fn = fns[0]
            kinds = {s[3] for s in sites[k]}
            if fn in definitions:
                found[k] = [(fn,) + definitions[fn] + ("definition" if "definition" in kinds else "prototype",)]
            else:
                found[k] = [("",) + comments[k] + ("comment",)]
        else:
            found[k] = [("",) + comments[k] + ("comment",)]
    for k, target in (extra_same_as or {}).items():
        comments.setdefault(k, ("", 0))
        found.setdefault(k, [("", "", 0, "comment")])
        same_as.setdefault(k, target)
    for k, target in same_as.items():
        hits = [h for h in found.get(target, []) if h[0]]
        if hits and not any(h[0] for h in found[k]):
            found[k] = [(fn, path, line, "same-as") for fn, path, line, _ in hits]
    # one function claimed by several ids of the same EXE: a mistyped or misplaced id; mark every claim doubtful
    claims = defaultdict(set)
    for k, hits in found.items():
        for fn, _, _, evidence in hits:
            if fn and evidence != "conflict?":
                claims[(k[0], fn)].add(k)
    for k, hits in list(found.items()):
        if len(hits) == 1 and hits[0][0] and not hits[0][3].endswith("?") and len(claims[(k[0], hits[0][0])]) > 1:
            fn, path, line, evidence = hits[0]
            # point at the tag(s) that made the claim, not at the function
            found[k] = ([(fn, s[1], s[4], evidence + "?") for s in sites[k]] if sites.get(k)
                        else [(fn, path, line, evidence + "?")])
    return found


def index_rows(found):
    """Flatten scan_remom() output into TSV rows (dicts) in key order. Powered by Claude."""
    rows = []
    for k in sorted(found):
        for fn, path, line, evidence in found[k]:
            rows.append(dict(zip(COLUMNS, [k[0], key_text(k), k[1], k[2], k[3], fn, path, line, evidence])))
    return rows


def write_index(rows, out_path: Path) -> None:
    """Write the index TSV (tab-separated, header row, LF). Powered by Claude."""
    out_path.parent.mkdir(parents=True, exist_ok=True)
    with open(out_path, "w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, fieldnames=COLUMNS, delimiter="\t", lineterminator="\n")
        w.writeheader()
        w.writerows(rows)


def read_index(path: Path):
    """Read an index TSV back into row dicts (segment/proc/line as int). Powered by Claude."""
    with open(path, newline="", encoding="utf-8") as f:
        rows = list(csv.DictReader(f, delimiter="\t"))
    for row in rows:
        for field in ("segment", "proc", "line"):
            row[field] = int(row[field])
    return rows


def git_commit(root) -> str:
    """The ReMoM checkout's HEAD commit plus '-dirty' when it has uncommitted changes, or '' without git. Powered by Claude."""
    try:
        head = subprocess.run(["git", "-C", str(root), "rev-parse", "HEAD"], capture_output=True, text=True, check=True).stdout.strip()
        dirty = subprocess.run(["git", "-C", str(root), "status", "--porcelain"], capture_output=True, text=True, check=True).stdout.strip()
        return head + ("-dirty" if dirty else "")
    except (OSError, subprocess.CalledProcessError):
        return ""


def take_snapshot(remom_root, out_path: Path):
    """Scan ReMoM and write the index TSV plus its manifest; return the rows. Powered by Claude."""
    malformed, misplaced = [], []
    rows = index_rows(scan_remom(str(remom_root), malformed, load_same_as(), misplaced))
    write_index(rows, out_path)
    manifest = {
        "remom": str(remom_root),
        "commit": git_commit(remom_root),
        "taken": datetime.now().astimezone().isoformat(timespec="seconds"),
        "ids": len({row["id"] for row in rows}),
        "rows": len(rows),
        "malformed_ids": [{"file": f, "line": n, "text": s} for f, n, s in malformed],
        "misplaced_tags": [{"file": f, "line": n, "id": i, "function": fn} for f, n, i, fn in misplaced],
    }
    Path(str(out_path) + ".manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    return rows


def main(argv=None):
    """Command-line entry point. Powered by Claude."""
    p = argparse.ArgumentParser(description=__doc__.split("\n\n")[1].strip())
    p.add_argument("--remom", default=DEFAULT_REMOM, help=f"the ReMoM checkout. Default: {DEFAULT_REMOM}")
    p.add_argument("--out", type=Path, default=DEFAULT_OUT, help=f"TSV to write. Default: {DEFAULT_OUT}")
    args = p.parse_args(argv)
    rows = take_snapshot(args.remom, args.out)
    print("remom_index: %s: %d ids, %d rows" % (args.out, len({r["id"] for r in rows}), len(rows)))
    return 0


if __name__ == "__main__":
    sys.exit(main())
