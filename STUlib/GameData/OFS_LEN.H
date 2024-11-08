#pragma once

/*

    assert(file_pointer_position == 46900);  / * 0xB734 * /

    fread(_UNITS, 1009, 32, file_pointer);

    assert(file_pointer_position == 79188);  / * 0x13554 * /

    79188 - 46900 = 32288  0x7E20
    1009 * 32 = 32288

*/

#define GAM_OFS_UNITS 0xB734
#define GAM_LEN_UNITS 32288
