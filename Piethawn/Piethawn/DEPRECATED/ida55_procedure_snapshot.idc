#include <idc.idc>

static write_line(handle, key, value)
{
  fprintf(handle, "%s=%s\n", key, value);
}

static write_num(handle, key, value)
{
  fprintf(handle, "%s=%d\n", key, value);
}

static write_hex(handle, key, value)
{
  fprintf(handle, "%s=%a\n", key, value);
}

static dump_segments(handle)
{
  auto seg, idx;

  seg = FirstSeg();
  idx = 0;
  while ( seg != BADADDR )
  {
    fprintf(handle, "segment[%d].name=%s\n", idx, SegName(seg));
    fprintf(handle, "segment[%d].start_ea=%a\n", idx, seg);
    fprintf(handle, "segment[%d].end_ea=%a\n", idx, SegEnd(seg));
    idx = idx + 1;
    seg = NextSeg(seg);
  }
  write_num(handle, "segment_count", idx);
}

static dump_procedures(handle)
{
  auto ea, idx, start_ea, end_ea;

  ea = NextFunction(0);
  idx = 0;
  while ( ea != BADADDR )
  {
    start_ea = GetFunctionAttr(ea, FUNCATTR_START);
    end_ea = GetFunctionAttr(ea, FUNCATTR_END);

    fprintf(handle, "procedure[%d].name=%s\n", idx, GetFunctionName(ea));
    fprintf(handle, "procedure[%d].segment=%s\n", idx, SegName(start_ea));
    fprintf(handle, "procedure[%d].entry_ea=%a\n", idx, ea);
    fprintf(handle, "procedure[%d].start_ea=%a\n", idx, start_ea);
    fprintf(handle, "procedure[%d].end_ea=%a\n", idx, end_ea);
    fprintf(handle, "procedure[%d].size=%d\n", idx, end_ea - start_ea);
    fprintf(handle, "procedure[%d].flags=%d\n", idx, GetFunctionAttr(ea, FUNCATTR_FLAGS));

    idx = idx + 1;
    ea = NextFunction(ea);
  }
  write_num(handle, "procedure_count", idx);
}

static main(void)
{
  auto old_batch, out_path, handle;

  old_batch = Batch(1);
  out_path = GetIdbPath() + ".ida55-procedure-snapshot.txt";
  handle = fopen(out_path, "w");
  if ( handle == 0 )
  {
    Batch(old_batch);
    Exit(2);
  }

  write_line(handle, "snapshot_schema", "ida55-procedure-snapshot-v1");
  write_line(handle, "idb_path", GetIdbPath());
  write_line(handle, "input_file", GetInputFile());
  write_line(handle, "input_file_path", GetInputFilePath());
  write_hex(handle, "min_ea", MinEA());
  write_hex(handle, "max_ea", MaxEA());
  write_hex(handle, "begin_ea", BeginEA());
  dump_segments(handle);
  dump_procedures(handle);

  fclose(handle);
  Message("Wrote IDA55 procedure snapshot to %s\n", out_path);
  Batch(old_batch);
  Exit(0);
}

