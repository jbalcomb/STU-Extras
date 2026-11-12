#include <idc.idc>

static write_line(handle, key, value)
{
  fprintf(handle, "%s=%s\n", key, value);
}

static write_segment_summary(handle)
{
  auto seg, idx, line;

  seg = FirstSeg();
  idx = 0;
  while ( seg != BADADDR )
  {
    fprintf(handle, "segment[%d]=%s,%a,%a\n", idx, SegName(seg), seg, SegEnd(seg));
    seg = NextSeg(seg);
    idx = idx + 1;
  }
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
  write_line(handle, "root_filename", GetRootFilename());
  write_line(handle, "begin_ea", ltoa(BeginEA(), 16));
  write_line(handle, "min_ea", ltoa(MinEA(), 16));
  write_line(handle, "max_ea", ltoa(MaxEA(), 16));
  write_segment_summary(handle);

  fclose(handle);
  Message("Wrote IDA settings dump to %s\n", out_path);
}
