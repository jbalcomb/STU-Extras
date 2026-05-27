# -*- coding: utf-8 -*-
"""
dseg_map_compare.py

Scans all .c and .h files under C:\STU\devel\ReMoM for inline dseg address
annotations of the form:
    // WZD dseg:XXXX
    // MGC dseg:XXXX

Extracts variable name, type, and initializer from the declaration that
follows each annotation, then produces a joined CSV and Markdown table
comparing WIZARDS.EXE (WZD) and MAGIC.EXE (MGC) offsets.

XREFs are not available from current exports; that column is left as N/A.

Output:
    C:\STU\devel\STU-Extras\STU-DOSBox\MoM-dseg-layout.md
    C:\STU\devel\STU-Extras\STU-DOSBox\MoM-dseg-layout.csv
"""

import os
import re
import io

REMOM_ROOT = r"C:\STU\devel\ReMoM"
OUT_DIR    = r"C:\STU\devel\STU-Extras\STU-DOSBox"
OUT_MD     = os.path.join(OUT_DIR, "MoM-dseg-layout.md")
OUT_CSV    = os.path.join(OUT_DIR, "MoM-dseg-layout.csv")

# Matches:  // WZD dseg:1234  or  // MGC dseg:ABCD
ANN_RE = re.compile(r"//\s*(WZD|MGC)\s+dseg:([0-9A-Fa-f]{1,6})", re.IGNORECASE)

# Loose declaration parser — grabs C type tokens + variable name + optional initializer.
# Handles:  int16_t _turn = 0;   uint32_t random_seed;   char far *_ptr = NULL;
DECL_RE = re.compile(
    r"^"
    r"(?:extern\s+|static\s+|far\s+)?"           # optional storage class
    r"(?P<type>"
        r"(?:(?:unsigned|signed|long|short|far|near|const)\s+)*"
        r"(?:int|char|void|uint8_t|uint16_t|uint32_t|uint64_t"
        r"|int8_t|int16_t|int32_t|int64_t|float|double"
        r"|[A-Za-z_][A-Za-z0-9_]*)"
        r"(?:\s*\*+)?"
    r")"
    r"\s+"
    r"(?P<name>[A-Za-z_][A-Za-z0-9_]*)"
    r"(?:\s*=\s*(?P<init>[^;,\n]+?))?"
    r"\s*[;,\[]"
)


def scan_files(root):
    """Walk root and return list of .c / .h file paths."""
    paths = []
    for dirpath, _, filenames in os.walk(root):
        for fn in filenames:
            if fn.lower().endswith((".c", ".h")):
                paths.append(os.path.join(dirpath, fn))
    return paths


def parse_file(path, wzd_map, mgc_map):
    """Parse one file; populate wzd_map and mgc_map dicts keyed by variable name."""
    try:
        with io.open(path, "r", encoding="utf-8", errors="replace") as fh:
            lines = fh.readlines()
    except OSError:
        return

    rel = os.path.relpath(path, REMOM_ROOT)

    for i, line in enumerate(lines):
        m = ANN_RE.search(line)
        if not m:
            continue
        binary  = m.group(1).upper()   # WZD or MGC
        offset  = m.group(2).upper().zfill(4)

        # Look at the next non-blank, non-comment line for the declaration.
        decl_line = ""
        for j in range(i + 1, min(i + 4, len(lines))):
            stripped = lines[j].strip()
            if stripped and not stripped.startswith("//") and not stripped.startswith("/*"):
                decl_line = stripped
                break

        dm = DECL_RE.match(decl_line)
        if not dm:
            continue   # skip annotations not immediately followed by a parseable declaration
        var_type = dm.group("type").strip()
        var_name = dm.group("name").strip()
        var_init = (dm.group("init") or "").strip()

        entry = {
            "offset": offset,
            "type":   var_type,
            "init":   var_init,
            "file":   rel,
            "line":   i + 1,
        }

        target = wzd_map if binary == "WZD" else mgc_map
        # Keep first occurrence per name; note duplicates.
        if var_name not in target:
            target[var_name] = entry
        else:
            # Mark duplicate but keep first
            target[var_name]["dup"] = True


def sanitize(s):
    """Replace non-ASCII chars with '?' for Python 2 compatibility."""
    if isinstance(s, unicode):
        return s.encode("ascii", "replace").decode("ascii")
    return s


