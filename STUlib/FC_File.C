
#define _CRT_SECURE_NO_WARNINGS 1

#include <malloc.h>
#include <stdint.h>
#include <stdio.h>



/*
FILE* stream;

int    i = 10;
double fp = 1.5;
char   s[] = "this is a string";
char   c = '\n';

fopen_s(&stream, "fprintf.out", "w");
fprintf(stream, "%s%c", s, c);
fprintf(stream, "%d\n", i);
fprintf(stream, "%f\n", fp);
fclose(stream);
*/



void FC_Load_File(char* file_path_name, uint8_t* file_bytes, size_t file_byte_count)
{
	FILE* file_pointer_stream;
	uint8_t * file_buffer;
	size_t file_buffer_element_size;
	size_t file_buffer_element_count;

	file_pointer_stream = fopen(file_path_name, "rb");

	if (NULL == file_pointer_stream)
	{
		fprintf(stderr, "FAILURE: Could not open file.");
		// return NULL;
	}

	if (NULL != file_pointer_stream)
	{

		file_buffer_element_size = sizeof(unsigned char);

		fseek(file_pointer_stream, SEEK_END, 0);
		file_buffer_element_count = ftell(file_pointer_stream);

		file_buffer = (uint8_t*)malloc(file_buffer_element_size * file_buffer_element_count);

		if (NULL == file_buffer)
		{
			fprintf(stderr, "FAILURE: Could not allocate memory.");
			// return NULL;
		}

		if (NULL != file_buffer)
		{
			fread(file_buffer, file_buffer_element_size, file_buffer_element_count, file_pointer_stream);
		}

		if (NULL != file_pointer_stream)
		{
			fclose(file_pointer_stream);
		}

	}

}