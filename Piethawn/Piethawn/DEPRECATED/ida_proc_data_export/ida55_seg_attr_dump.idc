// ida55_seg_attr_dump.idc
//
// Diagnostic: dump raw segment attributes for all segments so we can
// identify the correct SEGATTR_* value to use as an overlay filter.
//
// Output: <idb_dir>\<db_basename>-seg-attr-dump.fwv
//
// Columns: SEG_IDX  SEG_EA  SEGATTR_TYPE  SEGATTR_COMB  SEGATTR_SEL  SEG_NAME
//
// Cross-reference output against WIZARDS.borland-ovrinfo.json to identify
// which attribute distinguishes CODE+OVERLAY segments from CODE segments.

#include <idc.idc>

// Two versions of SEGATTR constants exist in idc.idc (IDA 4.x vs 5.x layout).
// Use the second (larger) set which matches IDA 5.5:
#define MY_SEGATTR_TYPE   182
#define MY_SEGATTR_COMB    41
#define MY_SEGATTR_SEL     46

static pad_right(s, width)
{
    auto result, slen, i;
    result = s;
    slen = strlen(s);
    for (i = slen; i < width; i = i + 1)
        result = result + " ";
    return result;
}

static last_pos_of(s, ch)
{
    auto i, found;
    found = -1;
    for (i = 0; i < strlen(s); i = i + 1)
        if (substr(s, i, i + 1) == ch)
            found = i;
    return found;
}

static path_dir(path)
{
    auto sep;
    sep = last_pos_of(path, "\\");
    if (sep == -1) sep = last_pos_of(path, "/");
    if (sep == -1) return "";
    return substr(path, 0, sep + 1);
}

static strip_ext(name)
{
    auto dot;
    dot = last_pos_of(name, ".");
    if (dot == -1) return name;
    return substr(name, 0, dot);
}

static path_basename(path)
{
    auto sep;
    sep = last_pos_of(path, "\\");
    if (sep == -1) sep = last_pos_of(path, "/");
    if (sep == -1) return path;
    return substr(path, sep + 1, -1);
}

static main(void)
{
    auto idb_path, idb_dir, db_name, out_path;
    auto handle;
    auto seg_ea, seg_name, seg_idx;
    auto t, comb, sel;
    auto hdr;

    Batch(1);

    idb_path = GetIdbPath();
    idb_dir  = path_dir(idb_path);
    db_name  = strip_ext(path_basename(idb_path));
    out_path = idb_dir + db_name + "-seg-attr-dump.fwv";

    Message("ida55_seg_attr_dump: output -> %s\n", out_path);

    handle = fopen(out_path, "w");
    if (handle == 0)
    {
        Message("ida55_seg_attr_dump: ERROR - could not open: %s\n", out_path);
        Batch(0);
        return;
    }

    fprintf(handle, "# ida55_seg_attr_dump fwv-v1\n");
    fprintf(handle, "# db: %s\n", db_name);
    fprintf(handle, "#\n");

    hdr = form("%-6s %-10s %-6s %-6s %-6s %s",
        "IDX", "SEG_EA", "TYPE", "COMB", "SEL", "SEG_NAME");
    fprintf(handle, "%s\n", hdr);
    fprintf(handle, "------ ---------- ------ ------ ------ --------\n");

    seg_idx = 0;
    seg_ea  = FirstSeg();
    while (seg_ea != BADADDR)
    {
        seg_name = SegName(seg_ea);
        if (seg_name == 0) seg_name = "";
        t    = GetSegmentAttr(seg_ea, MY_SEGATTR_TYPE);
        comb = GetSegmentAttr(seg_ea, MY_SEGATTR_COMB);
        sel  = GetSegmentAttr(seg_ea, MY_SEGATTR_SEL);

        fprintf(handle, "%-6d 0x%08X %-6d %-6d %-6d %s\n",
            seg_idx, seg_ea, t, comb, sel, seg_name);

        seg_idx = seg_idx + 1;
        seg_ea  = NextSeg(seg_ea);
    }

    fclose(handle);
    Message("ida55_seg_attr_dump: wrote %d segments -> %s\n", seg_idx, out_path);

    Batch(0);
}
