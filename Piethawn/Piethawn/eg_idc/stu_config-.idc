// J:\STU\WD\IDA55\idc\stu_config.idc


#include <idc.idc>

static main(void)
{
    
    stu_config_get();
    
    stu_config_set();
    
    stu_config_get();

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
