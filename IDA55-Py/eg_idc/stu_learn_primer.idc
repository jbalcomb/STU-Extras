#include <idc.idc>

static main(void)
{
    // stu_learn_get_ea();
	stu_learn_itr_sgmt();
}



static stu_learn_get_ea(void)
{
    auto ea_begin;
    auto ea_min;
    auto ea_max;
    auto ea_first_seg;
    
    ea_begin = BeginEA();
    ea_min = MinEA();
    ea_max = MaxEA();
    ea_first_seg = FirstSeg();

    Message("ea_begin: 0x%08X\n", ea_begin);
    Message("ea_min:   0x%08X\n", ea_min);
    Message("ea_max:   0x%08X\n", ea_max);
    Message("ea_first_seg:   0x%08X\n", ea_first_seg);
}

// Compiling file 'C:\STU_WD_IDA55\IDA55\idc\stu_learn_primer.idc'...
// Executing function 'main'...
// ea_begin: 0x00010000
// ea_min:   0x00010000
// ea_max:   0x00056886
// ea_first_seg:   0x00010000



// Get first segment
// long    FirstSeg();  					// returns: linear address of the start of the first segment;  BADADDR - no segments are defined
// Get next segment
// long    NextSeg(long ea);               	// returns: linear address of the start of the next segment;   BADADDR - no next segment
// Get name of a segment
// string  SegName(long ea);  				// returns: 0 - no segment at the specified address

static stu_learn_itr_sgmt(void)
{
    auto ea_first_seg;
	auto ea_next_seg;
	
	auto first_segment_name;
	auto next_segment_name;
	
    ea_first_seg = FirstSeg();

    
	first_segment_name = SegName(ea_first_seg);
	
	Message("ea_first_seg:   0x%08X\n", ea_first_seg);
	Message("first_segment_name: %s\n", first_segment_name);
	
	ea_next_seg = NextSeg(ea_first_seg);
	next_segment_name = SegName(ea_next_seg);
	
	Message("ea_next_seg:   0x%08X\n", ea_next_seg);
	Message("next_segment_name: %s\n", next_segment_name);

}
