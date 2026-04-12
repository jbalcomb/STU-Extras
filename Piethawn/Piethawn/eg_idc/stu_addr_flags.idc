/*
    Address Flags
*/

/*
Each byte of the program has 32-bit flags
(low 8 bits keep the byte value)

Get internal flags
    ea - linear address
    returns: 32-bit value of internal flags
long    GetFlags        (long ea);              // get internal flags for ea
*/

/*
  31  30  29  28  27  26  25  24  23  22  21  20  19  18  17  16  15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0
[ 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 ]
                                                                                    [       ] | [                               ] -- value
                                                                                      |       |
                                                                                      |       \-- Has Value {false = 0, true = 1}
                                                                                      |
                                                                                      \-- Class (CLS) / Type {}


#define FF_IVL  0x00000100L             // Byte has value ?
#define hasValue(F)     ((F & FF_IVL) != 0)     // any defined value?



0000 0000 0000 0000 0000 0000 0000 0000 
0000 0000 0000 0000 0000 0110 0000 0000
#define MS_CLS  0x00000600L             // Mask for typing


*/

/*

// Get internal flags
//      ea - linear address
// returns: 32-bit value of internal flags. See start of IDC.IDC file
// for explanations.

long    GetFlags        (long ea);              // get internal flags for ea



 *      Each byte of the program has 32-bit flags
 *      (low 8 bits keep the byte value).
 *      These 32 bits are used in GetFlags/SetFlags functions.
 *      You may freely examine these bits using GetFlags()


//
//      Flag bit definitions (for GetFlags())
//

#define MS_VAL  0x000000FFL             // Mask for byte value
#define FF_IVL  0x00000100L             // Byte has value ?


// Do flags contain byte value? (i.e. has the byte a value?)
// if not, the byte is uninitialized.

#define hasValue(F)     ((F & FF_IVL) != 0)     // any defined value?


// Get byte value from flags
// Get value of byte provided that the byte is initialized.
// This macro works ok only for 8-bit byte machines.

#define byteValue(F)    (F & MS_VAL)    // quick replacement for Byte()


// Is the byte initialized?

#define isLoaded(ea)    hasValue(GetFlags(ea))  // any defined value?

#define MS_CLS  0x00000600L             // Mask for typing
#define FF_CODE 0x00000600L             // Code ?
#define FF_DATA 0x00000400L             // Data ?
#define FF_TAIL 0x00000200L             // Tail ?
#define FF_UNK  0x00000000L             // Unknown ?


#define isCode(F)       ((F & MS_CLS) == FF_CODE) // is code byte?
#define isData(F)       ((F & MS_CLS) == FF_DATA) // is data byte?
#define isTail(F)       ((F & MS_CLS) == FF_TAIL) // is tail byte?
#define isUnknown(F)    ((F & MS_CLS) == FF_UNK)  // is unexplored byte?
#define isHead(F)       ((F & FF_DATA) != 0)      // is start of code/data?

//
//      Common bits
//

#define MS_COMM 0x000FF800L             // Mask of common bits
#define FF_COMM 0x00000800L             // Has comment?
#define FF_REF  0x00001000L             // has references?
#define FF_LINE 0x00002000L             // Has next or prev cmt lines ?
#define FF_NAME 0x00004000L             // Has user-defined name ?
#define FF_LABL 0x00008000L             // Has dummy name?
#define FF_FLOW 0x00010000L             // Exec flow from prev instruction?
#define FF_VAR  0x00080000L             // Is byte variable ?
#define FF_ANYNAME      (FF_LABL|FF_NAME)

#define isFlow(F)       ((F & FF_FLOW) != 0)
#define isVar(F)        ((F & FF_VAR ) != 0)
#define isExtra(F)      ((F & FF_LINE) != 0)
#define isRef(F)        ((F & FF_REF)  != 0)
#define hasName(F)      ((F & FF_NAME) != 0)
#define hasUserName(F)  ((F & FF_ANYNAME) == FF_NAME)

#define MS_0TYPE 0x00F00000L            // Mask for 1st arg typing
#define FF_0VOID 0x00000000L            // Void (unknown)?
#define FF_0NUMH 0x00100000L            // Hexadecimal number?
#define FF_0NUMD 0x00200000L            // Decimal number?
#define FF_0CHAR 0x00300000L            // Char ('x')?
#define FF_0SEG  0x00400000L            // Segment?
#define FF_0OFF  0x00500000L            // Offset?
#define FF_0NUMB 0x00600000L            // Binary number?
#define FF_0NUMO 0x00700000L            // Octal number?
#define FF_0ENUM 0x00800000L            // Enumeration?
#define FF_0FOP  0x00900000L            // Forced operand?
#define FF_0STRO 0x00A00000L            // Struct offset?
#define FF_0STK  0x00B00000L            // Stack variable?

#define MS_1TYPE 0x0F000000L            // Mask for 2nd arg typing
#define FF_1VOID 0x00000000L            // Void (unknown)?
#define FF_1NUMH 0x01000000L            // Hexadecimal number?
#define FF_1NUMD 0x02000000L            // Decimal number?
#define FF_1CHAR 0x03000000L            // Char ('x')?
#define FF_1SEG  0x04000000L            // Segment?
#define FF_1OFF  0x05000000L            // Offset?
#define FF_1NUMB 0x06000000L            // Binary number?
#define FF_1NUMO 0x07000000L            // Octal number?
#define FF_1ENUM 0x08000000L            // Enumeration?
#define FF_1FOP  0x09000000L            // Forced operand?
#define FF_1STRO 0x0A000000L            // Struct offset?
#define FF_1STK  0x0B000000L            // Stack variable?

// The following macros answer questions like
//   'is the 1st (or 2nd) operand of instruction or data of the given type'?
// Please note that data items use only the 1st operand type (is...0)

#define isDefArg0(F)    ((F & MS_0TYPE) != FF_0VOID)
#define isDefArg1(F)    ((F & MS_1TYPE) != FF_1VOID)
#define isDec0(F)       ((F & MS_0TYPE) == FF_0NUMD)
#define isDec1(F)       ((F & MS_1TYPE) == FF_1NUMD)
#define isHex0(F)       ((F & MS_0TYPE) == FF_0NUMH)
#define isHex1(F)       ((F & MS_1TYPE) == FF_1NUMH)
#define isOct0(F)       ((F & MS_0TYPE) == FF_0NUMO)
#define isOct1(F)       ((F & MS_1TYPE) == FF_1NUMO)
#define isBin0(F)       ((F & MS_0TYPE) == FF_0NUMB)
#define isBin1(F)       ((F & MS_1TYPE) == FF_1NUMB)
#define isOff0(F)       ((F & MS_0TYPE) == FF_0OFF)
#define isOff1(F)       ((F & MS_1TYPE) == FF_1OFF)
#define isChar0(F)      ((F & MS_0TYPE) == FF_0CHAR)
#define isChar1(F)      ((F & MS_1TYPE) == FF_1CHAR)
#define isSeg0(F)       ((F & MS_0TYPE) == FF_0SEG)
#define isSeg1(F)       ((F & MS_1TYPE) == FF_1SEG)
#define isEnum0(F)      ((F & MS_0TYPE) == FF_0ENUM)
#define isEnum1(F)      ((F & MS_1TYPE) == FF_1ENUM)
#define isFop0(F)       ((F & MS_0TYPE) == FF_0FOP)
#define isFop1(F)       ((F & MS_1TYPE) == FF_1FOP)
#define isStroff0(F)    ((F & MS_0TYPE) == FF_0STRO)
#define isStroff1(F)    ((F & MS_1TYPE) == FF_1STRO)
#define isStkvar0(F)    ((F & MS_0TYPE) == FF_0STK)
#define isStkvar1(F)    ((F & MS_1TYPE) == FF_1STK)

//
//      Bits for DATA bytes
//

#define DT_TYPE 0xF0000000L             // Mask for DATA typing

#define FF_BYTE 0x00000000L             // byte
#define FF_WORD 0x10000000L             // word
#define FF_DWRD 0x20000000L             // dword
#define FF_QWRD 0x30000000L             // qword
#define FF_TBYT 0x40000000L             // tbyte
#define FF_ASCI 0x50000000L             // ASCII ?
#define FF_STRU 0x60000000L             // Struct ?
#define FF_OWRD 0x70000000L             // octaword (16 bytes)
#define FF_FLOAT 0x80000000L            // float
#define FF_DOUBLE 0x90000000L           // double
#define FF_PACKREAL 0xA0000000L         // packed decimal real
#define FF_ALIGN    0xB0000000L         // alignment directive

#define isByte(F)     (isData(F) && (F & DT_TYPE) == FF_BYTE)
#define isWord(F)     (isData(F) && (F & DT_TYPE) == FF_WORD)
#define isDwrd(F)     (isData(F) && (F & DT_TYPE) == FF_DWRD)
#define isQwrd(F)     (isData(F) && (F & DT_TYPE) == FF_QWRD)
#define isOwrd(F)     (isData(F) && (F & DT_TYPE) == FF_OWRD)
#define isTbyt(F)     (isData(F) && (F & DT_TYPE) == FF_TBYT)
#define isFloat(F)    (isData(F) && (F & DT_TYPE) == FF_FLOAT)
#define isDouble(F)   (isData(F) && (F & DT_TYPE) == FF_DOUBLE)
#define isPackReal(F) (isData(F) && (F & DT_TYPE) == FF_PACKREAL)
#define isASCII(F)    (isData(F) && (F & DT_TYPE) == FF_ASCI)
#define isStruct(F)   (isData(F) && (F & DT_TYPE) == FF_STRU)
#define isAlign(F)    (isData(F) && (F & DT_TYPE) == FF_ALIGN)

//
//      Bits for CODE bytes
//

#define MS_CODE 0xF0000000L
#define FF_FUNC 0x10000000L             // function start?
#define FF_IMMD 0x40000000L             // Has Immediate value ?
#define FF_JUMP 0x80000000L             // Has jump table

*/

