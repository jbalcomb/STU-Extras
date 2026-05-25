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

static idb_base_path(void)
{
  auto path, dot, scan, next;

  path = GetIdbPath();
  dot = strstr(path, ".");
  scan = dot + 1;
  while ( dot != -1 )
  {
    next = strstr(substr(path, scan, -1), ".");
    if ( next == -1 )
      break;
    dot = scan + next;
    scan = dot + 1;
  }

  if ( dot == -1 )
    return path;
  return substr(path, 0, dot);
}

static dump_export_settings(out_path, phase)
{
  auto handle;

  handle = fopen(out_path, "w");
  if ( handle == 0 )
  {
    Warning("Could not open settings dump: %s", out_path);
    return 0;
  }

  write_line(handle, "phase", phase);
  write_line(handle, "idb_path", GetIdbPath());
  write_line(handle, "input_file", GetInputFile());
  write_line(handle, "input_file_path", GetInputFilePath());
  write_hex(handle, "screen_ea", ScreenEA());
  write_hex(handle, "begin_ea", BeginEA());
  write_hex(handle, "min_ea", MinEA());
  write_hex(handle, "max_ea", MaxEA());
  write_num(handle, "INF_ENTAB", GetCharPrm(INF_ENTAB));
  write_num(handle, "INF_INDENT", GetCharPrm(INF_INDENT));
  write_num(handle, "INF_COMMENT", GetCharPrm(INF_COMMENT));
  write_num(handle, "INF_XREFNUM", GetCharPrm(INF_XREFNUM));
  write_num(handle, "INF_SHOWPREF", GetCharPrm(INF_SHOWPREF));
  write_num(handle, "INF_BORDER", GetCharPrm(INF_BORDER));
  write_num(handle, "INF_PREFSEG", GetCharPrm(INF_PREFSEG));
  write_num(handle, "INF_PREFFLAG", GetCharPrm(INF_PREFFLAG));
  write_num(handle, "INF_CMTFLAG", GetCharPrm(INF_CMTFLAG));
  write_num(handle, "INF_ORG", GetCharPrm(INF_ORG));
  write_num(handle, "INF_ASSUME", GetCharPrm(INF_ASSUME));
  write_num(handle, "INF_BINPREF", GetShortPrm(INF_BINPREF));
  write_num(handle, "INF_ASMTYPE", GetCharPrm(INF_ASMTYPE));
  write_num(handle, "INF_MAXREF", GetLongPrm(INF_MAXREF));

  fclose(handle);
  Message("Wrote settings dump: %s\n", out_path);
  return 1;
}

static write_restore_script(out_path)
{
  auto handle;

  handle = fopen(out_path, "w");
  if ( handle == 0 )
  {
    Warning("Could not open restore script: %s", out_path);
    return 0;
  }

  fprintf(handle, "#include <idc.idc>\n\n");
  fprintf(handle, "static main(void)\n{\n");
  fprintf(handle, "  SetCharPrm(INF_ENTAB, %d);\n", GetCharPrm(INF_ENTAB));
  fprintf(handle, "  SetCharPrm(INF_INDENT, %d);\n", GetCharPrm(INF_INDENT));
  fprintf(handle, "  SetCharPrm(INF_COMMENT, %d);\n", GetCharPrm(INF_COMMENT));
  fprintf(handle, "  SetCharPrm(INF_XREFNUM, %d);\n", GetCharPrm(INF_XREFNUM));
  fprintf(handle, "  SetCharPrm(INF_SHOWPREF, %d);\n", GetCharPrm(INF_SHOWPREF));
  fprintf(handle, "  SetCharPrm(INF_BORDER, %d);\n", GetCharPrm(INF_BORDER));
  fprintf(handle, "  SetCharPrm(INF_PREFSEG, %d);\n", GetCharPrm(INF_PREFSEG));
  fprintf(handle, "  SetCharPrm(INF_PREFFLAG, %d);\n", GetCharPrm(INF_PREFFLAG));
  fprintf(handle, "  SetCharPrm(INF_CMTFLAG, %d);\n", GetCharPrm(INF_CMTFLAG));
  fprintf(handle, "  SetCharPrm(INF_ORG, %d);\n", GetCharPrm(INF_ORG));
  fprintf(handle, "  SetCharPrm(INF_ASSUME, %d);\n", GetCharPrm(INF_ASSUME));
  fprintf(handle, "  SetShortPrm(INF_BINPREF, %d);\n", GetShortPrm(INF_BINPREF));
  fprintf(handle, "  SetCharPrm(INF_ASMTYPE, %d);\n", GetCharPrm(INF_ASMTYPE));
  fprintf(handle, "  SetLongPrm(INF_MAXREF, %d);\n", GetLongPrm(INF_MAXREF));
  fprintf(handle, "  Message(\"Restored IDA55 clean export settings.\\n\");\n");
  fprintf(handle, "}\n");

  fclose(handle);
  Message("Wrote restore script: %s\n", out_path);
  return 1;
}

static apply_clean_export_profile(void)
{
  SetCharPrm(INF_ENTAB, 0);
  SetCharPrm(INF_INDENT, 0);
  SetCharPrm(INF_COMMENT, 0);
  SetCharPrm(INF_XREFNUM, 0);
  SetCharPrm(INF_SHOWPREF, 0);
  SetCharPrm(INF_BORDER, 0);
  SetCharPrm(INF_PREFFLAG, 0);
  SetCharPrm(INF_CMTFLAG, SW_NOCMT);
  SetCharPrm(INF_ORG, 1);
  SetCharPrm(INF_ASSUME, 1);
  SetShortPrm(INF_BINPREF, 0);
  SetCharPrm(INF_ASMTYPE, 1);
  SetLongPrm(INF_MAXREF, 0);
}

