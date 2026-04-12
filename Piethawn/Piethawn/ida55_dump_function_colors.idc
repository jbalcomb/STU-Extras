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

static dump_functions(handle)
{
  auto ea, idx, start_ea, end_ea, color_attr, color_item;

  ea = NextFunction(0);
  idx = 0;
  while ( ea != BADADDR )
  {
    start_ea = GetFunctionAttr(ea, FUNCATTR_START);
    end_ea = GetFunctionAttr(ea, FUNCATTR_END);
    color_attr = GetFunctionAttr(ea, FUNCATTR_COLOR);
    color_item = GetColor(ea, CIC_FUNC);

    fprintf(handle, "function[%d].name=%s\n", idx, GetFunctionName(ea));
    fprintf(handle, "function[%d].segment=%s\n", idx, SegName(ea));
    fprintf(handle, "function[%d].entry_ea=%a\n", idx, ea);
    fprintf(handle, "function[%d].start_ea=%a\n", idx, start_ea);
    fprintf(handle, "function[%d].end_ea=%a\n", idx, end_ea);
    fprintf(handle, "function[%d].size=%d\n", idx, end_ea - start_ea);
    fprintf(handle, "function[%d].funcattr_color=%d\n", idx, color_attr);
    fprintf(handle, "function[%d].funcattr_color_hex=%08X\n", idx, color_attr);
    fprintf(handle, "function[%d].item_color=%d\n", idx, color_item);
    fprintf(handle, "function[%d].item_color_hex=%08X\n", idx, color_item);
    fprintf(handle, "function[%d].has_user_color=%d\n", idx, color_item != DEFCOLOR);

    idx = idx + 1;
    ea = NextFunction(ea);
  }

  write_num(handle, "function_count", idx);
}

static main()
{
  auto out_path, handle;

  out_path = GetIdbPath() + ".ida55-function-colors.txt";
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
  write_num(handle, "default_color", DEFCOLOR);
  fprintf(handle, "default_color_hex=%08X\n", DEFCOLOR);
  dump_functions(handle);

  fclose(handle);
  Message("Wrote IDA function color dump to %s\n", out_path);
}
