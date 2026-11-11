"""
moo2_debug_func_index.py

Reduce the MoO2 (ORION2.EXE) Watcom debug-info extract to a fixed-width
function index for the cross-game (MoO1 / MoM / MoO2 / ReMoM) name mapping.

Input:  MasterOfOrion2-DebugExtract.md  (master "## Modules", "## Global symbols",
        and "## Modules in detail" sections)
Output: MasterOfOrion2-FunctionIndex.fwv  (one row per code symbol)

Columns:
    KEY      m<module#>f<ordinal>  e.g. m142f003  (module / function ordinal)
    MOD      module number in the debug info (link order)
    ORD      1-based ordinal of the code body within its module, by address
    ALIAS    n/m when m names share one address (linker-folded identical bodies);
             KEY gets a _n suffix for the 2nd and later names
    LINEAR   linear address the program runs at
    SEGOFF   LE object:offset as the debug info gives it
    FILEOFS  file offset in ORION2.EXE
    SIZE     bytes; from routine debug info when SZ=D, else estimated (SZ=E)
             as the distance to the next code symbol, clipped to the module range
    SZ       D = debug-info size, E = estimate (may include unnamed static code)
    S        S = static (module-local) symbol
    NAME     C name (Watcom decoration removed; routine name from debug info when known)
    SYMBOL   raw linker symbol
    MODULE   module (object file) path as recorded
    TYPE     C type from routine debug info, when the module has it

Usage:
    python moo2_debug_func_index.py [extract.md] [-o out.fwv]
"""

import argparse
import re
import sys
from pathlib import Path

DEFAULT_IN = Path(r"C:\STU\devel\ReMoM\doc\@MoO2\MasterOfOrion2-DebugExtract.md")

ADDR_RE = re.compile(r"^(\d+):0x([0-9A-Fa-f]+) \(linear 0x([0-9A-Fa-f]+)\)$")
RANGE_RE = re.compile(r"(\d+):0x([0-9A-Fa-f]+)\+0x([0-9A-Fa-f]+)")
DETAIL_MOD_RE = re.compile(r"^### (\d+): (.*)$")
ROUTINE_ADDR_RE = re.compile(
    r"^- Address: (\d+):0x([0-9A-Fa-f]+) \(linear 0x([0-9A-Fa-f]+)\), ([\d,]+) bytes"
)


def split_row(line):
    """Split a markdown table row into stripped cells. Powered by Claude."""
    return [c.strip() for c in line.strip().strip("|").split("|")]


def c_name(symbol):
    """Strip Watcom decoration: trailing '_' = register call, leading '_' = cdecl. Powered by Claude."""
    if symbol.endswith("_") and not symbol.endswith("__") and len(symbol) > 1:
        return symbol[:-1]
    if symbol.startswith("_") and not symbol.startswith("__") and len(symbol) > 1:
        return symbol[1:]
    return symbol


def parse_extract(path):
    """Parse module table, global symbol table, and per-module routine details. Powered by Claude."""
    modules = {}        # module path -> (number, [(seg, start, end)])
    symbols = []        # dicts for code symbols
    routines = {}       # (seg, offset, name.lower()) -> [name, size, type]

    section = None
    cur_mod = None
    cur_routine = None
    last_key = None
    with open(path, encoding="utf-8") as fh:
        for line in fh:
            line = line.rstrip("\n")
            if line.startswith("## "):
                section = line[3:].strip()
                continue

            if section == "Modules" and line.startswith("| ") and not line.startswith("| #"):
                cells = split_row(line)
                if len(cells) < 4 or not cells[0].isdigit():
                    continue
                ranges = [
                    (int(s), int(o, 16), int(o, 16) + int(n, 16))
                    for s, o, n in RANGE_RE.findall(cells[3])
                ]
                modules[cells[1]] = (int(cells[0]), ranges)

            elif section == "Global symbols" and line.startswith("| ") and "(linear" in line:
                cells = split_row(line)
                addr, fofs, kind, sym, mod = cells[0], cells[1], cells[2], cells[3], cells[4]
                if "code" not in kind:
                    continue
                m = ADDR_RE.match(addr)
                if not m:
                    print(f"WARN: bad address: {line}", file=sys.stderr)
                    continue
                symbols.append({
                    "seg": int(m.group(1)),
                    "off": int(m.group(2), 16),
                    "linear": int(m.group(3), 16),
                    "fileofs": fofs,
                    "static": "static" in kind,
                    "symbol": sym,
                    "module": mod,
                })

            elif section == "Modules in detail":
                dm = DETAIL_MOD_RE.match(line)
                if dm:
                    cur_mod = int(dm.group(1))
                    cur_routine = None
                    continue
                if line.startswith("#### "):
                    title = line[5:].strip()
                    cur_routine = None if title in ("Global symbols", "Module variables",
                                                    "Line numbers") else title
                    continue
                if cur_routine is None:
                    continue
                ra = ROUTINE_ADDR_RE.match(line)
                if ra:
                    key = (int(ra.group(1)), int(ra.group(2), 16), cur_routine.lower())
                    routines[key] = [cur_routine, int(ra.group(4).replace(",", "")), ""]
                    last_key = key
                elif line.startswith("- Type: ") and routines.get(last_key, [None])[0] == cur_routine:
                    routines[last_key][2] = line[len("- Type: "):].strip()
    return modules, symbols, routines


