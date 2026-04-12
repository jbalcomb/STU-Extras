// stu_dseg_info.idc
// C:\STU_DASM\IDA55\idc\stu_dseg_info.idc

#include <idc.idc>
#include <stu_def.idc>

/*

Address:
Name:

[x] Local name

e.g.,
No XREF
loc_7B881
7B88:0006                                                 loc_7B886:
7B886

7B88:0001
Jmp XREF
loc_7B8F7
7B88:0077

7B88:00B2 89 46 E2                                        mov     [bp+Landlubber_Count], ax
7B923
*/

/*
Move_Units()
0000
091F

7B88:0000
7B88:091E

*/

/*

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

//
//      Bits for CODE bytes
//

#define MS_CODE 0xF0000000L
#define FF_FUNC 0x10000000L             // function start?
#define FF_IMMD 0x40000000L             // Has Immediate value ?
#define FF_JUMP 0x80000000L             // Has jump table

isCode  

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

*/


#define hasLabel (F)     ((F & FF_LABL) != 0)


static main(void)
{
	auto ea;
	auto function_name, ea_func_start, ea_func_end;
	auto x,y;
	auto has_label;
	auto has_ref;
	
	ea = ScreenEA();
	
	function_name = GetFunctionName(ea);
	ea_func_start = GetFunctionAttr(ea, FUNCATTR_START);
	ea_func_end = GetFunctionAttr(ea, FUNCATTR_END);
	
	Message("function_name: %s\n", function_name);
	Message("ea_func_start: 0x%08X\n", ea_func_start);
	Message("ea_func_end: 0x%08X\n", ea_func_end);

	Message("\n*** Code references to " + atoa(ea) + " (only non-trivial refs)\n");
	x = ea;
	for ( y=RfirstB0(x); y != BADADDR; y=RnextB0(x,y) )
		Message("0x%08X is referred from 0x%08X %s\n", x, y, xrefchar());


	stu_del_loc_range(ea_func_start, ea_func_end);
	
}



static stu_del_loc_range(begin, end)
{
	auto ea;
	auto has_label;
	auto has_ref;
	
	for(ea = begin; ea < end; ea = NextAddr(ea))
	{
		has_label = ((GetFlags(ea) & FF_LABL) != 0);
		has_ref = isRef(GetFlags(ea));
		if(has_label && !has_ref)
		{
			MakeName(ea, "");
		}
	}

}


