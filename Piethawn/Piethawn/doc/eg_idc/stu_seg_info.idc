// stu_seg_info.idc
// J:\STU\WD\IDA55\idc\stu_seg_info.idc

#include <idc.idc>


static main(void)
{
    stu_seg_info();
}


static stu_seg_info(void)
{
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
        
    }

    file_cursor_position = ftell(file_handle);
    Message("file_cursor_position: %d\n", file_cursor_position);
    
    fclose(file_handle);
    
    
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
    
}
