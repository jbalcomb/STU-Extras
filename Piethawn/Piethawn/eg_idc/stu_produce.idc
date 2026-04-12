// J:\STU\WD\IDA55\idc\stu_produce.idc

// int GenerateFile(long type, long file_handle, long ea1, long ea2, long flags);

// WriteMap(file)
//        GenerateFile(OFILE_MAP, fopen(file,"w"), 0, BADADDR, GENFLG_MAPSEGS|GENFLG_MAPNAME)
// WriteTxt(file,ea1,ea2)
//        GenerateFile(OFILE_ASM,fopen(file,"w"), ea1, ea2, 0)
// WriteExe(file)
//        GenerateFile(OFILE_EXE,fopen(file,"wb"), 0, BADADDR, 0)

// output file types:
// OFILE_MAP  0
// OFILE_EXE  1
// OFILE_IDC  2
// OFILE_LST  3
// OFILE_ASM  4
// OFILE_DIF  5

// output control flags:
// GENFLG_MAPSEGS 0x0001          // map: generate map of segments
// GENFLG_MAPNAME 0x0002          // map: include dummy names
// GENFLG_MAPDMNG 0x0004          // map: demangle names
// GENFLG_MAPLOC  0x0008          // map: include local names
// GENFLG_IDCTYPE 0x0008          // idc: gen only information about types
// GENFLG_ASMTYPE 0x0010          // asm&lst: gen information about types too
// GENFLG_GENHTML 0x0020          // asm&lst: generate html (gui version only)
// GENFLG_ASMINC  0x0040          // asm&lst: gen information only about types

// NOTE:
// File -> Produce file -> Create MAP file...
//   GenerateFile( OFILE_MAP, fopen( file_name_map, "w" ), 0, BADADDR, 0 );
// File -> Produce file -> Create ASM file...
//   GenerateFile( OFILE_ASM, fopen( file_name_asm, "w" ), 0, BADADDR, 0 );
// File -> Produce file -> Create INC file...
//   GenerateFile( OFILE_ASM, fopen( file_name_inc, "w" ), 0, BADADDR, GENFLG_ASMTYPE | GENFLG_ASMINC);
// File -> Produce file -> Create LST file...
//   GenerateFile( OFILE_LST, fopen( file_name_lst, "w" ), 0, BADADDR, 0 );
// File -> Produce file -> Dump database to IDC file...
//   GenerateFile( OFILE_IDC, fopen( file_name_idc_flag0, "w" ), 0, BADADDR, 0 );
// File -> Produce file -> Dump typeinfo to IDC file...
//   GenerateFile( OFILE_IDC, fopen( file_name_idc_flag8, "w" ), 0, BADADDR, GENFLG_IDCTYPE );

#include <idc.idc>

static main(void)
{
    
    produce_files();
    
}

static produce_files(void)
{
    
    auto file_name;
    auto file_name_asm;
    auto file_name_idc;
    auto file_name_inc;
    auto file_name_lst;
    auto file_name_map;

    auto file_name_idc_flag0;
    auto file_name_idc_flag8;

    Message("\n\n------ Producing the output files... --------\n");

    file_name = GetIdbPath();

    Message("file_name: %s\n", file_name);
    
    file_name_asm = substr( file_name, 0, strstr(file_name, "." ) ) + "_test" + ".asm";
    file_name_idc = substr( file_name, 0, strstr(file_name, "." ) ) + "_test" + ".idc";
    file_name_inc = substr( file_name, 0, strstr(file_name, "." ) ) + "_test" + ".inc";
    file_name_lst = substr( file_name, 0, strstr(file_name, "." ) ) + "_test" + ".lst";
    file_name_map = substr( file_name, 0, strstr(file_name, "." ) ) + "_test" + ".map";

    Message("file_name_asm: %s\n", file_name_asm);
    Message("file_name_idc: %s\n", file_name_idc);
    Message("file_name_inc: %s\n", file_name_inc);
    Message("file_name_lst: %s\n", file_name_lst);
    Message("file_name_map: %s\n", file_name_map);
    
    
    GenerateFile( OFILE_ASM, fopen( file_name_asm, "w" ), 0, BADADDR, 0 );
    GenerateFile( OFILE_IDC, fopen( file_name_idc, "w" ), 0, BADADDR, 0 );
    // GenerateFile( OFILE_ASM, fopen( file_name_inc, "w" ), 0, BADADDR, GENFLG_ASMINC );
    // GenerateFile( OFILE_ASM, fopen( file_name_inc, "w" ), 0, BADADDR, GENFLG_ASMTYPE );
    GenerateFile( OFILE_ASM, fopen( file_name_inc, "w" ), 0, BADADDR, GENFLG_ASMTYPE | GENFLG_ASMINC);
    GenerateFile( OFILE_LST, fopen( file_name_lst, "w" ), 0, BADADDR, 0 );
    GenerateFile( OFILE_MAP, fopen( file_name_map, "w" ), 0, BADADDR, 0 );
    
    
    file_name_idc_flag0 = substr( file_name, 0, strstr(file_name, "." ) ) + "_flag0" + ".idc";
    file_name_idc_flag8 = substr( file_name, 0, strstr(file_name, "." ) ) + "_flag8" + ".idc";
    
    Message("file_name_idc_flag0: %s\n", file_name_idc_flag0);
    Message("file_name_idc_flag8: %s\n", file_name_idc_flag8);
    
    GenerateFile( OFILE_IDC, fopen( file_name_idc_flag0, "w" ), 0, BADADDR, 0 );
    GenerateFile( OFILE_IDC, fopen( file_name_idc_flag8, "w" ), 0, BADADDR, GENFLG_IDCTYPE );
    
    
    // GenerateFile(OFILE_MAP, fopen(file,"w"), 0, BADADDR, GENFLG_MAPSEGS|GENFLG_MAPNAME)

    Message("All done, exiting...\n");

}
