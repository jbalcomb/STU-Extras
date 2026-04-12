#include <idc.idc>



// SEEALSO:  stu_seg_info.idc  static stu_seg_info(void)  J:\STU\WD\IDA55\idc\stu_seg_info.idc
// SEEALSO:  stu_learning.idc  static stu_seg_info(void)



static main(void)
{
	// stu_itr_sgmt();
	stu_sgmt_info();
	// stu_program_segmentation();
}

static stu_itr_sgmt(void)
{
    auto ea_sgmt_first;
	auto itr_ea_sgmt_next;
	auto segment_name;

    ea_sgmt_first = FirstSeg();
	for(itr_ea_sgmt_next = ea_sgmt_first; itr_ea_sgmt_next != BADADDR; itr_ea_sgmt_next = NextSeg(itr_ea_sgmt_next))
    {
		Message("itr_ea_sgmt_next:   0x%08X\n", itr_ea_sgmt_next);
		segment_name = SegName(itr_ea_sgmt_next);
		Message("segment_name: %s\n", segment_name);
		
		// seg_start_addr = GetSegmentAttr(itr_ea_func_next, SEGATTR_START);   // starting address
        // seg_end_addr = GetSegmentAttr(itr_ea_func_next, SEGATTR_END);       // ending address
	}
}



static stu_sgmt_info(void)
{
    auto ea_sgmt_first;
	auto itr_ea_sgmt_next;
	auto segment_name;
	auto seg_start_addr;
    auto seg_end_addr;

	auto seg_org_base;
	auto seg_align;
	auto seg_comb;
	auto seg_perm;
	auto seg_bitness;
	auto seg_flags;
	auto seg_sel;
	auto seg_es;
	auto seg_cs;
	auto seg_ss;
	auto seg_ds;
	auto seg_fs;
	auto seg_gs;
	auto seg_type;
	auto seg_color;
	
	auto prog_sgmt_name;
	auto prog_sgmt_start;
	auto prog_sgmt_end;

    ea_sgmt_first = FirstSeg();
	
	itr_ea_sgmt_next = NextSeg(itr_ea_sgmt_next);

	segment_name = SegName(itr_ea_sgmt_next);
	seg_start_addr = GetSegmentAttr(itr_ea_sgmt_next, SEGATTR_START);   // starting address
	seg_end_addr = GetSegmentAttr(itr_ea_sgmt_next, SEGATTR_END);       // ending address

	seg_org_base = GetSegmentAttr(itr_ea_sgmt_next, SEGATTR_ORGBASE);
	seg_align = GetSegmentAttr(itr_ea_sgmt_next, SEGATTR_ALIGN);		// alignment
	seg_comb = GetSegmentAttr(itr_ea_sgmt_next, SEGATTR_COMB);			// combination
	seg_perm = GetSegmentAttr(itr_ea_sgmt_next, SEGATTR_PERM);			// permissions
	seg_bitness = GetSegmentAttr(itr_ea_sgmt_next, SEGATTR_BITNESS);	// bitness (0: 16, 1: 32, 2: 64 bit segment)
	seg_flags = GetSegmentAttr(itr_ea_sgmt_next, SEGATTR_FLAGS);		// segment flags
	seg_sel = GetSegmentAttr(itr_ea_sgmt_next, SEGATTR_SEL);			// segment selector
	seg_es = GetSegmentAttr(itr_ea_sgmt_next, SEGATTR_ES);        		// default ES value
	seg_cs = GetSegmentAttr(itr_ea_sgmt_next, SEGATTR_CS);        		// default CS value
	seg_ss = GetSegmentAttr(itr_ea_sgmt_next, SEGATTR_SS);        		// default SS value
	seg_ds = GetSegmentAttr(itr_ea_sgmt_next, SEGATTR_DS);        		// default DS value
	seg_fs = GetSegmentAttr(itr_ea_sgmt_next, SEGATTR_FS);        		// default FS value
	seg_gs = GetSegmentAttr(itr_ea_sgmt_next, SEGATTR_GS);        		// default GS value
	seg_type = GetSegmentAttr(itr_ea_sgmt_next, SEGATTR_TYPE);      	// segment type
	seg_color = GetSegmentAttr(itr_ea_sgmt_next, SEGATTR_COLOR);     	// segment color

	Message("0x%08X\n", itr_ea_sgmt_next);
	Message("%s\n", segment_name);
	Message("0x%08X\n", seg_start_addr);
	Message("0x%08X\n", seg_end_addr);
	
	Message("0x%08X\n", seg_org_base);
	Message("0x%08X\n", seg_align);
	Message("0x%08X\n", seg_comb);
	Message("0x%08X\n", seg_perm);
	Message("0x%08X\n", seg_bitness);
	Message("0x%08X\n", seg_flags);
	Message("0x%08X\n", seg_sel);
	Message("0x%08X\n", seg_es);
	Message("0x%08X\n", seg_cs);
	Message("0x%08X\n", seg_ss);
	Message("0x%08X\n", seg_ds);
	Message("0x%08X\n", seg_fs);
	Message("0x%08X\n", seg_gs);
	Message("0x%08X\n", seg_type);
	Message("0x%08X\n", seg_color);
	
	// 'Program Segmentation' View
	prog_sgmt_name = segment_name;
	prog_sgmt_start = seg_start_addr - itr_ea_sgmt_next;
	prog_sgmt_end = seg_end_addr - itr_ea_sgmt_next;

	Message("%s, %08X, %08X\n", segment_name, prog_sgmt_start, prog_sgmt_end);

}
	// 0x00010000
	// seg000
	// 0x00010000
	// 0x00013209
	// 0x00000000
	// 0x00000001
	// 0x00000002
	// 0x00000000
	// 0x00000000
	// 0x00000010
	// 0x00001000
	// 0xFFFFFFFF
	// 0xFFFFFFFF
	// 0xFFFFFFFF
	// 0x000031D1
	// 0xFFFFFFFF
	// 0xFFFFFFFF
	// 0x00000002
	// 0xFFFFFFFF



