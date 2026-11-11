"""
moo2_debug_modules_to_fwv.py

Convert the per-module MoO2 (ORION2.EXE) Watcom debug-info Markdown files
(doc/@MoO2/modules/NNNN-<name>.md) to fixed-width (.fwv) files.

Each output file is:
    key=value header lines   (module_num, module, source, range[n])
    then one [Section] per table, each an aligned FWV table (header row + rows),
    sections separated by a blank line. Empty cells are '-'. CRLF line endings.

Sections (only those the module has):
    [Global symbols]    KEY ORD ALIAS LINEAR SEGOFF KIND NAME SYMBOL CHUNK
    [Module variables]  LINEAR SEGOFF NAME TYPE
    [Routines]          KEY LINEAR SEGOFF SIZE PROLOGUE EPILOGUE RET_OFS FAR PARAM_REGS NAME TYPE CHUNK
    [Locals]            KEY ROUTINE BLOCK LOCAL LOCATION TYPE
    [Types]             NUM KIND NAME DETAILS
    [Type members]      TYPE_NUM MEMBER_KIND MEMBER OFFSET_VALUE TYPE TYPE_NAME
    [Line numbers]      KEY LINE LINEAR SEGOFF FILEOFS

KEY is the m<module>f<ordinal>[_<alias>] function key, the same keys as
MasterOfOrion2-FunctionIndex.fwv (built by moo2_debug_func_index.py).
In [Locals], BLOCK is the nested scope ("Block at" section) linear address and
size, attributed to the routine whose range contains it.

Usage:
    python moo2_debug_modules_to_fwv.py [modules_dir] [-o out_dir] [files...]
"""

import argparse
import re
import sys
from pathlib import Path

from moo2_debug_func_index import c_name, func_key, fwv_lines, write_text_crlf

DEFAULT_DIR = Path(r"C:\STU\devel\ReMoM\doc\@MoO2\modules")

ADDR_RE = re.compile(r"(\d+):0x([0-9A-Fa-f]+) \(linear 0x([0-9A-Fa-f]+)\)")
MODNUM_RE = re.compile(r"^Module (\d+) of the debug info")
RANGE_RE = re.compile(r"(\d+):0x([0-9A-Fa-f]+) \(linear 0x([0-9A-Fa-f]+)\), ([\d,]+) bytes")
ROUTINE_ADDR_RE = re.compile(
    r"^- Address: (\d+):0x([0-9A-Fa-f]+) \(linear 0x([0-9A-Fa-f]+)\), ([\d,]+) bytes(?:, in `([^`]*)`)?"
)
PROLOGUE_RE = re.compile(
    r"^- Prologue (\d+) bytes, epilogue (\d+) bytes, return address at frame offset (0x[0-9A-Fa-f]+|\d+)(, far)?$"
)
BLOCK_RE = re.compile(r"^Block at (\d+):0x([0-9A-Fa-f]+) \(linear 0x([0-9A-Fa-f]+)\), ([\d,]+) bytes$")
TYPE_TITLE_RE = re.compile(r"^(.*) \(#(\d+), (.*)\)$")


def split_row(line):
    """Split a markdown table row into stripped cells. Powered by Claude."""
    return [c.strip() for c in line.strip().strip("|").split("|")]


def addr_parts(text):
    """Return (seg, off, linear) ints from 'S:0xOFF (linear 0xLIN)', or None. Powered by Claude."""
    m = ADDR_RE.search(text)
    if not m:
        return None
    return int(m.group(1)), int(m.group(2), 16), int(m.group(3), 16)


def fmt_segoff(seg, off):
    """Format an LE object:offset. Powered by Claude."""
    return f"{seg}:0x{off:08X}"


def fmt_lin(lin):
    """Format a linear address. Powered by Claude."""
    return f"0x{lin:08X}"


