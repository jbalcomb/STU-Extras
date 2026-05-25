#include <idc.idc>

#define SEGMENT_FWV_PATH "C:/STU/devel/STU-Extras/Piethawn/Piethawn/ida_automation/ida_duplicate_segments_mgc_wzd.fwv"
#define MGC_PART_PATH    "C:/STU/devel/STU-Extras/Piethawn/Piethawn/ida_automation/ida_duplicate_procedures_mgc.part.fwv"
#define WZD_PART_PATH    "C:/STU/devel/STU-Extras/Piethawn/Piethawn/ida_automation/ida_duplicate_procedures_wzd.part.fwv"
#define OUTPUT_FWV_PATH  "C:/STU/devel/STU-Extras/Piethawn/Piethawn/ida_automation/ida_duplicate_procedures_mgc_wzd.fwv"

static trim_line(text)
{
  auto start, end, ch;

  start = 0;
  end = strlen(text);

  while ( start < end )
  {
    ch = substr(text, start, start + 1);
    if ( ch != " " && ch != "\t" && ch != "\r" && ch != "\n" )
      break;
    start = start + 1;
  }

  while ( end > start )
  {
    ch = substr(text, end - 1, end);
    if ( ch != " " && ch != "\t" && ch != "\r" && ch != "\n" )
      break;
    end = end - 1;
  }

  return substr(text, start, end);
}

static read_text_line(handle)
{
  auto line, ch;

  line = "";
  ch = fgetc(handle);
  while ( ch != -1 )
  {
    if ( ch == 0x0A )
      return line;
    if ( ch != 0x0D )
      line = line + ch;
    ch = fgetc(handle);
  }

  if ( line != "" )
    return line;
  return -1;
}

static first_word(text)
{
  auto i, ch;

  text = trim_line(text);
  i = 0;
  while ( i < strlen(text) )
  {
    ch = substr(text, i, i + 1);
    if ( ch == " " || ch == "\t" )
      return substr(text, 0, i);
    i = i + 1;
  }
  return text;
}

static second_word(text)
{
  auto i, ch;

  text = trim_line(text);
  i = 0;
  while ( i < strlen(text) )
  {
    ch = substr(text, i, i + 1);
    if ( ch == " " || ch == "\t" )
      break;
    i = i + 1;
  }
  while ( i < strlen(text) )
  {
    ch = substr(text, i, i + 1);
    if ( ch != " " && ch != "\t" )
      break;
    i = i + 1;
  }
  return first_word(substr(text, i, -1));
}

static rest_after_second_word(text)
{
  auto i, ch;

  text = trim_line(text);
  i = 0;
  while ( i < strlen(text) )
  {
    ch = substr(text, i, i + 1);
    if ( ch == " " || ch == "\t" )
      break;
    i = i + 1;
  }
  while ( i < strlen(text) )
  {
    ch = substr(text, i, i + 1);
    if ( ch != " " && ch != "\t" )
      break;
    i = i + 1;
  }
  while ( i < strlen(text) )
  {
    ch = substr(text, i, i + 1);
    if ( ch == " " || ch == "\t" )
      break;
    i = i + 1;
  }
  return trim_line(substr(text, i, -1));
}

static idb_kind(void)
{
  auto path, input;

  path = GetIdbPath();
  input = GetInputFile();
  if ( strstr(path, "MAGIC") != -1 || strstr(input, "MAGIC") != -1 )
    return "MGC";
  if ( strstr(path, "WIZARDS") != -1 || strstr(input, "WIZARDS") != -1 )
    return "WZD";
  return "";
}

static procedure_segment_id(segment)
{
  if ( substr(segment, 0, 3) == "seg" && strlen(segment) == 6 )
    return "s" + substr(segment, 3, 6);
  return segment;
}

static zero_pad3(value)
{
  if ( value < 10 )
    return form("00%d", value);
  if ( value < 100 )
    return form("0%d", value);
  return form("%d", value);
}

static procedure_id(segment, ordinal, name)
{
  if ( name == "" )
    name = "missing";
  return procedure_segment_id(segment) + "p" + zero_pad3(ordinal) + ":" + name;
}

static emit_current_idb_part(part_path)
{
  auto handle, ea, segment, ordinal, last_segment;

  handle = fopen(part_path, "w");
  if ( handle == 0 )
  {
    Message("DUPPROC open-failed %s\n", part_path);
    return 0;
  }

  last_segment = "";
  ordinal = 0;
  ea = NextFunction(0);
  while ( ea != BADADDR )
  {
    segment = SegName(GetFunctionAttr(ea, FUNCATTR_START));
    if ( segment != last_segment )
    {
      last_segment = segment;
      ordinal = 1;
    }
    else
    {
      ordinal = ordinal + 1;
    }

    fprintf(handle, "%s %d %s\n", segment, ordinal, procedure_id(segment, ordinal, GetFunctionName(ea)));
    ea = NextFunction(ea);
  }

  fclose(handle);
  Message("DUPPROC wrote current IDB part %s\n", part_path);
  return 1;
}

