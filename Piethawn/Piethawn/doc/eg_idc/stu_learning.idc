// J:\STU\WD\IDA55\idc\stu_learning.idc

/*
NOTES:
global variables are not supported
no initialization allowed in declarations
+= is not supported
string concatenation: "Hello" + "World" yields "HelloWorld"
Message() ~= printf()
form() ~= sprintf
function parameters are strictly call-by-value (i.e., no call-by-reference, no pointers)

"ea" ?Effective Address? ... "Linear Address", "Virtual Address"

*/

#include <idc.idc>
// #include <stu_seg_info.idc>
// #include <stu_proc_info.idc>

// J:\STU\WD\IDA55\idc\idc.idc
// #define BADADDR         -1                 // Not allowed address value
// #define BADSEL          -1                 // Not allowed selector value/number
// #define MinEA()                 GetLongPrm(INF_MIN_EA)
// #define MaxEA()                 GetLongPrm(INF_MAX_EA)
// #define BeginEA()               GetLongPrm(INF_BEGIN_EA)


static hello_world(void)
{
    Message("hello, world\n");
}

static main(void)
{
    // Message("hello, world\n");
    // hello_world();
    
    /*
    ea_begin = BeginEA();
    ea_min = MinEA();
    ea_max = MaxEA();
    ea_first_seg = FirstSeg();
    Message("ea_begin: %08X\n", ea_begin);
    Message("ea_min:   %08X\n", ea_min);
    Message("ea_max:   %08X\n", ea_max);
    Message("ea_first_seg:   %08X\n", ea_first_seg);
    */
    
    

}


