// stu_func_rename.idc
// J:\STU\WD\IDA55\idc\stu_func_rename.idc

#include <idc.idc>

#ifndef TRUE
#define TRUE 0
#endif
#ifndef FALSE
#define FALSE 1
#endif

#define SUCCESS_MakeNameEx 1
#define FAILURE_MakeNameEx 0

#define crlf()  Message("\n");


/*

Renames all function to the format /best/ suited for reassembly in TASM/MASM

Function Name Format:
_f040102_DOS_FindFile




How to get, store, and assign the 'function total count'?
...IDC does not have arrays...
index a string by segment number?
two loops...
get 'function total count' and assign to 'array-like string'
element size...
...string length of maximum function total count?

Per Segment, function_count_total
Per IDB, function_count_total_maximum
.:. need to track segment change


// DEBUG: INFO: FINAL: function_count_total_maximum: 180 [seg000]
// DEBUG: INFO: FINAL: function_count_total_maximum: 107 [_AIL_]

.:. _fsssnnnmmm



*/


static main(void)
{
    stu_func_rename_todo();
    // stu_func_rename();
}

static stu_func_rename_todo(void)
{
        underscore_position_first = strstr(function_name, '_');
        Message("underscore_position_first: %d\n", underscore_position_first);
        // if (underscore_position_first == -1) { Message("No underscores found in Function Name\n");
        if (underscore_position_first != 0)
        {
            Message("DEBUG: ERROR: Function Name is missing underscore prefix!\n");
        }
        
        function_name_new = form("_f%02d%02d_%s", itr_segment_number, itr_function_number, function_name);
        
        Message("OLD: %s, NEW: %s\n", function_name, function_name_new);
        
        // success MakeNameEx(long ea, string name, long flags);  // returns: 1-ok, 0-failure

}