def parse_module_md(path):
    """Parse one module Markdown file into header info and per-section tables. Powered by Claude."""
    mod = {"path": None, "num": None, "ranges": [], "globals": [], "vars": [], "routines": [],
           "locals": [], "types": [], "members": [], "lines": [], "unparsed": []}
    h4 = None           # current #### title
    h5 = None           # current ##### title
    routine = None      # current routine dict
    block = None        # current block (seg, off, lin, size)
    header = None       # current table header cells
    with open(path, encoding="utf-8") as fh:
        for raw in fh:
            line = raw.rstrip("\n").rstrip("\r")
            if not line.strip():
                header = None
                continue
            if line.startswith("# "):
                mod["path"] = line[2:].strip()
                continue
            if line.startswith("#### "):
                h4, h5, header, routine, block = line[5:].strip(), None, None, None, None
                bm = BLOCK_RE.match(h4)
                if bm:
                    block = (int(bm.group(1)), int(bm.group(2), 16), int(bm.group(3), 16),
                             int(bm.group(4).replace(",", "")))
                elif h4 not in ("Global symbols", "Module variables", "Types", "Line numbers"):
                    routine = {"name": h4, "seg": None, "off": None, "lin": None, "size": None,
                               "chunk": "", "type": "", "prologue": "", "epilogue": "",
                               "ret_ofs": "", "far": "", "params": ""}
                    mod["routines"].append(routine)
                continue
            if line.startswith("##### "):
                h5, header = line[6:].strip(), None
                continue
            m = MODNUM_RE.match(line)
            if m:
                mod["num"] = int(m.group(1))
                continue
            if line.startswith("Ranges: "):
                for s, o, lin, n in RANGE_RE.findall(line):
                    mod["ranges"].append((int(s), int(o, 16), int(lin, 16), int(n.replace(",", ""))))
                continue
            if line == "No local variables.":
                continue

            if routine is not None and line.startswith("- "):
                ra = ROUTINE_ADDR_RE.match(line)
                pm = PROLOGUE_RE.match(line)
                if ra:
                    routine.update(seg=int(ra.group(1)), off=int(ra.group(2), 16),
                                   lin=int(ra.group(3), 16), size=int(ra.group(4).replace(",", "")),
                                   chunk=ra.group(5) or "")
                elif line.startswith("- Type: "):
                    routine["type"] = line[len("- Type: "):].strip()
                elif pm:
                    routine.update(prologue=pm.group(1), epilogue=pm.group(2), ret_ofs=pm.group(3),
                                   far="far" if pm.group(4) else "")
                elif line.startswith("- Parameter locations: "):
                    routine["params"] = line[len("- Parameter locations: "):].replace(" ", "")
                else:
                    mod["unparsed"].append(line)
                continue

            if line.startswith("|"):
                cells = split_row(line)
                if header is None:
                    header = cells
                    continue
                if all(set(c) <= set("-:") for c in cells):
                    continue
                row = dict(zip(header, cells))
                if h4 == "Global symbols":
                    mod["globals"].append(row)
                elif h4 == "Module variables":
                    mod["vars"].append(row)
                elif h4 == "Line numbers":
                    mod["lines"].append(row)
                elif h4 == "Types" and h5 is None:
                    mod["types"].append(row)
                elif h4 == "Types" and h5 is not None:
                    mod["members"].append((h5, row))
                elif routine is not None or block is not None:
                    mod["locals"].append((routine["name"] if routine else None, block, row))
                else:
                    mod["unparsed"].append(line)
                continue

            mod["unparsed"].append(line)
    return mod


