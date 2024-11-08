/*
	4C
	Array (Library)

	¿ Per Data-Type ?
	¿ Signed / Unsigned ?


*/

#include "FC_Array.H"

#include <malloc.h>
#include <stdio.h>



void print_array(unsigned char * array, uint64_t length)
{
	uint64_t itr_array;  // 'unsigned __int64' ? !~== 'int' ?

	for (itr_array = 0; itr_array < length; itr_array++)
	{
		printf("array[%llu]: 0x%02X\n", itr_array, array[itr_array]);
	}

}

void print_strings(char * strings[], uint64_t count)
{
	uint64_t itr_strings;

	for (itr_strings = 0; itr_strings < count; itr_strings++)
	{
		printf("strings[%llu]: %s\n", itr_strings, strings[itr_strings]);
	}

}

void print_array_uint16(void const * array, uint64_t length)
{
	uint16_t * array_ptr;
	uint64_t itr_array;

	array_ptr = (uint16_t*)array;

	for (itr_array = 0; itr_array < length; itr_array++)
	{
		printf("array[%llu]: 0x%04X\n", itr_array, array_ptr[itr_array]);
	}

}


/*

*/
// int8_t element_in_array(byte_ptr array, byte_ptr element)
void element_in_array(unsigned char* array, uint64_t element_count, unsigned char element)
{
	int itr_array;
	int element_in_array;

	element_in_array = FC_FALSE;
	for (itr_array = 0; itr_array < element_count; itr_array++)
	{
		if (array[itr_array] == element)
		{
			element_in_array = FC_TRUE;
		}
	}

}

void unique_array(unsigned char* array, uint64_t element_count, uint64_t element_size)
{
	unsigned char* unique_array;
	int itr_element_count;
	unsigned char element;
	int unique_array_idx;
	int itr_unique_array;
	int element_in_array;

	unique_array = (unsigned char*)malloc(element_count * element_size);
	unique_array_idx = 0;
	element_in_array = FC_FALSE;

	for (itr_element_count = 0; itr_element_count < element_count; itr_element_count++)
	{
		element = array[itr_element_count];

		// if element not in unique_array, add element to unique_array
		element_in_array = FC_FALSE;
		for (itr_unique_array = 0; itr_unique_array < unique_array_idx; itr_unique_array++)
		{
			if (unique_array[itr_unique_array] == element)
			{
				element_in_array = FC_TRUE;
			}
		}
		if (element_in_array == FC_FALSE)
		{
			unique_array[unique_array_idx] = element;
		}

	}
}


void unique_array_uint8(void const* array, uint64_t array_length, uint8_t** unique_array, uint64_t* unique_array_length)
{

	uint8_t* array_ptr;
	// uint16_t* unique_array;
	uint8_t* unique_array_ptr;
	uint64_t itr_array;
	uint64_t itr_unique_array;
	uint64_t unique_array_idx;
	uint8_t element_in_array;


	array_ptr = (uint8_t*)array;

	unique_array_ptr = (uint8_t*)malloc(array_length * sizeof(uint8_t));
	if (NULL == unique_array_ptr)
	{
		// FAILURE: malloc()
		// return FC_FAILURE
	}

	unique_array_idx = 0;

	// unique_array[0] = array_ptr[0];
	for (itr_array = 0; itr_array < array_length; itr_array++)
	{
		// if element not in unique_array, add element to unique_array
		element_in_array = FC_FALSE;
		for (itr_unique_array = 0; itr_unique_array < unique_array_idx; itr_unique_array++)
		{
			if (*(unique_array_ptr + itr_unique_array) == *(array_ptr + itr_array))
			{
				element_in_array = FC_TRUE;
			}
		}
		if (element_in_array == FC_FALSE)
		{
			*(unique_array_ptr + unique_array_idx) = *(array_ptr + itr_array);
			unique_array_idx++;
		}
	}
	*unique_array = unique_array_ptr;
	*unique_array_length = unique_array_idx;
}


void unique_array_uint16(void const* array, uint64_t array_length, uint16_t ** unique_array, uint64_t * unique_array_length)
{

	uint16_t* array_ptr;
	// uint16_t* unique_array;
	uint16_t* unique_array_ptr;
	uint64_t itr_array;
	uint64_t itr_unique_array;
	uint64_t unique_array_idx;
	uint8_t element_in_array;


	array_ptr = (uint16_t*)array;

	unique_array_ptr = (uint16_t *)malloc(array_length * sizeof(uint16_t));
	if (NULL == unique_array_ptr)
	{
		// FAILURE: malloc()
		// return FC_FAILURE
	}

	unique_array_idx = 0;

	// unique_array[0] = array_ptr[0];
	for (itr_array = 0; itr_array < array_length; itr_array++)
	{
		// if element not in unique_array, add element to unique_array
		element_in_array = FC_FALSE;
		for (itr_unique_array = 0; itr_unique_array < unique_array_idx; itr_unique_array++)
		{
			if (*(unique_array_ptr + itr_unique_array) == *(array_ptr + itr_array))
			{
				element_in_array = FC_TRUE;
			}
		}
		if (element_in_array == FC_FALSE)
		{
			*(unique_array_ptr + unique_array_idx) = *(array_ptr + itr_array);
			unique_array_idx++;
		}
	}
	*unique_array = unique_array_ptr;
	*unique_array_length = unique_array_idx;
}