static stu_func_rename(void)
{
    auto ea_first_seg;
    
    auto itr_ea_func_next;
    
    auto seg_start_addr;
    auto seg_start_addr_current;
    
    auto itr_segment_number;
    auto segment_number;
    auto segment_name;
    
    auto itr_function_number;
    auto function_number;
    auto function_name;
    
    auto skip_segment_flag;
    
    auto segment_function_count_total;
    auto segment_function_count_total_maximum;
    auto segment_function_count_total_maximum_segment_name;
    auto segment_function_count_total_maximum_string;
    auto segment_function_count_total_maximum_string_length;
    
    auto string_array_element_size;
    
    
    ea_first_seg = FirstSeg();
    
    
    // -------------------------------------------------------------
    //  Get Segment Function Count Total Maximum
    //      for String-Array Element Size
    // -------------------------------------------------------------
    
    
    for(itr_ea_func_next = ea_first_seg; itr_ea_func_next != BADADDR; itr_ea_func_next = NextFunction(itr_ea_func_next))
    {

        seg_start_addr = GetSegmentAttr(itr_ea_func_next, SEGATTR_START);

        // End previous segment, Start next segment
        if (seg_start_addr_current != seg_start_addr)
        {
            // End previous segment
            // Message("DEBUG: segment_function_count_total: %d\n", segment_function_count_total);
            // DEL segment_count_total_string_form = form("%02d", segment_count_total_string);
            // DEL segment_count_total_array_string_form = segment_count_total_array_string_form + segment_count_total_string_form;
            
            // Start next segment
            itr_segment_number = itr_segment_number + 1;
            seg_start_addr_current = seg_start_addr;
            // Reset 'Per Segment' parameters / options
            itr_function_number = 1;
            segment_function_count_total = 0;
            skip_segment_flag = 0;
            // DEL Message("DEBUG: INFO: Segment Change!\n");
        }
        
        segment_number = itr_segment_number;

        segment_name = SegName(itr_ea_func_next);
        // if(segment_name == 0) { Message("no segment at the specified address\n"); }

        function_number = itr_function_number;

        function_name = GetFunctionName(itr_ea_func_next);
        // if(function_name == 0) { Message("no function at the specified address\n"); }

        
        if (segment_function_count_total < itr_function_number)
        {
            segment_function_count_total = itr_function_number;
            // DEL Message("DEBUG: segment_function_count_total: %d\n", segment_function_count_total);
        }
        
        
        
        //  seg000  AKA CRTL
        if (seg_start_addr == 0x10000)
        {
            if (skip_segment_flag == 0)
            {
                // Message("DEBUG: WARN: Skipping Segment: %d, %s\n", segment_number, segment_name);
            }
            skip_segment_flag = 1;
        }
        //  seg???  AKA _AIL_
        if (seg_start_addr == 0x2E352)
        {
            if (skip_segment_flag == 0)
            {
                // Message("DEBUG: WARN: Skipping Segment: %d, %s\n", segment_number, segment_name);
            }
            skip_segment_flag = 1;
        }
        //  seg???  AKA SOUND
        if (seg_start_addr == 0x2FC56)
        {
            if (skip_segment_flag == 0)
            {
                // Message("DEBUG: WARN: Skipping Segment: %d, %s\n", segment_number, segment_name);
            }
            skip_segment_flag = 1;
        }
        //  seg???  _OVRTEXT_
        if (seg_start_addr == 0x30B77)
        {
            if (skip_segment_flag == 0)
            {
                // Message("DEBUG: WARN: Skipping Segment: %d, %s\n", segment_number, segment_name);
            }
            skip_segment_flag = 1;
        }
        //  seg???  _OVRDATA_
        if (seg_start_addr == 0x314C0)
        {
            if (skip_segment_flag == 0)
            {
                // Message("DEBUG: WARN: Skipping Segment: %d, %s\n", segment_number, segment_name);
            }
            skip_segment_flag = 1;
        }
        //  seg???  _STUB_
        if (seg_start_addr == 0x31560)
        {
            if (skip_segment_flag == 0)
            {
                // Message("DEBUG: WARN: Skipping Segment: %d, %s\n", segment_number, segment_name);
            }
            skip_segment_flag = 1;
        }
        //  seg???  _EXEINFO_
        if (seg_start_addr == 0x31580)
        {
            if (skip_segment_flag == 0)
            {
                // Message("DEBUG: WARN: Skipping Segment: %d, %s\n", segment_number, segment_name);
            }
            skip_segment_flag = 1;
        }
        
        if (skip_segment_flag != 1)
        {
            // Message("%d, %s, %d, %s\n", function_number, function_name, segment_number, segment_name);

            if (segment_function_count_total_maximum < segment_function_count_total)
            {
                segment_function_count_total_maximum = segment_function_count_total;
                // DEL Message("DEBUG: INFO: NEW: segment_function_count_total_maximum: %d\n", segment_function_count_total_maximum);
                segment_function_count_total_maximum_segment_name = segment_name;
            }
        }    
        
        itr_function_number = itr_function_number + 1;
        
    }
    
    // Message("DEBUG: INFO: FINAL: segment_function_count_total_maximum: %d\n", segment_function_count_total_maximum);
    // Message("DEBUG: INFO: FINAL: segment_function_count_total_maximum_segment_name: %s\n", segment_function_count_total_maximum_segment_name);
    // // DEBUG: INFO: FINAL: segment_function_count_total_maximum: 85
    // // DEBUG: INFO: FINAL: segment_function_count_total_maximum_segment_name: seg034

    segment_function_count_total_maximum_string = ltoa(segment_function_count_total_maximum, 10);   
    segment_function_count_total_maximum_string_length = strlen(segment_function_count_total_maximum_string);
    // Message("segment_function_count_total_maximum_string: %s\n", segment_function_count_total_maximum_string);
    // Message("segment_function_count_total_maximum_string_length: %d\n", segment_function_count_total_maximum_string_length);
    // // segment_function_count_total_maximum_string: 85
    // // segment_function_count_total_maximum_string_length: 2
    
    string_array_element_size = segment_function_count_total_maximum_string_length;
    
    Message("string_array_element_size: %d\n", string_array_element_size);
    
    
    // ------------------------------ //
    //  Build String-Array
    //      of Function Count (total)
    //      per Segment
    // ------------------------------ //
    
    for(itr_ea_func_next = ea_first_seg; itr_ea_func_next != BADADDR; itr_ea_func_next = NextFunction(itr_ea_func_next))
    {

        seg_start_addr = GetSegmentAttr(itr_ea_func_next, SEGATTR_START);

        // End previous segment, Start next segment
        if (seg_start_addr_curr != seg_start_addr)
        {
            // End previous segment
            Message("DEBUG: segment_function_count_total: %d\n", segment_function_count_total);
            segment_count_total_string_form = form("%02d", segment_count_total_string);
            segment_count_total_array_string_form = segment_count_total_array_string_form + segment_count_total_string_form;
            
            // Start next segment
            itr_segment_number = itr_segment_number + 1;
            seg_start_addr_curr = seg_start_addr;
            // Reset 'Per Segment' parameters / options
            itr_segment_function_number = 1;
            segment_function_count_total = 0;
            skip_segment_flag = 0;
            Message("DEBUG: INFO: Segment Change!\n");
        }
        
        segment_number = itr_segment_number;

        segment_name = SegName(itr_ea_func_next);
        // if(segment_name == 0) { Message("no segment at the specified address\n"); }

        function_number = itr_segment_function_number;

        function_name = GetFunctionName(itr_ea_func_next);
        // if(function_name == 0) { Message("no function at the specified address\n"); }

        //Message("%d, %s, %d, %s\n", function_number, function_name, segment_number, segment_name);

        
        if (segment_function_count_total < itr_segment_function_number)
        {
            segment_function_count_total = itr_segment_function_number;
            //Message("DEBUG: segment_function_count_total: %d\n", segment_function_count_total);
        }
        
        
        
        //  seg000  AKA CRTL
        if (seg_start_addr == 0x10000)
        {
            if (skip_segment_flag == 0)
            {
                Message("DEBUG: WARN: Skipping Segment: %d, %s\n", segment_number, segment_name);
            }
            skip_segment_flag = 1;
        }
        //  seg???  AKA _AIL_
        if (seg_start_addr == 0x2E352)
        {
            if (skip_segment_flag == 0)
            {
                Message("DEBUG: WARN: Skipping Segment: %d, %s\n", segment_number, segment_name);
            }
            skip_segment_flag = 1;
        }
        //  seg???  AKA SOUND
        if (seg_start_addr == 0x2FC56)
        {
            if (skip_segment_flag == 0)
            {
                Message("DEBUG: WARN: Skipping Segment: %d, %s\n", segment_number, segment_name);
            }
            skip_segment_flag = 1;
        }
        //  seg???  _OVRTEXT_
        if (seg_start_addr == 0x30B77)
        {
            if (skip_segment_flag == 0)
            {
                Message("DEBUG: WARN: Skipping Segment: %d, %s\n", segment_number, segment_name);
            }
            skip_segment_flag = 1;
        }
        //  seg???  _OVRDATA_
        if (seg_start_addr == 0x314C0)
        {
            if (skip_segment_flag == 0)
            {
                Message("DEBUG: WARN: Skipping Segment: %d, %s\n", segment_number, segment_name);
            }
            skip_segment_flag = 1;
        }
        //  seg???  _STUB_
        if (seg_start_addr == 0x31560)
        {
            if (skip_segment_flag == 0)
            {
                Message("DEBUG: WARN: Skipping Segment: %d, %s\n", segment_number, segment_name);
            }
            skip_segment_flag = 1;
        }
        //  seg???  _EXEINFO_
        if (seg_start_addr == 0x31580)
        {
            if (skip_segment_flag == 0)
            {
                Message("DEBUG: WARN: Skipping Segment: %d, %s\n", segment_number, segment_name);
            }
            skip_segment_flag = 1;
        }
        // //  stub048
        // if (seg_start_addr == 0x317E0)
        // {
            // if (skip_segment_flag == 0)
            // {
                // Message("DEBUG: WARN: Skipping Segment: %d, %s\n", segment_number, segment_name);
            // }
            // skip_segment_flag = 1;
        // }

                
        if (skip_segment_flag != 1)
        {
            Message("%d, %s, %d, %s\n", function_number, function_name, segment_number, segment_name);
            
            if (function_count_total_maximum < segment_function_count_total)
            {
                function_count_total_maximum = segment_function_count_total;
                Message("DEBUG: INFO: NEW: function_count_total_maximum: %d\n", function_count_total_maximum);
                function_count_total_maximum_segment_name = segment_name;
            }
        }    
        
        itr_segment_function_number = itr_segment_function_number + 1;
        
    }
    
    
    /*
    auto ea_begin;
    auto ea_min;
    auto ea_max;
    auto ea_first_seg;

    auto ea_proc_first;
    auto ea_proc_next;
    auto proc_name_first;
    auto proc_name_next;

    auto itr_segment_function_number;
    auto function_number;
    auto itr_ea_func_next;
    auto function_name;
    
    auto itr_segment_number;
    auto segment_number;
    auto segment_name;
    auto seg_start_addr_curr;
    auto seg_start_addr;
    auto seg_end_addr;
    
    auto underscore_position_first;
    
    auto function_name_new;
    
    auto segment_function_count_total;
    auto function_count_total_maximum;
    auto function_count_total_maximum_current;
    auto function_count_total_maximum_segment_name;
    
    auto skip_segment_flag;
    
    
    auto function_count_total_maximum_string;
    auto function_count_total_maximum_string_length;

    auto segment_count_total_array_string;
    auto segment_count_total_array_string_length;
    auto segment_count_total_array_string_form;
    auto segment_count_total_array_string_form_length;
    auto segment_count_total_string;
    auto segment_count_total_string_form;
    auto array_string_element_size;
    auto seg004__segment_count_total;
    auto seg004__segment_count_total_number;
    
    segment_count_total_array_string = "";
    segment_count_total_array_string_form = "";
    array_string_element_size = 2;

    
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

    itr_segment_number = 0;
    itr_segment_function_number = 1;
    seg_start_addr = BADADDR;
    seg_start_addr_curr = BADADDR;
    segment_function_count_total = 0;
    function_count_total_maximum = 0;
    function_count_total_maximum_current = 0;
    function_count_total_maximum_string_length = 0;
    skip_segment_flag = 0;
    //ea_min = 0x55CA0;  // DEBUG: 3rd to last segment, 5th to last function; i.e., start at the last function in the 65th segment
    // Segemnts seg001 through seg007; NOTE: 0x14787 is actually the start address of seg008
    //for(itr_ea_func_next = 0x13209; itr_ea_func_next < 0x14787; itr_ea_func_next = NextFunction(itr_ea_func_next))
    ea_min = 0x13209;  // seg001 Start address
    for(itr_ea_func_next = ea_min; itr_ea_func_next != BADADDR; itr_ea_func_next = NextFunction(itr_ea_func_next))
    {

        seg_start_addr = GetSegmentAttr(itr_ea_func_next, SEGATTR_START);   // starting address

        // Start /new/ segment on segment change; not sure on the logic of handling the first/initial segment - seems like I ought to just be using FirstSeg()
        if (seg_start_addr_curr != seg_start_addr)
        {
            Message("DEBUG: segment_function_count_total: %d\n", segment_function_count_total);
            segment_count_total_string = ltoa(segment_function_count_total, 10);
            segment_count_total_array_string = segment_count_total_array_string + segment_count_total_string;
            segment_count_total_string_form = form("%02d", segment_count_total_string);
            segment_count_total_array_string_form = segment_count_total_array_string_form + segment_count_total_string_form;
            itr_segment_number = itr_segment_number + 1;
            seg_start_addr_curr = seg_start_addr;
            // Reset 'Per Segment' parameters / options
            itr_segment_function_number = 1;
            segment_function_count_total = 0;
            skip_segment_flag = 0;
            Message("DEBUG: INFO: Segment Change!\n");
        }
        
        segment_number = itr_segment_number;

        segment_name = SegName(itr_ea_func_next);
        // if(segment_name == 0) { Message("no segment at the specified address\n"); }

        function_number = itr_segment_function_number;

        function_name = GetFunctionName(itr_ea_func_next);
        // if(function_name == 0) { Message("no function at the specified address\n"); }

        //Message("%d, %s, %d, %s\n", function_number, function_name, segment_number, segment_name);

        
        if (segment_function_count_total < itr_segment_function_number)
        {
            segment_function_count_total = itr_segment_function_number;
            //Message("DEBUG: segment_function_count_total: %d\n", segment_function_count_total);
        }
        
        
        
        //  seg000  AKA CRTL
        if (seg_start_addr == 0x10000)
        {
            if (skip_segment_flag == 0)
            {
                Message("DEBUG: WARN: Skipping Segment: %d, %s\n", segment_number, segment_name);
            }
            skip_segment_flag = 1;
        }
        //  seg???  AKA _AIL_
        if (seg_start_addr == 0x2E352)
        {
            if (skip_segment_flag == 0)
            {
                Message("DEBUG: WARN: Skipping Segment: %d, %s\n", segment_number, segment_name);
            }
            skip_segment_flag = 1;
        }
        //  seg???  AKA SOUND
        if (seg_start_addr == 0x2FC56)
        {
            if (skip_segment_flag == 0)
            {
                Message("DEBUG: WARN: Skipping Segment: %d, %s\n", segment_number, segment_name);
            }
            skip_segment_flag = 1;
        }
        //  seg???  _OVRTEXT_
        if (seg_start_addr == 0x30B77)
        {
            if (skip_segment_flag == 0)
            {
                Message("DEBUG: WARN: Skipping Segment: %d, %s\n", segment_number, segment_name);
            }
            skip_segment_flag = 1;
        }
        //  seg???  _OVRDATA_
        if (seg_start_addr == 0x314C0)
        {
            if (skip_segment_flag == 0)
            {
                Message("DEBUG: WARN: Skipping Segment: %d, %s\n", segment_number, segment_name);
            }
            skip_segment_flag = 1;
        }
        //  seg???  _STUB_
        if (seg_start_addr == 0x31560)
        {
            if (skip_segment_flag == 0)
            {
                Message("DEBUG: WARN: Skipping Segment: %d, %s\n", segment_number, segment_name);
            }
            skip_segment_flag = 1;
        }
        //  seg???  _EXEINFO_
        if (seg_start_addr == 0x31580)
        {
            if (skip_segment_flag == 0)
            {
                Message("DEBUG: WARN: Skipping Segment: %d, %s\n", segment_number, segment_name);
            }
            skip_segment_flag = 1;
        }
        // //  stub048
        // if (seg_start_addr == 0x317E0)
        // {
            // if (skip_segment_flag == 0)
            // {
                // Message("DEBUG: WARN: Skipping Segment: %d, %s\n", segment_number, segment_name);
            // }
            // skip_segment_flag = 1;
        // }

                
        if (skip_segment_flag != 1)
        {
            Message("%d, %s, %d, %s\n", function_number, function_name, segment_number, segment_name);
            
            if (function_count_total_maximum < segment_function_count_total)
            {
                function_count_total_maximum = segment_function_count_total;
                Message("DEBUG: INFO: NEW: function_count_total_maximum: %d\n", function_count_total_maximum);
                function_count_total_maximum_segment_name = segment_name;
            }
        }
        //else
        //{
        //    skip_segment_flag = 0;
        //}
        
        
        itr_segment_function_number = itr_segment_function_number + 1;
        
    }

    Message("DEBUG: INFO: FINAL: function_count_total_maximum: %d\n", function_count_total_maximum);
    Message("DEBUG: INFO: FINAL: function_count_total_maximum_segment_name: %s\n", function_count_total_maximum_segment_name);
    // DEBUG: INFO: FINAL: function_count_total_maximum: 85
    // DEBUG: INFO: FINAL: function_count_total_maximum_segment_name: seg034



    // /array/ of segment function counts
    // nn
    // substr?
    // array element size = 2
    // string  ltoa            (long n,long radix);    // convert to ascii string
    // long    strlen          (string str);           // calculate length
    function_count_total_maximum_string = ltoa(function_count_total_maximum, 10);   
    function_count_total_maximum_string_length = strlen(function_count_total_maximum_string);
    Message("function_count_total_maximum_string: %s\n", function_count_total_maximum_string);
    Message("function_count_total_maximum_string_length: %d\n", function_count_total_maximum_string_length);
    // function_count_total_maximum_string: 85
    // function_count_total_maximum_string_length: 2
    
    // segment_count_total = 65
    // segment_count_total * array_element_size = 65 * 2 = 130
    // 
    crlf();
    Message("segment_count_total_array_string: %s\n", segment_count_total_array_string);
    crlf();
    Message("segment_count_total_array_string_form: %s\n", segment_count_total_array_string_form);
    //segment_count_total_array_string: 095827610197175123216111699141473258312546231964885107123625385412251171632225385412251171632
    //segment_count_total_array_string_form: 00090508020706101907170512321601116909141407320508031205460203190648851071236020538540122050117010603020202053854012205011701060302

    segment_count_total_array_string_length = strlen(segment_count_total_array_string);
    segment_count_total_array_string_form_length = strlen(segment_count_total_array_string_form);
    Message("segment_count_total_array_string_length: %d\n", segment_count_total_array_string_length);
    Message("segment_count_total_array_string_form_length: %d\n", segment_count_total_array_string_form_length);
    
    // seg004: segment_count_total = 2
    seg004__segment_count_total = substr(segment_count_total_array_string_form, (2 * 4), (2 + (2 * 4)));
    Message("seg004__segment_count_total: %s\n", seg004__segment_count_total);
    
    seg004__segment_count_total_number = atol(seg004__segment_count_total);
    Message("seg004__segment_count_total_number: %d\n", seg004__segment_count_total_number);
    */

    /*
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
        
        function_name_new = form("_f%02d%02d_%s", itr_segment_number, itr_function_number, function_name);
        
        Message("OLD: %s, NEW: %s\n", function_name, function_name_new);
        
        // success MakeNameEx(long ea, string name, long flags);  // returns: 1-ok, 0-failure
        
    }
    */
    
}