static stu_config_get(void)
{
    auto max_xref_depth;
    auto indention_for_instructions;
    auto indention_for_comments;
    auto number_of_references_to_generate;
    
    auto use_tab_chars_in_the_output_file;
    auto line_prefixes_with_segment_name;

    
    auto target_assembler_number;
    
    auto show_line_prefixes;
    
    auto xrefs_representation;
    auto INF_XREFS__SW_SEGXRF;
    auto INF_XREFS__SW_XRFMRK;
    auto INF_XREFS__SW_XRFFNC;
    auto INF_XREFS__SW_XRFVAL;
    
    
    auto comments;
    
    auto show_bad_instructions;
    
    
    auto line_prefix_type;
    auto INF_PREFFLAG__PREF_SEGADR;
    auto INF_PREFFLAG__PREF_FNCOFF;
    auto INF_PREFFLAG__PREF_STACK;
    
    
    auto generate_org_directives;
    auto generate_assume_directives;
    
    auto address_of_main;
    
    auto number_of_opcode_bytes;
    
    
    max_xref_depth = GetLongPrm(INF_MAXREF);     // max xref depth
    indention_for_instructions = GetCharPrm(INF_INDENT);       // Indention for instructions
    indention_for_comments = GetCharPrm(INF_COMMENT);      // Indention for comments
    number_of_references_to_generate = GetCharPrm(INF_XREFNUM);      // Number of references to generate (0 - xrefs won't be generated at all)
    use_tab_chars_in_the_output_file = GetCharPrm(INF_ENTAB);        // Use '\t' chars in the output file?

    show_line_prefixes = GetCharPrm(INF_SHOWPREF);   // Show line prefixes?
    line_prefixes_with_segment_name = GetCharPrm(INF_PREFSEG);    // line prefixes with segment name?

    target_assembler_number = GetCharPrm(INF_ASMTYPE);    // target assembler number (0..n)


    xrefs_representation = GetCharPrm(INF_XREFS);   // xrefs representation
    /*
        #define         SW_SEGXRF       0x01    //              show segments in xrefs?
        #define         SW_XRFMRK       0x02    //              show xref type marks?
        #define         SW_XRFFNC       0x04    //              show function offsets?
        #define         SW_XRFVAL       0x08    //              show xref values? (otherwise-"...")
    */
    /* ... 2nd byte: ( ( strtype >> 8 ) & 0xFF ) ... 3rd byte: ( ( strtype >> 16 ) & 0xFF ) ... */
    INF_XREFS__SW_SEGXRF = (xrefs_representation & SW_SEGXRF);
    INF_XREFS__SW_XRFMRK = (xrefs_representation & SW_XRFMRK);
    INF_XREFS__SW_XRFFNC = (xrefs_representation & SW_XRFFNC);
    INF_XREFS__SW_XRFVAL = (xrefs_representation & SW_XRFVAL);
    
    
    comments = GetCharPrm(INF_CMTFLAG);    // comments
    /*
        #define         SW_RPTCMT       0x01    //              show repeatable comments?
        #define         SW_ALLCMT       0x02    //              comment all lines?
        #define         SW_NOCMT        0x04    //              no comments at all
        #define         SW_LINNUM       0x08    //              show source line numbers
        #define         SW_MICRO        0x10    //              show microcode (if implemented)
    */
    
    show_bad_instructions = GetCharPrm(INF_SHOWBADS);   // show bad instructions?

    line_prefix_type = GetCharPrm(INF_PREFFLAG);   // line prefix type:
    /*
        #define         PREF_SEGADR     0x01    //              show segment addresses? // Include segment addresses
        #define         PREF_FNCOFF     0x02    //              show function offsets?  // Function offsets
                                                                                        // ? Use segment names SEE: INF_PREFSEG
        #define         PREF_STACK      0x04    //              show stack pointer?     // ?
    */
    INF_PREFFLAG__PREF_SEGADR = (line_prefix_type & PREF_SEGADR);
    INF_PREFFLAG__PREF_FNCOFF = (line_prefix_type & PREF_FNCOFF);
    INF_PREFFLAG__PREF_STACK = (line_prefix_type & PREF_STACK);
    
    
    
    generate_org_directives = GetCharPrm(INF_ORG);        // Generate 'org' directives?
    generate_assume_directives = GetCharPrm(INF_ASSUME);     // Generate 'assume' directives?

    address_of_main = GetCharPrm(INF_MAIN);       // address of main()

    number_of_opcode_bytes = GetShortPrm(INF_BINPREF);   // # of instruction bytes to show in line prefix
    
    

    Message("max_xref_depth: %d\n", max_xref_depth, max_xref_depth);
    Message("indention_for_instructions: %d\n", indention_for_instructions);
    Message("indention_for_comments: %d\n", indention_for_comments);
    Message("number_of_references_to_generate: %d\n", number_of_references_to_generate);
    Message("use_tab_chars_in_the_output_file: %d\n", use_tab_chars_in_the_output_file);

    Message("show_line_prefixes: %d\n", show_line_prefixes);
    Message("line_prefixes_with_segment_name: %d\n", line_prefixes_with_segment_name);
    
    Message("target_assembler_number: %d\n", target_assembler_number);
    
    
    Message("xrefs_representation: %d (0x%08X)\n", xrefs_representation, xrefs_representation);
    Message("INF_XREFS__SW_SEGXRF: 0x%08X\n", INF_XREFS__SW_SEGXRF);    // show segments in xrefs?
    Message("INF_XREFS__SW_XRFMRK: 0x%08X\n", INF_XREFS__SW_XRFMRK);    // show xref type marks?
    Message("INF_XREFS__SW_XRFFNC: 0x%08X\n", INF_XREFS__SW_XRFFNC);    // show function offsets?
    Message("INF_XREFS__SW_XRFVAL: 0x%08X\n", INF_XREFS__SW_XRFVAL);    // show xref values? (otherwise-"...")
    
    
    Message("comments: %d (0x%08X)\n", comments, comments);
    
    Message("show_bad_instructions: %d\n", show_bad_instructions);
    
    Message("line_prefix_type: %d (0x%08X)\n", line_prefix_type, line_prefix_type);
    Message("INF_PREFFLAG__PREF_SEGADR: 0x%08X\n", INF_PREFFLAG__PREF_SEGADR);  // show segment addresses?
    Message("INF_PREFFLAG__PREF_FNCOFF: 0x%08X\n", INF_PREFFLAG__PREF_FNCOFF);  // show function offsets?
    Message("INF_PREFFLAG__PREF_STACK: 0x%08X\n", INF_PREFFLAG__PREF_STACK);    // show stack pointer?
    /*
        line_prefix_type: 4 (0x00000004)
        INF_PREFFLAG__PREF_SEGADR: 0x00000000
        INF_PREFFLAG__PREF_FNCOFF: 0x00000000
        INF_PREFFLAG__PREF_STACK: 0x00000004
    */

/*
xrefs_representation: 15 (0x0000000F)
comments: 11 (0x0000000B)
show_bad_instructions: 1
line_prefix_type: 5 (0x00000005) // ? [o] Function offsets [x] Include segment addresses [x] Use segment names ?

*/
    Message("generate_org_directives: %d (0x%08X)\n", generate_org_directives, generate_org_directives);
    Message("generate_assume_directives: %d (0x%08X)\n", generate_assume_directives, generate_assume_directives);
    
    Message("address_of_main: %d (0x%08X)\n", address_of_main, address_of_main);
    
    Message("number_of_opcode_bytes: %d\n", number_of_opcode_bytes);



/*
generate_org_directives: 1 (0x00000001)
generate_assume_directives: 1 (0x00000001)

address_of_main: 9 (0x00000009)
*/

    
/*
    max_xref_depth : 16
    indention_for_instructions : 0
    indention_for_comments : 0
    number_of_references_to_generate : 0
    use_tab_chars_in_the_output_file : 0
    show_line_prefixes : 0
    line_prefixes_with_segment_name : 1
*/

/*
IDA55 -> Options -> General -> Analysis -> Target assembler
target_assembler_number : 0 // "Generic for Intel 80x86"
target_assembler_number : 1 // "Borland TASM in Ideal mode"
*/
/*
// Disassembly -> Comments
INF_CMTFLAG
// Disassembly -> Repeatable comments

// Disassembly -> Auto comments
*/
/*
IDA55 -> Options General -> -> Cross-reference
Cross reference parts:
xrefs_representation = GetCharPrm(INF_XREFS);   // xrefs representation
Cross-reference -> Display xref values
    SW_XRFVAL       0x08    //              show xref values? (otherwise-"...")
Cross-reference -> Display xref type mark
    SW_XRFMRK       0x02    //              show xref type marks?
Cross-reference -> Display function offsets
    SW_XRFFNC       0x04    //              show function offsets?
Cross-reference -> Display segments in xrefs
    SW_SEGXRF       0x01    //              show segments in xrefs?

e.g., INF_XREFS__SW_SEGXRF = (xrefs_representation & SW_SEGXRF); if INF_XREFS__SW_SEGXRF == 0 then SW_SEGXRF is 0 / off / disabled

xrefs_representation: 15 (0x0000000F)
INF_XREFS__SW_SEGXRF: 0x00000001
INF_XREFS__SW_XRFMRK: 0x00000002
INF_XREFS__SW_XRFFNC: 0x00000004
INF_XREFS__SW_XRFVAL: 0x00000008

xrefs_representation: 12 (0x0000000C)
INF_XREFS__SW_SEGXRF: 0x00000000
INF_XREFS__SW_XRFMRK: 0x00000000
INF_XREFS__SW_XRFFNC: 0x00000004
INF_XREFS__SW_XRFVAL: 0x00000008
*/
/*
IDA55 -> Options -> General -> Disassembly
Display disassembly line parts:
[ ] Stack pointer
line_prefix_type = GetCharPrm(INF_PREFFLAG);   // line prefix type:
PREF_STACK      0x04    //              show stack pointer?     // ?
*/
  
/*

Disassembly -> Instructions indentation: [0]
indention_for_instructions : 00000000

Disassembly -> Comments indentation: [24]

indention_for_comments : 00000018

number_of_references_to_generate : 000000FF
Cross-references -> Number of displayed xrefs [255]

use_tab_chars_in_the_output_file : 00000001

max_xref_depth : 16
indention_for_instructions : 0
indention_for_comments : 24
number_of_references_to_generate : 255
use_tab_chars_in_the_output_file : 1


*/

}