static stu_program_segmentation(void)
{
    auto ea_sgmt_first;
	auto itr_ea_sgmt_next;
	auto segment_name;
	auto seg_start_addr;
    auto seg_end_addr;
	
	auto seg_org_base;
	auto seg_align;
	auto seg_comb;
	auto seg_perm;
	auto seg_bitness;
	auto seg_flags;
	auto seg_sel;
	auto seg_es;
	auto seg_cs;
	auto seg_ss;
	auto seg_ds;
	auto seg_fs;
	auto seg_gs;
	auto seg_type;
	auto seg_color;

    ea_sgmt_first = FirstSeg();
	for(itr_ea_sgmt_next = ea_sgmt_first; itr_ea_sgmt_next != BADADDR; itr_ea_sgmt_next = NextSeg(itr_ea_sgmt_next))
    {
		segment_name = SegName(itr_ea_sgmt_next);
		seg_start_addr = GetSegmentAttr(itr_ea_sgmt_next, SEGATTR_START);   // starting address
        seg_end_addr = GetSegmentAttr(itr_ea_sgmt_next, SEGATTR_END);       // ending address
		
		seg_org_base = GetSegmentAttr(itr_ea_sgmt_next, SEGATTR_ORGBASE);
		seg_align = GetSegmentAttr(itr_ea_sgmt_next, SEGATTR_ALIGN);		// alignment
		seg_comb = GetSegmentAttr(itr_ea_sgmt_next, SEGATTR_COMB);			// combination
		seg_perm = GetSegmentAttr(itr_ea_sgmt_next, SEGATTR_PERM);			// permissions
		seg_bitness = GetSegmentAttr(itr_ea_sgmt_next, SEGATTR_BITNESS);	// bitness (0: 16, 1: 32, 2: 64 bit segment)
		seg_flags = GetSegmentAttr(itr_ea_sgmt_next, SEGATTR_FLAGS);		// segment flags
		seg_sel = GetSegmentAttr(itr_ea_sgmt_next, SEGATTR_SEL);			// segment selector
		seg_es = GetSegmentAttr(itr_ea_sgmt_next, SEGATTR_ES);        		// default ES value
		seg_cs = GetSegmentAttr(itr_ea_sgmt_next, SEGATTR_CS);        		// default CS value
		seg_ss = GetSegmentAttr(itr_ea_sgmt_next, SEGATTR_SS);        		// default SS value
		seg_ds = GetSegmentAttr(itr_ea_sgmt_next, SEGATTR_DS);        		// default DS value
		seg_fs = GetSegmentAttr(itr_ea_sgmt_next, SEGATTR_FS);        		// default FS value
		seg_gs = GetSegmentAttr(itr_ea_sgmt_next, SEGATTR_GS);        		// default GS value
		seg_type = GetSegmentAttr(itr_ea_sgmt_next, SEGATTR_TYPE);      	// segment type
		seg_color = GetSegmentAttr(itr_ea_sgmt_next, SEGATTR_COLOR);     	// segment color
		
		Message("0x%08X, %s, 0x%08X, 0x%08X\n", itr_ea_sgmt_next, segment_name, seg_start_addr, seg_end_addr);
		
	}
}
