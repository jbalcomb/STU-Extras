#include <idc.idc>

/*

// Get next defined item (instruction or data) in the program
//      ea    - linear address to start search from
//      maxea - the search will stop at the address
//              maxea is not included in the search range
// returns: BADADDR - no (more) defined items

long    NextHead        (long ea, long maxea);  // returns next defined item address
                                                // BADADDR if no such address exists

// Get visible name of program byte
// This function returns name of byte as it is displayed on the screen.
// If a name contains illegal characters, IDA replaces them by the substitution
// character during displaying. See IDA.CFG for the definition of the
// substitution character.
//      from - the referring address. may be BADADDR.
//             Allows to retrieve local label addresses in functions.
//             If a local name is not found, then a global name is returned.
//      ea   - linear address
// returns: 0 - byte has no name

string  NameEx          (long from, long ea);   // get visible name of the byte

// Get true name of program byte
// This function returns name of byte as is without any replacements.
//      from - the referring address. may be BADADDR.
//             Allows to retrieve local label addresses in functions.
//             If a local name is not found, then a global name is returned.
//      ea   - linear address
// returns: 0 - byte has no name

string  GetTrueNameEx   (long from, long ea);   // get true name of the byte

*/

static main(void)
{
    stu_foreach_head();
}

static stu_foreach_head(void)
{
    auto ea_begin;
    auto ea_min;
    auto ea_max;
    auto ea_first_seg;
    
    auto next_head;
    auto itr_ea_head_next;
    
    auto head_name_visible;
    auto head_name_true;

    auto file_name;
    auto file_handle;
    auto file_cursor_position;
    
    ea_begin = BeginEA();
    ea_min = MinEA();
    ea_max = MaxEA();
    ea_first_seg = FirstSeg();

    Message("ea_begin: 0x%08X\n", ea_begin);
    Message("ea_min:   0x%08X\n", ea_min);
    Message("ea_max:   0x%08X\n", ea_max);
    Message("ea_first_seg:   0x%08X\n", ea_first_seg);

    // next_head = NextHead(ea_min, ea_max);
    // if (next_head == BADADDR)
        
    Message("\n\n------ Creating the output file.... --------\n");
    
    file_name = GetIdbPath();
    Message("file_name: %s\n", file_name);
    file_name = substr(file_name, 0, strstr(file_name, ".")) + "_heads" + ".csv";
    Message("file_name: %s\n", file_name);
    file_handle = fopen(file_name, "w");
    file_cursor_position = ftell(file_handle);
    Message("file_cursor_position: %d\n", file_cursor_position);
    
    Message("head_addr, head_name_visible, head_name_true\n");
    
    writestr(file_handle, "head_addr, head_name_visible, head_name_true\n");
    
    for(itr_ea_head_next = ea_min; itr_ea_head_next != BADADDR; itr_ea_head_next = NextHead(itr_ea_head_next, ea_max))
    {
        // Message("itr_ea_head_next: %08X\n", itr_ea_head_next);

        head_name_visible = NameEx(0, itr_ea_head_next);
        head_name_true = GetTrueNameEx(0, itr_ea_head_next);
        
        Message("itr_ea_head_next: 0x%08X, head_name_visible: %s, head_name_true: %s\n", itr_ea_head_next, head_name_visible, head_name_true);
        
        fprintf(
            file_handle,
            "0x%08X, %s, %s\n", 
            itr_ea_head_next, head_name_visible, head_name_true
        );
        
    }

    file_cursor_position = ftell(file_handle);
    Message("file_cursor_position: %d\n", file_cursor_position);
    
    fclose(file_handle);
    
}