static stu_config_set(void)
{
/*
SEE: J:\STU-IDA\IDA Pro 5.5 - Config Options.txt
idc.idc
success SetLongPrm (long offset,long value);
success SetShortPrm(long offset,long value);
success SetCharPrm (long offset,long value);

#define INF_MAXREF      67              // int32;   max xref depth
#define INF_INDENT      73              // char;    Indention for instructions
#define INF_COMMENT     74              // char;    Indention for comments
#define INF_XREFNUM     75              // char;    Number of references to generate //          0 - xrefs won't be generated at all
#define INF_ENTAB       76              // char;    Use '\t' chars in the output file?

#define INF_BORDER      82              // char;    Generate borders?
#define INF_NULL        83              // char;    Generate empty lines?

#define INF_GENFLAGS    84              // char;    General flags:
#define         INFFL_LZERO     0x01    //              Generate leading zeroes in numbers
#define         INFFL_ALLASM    0x02    //              May use constructs not supported by
                                        //              the target assembler

#define INF_SHOWPREF    85              // char;    Show line prefixes?
#define INF_PREFSEG     86              // char;    line prefixes with segment name?

#define INF_ASMTYPE     87              // char;    target assembler number (0..n)

#define INF_BASEADDR    88              // int32;   base paragraph of the program

#define INF_XREFS       92              // char;    xrefs representation:
#define         SW_SEGXRF       0x01    //              show segments in xrefs?
#define         SW_XRFMRK       0x02    //              show xref type marks?
#define         SW_XRFFNC       0x04    //              show function offsets?
#define         SW_XRFVAL       0x08    //              show xref values? (otherwise-"...")

#define INF_BINPREF     93              // short;   # of instruction bytes to show in line prefix
                                        
#define INF_CMTFLAG     95              // char;    comments:
#define         SW_RPTCMT       0x01    //              show repeatable comments?
#define         SW_ALLCMT       0x02    //              comment all lines?
#define         SW_NOCMT        0x04    //              no comments at all
#define         SW_LINNUM       0x08    //              show source line numbers
#define         SW_MICRO        0x10    //              show microcode (if implemented)

#define INF_SHOWBADS    97              // char;    show bad instructions?
                                        //          an instruction is bad if it appears
                                        //          in the ash.badworks array

#define INF_PREFFLAG    98              // char;    line prefix type:
#define         PREF_SEGADR     0x01    //              show segment addresses?
#define         PREF_FNCOFF     0x02    //              show function offsets?
#define         PREF_STACK      0x04    //              show stack pointer?


#define INF_ORG         127             // char;    Generate 'org' directives?
#define INF_ASSUME      128             // char;    Generate 'assume' directives?

#define INF_MAIN        138             // int32;   address of main()


#define INF_STRTYPE     154             // int32;   current ascii string type
                                        //          is considered as several bytes:
                                        //      low byte:
#define         ASCSTR_TERMCHR  0       //              Character-terminated ASCII string
#define         ASCSTR_C        0       //              C-string, zero terminated
#define         ASCSTR_PASCAL   1       //              Pascal-style ASCII string (length byte)
#define         ASCSTR_LEN2     2       //              Pascal-style, length is 2 bytes
#define         ASCSTR_UNICODE  3       //              Unicode string
#define         ASCSTR_LEN4     4       //              Delphi string, length is 4 bytes
#define         ASCSTR_ULEN2    5       //              Pascal-style Unicode, length is 2 bytes
#define         ASCSTR_ULEN4    6       //              Pascal-style Unicode, length is 4 bytes
#define         ASCSTR_LAST     6       //              Last string type
                                        //      2nd byte - termination chracters for ASCSTR_TERMCHR:
#define         STRTERM1(strtype)       ((strtype>>8)&0xFF)
                                        //      3d byte:
#define         STRTERM2(strtype)       ((strtype>>16)&0xFF)
                                        //              The termination characters are kept in
                                        //              the 2nd and 3d bytes of string type
                                        //              if the second termination character is
                                        //              '\0', then it is ignored.


#define INF_COMPILER    183             // uchar;   compiler
#define      COMP_MASK        0x0F      //              mask to apply to get the pure compiler id
#define         COMP_UNK      0x00      // Unknown
#define         COMP_MS       0x01      // Visual C++
#define         COMP_BC       0x02      // Borland C++
#define         COMP_WATCOM   0x03      // Watcom C++
#define         COMP_GNU      0x06      // GNU C++
#define         COMP_VISAGE   0x07      // Visual Age C++
#define         COMP_BP       0x08      // Delphi

*/

    SetCharPrm(INF_ENTAB, 0);       // Use '\t' chars in the output file?
    
    SetCharPrm(INF_MAXREF,0);       // max xref depth
    
    SetCharPrm(INF_INDENT, 0);      // Indention for instructions
    SetCharPrm(INF_COMMENT, 0);     // Indention for comments
    SetCharPrm(INF_XREFNUM, 0);     // Number of references to generate (0 - xrefs won't be generated at all)
    
    SetCharPrm(INF_ASMTYPE, 1);     // target assembler number (0..n)
    
    SetCharPrm(INF_SHOWPREF, 0);    // Show line prefixes?
    
    // line_prefix_type = GetCharPrm(INF_PREFFLAG);   // line prefix type:
    // PREF_STACK      0x04    //              show stack pointer?
    // SetCharPrm(INF_PREFFLAG, PREF_STACK);
    /*
        line_prefix_type: 4 (0x00000004)
        INF_PREFFLAG__PREF_SEGADR: 0x00000000
        INF_PREFFLAG__PREF_FNCOFF: 0x00000000
        INF_PREFFLAG__PREF_STACK: 0x00000004
    */
    SetCharPrm(INF_PREFFLAG, 0);
    /*
        line_prefix_type: 0 (0x00000000)
        INF_PREFFLAG__PREF_SEGADR: 0x00000000
        INF_PREFFLAG__PREF_FNCOFF: 0x00000000
        INF_PREFFLAG__PREF_STACK: 0x00000000
    */

    
    //SetCharPrm(INF_CMTFLAG, 0);    // comments
    SetCharPrm(INF_CMTFLAG, SW_NOCMT);    // comments
    // comments = GetCharPrm(INF_CMTFLAG);    // comments
    /*
        #define         SW_RPTCMT       0x01    //              show repeatable comments?
        #define         SW_ALLCMT       0x02    //              comment all lines?
        #define         SW_NOCMT        0x04    //              no comments at all
        #define         SW_LINNUM       0x08    //              show source line numbers
        #define         SW_MICRO        0x10    //              show microcode (if implemented)
    */
    
    SetCharPrm(INF_ORG, 1);            // Generate 'org' directives?
    SetCharPrm(INF_ASSUME, 1);         // Generate 'assume' directives?


    // xrefs_representation = GetCharPrm(INF_XREFS);   // xrefs representation
    /*
        #define         SW_SEGXRF       0x01    //              show segments in xrefs?
        #define         SW_XRFMRK       0x02    //              show xref type marks?
        #define         SW_XRFFNC       0x04    //              show function offsets?
        #define         SW_XRFVAL       0x08    //              show xref values? (otherwise-"...")
    */
    // SetCharPrm(INF_XREFS, (SW_SEGXRF | SW_XRFMRK | SW_XRFFNC | SW_XRFVAL));
    /*
        xrefs_representation: 15 (0x0000000F)
        INF_XREFS__SW_SEGXRF: 0x00000001
        INF_XREFS__SW_XRFMRK: 0x00000002
        INF_XREFS__SW_XRFFNC: 0x00000004
        INF_XREFS__SW_XRFVAL: 0x00000008
    */
    // SetCharPrm(INF_XREFS, (SW_SEGXRF | SW_XRFFNC));
    /*
        xrefs_representation: 5 (0x00000005)
        INF_XREFS__SW_SEGXRF: 0x00000001
        INF_XREFS__SW_XRFMRK: 0x00000000
        INF_XREFS__SW_XRFFNC: 0x00000004
        INF_XREFS__SW_XRFVAL: 0x00000000
    */

    // number_of_opcode_bytes = GetShortPrm(INF_BINPREF);   // # of instruction bytes to show in line prefix
    /*
    SetShortPrm(INF_BINPREF, 10);   // # of instruction bytes to show in line prefix
    number_of_opcode_bytes: 10
    /*
    /*
    SetShortPrm(INF_BINPREF, 0);   // # of instruction bytes to show in line prefix
    number_of_opcode_bytes: 0
    */
    SetShortPrm(INF_BINPREF, 0);   // # of instruction bytes to show in line prefix


// ? Right margin ? (What even does this do?)
// ? Spaces for tabulation ?

}