static lookup_part_id(part_path, segment, ordinal)
{
  auto handle, line, seg, ord, id_text;

  handle = fopen(part_path, "r");
  if ( handle == 0 )
    return "";

  line = read_text_line(handle);
  while ( line != -1 )
  {
    seg = first_word(line);
    ord = atol(second_word(line));
    if ( seg == segment && ord == ordinal )
    {
      id_text = rest_after_second_word(line);
      fclose(handle);
      return id_text;
    }
    line = read_text_line(handle);
  }

  fclose(handle);
  return "";
}

static part_has_segment_ordinal(part_path, segment, ordinal)
{
  return lookup_part_id(part_path, segment, ordinal) != "";
}

static max_left_width(mgc_part, wzd_part)
{
  auto width, segment_handle, line, mgc_segment, wzd_segment, ordinal, id_text;

  width = strlen("MAGIC");
  segment_handle = fopen(SEGMENT_FWV_PATH, "r");
  if ( segment_handle == 0 )
    return width;

  line = read_text_line(segment_handle);
  while ( line != -1 )
  {
    mgc_segment = first_word(line);
    wzd_segment = second_word(line);
    if ( mgc_segment != "" && mgc_segment != "MAGIC" )
    {
      ordinal = 1;
      while ( part_has_segment_ordinal(mgc_part, mgc_segment, ordinal) || part_has_segment_ordinal(wzd_part, wzd_segment, ordinal) )
      {
        id_text = lookup_part_id(mgc_part, mgc_segment, ordinal);
        if ( id_text == "" )
          id_text = procedure_id(mgc_segment, ordinal, "");
        if ( strlen(id_text) > width )
          width = strlen(id_text);
        ordinal = ordinal + 1;
      }
    }
    line = read_text_line(segment_handle);
  }

  fclose(segment_handle);
  return width;
}

static write_padding(handle, count)
{
  while ( count > 0 )
  {
    fprintf(handle, " ");
    count = count - 1;
  }
}

static write_fixed_row(handle, left_width, left_text, right_text)
{
  fprintf(handle, "%s", left_text);
  write_padding(handle, left_width - strlen(left_text));
  fprintf(handle, "  %s\n", right_text);
}

static combine_parts(mgc_part, wzd_part)
{
  auto out_handle, segment_handle, line, mgc_segment, wzd_segment, ordinal;
  auto left_width, left_id, right_id, rows;

  segment_handle = fopen(SEGMENT_FWV_PATH, "r");
  if ( segment_handle == 0 )
  {
    Message("DUPPROC open-failed %s\n", SEGMENT_FWV_PATH);
    return 0;
  }

  out_handle = fopen(OUTPUT_FWV_PATH, "w");
  if ( out_handle == 0 )
  {
    fclose(segment_handle);
    Message("DUPPROC open-failed %s\n", OUTPUT_FWV_PATH);
    return 0;
  }

  left_width = max_left_width(mgc_part, wzd_part);
  rows = 0;
  write_fixed_row(out_handle, left_width, "MAGIC", "WIZARDS");

  line = read_text_line(segment_handle);
  while ( line != -1 )
  {
    mgc_segment = first_word(line);
    wzd_segment = second_word(line);
    if ( mgc_segment != "" && mgc_segment != "MAGIC" )
    {
      ordinal = 1;
      while ( part_has_segment_ordinal(mgc_part, mgc_segment, ordinal) || part_has_segment_ordinal(wzd_part, wzd_segment, ordinal) )
      {
        left_id = lookup_part_id(mgc_part, mgc_segment, ordinal);
        right_id = lookup_part_id(wzd_part, wzd_segment, ordinal);
        if ( left_id == "" )
          left_id = procedure_id(mgc_segment, ordinal, "");
        if ( right_id == "" )
          right_id = procedure_id(wzd_segment, ordinal, "");
        write_fixed_row(out_handle, left_width, left_id, right_id);
        rows = rows + 1;
        ordinal = ordinal + 1;
      }
    }
    line = read_text_line(segment_handle);
  }

  fclose(segment_handle);
  fclose(out_handle);
  Message("DUPPROC wrote %s rows=%d\n", OUTPUT_FWV_PATH, rows);
  return 1;
}

static file_exists(path)
{
  auto handle;

  handle = fopen(path, "r");
  if ( handle == 0 )
    return 0;
  fclose(handle);
  return 1;
}

static main(void)
{
  auto old_batch, kind, part_path;

  old_batch = Batch(1);
  kind = idb_kind();
  if ( kind == "" )
  {
    Message("DUPPROC could not identify current IDB as MAGIC or WIZARDS: %s\n", GetIdbPath());
    Batch(old_batch);
    Exit(2);
  }

  if ( kind == "MGC" )
    part_path = MGC_PART_PATH;
  else
    part_path = WZD_PART_PATH;

  if ( !emit_current_idb_part(part_path) )
  {
    Batch(old_batch);
    Exit(3);
  }

  Message("DUPPROC part extraction complete. Run combine_duplicate_procedure_parts.sh after both IDBs are extracted.\n");

  Batch(old_batch);
  Exit(0);
}
