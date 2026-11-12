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
