
#ifndef FC_ARRAY_H
#define FC_ARRAY_H


/*
    Arrays for C

    array
    array elements
    array size
    element size
    element count


    Data Type
    Data Type Size
    integer
    32-bit integer

    Representation
    Machine
    Human


*/

/*
    operate on arbitrary regions of storage

    functions that work on arbitrary arrays or blocks of memory, and functions that are specific to null-terminated arrays of characters and wide characters


*/

/*
    
    searching techniques
    linear and binary
    
    linear search algorithm

    Step by step descriptive logic to search element in array using linear search algorithm.


*/

/*

    bail if length is 0

    unique_array
        first element of array is never in unique_array


*/


//#include <stdint.h>
//typedef uint8_t byte;
//typedef uint8_t* byte_ptr;
// incl C++ in C - No Bueno!!  #include "STUlib.hpp"
#include <stdint.h>


typedef uint8_t FC_STATUS;


struct s_ARRAY
{
    uint8_t * elements;
    uint64_t element_count;
    uint64_t element_size;
    uint64_t array_size;
};
typedef struct s_ARRAY* array_ptr;



#define FC_FALSE    0
#define FC_TRUE     1

#define FC_NULL     0
#define FC_NULL_PTR (void *)



#ifdef __cplusplus
extern "C" {
#endif
    /* C and Assembly Function Prototypes */


void print_array(unsigned char * array, uint64_t length);
void print_strings(char* strings[], uint64_t count);

void print_array_uint16(void const* array, uint64_t length);


void unique_array_uint8(void const* array, uint64_t array_length, uint8_t** unique_array, uint64_t* unique_array_length);

// void unique_array_uint16(unsigned char * array, uint64_t element_count, uint64_t element_size);
void unique_array_uint16(void const* array, uint64_t array_length, uint16_t ** unique_array, uint64_t * unique_array_length);

void Bubble_Sort_uint8(uint8_t array[], uint64_t length);

void Bubble_Sort_uint16(uint16_t array[], uint64_t length);


void sortset_uint8(void const* array, uint64_t array_length, uint8_t** unique_array, uint64_t* unique_array_length);


#ifdef __cplusplus
}
#endif



#endif FC_ARRAY_H
