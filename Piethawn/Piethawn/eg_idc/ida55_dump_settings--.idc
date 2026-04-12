#include <idc.idc>

static write_line(handle, key, value)
{
  auto line;
  line = key + "=" + value + "\n";
  fputs(line, handle);
}

static safe_get_short_prm(prm)
{
  return ltoa(GetShortPrm(prm), 10);
}

static safe_get_long_prm(prm)
{
  return ltoa(GetLongPrm(prm), 10);
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
  write_line(handle, "demname_mode", safe_get_short_prm(INF_DEMNAMES));
  write_line(handle, "indent", safe_get_short_prm(INF_INDENT));
  write_line(handle, "comment_margin", safe_get_short_prm(INF_CMTIND));
  write_line(handle, "xref_number_format", safe_get_short_prm(INF_XREFFLAG));
  write_line(handle, "outflags", safe_get_long_prm(INF_OUTFLAGS));
  write_line(handle, "filetype", safe_get_short_prm(INF_FILETYPE));
  write_line(handle, "begin_ea", ltoa(BeginEA(), 16));
  write_line(handle, "min_ea", ltoa(MinEA(), 16));
  write_line(handle, "max_ea", ltoa(MaxEA(), 16));

  fclose(handle);
  Message("Wrote IDA settings dump to %s\n", out_path);
}
