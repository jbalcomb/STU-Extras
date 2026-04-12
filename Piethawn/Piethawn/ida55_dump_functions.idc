#include <idc.idc>

static write_line(handle, key, value)
{
  fprintf(handle, "%s=%s\n", key, value);
}

static write_hex(handle, key, value)
{
  fprintf(handle, "%s=%a\n", key, value);
}

static write_num(handle, key, value)
{
  fprintf(handle, "%s=%d\n", key, value);
}

static write_byte_hex(handle, value)
{
  fprintf(handle, "%02X", value & 0xFF);
}

static dump_segment_summary(handle)
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

static dump_chunk_bytes(handle, prefix, start_ea, end_ea)
{
  auto ea;

  fprintf(handle, "%s.bytes=", prefix);
  for ( ea = start_ea; ea < end_ea; ea = ea + 1 )
    write_byte_hex(handle, Byte(ea));
  fprintf(handle, "\n");
}

static dump_function_chunks(handle, funcea, func_idx)
{
  auto tailea, chunk_idx, chunk_start, chunk_end, refqty;

  tailea = FirstFuncFchunk(funcea);
  chunk_idx = 0;
  while ( tailea != BADADDR )
  {
    chunk_start = GetFchunkAttr(tailea, FUNCATTR_START);
    chunk_end = GetFchunkAttr(tailea, FUNCATTR_END);

    fprintf(handle, "function[%d].chunk[%d].start=%a\n", func_idx, chunk_idx, chunk_start);
    fprintf(handle, "function[%d].chunk[%d].end=%a\n", func_idx, chunk_idx, chunk_end);
    fprintf(handle, "function[%d].chunk[%d].size=%d\n", func_idx, chunk_idx, chunk_end - chunk_start);
    dump_chunk_bytes(handle, form("function[%d].chunk[%d]", func_idx, chunk_idx), chunk_start, chunk_end);

    chunk_idx = chunk_idx + 1;
    tailea = NextFuncFchunk(funcea, tailea);
  }

  write_num(handle, form("function[%d].chunk_count", func_idx), chunk_idx);

  refqty = GetFunctionAttr(funcea, FUNCATTR_REFQTY);
  write_num(handle, form("function[%d].tail_refqty", func_idx), refqty);
}

static dump_functions(handle)
{
  auto ea, idx, start_ea, end_ea, flags;

  ea = NextFunction(0);
  idx = 0;
  while ( ea != BADADDR )
  {
    start_ea = GetFunctionAttr(ea, FUNCATTR_START);
    end_ea = GetFunctionAttr(ea, FUNCATTR_END);
    flags = GetFunctionAttr(ea, FUNCATTR_FLAGS);

    fprintf(handle, "function[%d].name=%s\n", idx, GetFunctionName(ea));
    fprintf(handle, "function[%d].segment=%s\n", idx, SegName(ea));
    fprintf(handle, "function[%d].entry_ea=%a\n", idx, ea);
    fprintf(handle, "function[%d].start_ea=%a\n", idx, start_ea);
    fprintf(handle, "function[%d].end_ea=%a\n", idx, end_ea);
    fprintf(handle, "function[%d].size=%d\n", idx, end_ea - start_ea);
    fprintf(handle, "function[%d].flags=%d\n", idx, flags);
    fprintf(handle, "function[%d].frame=%d\n", idx, GetFunctionAttr(ea, FUNCATTR_FRAME));
    fprintf(handle, "function[%d].frsize=%d\n", idx, GetFunctionAttr(ea, FUNCATTR_FRSIZE));
    fprintf(handle, "function[%d].frregs=%d\n", idx, GetFunctionAttr(ea, FUNCATTR_FRREGS));
    fprintf(handle, "function[%d].argsize=%d\n", idx, GetFunctionAttr(ea, FUNCATTR_ARGSIZE));
    fprintf(handle, "function[%d].fpd=%d\n", idx, GetFunctionAttr(ea, FUNCATTR_FPD));
    dump_function_chunks(handle, ea, idx);

    idx = idx + 1;
    ea = NextFunction(ea);
  }

  write_num(handle, "function_count", idx);
}

static main()
{
  auto out_path, handle;

  out_path = GetIdbPath() + ".ida55-functions.txt";
  handle = fopen(out_path, "w");
  if ( handle == 0 )
  {
    Warning("Could not open output file: %s", out_path);
    return;
  }

  write_line(handle, "idb_path", GetIdbPath());
  write_line(handle, "input_file", GetInputFile());
  write_line(handle, "input_file_path", GetInputFilePath());
  write_hex(handle, "min_ea", MinEA());
  write_hex(handle, "max_ea", MaxEA());
  write_hex(handle, "begin_ea", BeginEA());
  dump_segment_summary(handle);
  dump_functions(handle);

  fclose(handle);
  Message("Wrote IDA function dump to %s\n", out_path);
}