void sorted_unique_array_uint8(void const* array, uint64_t array_length, uint8_t** sorted_array)
{

	uint8_t* array_ptr;
	uint8_t* sorted_array_ptr;
	uint64_t itr_array;
	// uint64_t itr_sorted_array;
	uint8_t array_is_sorted;
	uint64_t array_idx;
	// uint64_t sorted_array_idx;
	uint16_t minimum_array_element;


	array_ptr = (uint8_t*)array;

	sorted_array_ptr = (uint8_t*)malloc(array_length * sizeof(uint8_t));

	if (NULL == sorted_array_ptr)
	{
		// FAILURE: malloc()
		// return FC_FAILURE
	}

	array_is_sorted = FC_FALSE;
	while (array_is_sorted != FC_TRUE)
	{
		array_idx = 0;
		minimum_array_element = *(array_ptr + array_idx);

		for (itr_array = 0; itr_array < array_length; itr_array++)
		{
			if (*(array_ptr + itr_array) < minimum_array_element)
			{
				minimum_array_element = *(array_ptr + itr_array);
			}
		}
	}

}


void sorted_unique_array_uint16(void const* array, uint64_t array_length, uint16_t** sorted_array)
{

	uint16_t* array_ptr;
	uint16_t* sorted_array_ptr;
	uint64_t itr_array;
	// uint64_t itr_sorted_array;
	uint8_t array_is_sorted;
	uint64_t array_idx;
	// uint64_t sorted_array_idx;
	uint16_t minimum_array_element;


	array_ptr = (uint16_t*)array;

	sorted_array_ptr = (uint16_t*)malloc(array_length * sizeof(uint16_t));

	if (NULL == sorted_array_ptr)
	{
		// FAILURE: malloc()
		// return FC_FAILURE
	}

	array_is_sorted = FC_FALSE;
	while (array_is_sorted != FC_TRUE)
	{
		array_idx = 0;
		minimum_array_element = *(array_ptr + array_idx);

		for (itr_array = 0; itr_array < array_length; itr_array++)
		{
			if (*(array_ptr + itr_array) < minimum_array_element)
			{
				minimum_array_element = *(array_ptr + itr_array);
			}
		}
	}

}

void Bubble_Sort(int * array, int length)
{
	int temp;
	int swap_flag;
	swap_flag = 0;  /* FALSE */
	for (int i = 0; i < length - 1; ++i)
	{
		for (int x = 0; x < length - i - 1; ++x)
		{
			if (array[x] > array[x + 1])
			{
				temp = array[x];
				array[x] = array[x + 1];
				array[x + 1] = temp;
				swap_flag = 1;  /* TRUE */
			}
		}
		if (swap_flag == 0)
		{
			break;
		}
	}
}


void Bubble_Sort_uint8(uint8_t array[], uint64_t length)
{
	uint64_t itr_array;
	uint64_t itr_compare;
	uint8_t swap_holder;
	uint8_t swap_flag;
	uint64_t swap_count;

	swap_flag = 0;  /* FALSE */
	swap_count = 0;

	for (itr_array = 0; itr_array < length - 1; itr_array++)
	{
		for (itr_compare = 0; itr_compare < length - itr_array - 1; itr_compare++)
		{
			if (array[itr_compare] > array[itr_compare + 1])
			{
				swap_holder = array[itr_compare];
				array[itr_compare] = array[itr_compare + 1];
				array[itr_compare + 1] = swap_holder;
				swap_flag = 1;  /* TRUE */
				swap_count++;
				// printf("DEBUG: swapped: array[%llu] > array[%llu]\n", itr_compare, itr_compare + 1);
			}
		}

		if (swap_flag == 0)
		{
			break;
		}
	}

	// printf("swap_count: %llu\n", swap_count);
}


void Bubble_Sort_uint16(uint16_t array[], uint64_t length)
{
	uint64_t itr_array;
	uint64_t itr_compare;
	uint16_t swap_holder;
	uint8_t swap_flag;
	uint64_t swap_count;

	swap_flag = 0;  /* FALSE */
	swap_count = 0;

	for (itr_array = 0; itr_array < length - 1; itr_array++)
	{
		for (itr_compare = 0; itr_compare < length - itr_array - 1; itr_compare++)
		{
			if (array[itr_compare] > array[itr_compare + 1])
			{
				swap_holder = array[itr_compare];
				array[itr_compare] = array[itr_compare + 1];
				array[itr_compare + 1] = swap_holder;
				swap_flag = 1;  /* TRUE */
				swap_count++;
				// printf("DEBUG: swapped: array[%llu] > array[%llu]\n", itr_compare, itr_compare + 1);
			}
		}

		if (swap_flag == 0)
		{
			break;
		}
	}

	// printf("swap_count: %llu\n", swap_count);
}
