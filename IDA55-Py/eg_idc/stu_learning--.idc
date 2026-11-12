// J:\STU\WD\IDA55\idc\stu_learning.idc

/*
NOTES:
global variables are not supported
no initialization allowed in declarations
+= is not supported
string concatenation: "Hello" + "World" yields "HelloWorld"
Message() ~= printf()
form() ~= sprintf
function parameters are strictly call-by-value (i.e., no call-by-reference, no pointers)

"ea" ?Effective Address? ... "Linear Address", "Virtual Address"

*/

#include <idc.idc>
// #include <stu_seg_info.idc>
// #include <stu_proc_info.idc>

// J:\STU\WD\IDA55\idc\idc.idc
// #define BADADDR         -1                 // Not allowed address value
// #define BADSEL          -1                 // Not allowed selector value/number
// #define MinEA()                 GetLongPrm(INF_MIN_EA)
// #define MaxEA()                 GetLongPrm(INF_MAX_EA)
// #define BeginEA()               GetLongPrm(INF_BEGIN_EA)


static hello_world(void)
{
    Message("hello, world\n");
}

static main(void)
{
    // Message("hello, world\n");
    // hello_world();
    
    /*
    ea_begin = BeginEA();
    ea_min = MinEA();
    ea_max = MaxEA();
    ea_first_seg = FirstSeg();
    Message("ea_begin: %08X\n", ea_begin);
    Message("ea_min:   %08X\n", ea_min);
    Message("ea_max:   %08X\n", ea_max);
    Message("ea_first_seg:   %08X\n", ea_first_seg);
    */
    
    

}