#define BIT_MASK_01 0x00000001  // 0000 0000 0000 0000 0000 0000 0000 0001
#define BIT_MASK_02 0x00000002  // 0000 0000 0000 0000 0000 0000 0000 0010
#define BIT_MASK_03 0x00000004  // 0000 0000 0000 0000 0000 0000 0000 0100
#define BIT_MASK_04 0x00000008  // 0000 0000 0000 0000 0000 0000 0000 1000
#define BIT_MASK_05 0x00000010  // 0000 0000 0000 0000 0000 0000 0001 0000
#define BIT_MASK_06 0x00000020  // 0000 0000 0000 0000 0000 0000 0010 0000
#define BIT_MASK_07 0x00000040  // 0000 0000 0000 0000 0000 0000 0100 0000
#define BIT_MASK_08 0x00000080  // 0000 0000 0000 0000 0000 0000 1000 0000
#define BIT_MASK_09 0x00000100  // 0000 0000 0000 0000 0000 0001 0000 0000
#define BIT_MASK_10 0x00000200  // 0000 0000 0000 0000 0000 0010 0000 0000
#define BIT_MASK_11 0x00000400  // 0000 0000 0000 0000 0000 0100 0000 0000
#define BIT_MASK_12 0x00000800  // 0000 0000 0000 0000 0000 1000 0000 0000
#define BIT_MASK_13 0x00001000  // 0000 0000 0000 0000 0001 0000 0000 0000
#define BIT_MASK_14 0x00002000  // 0000 0000 0000 0000 0010 0000 0000 0000
#define BIT_MASK_15 0x00004000  // 0000 0000 0000 0000 0100 0000 0000 0000
#define BIT_MASK_16 0x00008000  // 0000 0000 0000 0000 1000 0000 0000 0000
#define BIT_MASK_17 0x00010000  // 0000 0000 0000 0001 0000 0000 0000 0000
#define BIT_MASK_18 0x00020000  // 0000 0000 0000 0010 0000 0000 0000 0000
#define BIT_MASK_19 0x00040000  // 0000 0000 0000 0100 0000 0000 0000 0000
#define BIT_MASK_20 0x00080000  // 0000 0000 0000 1000 0000 0000 0000 0000
#define BIT_MASK_21 0x00100000  // 0000 0000 0001 0000 0000 0000 0000 0000
#define BIT_MASK_22 0x00200000  // 0000 0000 0010 0000 0000 0000 0000 0000
#define BIT_MASK_23 0x00400000  // 0000 0000 0100 0000 0000 0000 0000 0000
#define BIT_MASK_24 0x00800000  // 0000 0000 1000 0000 0000 0000 0000 0000
#define BIT_MASK_25 0x01000000  // 0000 0001 0000 0000 0000 0000 0000 0000
#define BIT_MASK_26 0x02000000  // 0000 0010 0000 0000 0000 0000 0000 0000
#define BIT_MASK_27 0x04000000  // 0000 0100 0000 0000 0000 0000 0000 0000
#define BIT_MASK_28 0x08000000  // 0000 1000 0000 0000 0000 0000 0000 0000
#define BIT_MASK_29 0x10000000  // 0001 0000 0000 0000 0000 0000 0000 0000
#define BIT_MASK_30 0x20000000  // 0010 0000 0000 0000 0000 0000 0000 0000
#define BIT_MASK_31 0x40000000  // 0100 0000 0000 0000 0000 0000 0000 0000
#define BIT_MASK_32 0x80000000  // 1000 0000 0000 0000 0000 0000 0000 0000