def build_sections(mod):
    """Turn parsed module tables into (section title, columns, rows) lists. Powered by Claude."""
    num = mod["num"] if mod["num"] is not None else -1
    sections = []

    # Global symbols, with function keys for code symbols (ordinal per unique address).
    glob = []
    for g in mod["globals"]:
        a = addr_parts(g.get("Address", ""))
        glob.append((a, g))
    glob.sort(key=lambda t: (t[0][0], t[0][1]) if t[0] else (99, 0))
    code_addr_count = {}
    for a, g in glob:
        if a and "code" in g.get("Kind", ""):
            code_addr_count[a[:2]] = code_addr_count.get(a[:2], 0) + 1
    keys_by_addr = {}       # (seg, off) -> base key
    key_by_name = {}        # c_name.lower() -> key
    code_starts = []        # (linear, base key, name) per unique code address, ascending
    rows = []
    ordinal, prev, alias = 0, None, 0
    for a, g in glob:
        kind = g.get("Kind", "").replace(" ", "")
        sym = g.get("Name", "")
        key = ord_s = alias_s = ""
        if a and "code" in kind:
            if a[:2] != prev:
                ordinal += 1
                alias = 0
            prev = a[:2]
            alias += 1
            key = func_key(num, ordinal, alias)
            ord_s = str(ordinal)
            n = code_addr_count[a[:2]]
            alias_s = f"{alias}/{n}" if n > 1 else ""
            if a[:2] not in keys_by_addr:
                keys_by_addr[a[:2]] = key
                code_starts.append((a[2], key, c_name(sym)))
            key_by_name[c_name(sym).lower()] = key
        rows.append({
            "key": key, "ord": ord_s, "alias": alias_s,
            "linear": fmt_lin(a[2]) if a else "", "segoff": fmt_segoff(a[0], a[1]) if a else "",
            "kind": kind, "name": c_name(sym) if "code" in kind else sym, "symbol": sym,
            "chunk": g.get("Chunk file", "").strip("`"),
        })
    if rows:
        sections.append(("Global symbols", [
            ("KEY", "key"), ("ORD", "ord"), ("ALIAS", "alias"), ("LINEAR", "linear"),
            ("SEGOFF", "segoff"), ("KIND", "kind"), ("NAME", "name"), ("SYMBOL", "symbol"),
            ("CHUNK", "chunk")], rows))

    # Module variables.
    rows = []
    for v in mod["vars"]:
        a = addr_parts(v.get("Address", ""))
        rows.append({"linear": fmt_lin(a[2]) if a else v.get("Address", ""),
                     "segoff": fmt_segoff(a[0], a[1]) if a else "",
                     "name": v.get("Name", ""), "type": v.get("Type", "")})
    if rows:
        sections.append(("Module variables", [
            ("LINEAR", "linear"), ("SEGOFF", "segoff"), ("NAME", "name"), ("TYPE", "type")], rows))

    # Routines.
    def routine_key(r):
        """Find the function key for a routine by name, then by address. Powered by Claude."""
        k = key_by_name.get(r["name"].lower())
        if not k and r["seg"] is not None:
            k = keys_by_addr.get((r["seg"], r["off"]), "")
        return k or ""

    rows = []
    spans = []          # (lin_start, lin_end, key, name) for block / line attribution
    for r in mod["routines"]:
        k = routine_key(r)
        if r["lin"] is not None:
            spans.append((r["lin"], r["lin"] + r["size"], k, r["name"]))
        rows.append({
            "key": k, "linear": fmt_lin(r["lin"]) if r["lin"] is not None else "",
            "segoff": fmt_segoff(r["seg"], r["off"]) if r["seg"] is not None else "",
            "size": r["size"] if r["size"] is not None else "", "prologue": r["prologue"],
            "epilogue": r["epilogue"], "ret_ofs": r["ret_ofs"], "far": r["far"],
            "params": r["params"], "name": r["name"], "type": r["type"], "chunk": r["chunk"],
        })
    rows.sort(key=lambda x: x["linear"])
    if rows:
        sections.append(("Routines", [
            ("KEY", "key"), ("LINEAR", "linear"), ("SEGOFF", "segoff"), ("SIZE", "size"),
            ("PROLOGUE", "prologue"), ("EPILOGUE", "epilogue"), ("RET_OFS", "ret_ofs"),
            ("FAR", "far"), ("PARAM_REGS", "params"), ("NAME", "name"), ("TYPE", "type"),
            ("CHUNK", "chunk")], rows))

    def span_for(lin):
        """Return (key, name) of the routine containing a linear address; if the module has
        no routine record there, the nearest preceding code symbol in the module. Powered by Claude."""
        for lo, hi, k, n in spans:
            if lo <= lin < hi:
                return k, n
        best = ("", "")
        for start, k, n in code_starts:
            if start <= lin:
                best = (k, n)
        return best

    # Locals (routine locals and nested-block locals).
    rows = []
    for rname, block, loc in mod["locals"]:
        if block is not None:
            k, rname = span_for(block[2])
            blk = f"{fmt_lin(block[2])}+{block[3]}"
        else:
            k = next((s[2] for s in spans if s[3] == rname), "")
            blk = ""
        rows.append({"key": k, "routine": rname or "", "block": blk, "local": loc.get("Local", ""),
                     "location": loc.get("Location", ""), "type": loc.get("Type", "")})
    if rows:
        sections.append(("Locals", [
            ("KEY", "key"), ("ROUTINE", "routine"), ("BLOCK", "block"), ("LOCAL", "local"),
            ("LOCATION", "location"), ("TYPE", "type")], rows))

    # Types.
    rows = [{"num": t.get("#", ""), "kind": t.get("Kind", ""), "name": t.get("Name", ""),
             "details": t.get("Details", "")} for t in mod["types"]]
    if rows:
        sections.append(("Types", [
            ("NUM", "num"), ("KIND", "kind"), ("NAME", "name"), ("DETAILS", "details")], rows))

    # Type members (struct/union fields, enum constants).
    rows = []
    for title, m in mod["members"]:
        tm = TYPE_TITLE_RE.match(title)
        tname, tnum = (tm.group(1), tm.group(2)) if tm else (title, "")
        if "Field" in m:
            rows.append({"tnum": tnum, "mkind": "field", "member": m["Field"],
                         "ofs": m.get("Offset / bits", "").replace("offset ", ""),
                         "type": m.get("Type", ""), "tname": tname})
        else:
            rows.append({"tnum": tnum, "mkind": "constant", "member": m.get("Constant", ""),
                         "ofs": m.get("Value", ""), "type": "", "tname": tname})
    if rows:
        sections.append(("Type members", [
            ("TYPE_NUM", "tnum"), ("MEMBER_KIND", "mkind"), ("MEMBER", "member"),
            ("OFFSET_VALUE", "ofs"), ("TYPE", "type"), ("TYPE_NAME", "tname")], rows))

    # Line numbers.
    rows = []
    for ln in mod["lines"]:
        a = addr_parts(ln.get("Address", ""))
        rows.append({"key": span_for(a[2])[0] if a else "", "line": ln.get("Line", ""),
                     "linear": fmt_lin(a[2]) if a else "",
                     "segoff": fmt_segoff(a[0], a[1]) if a else "",
                     "fileofs": ln.get("File offset", "")})
    if rows:
        sections.append(("Line numbers", [
            ("KEY", "key"), ("LINE", "line"), ("LINEAR", "linear"), ("SEGOFF", "segoff"),
            ("FILEOFS", "fileofs")], rows))
    return sections


