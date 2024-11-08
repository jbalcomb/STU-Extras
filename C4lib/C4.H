/*
    C4 - ...for C  ("Forcie")

Array
Matrix
Collection
Set
Sorted-Set

Per Data-Type
width / size
signless, unsigned, signed


HOW-TO: Sort An Array
sort algorithm
comparison function
sort order
stability
array, element, index, value


{0, 1, 255}
vs.
{-1, 0, 1}


1, 2, 4, 8 byte values  (DB, DW, DD, DQ)

Because Intel, a byte is 8 bits, by definition
A "byte" is not a number  (and, therefore, has no "sign")

Because Intel, multi-byte values are stored in LSB byte-order
    2-byte value:  80F1     1000000011110001
    in-memory:     [F1,80]  [11110001,10000000]

order is preserved
order is guaranteed

"sorted" / "ordered"
"unique" / "distinct"


short-circuit
branchless
alignment
(micro-)optimizations
if bit 0 is 0, you can treat it as an even number
2, 4, 8 bytes at a time
if bit 0 is 0, it is divisible by two
if bits 0,1 are 0, it is divisible by four
if bits 0,1,2 are 0, it is divisible by eight
For unique, array is already sorted?
For sorted, array is already unique?

Issue for C with casting?
When does getting sign-extended happen implicitly?

When does a Character Array gets treated as a C-String?


Data-Type Awareness:
src array can be passed in as (void *)
malloc() MUST HAVE (complete) data-type

*/
#ifndef C4_H
#define C4_H

#include <stdint.h>

enum DataType
{
    dt_1B,
    dt_2B,
    dt_4B,
    dt_8B,
    dt_Signed_Number_OneByte,
    dt_Signed_Number_TwoByte,
    dt_Signed_Number_FourByte,
    dt_Signed_Number_EightByte
};



#ifdef __cplusplus
extern "C" {
#endif



    void unique_array_uint8(void const* array, uint64_t array_length, uint8_t** unique_array, uint64_t* unique_array_length);
    void unique_array_uint16(void const* array, uint64_t array_length, uint16_t** unique_array, uint64_t* unique_array_length);
    void Bubble_Sort_uint8(uint8_t array[], uint64_t length);
    void Bubble_Sort_uint16(uint16_t array[], uint64_t length);

    void sortset_uint8(void const* array, uint64_t array_length, uint8_t** unique_array, uint64_t* unique_array_length);



#ifdef __cplusplus
}
#endif



#endif C4_H