#define MASK_01 0x00000001  // 0000 0000 0000 0000 0000 0000 0000 0001
#define MASK_02 0x00000002  // 0000 0000 0000 0000 0000 0000 0000 0010
#define MASK_03 0x00000004  // 0000 0000 0000 0000 0000 0000 0000 0100
#define MASK_04 0x00000008  // 0000 0000 0000 0000 0000 0000 0000 1000
#define MASK_05 0x00000010  // 0000 0000 0000 0000 0000 0000 0001 0000
#define MASK_06 0x00000020  // 0000 0000 0000 0000 0000 0000 0010 0000
#define MASK_07 0x00000040  // 0000 0000 0000 0000 0000 0000 0100 0000
#define MASK_08 0x00000080  // 0000 0000 0000 0000 0000 0000 1000 0000
#define MASK_09 0x00000100  // 0000 0000 0000 0000 0000 0001 0000 0000
#define MASK_10 0x00000200  // 0000 0000 0000 0000 0000 0010 0000 0000
#define MASK_11 0x00000400  // 0000 0000 0000 0000 0000 0100 0000 0000
#define MASK_12 0x00000800  // 0000 0000 0000 0000 0000 1000 0000 0000
#define MASK_13 0x00001000  // 0000 0000 0000 0000 0001 0000 0000 0000
#define MASK_14 0x00002000  // 0000 0000 0000 0000 0010 0000 0000 0000
#define MASK_15 0x00004000  // 0000 0000 0000 0000 0100 0000 0000 0000
#define MASK_16 0x00008000  // 0000 0000 0000 0000 1000 0000 0000 0000
#define MASK_17 0x00010000  // 0000 0000 0000 0001 0000 0000 0000 0000
#define MASK_18 0x00020000  // 0000 0000 0000 0010 0000 0000 0000 0000
#define MASK_19 0x00040000  // 0000 0000 0000 0100 0000 0000 0000 0000
#define MASK_20 0x00080000  // 0000 0000 0000 1000 0000 0000 0000 0000
#define MASK_21 0x00100000  // 0000 0000 0001 0000 0000 0000 0000 0000
#define MASK_22 0x00200000  // 0000 0000 0010 0000 0000 0000 0000 0000
#define MASK_23 0x00400000  // 0000 0000 0100 0000 0000 0000 0000 0000
#define MASK_24 0x00800000  // 0000 0000 1000 0000 0000 0000 0000 0000
#define MASK_25 0x01000000  // 0000 0001 0000 0000 0000 0000 0000 0000
#define MASK_26 0x02000000  // 0000 0010 0000 0000 0000 0000 0000 0000
#define MASK_27 0x04000000  // 0000 0100 0000 0000 0000 0000 0000 0000
#define MASK_28 0x08000000  // 0000 1000 0000 0000 0000 0000 0000 0000
#define MASK_29 0x10000000  // 0001 0000 0000 0000 0000 0000 0000 0000
#define MASK_30 0x20000000  // 0010 0000 0000 0000 0000 0000 0000 0000
#define MASK_31 0x40000000  // 0100 0000 0000 0000 0000 0000 0000 0000
#define MASK_32 0x80000000  // 1000 0000 0000 0000 0000 0000 0000 0000

