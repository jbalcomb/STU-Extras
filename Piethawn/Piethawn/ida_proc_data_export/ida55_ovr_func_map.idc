// ida55_ovr_func_map.idc
//
// Build an overlay-segment -> function mapping table for Borland overlay
// executables (WIZARDS.EXE, MAGIC.EXE).
//
// The Borland overlay manager uses INT 3Fh to load segments.  At the handler
// entry, AX = 0-based segment table index.  This script emits that same index
// alongside every function in each segment, so a DOSBox INT 3Fh breakpoint
// can identify which function is being dispatched.
//
// IDA enumerates segments in the same order as the Borland segment table, so
// SEG_IDX here == AX at the INT 3Fh handler.
//
// Output file: <idb_dir>\<db_basename>.ovr-func-map.fwv
//
// FWV column layout:
//
//   SEG_IDX  SEG_EA    SEG_NAME   FUNC_EA   FUNC_NAME
//   4 dec    8 hex     10 chars   8 hex     variable (no truncation)
//
// One row per function.  Segments that contain no functions get a single row
// with FUNC_EA=00000000 and FUNC_NAME=(none).
//
// Usage:
//   Run via File > IDC Script in IDA 5.5, or via ida_automation\idaw.py.

#include <idc.idc>

#define FWV_SEP        " "
#define FWV_COL_SEGIDX 4
#define FWV_COL_SEGEA  8
#define FWV_COL_SEGNAM 10
#define FWV_COL_FUNCEA 8
// FUNC_NAME is last — no fixed width

// ---------------------------------------------------------------------------
// String helpers (same pattern as ida55_dseg_dump.idc)
// ---------------------------------------------------------------------------

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

static strip_ext(name)
{
    auto dot;
    dot = last_pos_of(name, ".");
    if (dot == -1)
        return name;
    return substr(name, 0, dot);
}

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

static pad_right(s, width)
{
    auto result, slen, i;
    slen = strlen(s);
    if (slen >= width)
        return substr(s, 0, width);
    result = s;
    for (i = slen; i < width; i = i + 1)
        result = result + " ";
    return result;
}

// ---------------------------------------------------------------------------
// Dump
// ---------------------------------------------------------------------------

static dump_seg_functions(handle, seg_idx, seg_ea, seg_name, seg_end)
{
    auto func_ea, func_name;
    auto col_idx, col_sea, col_sname, col_fea;
    auto found_any;

    col_idx   = pad_right(form("%04d", seg_idx),  FWV_COL_SEGIDX);
    col_sea   = pad_right(form("%08X", seg_ea),   FWV_COL_SEGEA);
    col_sname = pad_right(seg_name,               FWV_COL_SEGNAM);

    found_any = 0;

    // Walk functions whose entry point falls within [seg_ea, seg_end)
    func_ea = NextFunction(seg_ea - 1);
    while (func_ea != BADADDR && func_ea < seg_end)
    {
        func_name = GetFunctionName(func_ea);
        if (func_name == 0)
            func_name = "";
        col_fea = form("%08X", func_ea);
        fprintf(handle, "%s%s%s%s%s%s%s%s%s\n",
            col_idx,   FWV_SEP,
            col_sea,   FWV_SEP,
            col_sname, FWV_SEP,
            col_fea,   FWV_SEP,
            func_name);
        found_any = 1;
        func_ea = NextFunction(func_ea);
    }

    if (!found_any)
    {
        // Emit a placeholder row so every segment appears in the output
        fprintf(handle, "%s%s%s%s%s%s%s%s%s\n",
            col_idx,   FWV_SEP,
            col_sea,   FWV_SEP,
            col_sname, FWV_SEP,
            "00000000", FWV_SEP,
            "(none)");
    }
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------

static main(void)
{
    auto idb_path, idb_dir, db_name, out_path;
    auto handle;
    auto seg_ea, seg_end, seg_name, seg_idx;
    auto hdr, sep;

    Batch(1);

    idb_path = GetIdbPath();
    idb_dir  = path_dir(idb_path);
    db_name  = strip_ext(path_basename(idb_path));
    out_path = idb_dir + db_name + ".ovr-func-map.fwv";

    Message("ida55_ovr_func_map: output -> %s\n", out_path);

    handle = fopen(out_path, "w");
    if (handle == 0)
    {
        Message("ida55_ovr_func_map: ERROR - could not open: %s\n", out_path);
        Batch(0);
        return;
    }

    // Header
    fprintf(handle, "# ida55_ovr_func_map fwv-v1\n");
    fprintf(handle, "# db:         %s\n", db_name);
    fprintf(handle, "# idb_path:   %s\n", idb_path);
    fprintf(handle, "# input_file: %s\n", GetInputFile());
    fprintf(handle, "# SEG_IDX == AX at INT 3Fh handler (Borland overlay loader)\n");
    fprintf(handle, "#\n");

    hdr = form("%s%s%s%s%s%s%s%s%s",
        pad_right("SEG_IDX",  FWV_COL_SEGIDX), FWV_SEP,
        pad_right("SEG_EA",   FWV_COL_SEGEA),  FWV_SEP,
        pad_right("SEG_NAME", FWV_COL_SEGNAM), FWV_SEP,
        pad_right("FUNC_EA",  FWV_COL_FUNCEA), FWV_SEP,
        "FUNC_NAME");
    sep = form("%s%s%s%s%s%s%s%s%s",
        pad_right("----",     FWV_COL_SEGIDX), FWV_SEP,
        pad_right("--------", FWV_COL_SEGEA),  FWV_SEP,
        pad_right("----------", FWV_COL_SEGNAM), FWV_SEP,
        pad_right("--------", FWV_COL_FUNCEA), FWV_SEP,
        "---------");

    fprintf(handle, "%s\n", hdr);
    fprintf(handle, "%s\n", sep);

    // Walk segments in table order — index matches Borland segment table
    seg_idx = 0;
    seg_ea  = FirstSeg();
    while (seg_ea != BADADDR)
    {
        seg_end  = SegEnd(seg_ea);
        seg_name = SegName(seg_ea);
        if (seg_name == 0)
            seg_name = "";

        dump_seg_functions(handle, seg_idx, seg_ea, seg_name, seg_end);

        seg_idx = seg_idx + 1;
        seg_ea  = NextSeg(seg_ea);
    }

    fclose(handle);
    Message("ida55_ovr_func_map: wrote %d segments -> %s\n", seg_idx, out_path);

    Batch(0);
}
