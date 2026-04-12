
// Segment Type

#define SEG_NORM        0
#define SEG_XTRN        1       // * segment with 'extern' definitions  //   no instructions are allowed
#define SEG_CODE        2       // pure code segment
#define SEG_DATA        3       // pure data segment
#define SEG_IMP         4       // implementation segment
#define SEG_GRP         6       // * group of segments  //   no instructions are allowed
#define SEG_NULL        7       // zero-length segment
#define SEG_UNDF        8       // undefined segment type
#define SEG_BSS         9       // uninitialized segment
#define SEG_ABSSYM     10       // * segment with definitions of absolute symbols  //   no instructions are allowed
#define SEG_COMM       11       // * segment with communal definitions  //   no instructions are allowed
#define SEG_IMEM       12       // internal processor memory & sfr (8051)


// Get Type Name From Type

string get_segment_type_name(long type)
{
	auto segment_type_name;
	
	if(type == SEG_NORM) { segment_type_name = "SEG_NORM"; }
	if(type == SEG_XTRN) { segment_type_name = "SEG_XTRN"; }
	if(type == SEG_CODE) { segment_type_name = "SEG_CODE"; }
	if(type == SEG_DATA) { segment_type_name = "SEG_DATA"; }
	
	
	
	return segment_type_name;
}