def build_rows(wzd_map, mgc_map):
    """Merge both maps into sorted row list."""
    all_names = sorted(set(wzd_map) | set(mgc_map))
    rows = []
    for name in all_names:
        w = wzd_map.get(name)
        g = mgc_map.get(name)
        rows.append({
            "name":       sanitize(name),
            "type":       sanitize((w or g)["type"]),
            "wzd_offset": sanitize(w["offset"]) if w else "-",
            "mgc_offset": sanitize(g["offset"]) if g else "-",
            "init":       sanitize((w or g)["init"]),
            "wzd_file":   sanitize("{0}:{1}".format(w['file'], w['line'])) if w else "-",
            "mgc_file":   sanitize("{0}:{1}".format(g['file'], g['line'])) if g else "-",
            "xrefs":      "N/A",
        })
    return rows


def write_csv(rows):
    if not os.path.isdir(OUT_DIR):
        os.makedirs(OUT_DIR)
    import csv as csv_mod
    with open(OUT_CSV, "wb") as fh:
        writer = csv_mod.DictWriter(fh, fieldnames=[
            "name", "type", "wzd_offset", "mgc_offset", "init",
            "wzd_file", "mgc_file", "xrefs"
        ])
        writer.writeheader()
        for r in rows:
            encoded = {k: (v.encode("utf-8") if isinstance(v, unicode) else v)
                       for k, v in r.items()}
            writer.writerow(encoded)
    print("CSV written: " + OUT_CSV)


def write_md(rows, wzd_total, mgc_total):
    if not os.path.isdir(OUT_DIR):
        os.makedirs(OUT_DIR)
    lines = [
        "# MoM dseg Memory Layout: WIZARDS.EXE vs MAGIC.EXE\n",
        "\n",
        "Extracted from inline `// WZD dseg:XXXX` and `// MGC dseg:XXXX` annotations\n",
        "in ReMoM source files under `C:\\STU\\devel\\ReMoM`.\n",
        "\n",
        "- **WZD annotations found:** {0}\n".format(wzd_total),
        "- **MGC annotations found:** {0}\n".format(mgc_total),
        "- **Unique variables:** {0}\n".format(len(rows)),
        "- **XREFs:** not available from current exports (requires IDA IDC dump)\n",
        "\n",
        "| Variable | Type | WZD dseg | MGC dseg | Init | Source (WZD) |\n",
        "|----------|------|----------|----------|------|---------------|\n",
    ]
    for r in rows:
        init  = r["init"].replace("|", "\\|")[:40]
        wfile = r["wzd_file"].replace("\\", "/")
        if len(wfile) > 60:
            wfile = "..." + wfile[-57:]
        lines.append(
            "| `{name}` | `{typ}` | `{wo}` | `{mo}` | `{init}` | {wf} |\n".format(
                name=r['name'], typ=r['type'], wo=r['wzd_offset'],
                mo=r['mgc_offset'], init=init, wf=wfile
            )
        )
    import io
    text = u"".join(lines)
    with io.open(OUT_MD, "w", encoding="utf-8") as fh:
        fh.write(text)
    print("Markdown written: " + OUT_MD)


def main():
    print("Scanning " + REMOM_ROOT + " ...")
    files = scan_files(REMOM_ROOT)
    print("  {0} source files found".format(len(files)))

    wzd_map = {}
    mgc_map = {}
    for path in files:
        parse_file(path, wzd_map, mgc_map)

    print("  WZD unique variables: {0}".format(len(wzd_map)))
    print("  MGC unique variables: {0}".format(len(mgc_map)))

    rows = build_rows(wzd_map, mgc_map)
    print("  Combined unique variables: {0}".format(len(rows)))

    write_csv(rows)
    write_md(rows, len(wzd_map), len(mgc_map))

    both     = sum(1 for r in rows if r["wzd_offset"] != "-" and r["mgc_offset"] != "-")
    wzd_only = sum(1 for r in rows if r["mgc_offset"] == "-")
    mgc_only = sum(1 for r in rows if r["wzd_offset"] == "-")
    print("\nSummary:")
    print("  Both binaries : {0}".format(both))
    print("  WZD only      : {0}".format(wzd_only))
    print("  MGC only      : {0}".format(mgc_only))


if __name__ == "__main__":
    main()
