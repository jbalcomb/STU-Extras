#include <idc.idc>

static main(void)
{
  auto old_batch, out_path, handle;

  old_batch = Batch(1);
  out_path = GetIdbPath() + ".ida55-probe.txt";
  handle = fopen(out_path, "w");
  if ( handle == 0 )
  {
    Batch(old_batch);
    Exit(2);
  }

  fprintf(handle, "probe=ok\n");
  fprintf(handle, "idb_path=%s\n", GetIdbPath());
  fprintf(handle, "input_file_path=%s\n", GetInputFilePath());
  fclose(handle);

  Message("IDA55 probe wrote %s\n", out_path);
  Batch(old_batch);
  Exit(0);
}