static unhide_all_for_export(void)
{
  auto ea, flags, changed_functions, expanded_areas, touched_heads, max_ea;

  changed_functions = 0;
  expanded_areas = 0;
  touched_heads = 0;
  max_ea = MaxEA();

  ea = NextFunction(0);
  while ( ea != BADADDR )
  {
    flags = GetFunctionFlags(ea);
    if ( flags != -1 && (flags & FUNC_HIDDEN) )
    {
      if ( SetFunctionFlags(ea, flags & ~FUNC_HIDDEN) )
        changed_functions = changed_functions + 1;
    }
    if ( SetHiddenArea(ea, 1) )
      expanded_areas = expanded_areas + 1;
    ea = NextFunction(ea);
  }

  ea = MinEA();
  while ( ea != BADADDR && ea < max_ea )
  {
    touched_heads = touched_heads + 1;
    if ( SetHiddenArea(ea, 1) )
      expanded_areas = expanded_areas + 1;
    ea = NextHead(ea, max_ea);
  }

  Message("EXPORT unhide-all changed_functions=%d expanded_areas=%d touched_heads=%d\n",
          changed_functions, expanded_areas, touched_heads);
  return changed_functions + expanded_areas;
}

static generate_one_file(kind, out_path, ofile, flags)
{
  auto handle, result;

  handle = fopen(out_path, "w");
  if ( handle == 0 )
  {
    Message("EXPORT open-failed %s %s\n", kind, out_path);
    return 0;
  }

  result = GenerateFile(ofile, handle, 0, BADADDR, flags);
  fclose(handle);
  Message("EXPORT generated %s %s result=%d\n", kind, out_path, result);

  if ( result == -1 )
    return 0;
  return 1;
}

static generate_clean_exports(base)
{
  auto ok;

  ok = 1;
  if ( !generate_one_file("map", base + ".map", OFILE_MAP, 0) )
    ok = 0;
  if ( !generate_one_file("asm", base + ".asm", OFILE_ASM, 0) )
    ok = 0;
  if ( !generate_one_file("inc", base + ".inc", OFILE_ASM, GENFLG_ASMTYPE | GENFLG_ASMINC) )
    ok = 0;
  if ( !generate_one_file("lst", base + ".lst", OFILE_LST, 0) )
    ok = 0;
  return ok;
}

static main(void)
{
  auto old_batch, base, ok;
  auto original_entab, original_indent, original_comment, original_xrefnum;
  auto original_showpref, original_border, original_prefseg, original_prefflag, original_cmtflag;
  auto original_org, original_assume, original_binpref, original_asmtype, original_maxref;

  old_batch = Batch(1);
  Wait();

  base = idb_base_path();
  ok = 1;

  original_entab = GetCharPrm(INF_ENTAB);
  original_indent = GetCharPrm(INF_INDENT);
  original_comment = GetCharPrm(INF_COMMENT);
  original_xrefnum = GetCharPrm(INF_XREFNUM);
  original_showpref = GetCharPrm(INF_SHOWPREF);
  original_border = GetCharPrm(INF_BORDER);
  original_prefseg = GetCharPrm(INF_PREFSEG);
  original_prefflag = GetCharPrm(INF_PREFFLAG);
  original_cmtflag = GetCharPrm(INF_CMTFLAG);
  original_org = GetCharPrm(INF_ORG);
  original_assume = GetCharPrm(INF_ASSUME);
  original_binpref = GetShortPrm(INF_BINPREF);
  original_asmtype = GetCharPrm(INF_ASMTYPE);
  original_maxref = GetLongPrm(INF_MAXREF);

  dump_export_settings(base + ".ida55-clean-export-before.txt", "before");
  write_restore_script(base + ".ida55-clean-export-restore.idc");

  apply_clean_export_profile();
  unhide_all_for_export();
  dump_export_settings(base + ".ida55-clean-export-active.txt", "active");

  if ( !generate_clean_exports(base) )
    ok = 0;

  SetCharPrm(INF_ENTAB, original_entab);
  SetCharPrm(INF_INDENT, original_indent);
  SetCharPrm(INF_COMMENT, original_comment);
  SetCharPrm(INF_XREFNUM, original_xrefnum);
  SetCharPrm(INF_SHOWPREF, original_showpref);
  SetCharPrm(INF_BORDER, original_border);
  SetCharPrm(INF_PREFSEG, original_prefseg);
  SetCharPrm(INF_PREFFLAG, original_prefflag);
  SetCharPrm(INF_CMTFLAG, original_cmtflag);
  SetCharPrm(INF_ORG, original_org);
  SetCharPrm(INF_ASSUME, original_assume);
  SetShortPrm(INF_BINPREF, original_binpref);
  SetCharPrm(INF_ASMTYPE, original_asmtype);
  SetLongPrm(INF_MAXREF, original_maxref);
  dump_export_settings(base + ".ida55-clean-export-after.txt", "after");

  Batch(old_batch);

  if ( ok )
  {
    Message("IDA55 clean export completed.\n");
    Exit(0);
  }

  Warning("IDA55 clean export failed. Run the generated restore IDC if settings look wrong.");
  Exit(1);
}
