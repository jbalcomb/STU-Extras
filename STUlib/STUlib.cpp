
/*
C89 / C90 standard(ISO / IEC 9899:1990) :
4.9.9.2 The fseek function



long ftell( FILE *stream );

File position indicator on success or -1L if failure occurs.

*/

#include "STUlib.hpp"



long Length_of_File(const char* file_path_name)
{
	FILE* file_pointer;
	long file_length;

	file_pointer = fopen(file_path_name, "rb");

	if (NULL == file_pointer)
	{
		fprintf(stderr, "FAILURE: Could not open file.\n");
		return NULL;
	}

	fseek(file_pointer, 0L, SEEK_END);

	file_length = ftell(file_pointer);
	
	return file_length;
}


unsigned char * Load_Entire_File(const char* file_path_name)
{
	FILE* file_pointer;
	long file_length;
	unsigned char * file_buffer;

	file_length = Length_of_File(file_path_name);

	file_buffer = (unsigned char* ) malloc(file_length);

	if (NULL == file_buffer)
	{
		fprintf(stderr, "FAILURE: Could not allocate memory.\n");
		return NULL;
	}

	file_pointer = fopen(file_path_name, "rb");

	if (NULL == file_pointer)
	{
		fprintf(stderr, "FAILURE: Could not open file.\n");
		return NULL;
	}

	fread(file_buffer, sizeof(unsigned char), file_length, file_pointer);

	if (NULL != file_pointer)
	{
		fclose(file_pointer);
	}
	
	return file_buffer;
}