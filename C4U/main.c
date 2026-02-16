
#define _CRT_SECURE_NO_WARNINGS 1

#include "FuncExtract.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define MAX_SOURCE_FILES    256
#define MAX_PATH_LENGTH     1024


/*
	Parse_CMakeLists_For_Target

	Scans a CMakeLists.txt file for add_executable or add_library
	matching the given target name, and extracts the source file list.

	Returns the number of source files found, or -1 on error.
*/
static int Parse_CMakeLists_For_Target(
	char const* cmake_path,
	char const* target_name,
	char source_files[][MAX_PATH_LENGTH],
	int max_files,
	char const* base_dir
)
{
	FILE* fp;
	char line[FE_MAX_LINE_LENGTH];
	char collected[FE_MAX_LINE_LENGTH * 16];
	int file_count;
	size_t target_len;
	int collecting;

	fp = fopen(cmake_path, "r");
	if (fp == NULL)
	{
		fprintf(stderr, "Error: Cannot open CMakeLists.txt '%s'\n", cmake_path);
		return -1;
	}

	target_len = strlen(target_name);
	file_count = 0;
	collecting = 0;
	collected[0] = '\0';

	while (fgets(line, FE_MAX_LINE_LENGTH, fp) != NULL)
	{
		char* trimmed;
		char* pos;

		/* Trim leading whitespace */
		trimmed = line;
		while (*trimmed == ' ' || *trimmed == '\t')
		{
			trimmed++;
		}

		if (!collecting)
		{
			/* Look for add_executable( or add_library( */
			pos = strstr(trimmed, "add_executable(");
			if (pos == NULL)
			{
				pos = strstr(trimmed, "add_library(");
			}
			if (pos == NULL)
			{
				pos = strstr(trimmed, "add_executable (");
			}
			if (pos == NULL)
			{
				pos = strstr(trimmed, "add_library (");
			}

			if (pos != NULL)
			{
				/* Find the opening paren */
				char* open_paren = strchr(pos, '(');
				if (open_paren != NULL)
				{
					char* name_start = open_paren + 1;
					while (*name_start == ' ' || *name_start == '\t')
					{
						name_start++;
					}

					/* Check if target name matches */
					if (strncmp(name_start, target_name, target_len) == 0
						&& (name_start[target_len] == ' '
							|| name_start[target_len] == '\t'
							|| name_start[target_len] == '\n'
							|| name_start[target_len] == '\r'
							|| name_start[target_len] == ')'))
					{
						/* Start collecting from after the target name */
						char* after_name = name_start + target_len;
						strncpy(collected, after_name, sizeof(collected) - 1);
						collected[sizeof(collected) - 1] = '\0';

						if (strchr(collected, ')') != NULL)
						{
							/* Entire command on one line */
							collecting = 0;
						}
						else
						{
							collecting = 1;
							continue;
						}
					}
					else
					{
						continue;
					}
				}
			}
			else
			{
				continue;
			}
		}
		else
		{
			/* Continue collecting lines until we find ')' */
			size_t cur_len = strlen(collected);
			size_t add_len = strlen(trimmed);
			if (cur_len + add_len + 1 < sizeof(collected))
			{
				strcat(collected, " ");
				strcat(collected, trimmed);
			}

			if (strchr(trimmed, ')') != NULL)
			{
				collecting = 0;
			}
			else
			{
				continue;
			}
		}

		/* Parse the collected text for source file names */
		if (collected[0] != '\0' && !collecting)
		{
			char* tok_buf;
			char* token;
			char* close;

			/* Remove the closing paren and anything after */
			close = strchr(collected, ')');
			if (close != NULL)
			{
				*close = '\0';
			}

			tok_buf = collected;
			token = strtok(tok_buf, " \t\r\n");
			while (token != NULL && file_count < max_files)
			{
				size_t tlen = strlen(token);

				/* Skip CMake keywords/variables */
				if (token[0] == '$' || strcmp(token, "STATIC") == 0
					|| strcmp(token, "SHARED") == 0
					|| strcmp(token, "MODULE") == 0
					|| strcmp(token, "OBJECT") == 0
					|| strcmp(token, "WIN32") == 0
					|| strcmp(token, "MACOSX_BUNDLE") == 0
					|| strcmp(token, "EXCLUDE_FROM_ALL") == 0
					|| strcmp(token, "IMPORTED") == 0
					|| strcmp(token, "INTERFACE") == 0
					|| strcmp(token, "ALIAS") == 0
					|| strcmp(token, "PUBLIC") == 0
					|| strcmp(token, "PRIVATE") == 0)
				{
					token = strtok(NULL, " \t\r\n");
					continue;
				}

				/* Only include files with C-like extensions */
				if (tlen > 2 && (
					strcmp(token + tlen - 2, ".c") == 0 ||
					strcmp(token + tlen - 2, ".C") == 0 ||
					strcmp(token + tlen - 2, ".h") == 0 ||
					strcmp(token + tlen - 2, ".H") == 0 ||
					(tlen > 4 && strcmp(token + tlen - 4, ".cpp") == 0) ||
					(tlen > 4 && strcmp(token + tlen - 4, ".hpp") == 0) ||
					(tlen > 3 && strcmp(token + tlen - 3, ".cc") == 0) ||
					(tlen > 4 && strcmp(token + tlen - 4, ".cxx") == 0)
				))
				{
					/* Build full path: base_dir / token */
					if (base_dir != NULL && base_dir[0] != '\0')
					{
						snprintf(source_files[file_count], MAX_PATH_LENGTH,
							"%s/%s", base_dir, token);
					}
					else
					{
						strncpy(source_files[file_count], token, MAX_PATH_LENGTH - 1);
						source_files[file_count][MAX_PATH_LENGTH - 1] = '\0';
					}
					file_count++;
				}

				token = strtok(NULL, " \t\r\n");
			}

			collected[0] = '\0';
		}
	}

	fclose(fp);
	return file_count;
}


