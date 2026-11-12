// stu_proc_todolist.idc
// J:\STU\WD\IDA55\idc\stu_proc_todolist.idc

#include <idc.idc>

/*

Requirements?
...supports re-assembly (rasm)...

prefixing function names with an underscore (_) is required for C/C++ <-> ASM

during the reassembly process, it is quite handy to know where the function is located in IDA for double-checking the program-flow visually and synchronizing on-the-fly changes


Elsewhere, ...
-all (non-local) labels must be unique per assembly /module/ (i.e., .ASM file), including members of structures and enumerations
  .:. definitely prepend jump locations inside functions with the LOCAL LABEL /directive/, which is two at-symbols (@@)
-IIRC, there is something to immediately identifying structures and enumerations and/or their members
  .:. prepend with "s_" and "e_"
-




Function Name Format:
_f040102_DOS_FindFile


*/

static main(void)
{
    stu_proc_todolist();
}


static stu_proc_todolist(void)
{
    auto ea_begin;
    auto ea_min;
    auto ea_max;
    auto ea_first_seg;

    auto ea_proc_first;
    auto ea_proc_next;
    auto proc_name_first;
    auto proc_name_next;

    auto itr_function_number;
    auto itr_ea_func_next;
    auto function_name;
    
    auto itr_segment_number;
    auto segment_name;
    auto seg_start_addr_curr;
    auto seg_start_addr;
    auto seg_end_addr;
    
    auto underscore_position_first;
    
    ea_begin = BeginEA();
    ea_min = MinEA();
    ea_max = MaxEA();
    ea_first_seg = FirstSeg();
    
    Message("ea_begin: %08X\n", ea_begin);
    Message("ea_min:   %08X\n", ea_min);
    Message("ea_max:   %08X\n", ea_max);
    Message("ea_first_seg:   %08X\n", ea_first_seg);


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




    itr_function_number = 0;
    itr_segment_number = 0;
    seg_start_addr_curr = BADADDR;
    ea_min = 0x55CA0;  // DEBUG: 3rd to last segment, 5th to last function; i.e., start at the last function in the 65th segment
    for(itr_ea_func_next = ea_min; itr_ea_func_next != BADADDR; itr_ea_func_next = NextFunction(itr_ea_func_next))
    {
        
        function_name = GetFunctionName(itr_ea_func_next);
        // if(function_name == 0) { Message("no function at the specified address\n"); }
        
        segment_name = SegName(itr_ea_func_next);
        // if(segment_name == 0) { Message("no segment at the specified address\n"); }
        
        seg_start_addr = GetSegmentAttr(itr_ea_func_next, SEGATTR_START);   // starting address
        seg_end_addr = GetSegmentAttr(itr_ea_func_next, SEGATTR_END);       // ending address
        
        Message("0x%X, 0x%X\n", seg_start_addr_curr, seg_start_addr); // 0xFFFFFFFF, 0x56450
        
        
        itr_function_number = itr_function_number + 1;
        
        if (seg_start_addr_curr != seg_start_addr)
        {
            Message("Segment Change!\n");
            itr_segment_number = itr_segment_number + 1;
            seg_start_addr_curr = seg_start_addr;
        }
        
        Message("%d, %s, %d, 0x%08X, %s\n", itr_function_number, function_name, itr_segment_number, seg_start_addr, segment_name);
        
        Message("_%02d%02d_%s\n", itr_segment_number, itr_function_number, function_name);
        
        underscore_position_first = strstr(function_name, '_');
        Message("underscore_position_first: %d\n", underscore_position_first);
        // if (underscore_position_first == -1) { Message("No underscores found in Function Name\n");
        if (underscore_position_first != 0)
        {
            Message("DEBUG: ERROR: Function Name is missing underscore prefix!\n");
        }
        
    }
        
}