def convert(md_path, out_dir):
    """Convert one module .md to .fwv; return (out_path, row counts, unparsed lines). Powered by Claude."""
    mod = parse_module_md(md_path)
    out = [f"module_num={mod['num']}", f"module={mod['path']}", f"source={md_path.name}"]
    for i, (s, o, lin, n) in enumerate(mod["ranges"]):
        out.append(f"range[{i}]={fmt_segoff(s, o)} linear={fmt_lin(lin)} size={n}")
    counts = {}
    for title, cols, rows in build_sections(mod):
        out.append("")
        out.append(f"[{title}]")
        out.extend(fwv_lines(cols, rows))
        counts[title] = len(rows)
    out_path = out_dir / (md_path.stem + ".fwv")
    write_text_crlf(out_path, out)
    return out_path, counts, mod["unparsed"]


def main():
    """Command-line entry point. Powered by Claude."""
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[1])
    ap.add_argument("modules_dir", nargs="?", type=Path, default=DEFAULT_DIR)
    ap.add_argument("files", nargs="*", help="specific .md files (default: all in modules_dir)")
    ap.add_argument("-o", "--out-dir", type=Path, help="default: same folder as the .md files")
    args = ap.parse_args()
    files = [Path(f) for f in args.files] or sorted(args.modules_dir.glob("*.md"))
    out_dir = args.out_dir or args.modules_dir
    out_dir.mkdir(parents=True, exist_ok=True)
    totals = {}
    for f in files:
        out_path, counts, unparsed = convert(f, out_dir)
        for k, v in counts.items():
            totals[k] = totals.get(k, 0) + v
        for u in unparsed:
            print(f"WARN: {f.name}: unparsed line: {u}", file=sys.stderr)
    print(f"{len(files)} modules -> {out_dir}")
    for k, v in totals.items():
        print(f"  {k:<18} {v}")


if __name__ == "__main__":
    main()
