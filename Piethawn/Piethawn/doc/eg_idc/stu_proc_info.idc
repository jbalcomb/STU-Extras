// stu_proc_info.idc
// J:\STU\WD\IDA55\idc\stu_proc_info.idc

#include <idc.idc>


static main(void)
{
    stu_proc_info();
}


static stu_proc_info(void)
{
    auto ea_begin;
    auto ea_min;
    auto ea_max;
    auto ea_seg_first;

    auto ea_proc_first;
    auto ea_proc_next;
    auto proc_name_first;
    auto proc_name_next;
    
    auto itr_ea_proc_next;
    auto proc_name;


    ea_begin = BeginEA();
    ea_min = MinEA();
    ea_max = MaxEA();
    ea_seg_first = FirstSeg();                  // if(ea_seg_first == BADADDR) {  }
    
    Message("ea_begin: 0x%08X\n", ea_begin);
    Message("ea_min:   0x%08X\n", ea_min);
    Message("ea_max:   0x%08X\n", ea_max);
    Message("ea_seg_first: %08X\n", ea_seg_first);
    
    ea_proc_first = MinEA();
    proc_name_first = GetFunctionName(ea_min);
    Message("proc_name_first: %s\n", proc_name_first);
    ea_proc_next = NextFunction(ea_min);
    Message("ea_proc_next: 0x%08X\n", ea_proc_next);
    proc_name_next = GetFunctionName(ea_proc_next);
    Message("proc_name_next: %s\n", proc_name_next);
    
    // GetFunctionAttr(addr, FUNCATTR_END);
    
    for(itr_ea_proc_next = MinEA(); itr_ea_proc_next != BADADDR; itr_ea_proc_next = NextFunction(itr_ea_proc_next))
    {
        Message("itr_ea_proc_next: %08X\n", itr_ea_proc_next);

        proc_name = GetFunctionName(itr_ea_proc_next);
        // if(proc_name == 0) { Message("no procedure at the specified address\n"); }
        
        Message("proc_name: %s\n", proc_name);
    }
    
    // View -> Open subviews -> Functions ("Functions window")
    // Function name, Segment, Start, Length, R, F, L, S, B, T, =
    
    
}