static stu_seg_info(void)
{
    
    auto ea_begin;
    auto ea_min;
    auto ea_max;
    auto ea_first_seg;
    
    auto ea_seg_first;
    auto ea_seg_next;
    auto seg_name_first;
    auto seg_name_next;
    
    auto itr_ea_seg_next;
    auto seg_name;
    
    auto seg_start_addr;
    auto seg_end_addr;
    auto seg_alignment;
    auto seg_combination;
    auto seg_permissions;
    auto seg_bitness;
    auto seg_flags;
    auto seg_selector;
    auto seg_es_value;
    auto seg_cs_value;
    auto seg_ss_value;
    auto seg_ds_value;
    auto seg_fs_value;
    auto seg_gs_value;
    auto seg_type;
    auto seg_color;
    
    auto file_name;
    auto file_handle;
    auto file_cursor_position;
    
    
    /*
    ea_seg_first = MinEA();
    // if(ea_seg_first == BADADDR) {  }
    Message("ea_seg_first: %08X\n", ea_seg_first);
    
    seg_name_first = SegName(ea_seg_first);
    // if(seg_name_first == 0) { Message("no segment at the specified address\n"); }
    Message("seg_name_first: %s\n", seg_name_first);

    ea_seg_next = NextSeg(ea_seg_first);
    // if(ea_seg_next == BADADDR) {  }
    Message("ea_seg_next: %08X\n", ea_seg_next);

    seg_name_next = SegName(ea_seg_next);
    // if(seg_name_next == 0) { Message("no segment at the specified address\n"); }
    Message("seg_name_next: %s\n", seg_name_next);
    */
    
    /*
    for(itr_ea_seg_next = MinEA(); itr_ea_seg_next != BADADDR; itr_ea_seg_next = NextSeg(itr_ea_seg_next))
    {
        Message("itr_ea_seg_next: %08X\n", itr_ea_seg_next);
        
        seg_name = SegName(itr_ea_seg_next);
        // if(seg_name == 0) { Message("no segment at the specified address\n"); }
        
        Message("seg_name: %s\n", seg_name);
    }
    */
    
    /*
    for(itr_ea_seg_next = MinEA(); itr_ea_seg_next != BADADDR; itr_ea_seg_next = NextSeg(itr_ea_seg_next))
    {
        seg_name = SegName(itr_ea_seg_next);
        // if(seg_name == 0) { Message("no segment at the specified address\n"); }
        
        Message("%08X, %s\n", itr_ea_seg_next, seg_name);
        
    }
    */

    /*
    ea_seg_first = MinEA();
    // if(ea_seg_first == BADADDR) {  }
    Message("ea_seg_first: %08X\n", ea_seg_first);
    
    seg_name_first = SegName(ea_seg_first);
    // if(seg_name_first == 0) { Message("no segment at the specified address\n"); }
    Message("seg_name_first: %s\n", seg_name_first);
    
    seg_start_addr = GetSegmentAttr(ea_seg_first, SEGATTR_START);   // starting address
    seg_end_addr = GetSegmentAttr(ea_seg_first, SEGATTR_END);       // ending address
    seg_alignment = GetSegmentAttr(ea_seg_first, SEGATTR_ALIGN);    // alignment
    seg_combination = GetSegmentAttr(ea_seg_first, SEGATTR_COMB);   // combination
    seg_permissions = GetSegmentAttr(ea_seg_first, SEGATTR_PERM);   // permissions
    seg_bitness = GetSegmentAttr(ea_seg_first, SEGATTR_BITNESS);    // bitness (0: 16, 1: 32, 2: 64 bit segment)
    seg_flags = GetSegmentAttr(ea_seg_first, SEGATTR_FLAGS);        // segment flags
    seg_selector = GetSegmentAttr(ea_seg_first, SEGATTR_SEL);       // segment selector
    seg_es_value = GetSegmentAttr(ea_seg_first, SEGATTR_ES);        // default ES value
    seg_cs_value = GetSegmentAttr(ea_seg_first, SEGATTR_CS);        // default CS value
    seg_ss_value = GetSegmentAttr(ea_seg_first, SEGATTR_SS);        // default SS value
    seg_ds_value = GetSegmentAttr(ea_seg_first, SEGATTR_DS);        // default DS value
    seg_fs_value = GetSegmentAttr(ea_seg_first, SEGATTR_FS);        // default FS value
    seg_gs_value = GetSegmentAttr(ea_seg_first, SEGATTR_GS);        // default GS value
    seg_type = GetSegmentAttr(ea_seg_first, SEGATTR_TYPE);          // segment type
    seg_color = GetSegmentAttr(ea_seg_first, SEGATTR_COLOR);        // segment color
    
    Message("seg_start_addr: %08X\n", seg_start_addr);
    Message("seg_end_addr: %08X\n", seg_end_addr);
    Message("seg_alignment: %d\n", seg_alignment);
    Message("seg_combination: %08X\n", seg_combination);
    Message("seg_permissions: %08X\n", seg_permissions);
    Message("seg_bitness: %08X\n", seg_bitness);
    Message("seg_flags: %08X\n", seg_flags);
    Message("seg_selector: %08X\n", seg_selector);
    Message("seg_es_value: %08X\n", seg_es_value);
    Message("seg_cs_value: %08X\n", seg_cs_value);
    Message("seg_ss_value: %08X\n", seg_ss_value);
    Message("seg_ds_value: %08X\n", seg_ds_value);
    Message("seg_fs_value: %08X\n", seg_fs_value);
    Message("seg_gs_value: %08X\n", seg_gs_value);
    Message("seg_type: %08X\n", seg_type);
    Message("seg_color: %08X\n", seg_color);
    
    ea_seg_next = NextSeg(ea_seg_first);      
    // if(ea_seg_next == BADADDR) {  }
    Message("ea_seg_next: %08X\n", ea_seg_next);

    seg_name_next = SegName(ea_seg_next);
    // if(seg_name_next == 0) { Message("no segment at the specified address\n"); }
    Message("seg_name_next: %s\n", seg_name_next);

    seg_start_addr = GetSegmentAttr(ea_seg_next, SEGATTR_START);   // starting address
    seg_end_addr = GetSegmentAttr(ea_seg_next, SEGATTR_END);       // ending address
    seg_alignment = GetSegmentAttr(ea_seg_next, SEGATTR_ALIGN);    // alignment
    seg_combination = GetSegmentAttr(ea_seg_next, SEGATTR_COMB);   // combination
    seg_permissions = GetSegmentAttr(ea_seg_next, SEGATTR_PERM);   // permissions
    seg_bitness = GetSegmentAttr(ea_seg_next, SEGATTR_BITNESS);    // bitness (0: 16, 1: 32, 2: 64 bit segment)
    seg_flags = GetSegmentAttr(ea_seg_next, SEGATTR_FLAGS);        // segment flags
    seg_selector = GetSegmentAttr(ea_seg_next, SEGATTR_SEL);       // segment selector
    seg_es_value = GetSegmentAttr(ea_seg_next, SEGATTR_ES);        // default ES value
    seg_cs_value = GetSegmentAttr(ea_seg_next, SEGATTR_CS);        // default CS value
    seg_ss_value = GetSegmentAttr(ea_seg_next, SEGATTR_SS);        // default SS value
    seg_ds_value = GetSegmentAttr(ea_seg_next, SEGATTR_DS);        // default DS value
    seg_fs_value = GetSegmentAttr(ea_seg_next, SEGATTR_FS);        // default FS value
    seg_gs_value = GetSegmentAttr(ea_seg_next, SEGATTR_GS);        // default GS value
    seg_type = GetSegmentAttr(ea_seg_next, SEGATTR_TYPE);          // segment type
    seg_color = GetSegmentAttr(ea_seg_next, SEGATTR_COLOR);        // segment color
    
    Message("seg_start_addr: %08X\n", seg_start_addr);
    Message("seg_end_addr: %08X\n", seg_end_addr);
    Message("seg_alignment: %08X\n", seg_alignment);
    Message("seg_combination: %08X\n", seg_combination);
    Message("seg_permissions: %08X\n", seg_permissions);
    Message("seg_bitness: %08X\n", seg_bitness);
    Message("seg_flags: %08X\n", seg_flags);
    Message("seg_selector: %08X\n", seg_selector);
    Message("seg_es_value: %08X\n", seg_es_value);
    Message("seg_cs_value: %08X\n", seg_cs_value);
    Message("seg_ss_value: %08X\n", seg_ss_value);
    Message("seg_ds_value: %08X\n", seg_ds_value);
    Message("seg_fs_value: %08X\n", seg_fs_value);
    Message("seg_gs_value: %08X\n", seg_gs_value);
    Message("seg_type: %08X\n", seg_type);
    Message("seg_color: %08X\n", seg_color);
    */
    
    // if(seg_alignment == 1)
    // {
    //      byte
    // }
    // if(seg_alignment == 3)
    // {
    //      paragraph
    // }
    
    // if(seg_bitness == 0)
    // {
        // // 16-bit
    // }
    // else
    // {
        // if(seg_bitness == 1)
        // {
            // // 32
        // }
        // else
        // {
            // if(seg_bitness == 2)
            // {
                // // 64
            // }
            // else
            // {
                // // INVALID
            // }
        // }
    // }

    // if(seg_type == 2)
    // {
        // // public
    // }
    
    /*
    Message("seg_name, seg_start_addr, seg_end_addr, seg_alignment, seg_combination, seg_permissions, seg_bitness, seg_flags, seg_selector, seg_es_value, seg_cs_value, seg_ss_value, seg_ds_value, seg_fs_value, seg_gs_value, seg_type, seg_color\n");
    
    for(itr_ea_seg_next = MinEA(); itr_ea_seg_next != BADADDR; itr_ea_seg_next = NextSeg(itr_ea_seg_next))
    {
        seg_name = SegName(itr_ea_seg_next);
        // if(seg_name == 0) { Message("no segment at the specified address\n"); }
        
        seg_start_addr = GetSegmentAttr(itr_ea_seg_next, SEGATTR_START);   // starting address
        seg_end_addr = GetSegmentAttr(itr_ea_seg_next, SEGATTR_END);       // ending address
        seg_alignment = GetSegmentAttr(itr_ea_seg_next, SEGATTR_ALIGN);    // alignment
        seg_combination = GetSegmentAttr(itr_ea_seg_next, SEGATTR_COMB);   // combination
        seg_permissions = GetSegmentAttr(itr_ea_seg_next, SEGATTR_PERM);   // permissions
        seg_bitness = GetSegmentAttr(itr_ea_seg_next, SEGATTR_BITNESS);    // bitness (0: 16, 1: 32, 2: 64 bit segment)
        seg_flags = GetSegmentAttr(itr_ea_seg_next, SEGATTR_FLAGS);        // segment flags
        seg_selector = GetSegmentAttr(itr_ea_seg_next, SEGATTR_SEL);       // segment selector
        seg_es_value = GetSegmentAttr(itr_ea_seg_next, SEGATTR_ES);        // default ES value
        seg_cs_value = GetSegmentAttr(itr_ea_seg_next, SEGATTR_CS);        // default CS value
        seg_ss_value = GetSegmentAttr(itr_ea_seg_next, SEGATTR_SS);        // default SS value
        seg_ds_value = GetSegmentAttr(itr_ea_seg_next, SEGATTR_DS);        // default DS value
        seg_fs_value = GetSegmentAttr(itr_ea_seg_next, SEGATTR_FS);        // default FS value
        seg_gs_value = GetSegmentAttr(itr_ea_seg_next, SEGATTR_GS);        // default GS value
        seg_type = GetSegmentAttr(itr_ea_seg_next, SEGATTR_TYPE);          // segment type
        seg_color = GetSegmentAttr(itr_ea_seg_next, SEGATTR_COLOR);        // segment color
        
        Message("%s, %08X\, %08X, %08X, %08X, %08X, %08X, %08X, %08X, %08X, %08X, 08X, %08X, %08X, %08X, %08X, %08X\n", 
            seg_name, 
            seg_start_addr, 
            seg_end_addr, 
            seg_alignment, 
            seg_combination, 
            seg_permissions, 
            seg_bitness, 
            seg_flags, 
            seg_selector, 
            seg_es_value, 
            seg_cs_value, 
            seg_ss_value, 
            seg_ds_value, 
            seg_fs_value, 
            seg_gs_value, 
            seg_type, 
            seg_color);
    }
    */
    
    Message("\n\n------ Creating the output file.... --------\n");
    file_name = GetIdbPath();
    Message("file_name: %s\n", file_name);
    file_name = substr(file_name, 0, strstr(file_name, ".")) + "_segs" + ".csv";
    Message("file_name: %s\n", file_name);
    file_handle = fopen(file_name, "w");
    file_cursor_position = ftell(file_handle);
    Message("file_cursor_position: %d\n", file_cursor_position);
    
    
    Message("seg_name, seg_start_addr, seg_end_addr, seg_alignment, seg_combination, seg_permissions, seg_bitness, seg_flags, seg_selector, seg_es_value, seg_cs_value, seg_ss_value, seg_ds_value, seg_fs_value, seg_gs_value, seg_type, seg_color\n");
    
    writestr(file_handle, "seg_name, seg_start_addr, seg_end_addr, seg_alignment, seg_combination, seg_permissions, seg_bitness, seg_flags, seg_selector, seg_es_value, seg_cs_value, seg_ss_value, seg_ds_value, seg_fs_value, seg_gs_value, seg_type, seg_color\n");
    
    for(itr_ea_seg_next = MinEA(); itr_ea_seg_next != BADADDR; itr_ea_seg_next = NextSeg(itr_ea_seg_next))
    {
        seg_name = SegName(itr_ea_seg_next);
        // if(seg_name == 0) { Message("no segment at the specified address\n"); }
        
        seg_start_addr = GetSegmentAttr(itr_ea_seg_next, SEGATTR_START);   // starting address
        seg_end_addr = GetSegmentAttr(itr_ea_seg_next, SEGATTR_END);       // ending address
        seg_alignment = GetSegmentAttr(itr_ea_seg_next, SEGATTR_ALIGN);    // alignment
        seg_combination = GetSegmentAttr(itr_ea_seg_next, SEGATTR_COMB);   // combination
        seg_permissions = GetSegmentAttr(itr_ea_seg_next, SEGATTR_PERM);   // permissions
        seg_bitness = GetSegmentAttr(itr_ea_seg_next, SEGATTR_BITNESS);    // bitness (0: 16, 1: 32, 2: 64 bit segment)
        seg_flags = GetSegmentAttr(itr_ea_seg_next, SEGATTR_FLAGS);        // segment flags
        seg_selector = GetSegmentAttr(itr_ea_seg_next, SEGATTR_SEL);       // segment selector
        seg_es_value = GetSegmentAttr(itr_ea_seg_next, SEGATTR_ES);        // default ES value
        seg_cs_value = GetSegmentAttr(itr_ea_seg_next, SEGATTR_CS);        // default CS value
        seg_ss_value = GetSegmentAttr(itr_ea_seg_next, SEGATTR_SS);        // default SS value
        seg_ds_value = GetSegmentAttr(itr_ea_seg_next, SEGATTR_DS);        // default DS value
        seg_fs_value = GetSegmentAttr(itr_ea_seg_next, SEGATTR_FS);        // default FS value
        seg_gs_value = GetSegmentAttr(itr_ea_seg_next, SEGATTR_GS);        // default GS value
        seg_type = GetSegmentAttr(itr_ea_seg_next, SEGATTR_TYPE);          // segment type
        seg_color = GetSegmentAttr(itr_ea_seg_next, SEGATTR_COLOR);        // segment color
        
        Message("%s, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n", 
            seg_name, 
            seg_start_addr, 
            seg_end_addr, 
            seg_alignment, 
            seg_combination, 
            seg_permissions, 
            seg_bitness, 
            seg_flags, 
            seg_selector, 
            seg_es_value, 
            seg_cs_value, 
            seg_ss_value, 
            seg_ds_value, 
            seg_fs_value, 
            seg_gs_value, 
            seg_type, 
            seg_color);

        fprintf(
        file_handle,
        "%s, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n", 
        seg_name, seg_start_addr, seg_end_addr, seg_alignment, seg_combination, seg_permissions, seg_bitness, seg_flags, seg_selector, seg_es_value, seg_cs_value, seg_ss_value, seg_ds_value, seg_fs_value, seg_gs_value, seg_type, seg_color
        );
        
        // form()
        // writestr(file_handle, str);
    }

    file_cursor_position = ftell(file_handle);
    Message("file_cursor_position: %d\n", file_cursor_position);
    
    fclose(file_handle);
    


    /*
        Functions window:
        Function name  Segment  Start     Length    R  F  L  S  B  T  =
        _main          seg001   00000009  000003D0  R  F  .  .  B  T  .
    */
    
    // ending address of a function
    // GetFunctionAttr(addr, FUNCATTR_END);

    // // ea_cursor = ScreenEA();
    // // Message("funcend %08X\n"FindFuncEnd(screenea));
    // // funcend = FindFuncEnd(screenea);

    // // for ( ea=MinEA(); ea != BADADDR; ea=NextHead(ea, BADADDR) )
    // // {
    // //   if ( GetMnem(ea) == mnem )
    // //       SaveMatch(ea, "");
    // // }

    // // Message("\n\n------ Creating the output file.... --------\n");
    // // file = GetIdbPath();
    // // file = substr(file,0,strstr(file,".")) + ".asm";
    // // WriteTxt(file, 0, BADADDR);           // create the assembler file
    // // Message("All done, exiting...\n");

    // // Exit(0);                              // exit to OS, error code 0 - success

}