static scratch(void)
{
	auto ea_locbyname;
	auto str_locbyname;
	auto ea_loc;
	auto ea_xref_from_code;
	auto ea_xref_from_data;
	auto xref_count;
	auto function_name, ea_func_start, ea_func_end;
	auto ea_internal_flags;
    auto addr_name_visible;
    auto addr_name_true;
    auto addr_flags;
    auto addr_type;

	auto ea,flag,x,y;
	flag = 1;

	ea_loc = 0x0007B880;
	ea_loc = 0x0007B881;
	ea_loc = 0x0007B8F7;
	
	ea_locbyname = LocByName("Move_Units");
	Message("locbyname: 0x%08X\n", ea_locbyname);
	// >>> locbyname: 0x0007B880
	ea_locbyname = LocByName("loc_7B881");
	Message("locbyname: 0x%08X\n", ea_locbyname);
	// >>> locbyname: 0x0007B881
	ea_locbyname = LocByName("loc_7B8F7");
	Message("locbyname: 0x%08X\n", ea_locbyname);
	// >>> locbyname: 0x0007B8F7
	
	str_locbyname = "Move_Units";
	Message("%s:  0x%08X\n", str_locbyname, ea_locbyname);
	
	// RfirstB (long To);
	// DfirstB (long To);

	ea_xref_from_code = RfirstB (0x0007B881);
	Message("xref_from_code: 0x%08X\n", ea_xref_from_code);
	ea_xref_from_data = DfirstB (0x0007B881);
	Message("xref_from_data: 0x%08X\n", ea_xref_from_data);

	ea_xref_from_code = RfirstB (0x0007B8F7);
	Message("xref_from_code: 0x%08X\n", ea_xref_from_code);
	ea_xref_from_data = DfirstB (0x0007B8F7);
	Message("xref_from_data: 0x%08X\n", ea_xref_from_data);
// xref_from_code: 0x0007B880
// xref_from_data: 0xFFFFFFFF
// xref_from_code: 0x0007B8EB
// xref_from_data: 0xFFFFFFFF

	ea = 0x0007B881;
	Message("\n*** Code references to " + atoa(ea) + "\n");
	x = ea;
	for ( y=RfirstB(x); y != BADADDR; y=RnextB(x,y) )
		Message(atoa(x) + " is referred from " + atoa(y) + xrefchar() + "\n");
// *** Code references to 0001
// 0001 is referred from 0000 (ordinary flow)
	for ( y=RfirstB(x); y != BADADDR; y=RnextB(x,y) )
		Message("0x%08X is referred from 0x%08X %s\n", x, y, xrefchar());
// 0x0007B881 is referred from 0x0007B880  (ordinary flow)

	Message("\n*** Code references to " + atoa(ea) + " (only non-trivial refs)\n");
	x = ea;
	for ( y=RfirstB0(x); y != BADADDR; y=RnextB0(x,y) )
		Message("0x%08X is referred from 0x%08X %s\n", x, y, xrefchar());

	ea = 0x0007B8F7;
	Message("\n*** Code references to " + atoa(ea) + "\n");
	x = ea;
	for ( y=RfirstB(x); y != BADADDR; y=RnextB(x,y) )
		Message(atoa(x) + " is referred from " + atoa(y) + xrefchar() + "\n");
// *** Code references to 0001
// 0001 is referred from 0000 (ordinary flow)
	for ( y=RfirstB(x); y != BADADDR; y=RnextB(x,y) )
		Message("0x%08X is referred from 0x%08X %s\n", x, y, xrefchar());
// 0x0007B881 is referred from 0x0007B880  (ordinary flow)

	Message("\n*** Code references to " + atoa(ea) + " (only non-trivial refs)\n");
	x = ea;
	for ( y=RfirstB0(x); y != BADADDR; y=RnextB0(x,y) )
		Message("0x%08X is referred from 0x%08X %s\n", x, y, xrefchar());
// *** Code references to 0077
// 0077 is referred from 006B (jump near)
// 0x0007B8F7 is referred from 0x0007B8EB  (jump near)

// *** Code references to 0077 (only non-trivial refs)
// 0x0007B8F7 is referred from 0x0007B8EB  (jump near)


	ea = 0x0007B89D;
	Message("\n*** Code references to " + atoa(ea) + "\n");
	x = ea;
	for ( y=RfirstB(x); y != BADADDR; y=RnextB(x,y) )
		Message(atoa(x) + " is referred from " + atoa(y) + xrefchar() + "\n");
	for ( y=RfirstB(x); y != BADADDR; y=RnextB(x,y) )
		Message("0x%08X is referred from 0x%08X %s\n", x, y, xrefchar());
	Message("\n*** Code references to " + atoa(ea) + " (only non-trivial refs)\n");
	x = ea;
	for ( y=RfirstB0(x); y != BADADDR; y=RnextB0(x,y) )
		Message("0x%08X is referred from 0x%08X %s\n", x, y, xrefchar());
// *** Code references to 001D
// 001D is referred from 001C (ordinary flow)
// 0x0007B89D is referred from 0x0007B89C  (ordinary flow)
// *** Code references to 001D (only non-trivial refs)




// success MakeNameEx(long ea, string name, long flags);  // returns: 1-ok, 0-failure

	// Workie!! MakeName(0x0007B881, "");

	// ea = ScreenEA();
	// MakeName(ea, "");
	
	ea = ScreenEA();
	function_name = GetFunctionName(ea);
	Message("function_name: %s\n", function_name);
	// function_name: Move_Units

		// seg_start_addr = GetSegmentAttr(itr_ea_func_next, SEGATTR_START);   // starting address
        // seg_end_addr = GetSegmentAttr(itr_ea_func_next, SEGATTR_END);       // ending address

	ea_func_start = GetFunctionAttr(ea, FUNCATTR_START);
	ea_func_end = GetFunctionAttr(ea, FUNCATTR_END);
	Message("ea_func_start: 0x%08X\n", ea_func_start);
	Message("ea_func_end: 0x%08X\n", ea_func_end);
// ea_func_start: 0x0007B880
// ea_func_end: 0x0007C19F


	ea = 0x0007B886;
	
	// long    GetFlags        (long ea);              // get internal flags for ea
	ea_internal_flags = GetFlags(ea);

	// Message("ea: 0x%08X, addr_name_visible: %s, addr_name_true: %s, addr_flags: 0x%016X, addr_type: %s\n", itr_ea_addr_next, addr_name_visible, addr_name_true, addr_flags, addr_type);

	// hasValue(GetFlags(ea))

	ea = 0x0007B886;  // bogus
	addr_name_visible = NameEx(0, ea);
	addr_name_true = GetTrueNameEx(0, ea);
	addr_type = GetType(ea);
	addr_flags = GetFlags(ea);
	Message("%s, %s\n", addr_name_visible, addr_name_true);
	Message("0x%08X, 0x%016X, %s\n", ea, addr_flags, addr_type);
	// Message(hasLabel(GetFlags(ea)));
	Message("isCode: %d\n", isCode(GetFlags(ea)));
	Message("isFlow: %d\n", isFlow(GetFlags(ea)));
	Message("isVar: %d\n", isVar(GetFlags(ea)));
	Message("isExtra: %d\n", isExtra(GetFlags(ea)));
	Message("isRef: %d\n", isRef(GetFlags(ea)));
	Message("hasName: %d\n", hasName(GetFlags(ea)));
	Message("hasUserName: %d\n", hasUserName(GetFlags(ea)));
	Message("hasLabel: %d\n", ((GetFlags(ea) & FF_LABL) != 0));

	ea = 0x0007B923; // none
	addr_name_visible = NameEx(0, ea);
	addr_name_true = GetTrueNameEx(0, ea);
	addr_type = GetType(ea);
	addr_flags = GetFlags(ea);
	Message("%s, %s\n", addr_name_visible, addr_name_true);
	Message("0x%08X, 0x%016X, %s\n", ea, addr_flags, addr_type);
	// Message(hasLabel(GetFlags(ea)));
	Message("isCode: %d\n", isCode(GetFlags(ea)));
	Message("isFlow: %d\n", isFlow(GetFlags(ea)));
	Message("isVar: %d\n", isVar(GetFlags(ea)));
	Message("isExtra: %d\n", isExtra(GetFlags(ea)));
	Message("isRef: %d\n", isRef(GetFlags(ea)));
	Message("hasName: %d\n", hasName(GetFlags(ea)));
	Message("hasUserName: %d\n", hasUserName(GetFlags(ea)));
	Message("hasLabel: %d\n", ((GetFlags(ea) & FF_LABL) != 0));

	ea = 0x0007B8F7; // legit
	addr_name_visible = NameEx(0, ea);
	addr_name_true = GetTrueNameEx(0, ea);
	addr_type = GetType(ea);
	addr_flags = GetFlags(ea);
	Message("%s, %s\n", addr_name_visible, addr_name_true);
	Message("0x%08X, 0x%016X, %s\n", ea, addr_flags, addr_type);
	// Message(hasLabel(GetFlags(ea)));
	Message("isCode: %d\n", isCode(GetFlags(ea)));
	Message("isFlow: %d\n", isFlow(GetFlags(ea)));
	Message("isVar: %d\n", isVar(GetFlags(ea)));
	Message("isExtra: %d\n", isExtra(GetFlags(ea)));
	Message("isRef: %d\n", isRef(GetFlags(ea)));
	Message("hasName: %d\n", hasName(GetFlags(ea)));
	Message("hasUserName: %d\n", hasUserName(GetFlags(ea)));
	Message("hasUserName: %d\n", hasUserName(GetFlags(ea)));
	Message("hasLabel: %d\n", ((GetFlags(ea) & FF_LABL) != 0));

// 0x0007B886, 0x0000000000018756, 
// 0x0007B89D, 0x0000000040B1A789, 
// 0x0000000040000000
// 01000000000000000000000000000000
/*
loc_7B886, loc_7B886
0x0007B886, 0x0000000000018756, 
isCode: 1
isFlow: 1
isVar: 0
isExtra: 0
isRef: 0
hasName: 0
hasUserName: 0
hasLabel: 1

, 
0x0007B923, 0x0000000000010F50, 
isCode: 1
isFlow: 1
isVar: 0
isExtra: 0
isRef: 0
hasName: 0
hasUserName: 0
hasLabel: 0

loc_7B8F7, loc_7B8F7
0x0007B8F7, 0x000000000000B733, 
isCode: 1
isFlow: 0
isVar: 0
isExtra: 1
isRef: 1
hasName: 0
hasUserName: 0
hasUserName: 0
hasLabel: 1

*/

}