def build_rows(modules, symbols, routines):
    """Assign module ordinals and sizes to each code symbol. Powered by Claude."""
    symbols.sort(key=lambda s: (s["seg"], s["off"]))

    # Unique code start addresses (aliases share an address) for size estimates.
    starts = sorted({(s["seg"], s["off"]) for s in symbols})
    next_start = {starts[i]: starts[i + 1] for i in range(len(starts) - 1)}
    alias_count = {}
    for s in symbols:
        k = (s["seg"], s["off"])
        alias_count[k] = alias_count.get(k, 0) + 1

    by_module = {}
    for s in symbols:
        by_module.setdefault(s["module"], []).append(s)

    rows = []
    unknown_mods = set()
    for mod, syms in by_module.items():
        if mod in modules:
            mod_num, ranges = modules[mod]
        else:
            unknown_mods.add(mod)
            mod_num, ranges = -1, []
        ordinal = 0
        prev = None
        for s in syms:
            key = (s["seg"], s["off"])
            if key != prev:
                ordinal += 1
                alias = 0
            prev = key
            alias += 1
            n_alias = alias_count[key]
            rt = routines.get(key + (c_name(s["symbol"]).lower(),))
            if rt:
                name, size, ctype, sz_src = rt[0], rt[1], rt[2], "D"
            else:
                name, ctype, sz_src = c_name(s["symbol"]), "", "E"
                end = None
                for seg, lo, hi in ranges:
                    if seg == s["seg"] and lo <= s["off"] < hi:
                        end = hi
                nxt = next_start.get(key)
                if nxt and nxt[0] == s["seg"] and (end is None or nxt[1] < end):
                    end = nxt[1]
                size = (end - s["off"]) if end is not None else None
            rows.append({
                "key": func_key(mod_num, ordinal, alias),
                "mod": mod_num,
                "ord": ordinal,
                "alias": f"{alias}/{n_alias}" if n_alias > 1 else "-",
                "linear": f"0x{s['linear']:08X}",
                "segoff": f"{s['seg']}:0x{s['off']:08X}",
                "fileofs": s["fileofs"],
                "size": "-" if size is None else str(size),
                "sz": sz_src if size is not None else "-",
                "static": "S" if s["static"] else "-",
                "name": name,
                "symbol": s["symbol"],
                "module": mod,
                "type": ctype or "-",
                "_sort": (mod_num, ordinal, alias),
            })
    rows.sort(key=lambda r: r["_sort"])
    for mod in sorted(unknown_mods):
        print(f"WARN: module not in module table: {mod}", file=sys.stderr)
    return rows


COLUMNS = [
    ("KEY", "key"), ("MOD", "mod"), ("ORD", "ord"), ("ALIAS", "alias"), ("LINEAR", "linear"),
    ("SEGOFF", "segoff"), ("FILEOFS", "fileofs"), ("SIZE", "size"), ("SZ", "sz"),
    ("S", "static"), ("NAME", "name"), ("SYMBOL", "symbol"), ("MODULE", "module"),
    ("TYPE", "type"),
]


def func_key(mod_num, ordinal, alias=1):
    """Build the m<module>f<ordinal>[_<alias>] function key. Powered by Claude."""
    mod = f"{mod_num:03d}" if mod_num >= 0 else "???"
    return f"m{mod}f{ordinal:03d}" + (f"_{alias}" if alias > 1 else "")


def fwv_lines(columns, rows):
    """Render rows (dicts) as fixed-width lines: header, then rows; empty cells become '-'. Powered by Claude."""
    def cell(v):
        s = " ".join(str(v).split())
        return s if s else "-"
    cells = [[cell(r[k]) for _, k in columns] for r in rows]
    widths = [max([len(t)] + [len(c[i]) for c in cells]) for i, (t, _) in enumerate(columns)]
    lines = ["  ".join(t.ljust(w) for (t, _), w in zip(columns, widths)).rstrip()]
    for c in cells:
        lines.append("  ".join(v.ljust(w) for v, w in zip(c, widths)).rstrip())
    return lines


def write_text_crlf(out_path, lines):
    """Write lines with CRLF endings, matching the repo's existing .fwv files. Powered by Claude."""
    with open(out_path, "w", encoding="utf-8", newline="\r\n") as fh:
        fh.write("\n".join(lines) + "\n")


def write_fwv(rows, out_path, src_path):
    """Write the function index as a single fixed-width table. Powered by Claude."""
    write_text_crlf(out_path, fwv_lines(COLUMNS, rows))
    print(f"{len(rows)} functions -> {out_path} (from {src_path})")


def main():
    """Command-line entry point. Powered by Claude."""
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[1])
    ap.add_argument("extract", nargs="?", type=Path, default=DEFAULT_IN)
    ap.add_argument("-o", "--output", type=Path)
    args = ap.parse_args()
    out = args.output or args.extract.with_name("MasterOfOrion2-FunctionIndex.fwv")
    modules, symbols, routines = parse_extract(args.extract)
    rows = build_rows(modules, symbols, routines)
    n_d = sum(1 for r in rows if r["sz"] == "D")
    if n_d != len(routines):
        print(f"WARN: {len(routines) - n_d} debug routines not matched to a global symbol",
              file=sys.stderr)
    print(f"modules={len(modules)} code_symbols={len(symbols)} routines_with_debug={len(routines)} "
          f"rows_with_debug_size={n_d}")
    write_fwv(rows, out, args.extract)


if __name__ == "__main__":
    main()