/*
	Extract_Base_Directory

	Given a file path, extract the directory portion.
*/
static void Extract_Base_Directory(char const* file_path, char* dir_buf, size_t dir_buf_size)
{
	char const* last_sep;
	size_t len;

	dir_buf[0] = '\0';

	last_sep = strrchr(file_path, '/');
	if (last_sep == NULL)
	{
		last_sep = strrchr(file_path, '\\');
	}

	if (last_sep != NULL)
	{
		len = (size_t)(last_sep - file_path);
		if (len >= dir_buf_size)
		{
			len = dir_buf_size - 1;
		}
		memcpy(dir_buf, file_path, len);
		dir_buf[len] = '\0';
	}
}


static void Print_Usage(void)
{
	printf("FuncExtract - C Function Name Extractor\n\n");
	printf("Usage:\n");
	printf("  FuncExtract -t <target> -c <CMakeLists.txt> -o <output.txt>\n");
	printf("  FuncExtract -o <output.txt> file1.c file2.c ...\n\n");
	printf("Options:\n");
	printf("  -t <target>          CMake target name to search for\n");
	printf("  -c <CMakeLists.txt>  Path to CMakeLists.txt file\n");
	printf("  -o <output.txt>      Output file path\n");
	printf("  -h                   Show this help message\n");
}


int main(int argc, char* argv[])
{
	char const* target_name;
	char const* cmake_path;
	char const* output_path;
	char source_files[MAX_SOURCE_FILES][MAX_PATH_LENGTH];
	int source_count;
	int direct_files_start;
	FUNCTION_LIST func_list;
	FE_STATUS status;
	int i;
	char base_dir[MAX_PATH_LENGTH];

	target_name = NULL;
	cmake_path = NULL;
	output_path = NULL;
	source_count = 0;
	direct_files_start = -1;

	/* Parse command-line arguments */
	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
		{
			Print_Usage();
			return 0;
		}
		else if (strcmp(argv[i], "-t") == 0 && i + 1 < argc)
		{
			target_name = argv[++i];
		}
		else if (strcmp(argv[i], "-c") == 0 && i + 1 < argc)
		{
			cmake_path = argv[++i];
		}
		else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc)
		{
			output_path = argv[++i];
		}
		else if (argv[i][0] != '-')
		{
			/* Treat as direct source file */
			if (direct_files_start < 0)
			{
				direct_files_start = i;
			}
		}
		else
		{
			fprintf(stderr, "Unknown option: %s\n", argv[i]);
			Print_Usage();
			return 1;
		}
	}

	if (output_path == NULL)
	{
		fprintf(stderr, "Error: Output file (-o) is required.\n\n");
		Print_Usage();
		return 1;
	}

	/* Mode 1: Parse CMakeLists.txt for target sources */
	if (target_name != NULL && cmake_path != NULL)
	{
		Extract_Base_Directory(cmake_path, base_dir, MAX_PATH_LENGTH);

		source_count = Parse_CMakeLists_For_Target(
			cmake_path, target_name,
			source_files, MAX_SOURCE_FILES,
			base_dir
		);

		if (source_count < 0)
		{
			fprintf(stderr, "Error: Failed to parse CMakeLists.txt\n");
			return 1;
		}

		if (source_count == 0)
		{
			fprintf(stderr, "Warning: No source files found for target '%s'\n", target_name);
		}
		else
		{
			printf("Found %d source file(s) for target '%s':\n", source_count, target_name);
			for (i = 0; i < source_count; i++)
			{
				printf("  %s\n", source_files[i]);
			}
			printf("\n");
		}
	}
	/* Mode 2: Direct file list */
	else if (direct_files_start >= 0)
	{
		for (i = direct_files_start; i < argc && source_count < MAX_SOURCE_FILES; i++)
		{
			if (argv[i][0] != '-')
			{
				strncpy(source_files[source_count], argv[i], MAX_PATH_LENGTH - 1);
				source_files[source_count][MAX_PATH_LENGTH - 1] = '\0';
				source_count++;
			}
		}
	}
	else
	{
		fprintf(stderr, "Error: No source files specified.\n");
		fprintf(stderr, "Use -t/-c for CMake target mode, or pass files directly.\n\n");
		Print_Usage();
		return 1;
	}

	/* Initialize function list */
	status = Init_Function_List(&func_list, 256);
	if (status != FE_SUCCESS)
	{
		fprintf(stderr, "Error: Failed to allocate function list.\n");
		return 1;
	}

	/* Parse each source file */
	for (i = 0; i < source_count; i++)
	{
		printf("Parsing: %s\n", source_files[i]);
		status = Parse_C_Source_File(source_files[i], &func_list);
		if (status != FE_SUCCESS)
		{
			fprintf(stderr, "Warning: Failed to parse '%s' (status=%d)\n",
				source_files[i], (int)status);
		}
	}

	/* Write results */
	printf("\nExtracted %llu function(s).\n", (unsigned long long)func_list.count);

	status = Write_Function_List(output_path, &func_list);
	if (status != FE_SUCCESS)
	{
		fprintf(stderr, "Error: Failed to write output file '%s'\n", output_path);
		Free_Function_List(&func_list);
		return 1;
	}

	printf("Output written to: %s\n", output_path);

	Free_Function_List(&func_list);

	return 0;
}