static stu_proc_info(void)
{
    auto ea_begin;
    auto ea_min;
    auto ea_max;
    auto ea_first_seg;

    auto ea_proc_first;
    auto ea_proc_next;
    auto proc_name_first;
    auto proc_name_next;
    
    auto itr_ea_proc_next;
    auto proc_name;
    
    auto func_start_addr;
    auto func_end_addr;
    auto func_flags;
    auto func_frame_id;
    auto func_local_var_size;
    auto func_saved_regs_size;
    auto func_stack_bytes_purged_count;
    auto func_frame_ptr_delta;
    auto func_color_code;
    
    auto file_name;
    auto file_handle;
    auto file_cursor_position;
    
    
    /*
    ea_proc_first = MinEA();
    // if(ea_proc_first == BADADDR) {  }
    Message("ea_proc_first: %08X\n", ea_proc_first);
    proc_name_first = GetFunctionName(ea_proc_first);
    // if(proc_name_first == 0) { Message("no procedure at the specified address\n"); }
    Message("proc_name_first: %s\n", proc_name_first);
    
    ea_proc_next = NextFunction(ea_proc_first);
    // if(ea_proc_next == BADADDR) {  }
    Message("ea_proc_next: 0x%08X\n", ea_proc_next);
    proc_name_next = GetFunctionName(ea_proc_next);
    // if(proc_name_next == 0) { Message("no procedure at the specified address\n"); }
    Message("proc_name_next: %s\n", proc_name_next);
    */
    
    /*
    for(itr_ea_proc_next = MinEA(); itr_ea_proc_next != BADADDR; itr_ea_proc_next = NextFunction(itr_ea_proc_next))
    {
        Message("itr_ea_proc_next: %08X\n", itr_ea_proc_next);

        proc_name = GetFunctionName(itr_ea_proc_next);
        // if(proc_name == 0) { Message("no procedure at the specified address\n"); }
        
        Message("proc_name: %s\n", proc_name);
    }
    */
    
    /*
    for(itr_ea_proc_next = MinEA(); itr_ea_proc_next != BADADDR; itr_ea_proc_next = NextFunction(itr_ea_proc_next))
    {
        proc_name = GetFunctionName(itr_ea_proc_next);
        // if(proc_name == 0) { Message("no procedure at the specified address\n"); }
        
        Message("%08X, %s\n", itr_ea_proc_next, proc_name);
    }
    */
    
    /*
        idc.idc
        Line 2567
        long GetFunctionAttr(long ea, long attr);
        #define FUNCATTR_START    0
        #define FUNCATTR_END      8
        #define FUNCATTR_FLAGS   16
        #define FUNCATTR_FRAME   18
        #define FUNCATTR_FRSIZE  26
        #define FUNCATTR_FRREGS  34
        #define FUNCATTR_ARGSIZE 36
        #define FUNCATTR_FPD     44
        #define FUNCATTR_COLOR   52
        #define FUNCATTR_OWNER   18
        #define FUNCATTR_REFQTY  26
        
        IDA Pro - Help
        GetFunctionAttr  
        // ***********************************************
        // ** get a function attribute
        //         arguments:      ea - any address belonging to the function
        //                         attr - one of FUNCATTR_... constants
        //         returns:        -1 - error
        //                         otherwise returns the attribute value
        long GetFunctionAttr(long ea, long attr);
         FUNCATTR_START   -- function start address
         FUNCATTR_END     -- function end address
         FUNCATTR_FLAGS   -- function flags
         FUNCATTR_FRAME   -- function frame id
         FUNCATTR_FRSIZE  -- size of local variables
         FUNCATTR_FRREGS  -- size of saved registers area
         FUNCATTR_ARGSIZE -- number of bytes purged from the stack
         FUNCATTR_FPD     -- frame pointer delta
         FUNCATTR_COLOR   -- function color code
    */
    /*
    ea_proc_first = MinEA();
    // if(ea_proc_first == BADADDR) {  }
    Message("ea_proc_first: %08X\n", ea_proc_first);
    proc_name_first = GetFunctionName(ea_proc_first);
    // if(proc_name_first == 0) { Message("no procedure at the specified address\n"); }
    Message("proc_name_first: %s\n", proc_name_first);
    
    func_start_addr = GetFunctionAttr(ea_proc_first, FUNCATTR_START);    // function start address
    func_end_addr = GetFunctionAttr(ea_proc_first, FUNCATTR_END);      // function end address
    func_flags = GetFunctionAttr(ea_proc_first, FUNCATTR_FLAGS);    // function flags
    func_frame_id = GetFunctionAttr(ea_proc_first, FUNCATTR_FRAME);    // function frame id
    func_local_var_size = GetFunctionAttr(ea_proc_first, FUNCATTR_FRSIZE);   // size of local variables
    func_saved_regs_size = GetFunctionAttr(ea_proc_first, FUNCATTR_FRREGS);   // size of saved registers area
    func_stack_bytes_purged_count = GetFunctionAttr(ea_proc_first, FUNCATTR_ARGSIZE);  // number of bytes purged from the stack
    func_frame_ptr_delta = GetFunctionAttr(ea_proc_first, FUNCATTR_FPD);      // frame pointer delta
    func_color_code = GetFunctionAttr(ea_proc_first, FUNCATTR_COLOR);    // function color code

    Message("func_start_addr: %08X\n", func_start_addr);
    Message("func_end_addr: %08X\n", func_end_addr);
    Message("func_flags: %08X\n", func_flags);
    Message("func_frame_id: %08X\n", func_frame_id);
    Message("func_local_var_size: %08X\n", func_local_var_size);
    Message("func_saved_regs_size: %08X\n", func_saved_regs_size);
    Message("func_stack_bytes_purged_count: %08X\n", func_stack_bytes_purged_count);
    Message("func_frame_ptr_delta: %08X\n", func_frame_ptr_delta);
    Message("func_color_code: %08X\n", func_color_code);
    
    
    ea_proc_next = NextFunction(ea_proc_first);
    // if(ea_proc_next == BADADDR) {  }
    Message("ea_proc_next: 0x%08X\n", ea_proc_next);
    proc_name_next = GetFunctionName(ea_proc_next);
    // if(proc_name_next == 0) { Message("no procedure at the specified address\n"); }
    Message("proc_name_next: %s\n", proc_name_next);
    
    func_start_addr = GetFunctionAttr(ea_proc_next, FUNCATTR_START);    // function start address
    func_end_addr = GetFunctionAttr(ea_proc_next, FUNCATTR_END);      // function end address
    func_flags = GetFunctionAttr(ea_proc_next, FUNCATTR_FLAGS);    // function flags
    func_frame_id = GetFunctionAttr(ea_proc_next, FUNCATTR_FRAME);    // function frame id
    func_local_var_size = GetFunctionAttr(ea_proc_next, FUNCATTR_FRSIZE);   // size of local variables
    func_saved_regs_size = GetFunctionAttr(ea_proc_next, FUNCATTR_FRREGS);   // size of saved registers area
    func_stack_bytes_purged_count = GetFunctionAttr(ea_proc_next, FUNCATTR_ARGSIZE);  // number of bytes purged from the stack
    func_frame_ptr_delta = GetFunctionAttr(ea_proc_next, FUNCATTR_FPD);      // frame pointer delta
    func_color_code = GetFunctionAttr(ea_proc_next, FUNCATTR_COLOR);    // function color code

    Message("func_start_addr: %08X\n", func_start_addr);
    Message("func_end_addr: %08X\n", func_end_addr);
    Message("func_flags: %08X\n", func_flags);
    Message("func_frame_id: %08X\n", func_frame_id);
    Message("func_local_var_size: %08X\n", func_local_var_size);
    Message("func_saved_regs_size: %08X\n", func_saved_regs_size);
    Message("func_stack_bytes_purged_count: %08X\n", func_stack_bytes_purged_count);
    Message("func_frame_ptr_delta: %08X\n", func_frame_ptr_delta);
    Message("func_color_code: %08X\n", func_color_code);
    */
    
    /*
    Message("func_name, func_start_addr, func_end_addr, func_flags, func_frame_id, func_local_var_size, func_saved_regs_size, func_stack_bytes_purged_count, func_frame_ptr_delta, func_color_code\n");
    
    for(itr_ea_proc_next = MinEA(); itr_ea_proc_next != BADADDR; itr_ea_proc_next = NextFunction(itr_ea_proc_next))
    {
        proc_name = GetFunctionName(itr_ea_proc_next);
        // if(proc_name == 0) { Message("no procedure at the specified address\n"); }
        
        func_start_addr = GetFunctionAttr(itr_ea_proc_next, FUNCATTR_START);                    // function start address
        func_end_addr = GetFunctionAttr(itr_ea_proc_next, FUNCATTR_END);                        // function end address
        func_flags = GetFunctionAttr(itr_ea_proc_next, FUNCATTR_FLAGS);                         // function flags
        func_frame_id = GetFunctionAttr(itr_ea_proc_next, FUNCATTR_FRAME);                      // function frame id
        func_local_var_size = GetFunctionAttr(itr_ea_proc_next, FUNCATTR_FRSIZE);               // size of local variables
        func_saved_regs_size = GetFunctionAttr(itr_ea_proc_next, FUNCATTR_FRREGS);              // size of saved registers area
        func_stack_bytes_purged_count = GetFunctionAttr(itr_ea_proc_next, FUNCATTR_ARGSIZE);    // number of bytes purged from the stack
        func_frame_ptr_delta = GetFunctionAttr(itr_ea_proc_next, FUNCATTR_FPD);                 // frame pointer delta
        func_color_code = GetFunctionAttr(itr_ea_proc_next, FUNCATTR_COLOR);                    // function color code
        
        Message("%s, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n", 
            proc_name, 
            func_start_addr,
            func_end_addr,
            func_flags,
            func_frame_id,
            func_local_var_size,
            func_saved_regs_size,
            func_stack_bytes_purged_count,
            func_frame_ptr_delta,
            func_color_code
            );
    }
    */
    
    Message("\n\n------ Creating the output file.... --------\n");
    file_name = GetIdbPath();
    Message("file_name: %s\n", file_name);
    file_name = substr( file_name, 0, strstr( file_name, "." ) ) + "_funcs" + ".csv";
    Message("file_name: %s\n", file_name);
    file_handle = fopen(file_name, "w");
    file_cursor_position = ftell(file_handle);
    Message("file_cursor_position: %d\n", file_cursor_position);
    
    writestr(file_handle, "func_name, func_start_addr, func_end_addr, func_flags, func_frame_id, func_local_var_size, func_saved_regs_size, func_stack_bytes_purged_count, func_frame_ptr_delta, func_color_code\n");
    
    
    for(itr_ea_proc_next = MinEA(); itr_ea_proc_next != BADADDR; itr_ea_proc_next = NextFunction(itr_ea_proc_next))
    {
        proc_name = GetFunctionName(itr_ea_proc_next);
        // if(proc_name == 0) { Message("no procedure at the specified address\n"); }
        
        func_start_addr = GetFunctionAttr(itr_ea_proc_next, FUNCATTR_START);                    // function start address
        func_end_addr = GetFunctionAttr(itr_ea_proc_next, FUNCATTR_END);                        // function end address
        func_flags = GetFunctionAttr(itr_ea_proc_next, FUNCATTR_FLAGS);                         // function flags
        func_frame_id = GetFunctionAttr(itr_ea_proc_next, FUNCATTR_FRAME);                      // function frame id
        func_local_var_size = GetFunctionAttr(itr_ea_proc_next, FUNCATTR_FRSIZE);               // size of local variables
        func_saved_regs_size = GetFunctionAttr(itr_ea_proc_next, FUNCATTR_FRREGS);              // size of saved registers area
        func_stack_bytes_purged_count = GetFunctionAttr(itr_ea_proc_next, FUNCATTR_ARGSIZE);    // number of bytes purged from the stack
        func_frame_ptr_delta = GetFunctionAttr(itr_ea_proc_next, FUNCATTR_FPD);                 // frame pointer delta
        func_color_code = GetFunctionAttr(itr_ea_proc_next, FUNCATTR_COLOR);                    // function color code
        
        fprintf(
            file_handle,
            "%s, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n", 
            proc_name, 
            func_start_addr, 
            func_end_addr, 
            func_flags, 
            func_frame_id, 
            func_local_var_size, 
            func_saved_regs_size, 
            func_stack_bytes_purged_count, 
            func_frame_ptr_delta, 
            func_color_code
        );
    }
    
    file_cursor_position = ftell(file_handle);
    Message("file_cursor_position: %d\n", file_cursor_position);
    
    fclose(file_handle);
    
    Message("All done, exiting...\n");
    // Exit(0);                              // exit to OS, error code 0 - success


    // View -> Open subviews -> Functions ("Functions window")
    // Function name, Segment, Start, Length, R, F, L, S, B, T, =  

}
