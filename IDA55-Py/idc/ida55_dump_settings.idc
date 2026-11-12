#include <idc.idc>

static write_line(handle, key, value)
{
  fprintf(handle, "%s=%s\n", key, value);
}

static write_num(handle, key, value)
{
  fprintf(handle, "%s=%d\n", key, value);
}

static write_bool(handle, key, value)
{
  if ( value )
    fprintf(handle, "%s=1\n", key);
  else
    fprintf(handle, "%s=0\n", key);
}

static write_hex(handle, key, value)
{
  fprintf(handle, "%s=%a\n", key, value);
}

static write_segment_summary(handle)
{
  auto seg, idx;

  seg = FirstSeg();
  idx = 0;
  while ( seg != BADADDR )
  {
    fprintf(handle, "segment[%d].name=%s\n", idx, SegName(seg));
    fprintf(handle, "segment[%d].start=%a\n", idx, seg);
    fprintf(handle, "segment[%d].end=%a\n", idx, SegEnd(seg));
    fprintf(handle, "segment[%d].align=%d\n", idx, GetSegmentAttr(seg, SEGATTR_ALIGN));
    fprintf(handle, "segment[%d].comb=%d\n", idx, GetSegmentAttr(seg, SEGATTR_COMB));
    seg = NextSeg(seg);
    idx = idx + 1;
  }
  write_num(handle, "segment_count", idx);
}

static main()
{
  auto out_path, handle;

  out_path = GetIdbPath() + ".ida55-settings.txt";
  handle = fopen(out_path, "w");
  if ( handle == 0 )
  {
    Warning("Could not open output file: %s", out_path);
    return;
  }

  write_line(handle, "idb_path", GetIdbPath());
  write_line(handle, "input_file", GetInputFile());
  write_line(handle, "input_file_path", GetInputFilePath());
  write_hex(handle, "screen_ea", ScreenEA());
  write_hex(handle, "begin_ea", BeginEA());
  write_hex(handle, "min_ea", MinEA());
  write_hex(handle, "max_ea", MaxEA());
  write_num(handle, "indent", GetCharPrm(INF_INDENT));
  write_num(handle, "comment_indent", GetCharPrm(INF_COMMENT));
  write_num(handle, "tabs", GetCharPrm(INF_ENTAB));
  write_num(handle, "voids", GetCharPrm(INF_VOIDS));
  write_num(handle, "xref_show", GetCharPrm(INF_XREFNUM));
  write_num(handle, "tail_depth", GetLongPrm(INF_MAXREF));
  write_num(handle, "comment_flags", GetCharPrm(INF_CMTFLAG));
  write_bool(handle, "comments_all", (GetCharPrm(INF_CMTFLAG) & SW_ALLCMT) != 0);
  write_segment_summary(handle);

  fclose(handle);
  Message("Wrote IDA settings dump to %s\n", out_path);
}
