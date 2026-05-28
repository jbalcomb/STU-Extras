// ida55_dseg_dump.idc
//
// Dump the data segment (dseg) of the open IDA 5.5 database in fixed-width
// value (FWV) format.
//
// Output file: <idb_dir>\<db_basename>.fwv
// e.g. WIZARDS.idb  ->  WIZARDS.fwv
//      MAGIC.idb    ->  MAGIC.fwv
//
// FWV column layout (one row per defined head in dseg):
//
//   ADDR_LINEAR  DSEG_OFFS  NAME
//   8 hex chars  4 hex chars  variable length (no truncation)
//   e.g.:
//   0003E2E6 7846 random_seed
//
// NAME is the last column so it is emitted as-is with no padding or truncation.
// Rows with no user-assigned name carry the auto-generated IDA label.
// Iterates using NextHead() so only defined items are emitted (not every byte).
//
// Usage:
//   Run in IDA 5.5 via File > IDC Script, or via idaw.py automation wrapper.

#include <idc.idc>

// ---------------------------------------------------------------------------
// Fixed-width column sizes (chars)
// ---------------------------------------------------------------------------
#define FWV_COL_ADDR   8    // linear address, 8 hex digits, no prefix
#define FWV_COL_OFFS   4    // dseg offset, 4 hex digits, no prefix
// NAME column is last — no fixed width, no truncation
#define FWV_SEP       " "   // single space between columns

// ---------------------------------------------------------------------------
// String helpers
// ---------------------------------------------------------------------------

// Return the position of the LAST occurrence of ch in s, or -1 if not found.
static last_pos_of(s, ch)
{
    auto i, found, slen;
    slen = strlen(s);
    found = -1;
    for (i = 0; i < slen; i = i + 1)
    {
        if (substr(s, i, i + 1) == ch)
            found = i;
    }
    return found;
}

// Return the portion of path after the last path separator.
static path_basename(path)
{
    auto sep;
    sep = last_pos_of(path, "\\");
    if (sep == -1)
        sep = last_pos_of(path, "/");
    if (sep == -1)
        return path;
    return substr(path, sep + 1, -1);
}

// Strip the last extension from a filename (everything from the last dot).
static strip_ext(name)
{
    auto dot;
    dot = last_pos_of(name, ".");
    if (dot == -1)
        return name;
    return substr(name, 0, dot);
}

// Return the directory portion of path (including trailing separator).
static path_dir(path)
{
    auto sep;
    sep = last_pos_of(path, "\\");
    if (sep == -1)
        sep = last_pos_of(path, "/");
    if (sep == -1)
        return "";
    return substr(path, 0, sep + 1);
}

// Pad string s on the right with spaces to exactly `width` chars.
// Truncates with '>' marker if s exceeds width.
static pad_right(s, width)
{
    auto result, slen, i;
    slen = strlen(s);
    if (slen >= width)
    {
        // Truncate and mark overflow
        result = substr(s, 0, width - 1) + ">";
        return result;
    }
    result = s;
    for (i = slen; i < width; i = i + 1)
        result = result + " ";
    return result;
}

// ---------------------------------------------------------------------------
// Segment helpers
// ---------------------------------------------------------------------------

// Find the first segment whose name matches `name`.  Returns BADADDR if not found.
static find_seg_by_name(name)
{
    auto seg;
    seg = FirstSeg();
    while (seg != BADADDR)
    {
        if (SegName(seg) == name)
            return seg;
        seg = NextSeg(seg);
    }
    return BADADDR;
}

// ---------------------------------------------------------------------------
// FWV dump
// ---------------------------------------------------------------------------

static dump_dseg_fwv(handle, dseg_ea, dseg_end, dseg_base)
{
    auto ea, name, offs_val;
    auto row_addr, row_offs;

    for (ea = dseg_ea; ea < dseg_end && ea != BADADDR; ea = NextHead(ea, dseg_end))
    {
        name = GetTrueNameEx(0, ea);
        if (name == 0)
            name = "";

        offs_val = ea - dseg_base;

        row_addr = form("%08X", ea);
        row_offs = form("%04X", offs_val);

        fprintf(handle, "%s%s%s%s%s\n",
            row_addr, FWV_SEP,
            row_offs, FWV_SEP,
            name);
    }
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------

static main(void)
{
    auto idb_path, idb_dir, db_name, out_path;
    auto handle;
    auto dseg_ea, dseg_end, dseg_base;
    auto sep_line, hdr_addr, hdr_offs;

    Batch(1);

    // Derive output path: <idb_dir>\<db_basename>.fwv
    idb_path = GetIdbPath();
    idb_dir  = path_dir(idb_path);
    db_name  = strip_ext(path_basename(idb_path));
    out_path = idb_dir + db_name + ".fwv";

    Message("ida55_dseg_dump: output -> %s\n", out_path);

    // Locate the dseg segment
    dseg_ea = find_seg_by_name("dseg");
    if (dseg_ea == BADADDR)
    {
        Message("ida55_dseg_dump: ERROR - no segment named 'dseg' found\n");
        Batch(0);
        Exit(2);
    }
    dseg_end  = SegEnd(dseg_ea);
    dseg_base = dseg_ea;

    Message("ida55_dseg_dump: dseg linear [0x%08X, 0x%08X)\n", dseg_base, dseg_end);

    handle = fopen(out_path, "w");
    if (handle == 0)
    {
        Message("ida55_dseg_dump: ERROR - could not open output file: %s\n", out_path);
        Batch(0);
        Exit(2);
    }

    // --- Header ---
    hdr_addr = pad_right("ADDR_LIN", FWV_COL_ADDR);
    hdr_offs = pad_right("OFFS",     FWV_COL_OFFS);

    sep_line = pad_right("--------", FWV_COL_ADDR) + FWV_SEP +
               pad_right("----",     FWV_COL_OFFS) + FWV_SEP +
               "NAME";

    fprintf(handle, "# ida55_dseg_dump fwv-v1\n");
    fprintf(handle, "# db:         %s\n", db_name);
    fprintf(handle, "# idb_path:   %s\n", idb_path);
    fprintf(handle, "# input_file: %s\n", GetInputFile());
    fprintf(handle, "# dseg_base:  0x%08X\n", dseg_base);
    fprintf(handle, "# dseg_end:   0x%08X\n", dseg_end);
    fprintf(handle, "# dseg_size:  0x%08X (%d bytes)\n",
        dseg_end - dseg_base, dseg_end - dseg_base);
    fprintf(handle, "#\n");
    fprintf(handle, "%s%s%s%s%s\n",
        hdr_addr, FWV_SEP, hdr_offs, FWV_SEP, "NAME");
    fprintf(handle, "%s\n", sep_line);

    // --- Data rows ---
    dump_dseg_fwv(handle, dseg_ea, dseg_end, dseg_base);

    fclose(handle);

    Message("ida55_dseg_dump: done -> %s\n", out_path);

    Batch(0);
    Exit(0);
}
