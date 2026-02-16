
#define _CRT_SECURE_NO_WARNINGS 1

#include "FuncExtract.h"

#include <stdlib.h>
#include <string.h>


/*
	C keywords that could appear before '(' but are NOT function names
*/
static char const* g_c_keywords[] =
{
	"if", "else", "for", "while", "do", "switch", "case", "return",
	"sizeof", "typedef", "struct", "union", "enum", "goto",
	"break", "continue", "default", "extern", "static", "register",
	"volatile", "const", "inline", "restrict",
	"_Bool", "_Complex", "_Imaginary",
	NULL
};


/*
	String Processing Helpers
*/

uint8_t Is_Whitespace(char c)
{
	return (c == ' ' || c == '\t' || c == '\r' || c == '\n') ? FE_TRUE : FE_FALSE;
}


uint8_t Is_Alpha(char c)
{
	return ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_') ? FE_TRUE : FE_FALSE;
}


uint8_t Is_Alphanumeric(char c)
{
	return (Is_Alpha(c) || (c >= '0' && c <= '9')) ? FE_TRUE : FE_FALSE;
}


void Trim_Whitespace(char* str)
{
	char* start;
	char* end;
	size_t len;

	if (str == NULL || str[0] == '\0')
	{
		return;
	}

	start = str;
	while (*start != '\0' && Is_Whitespace(*start))
	{
		start++;
	}

	if (*start == '\0')
	{
		str[0] = '\0';
		return;
	}

	end = start + strlen(start) - 1;
	while (end > start && Is_Whitespace(*end))
	{
		end--;
	}

	len = (size_t)(end - start + 1);
	memmove(str, start, len);
	str[len] = '\0';
}


uint8_t Is_C_Keyword(char const* word)
{
	int i;

	for (i = 0; g_c_keywords[i] != NULL; i++)
	{
		if (strcmp(word, g_c_keywords[i]) == 0)
		{
			return FE_TRUE;
		}
	}

	return FE_FALSE;
}


/*
	Looks_Like_Function_Definition

	Checks whether a line (possibly combined with the next line)
	looks like a C function definition.

	Heuristic:
	  - Line contains '(' for a parameter list
	  - Token before '(' is an identifier (function name)
	  - Line does NOT end with ';' (that would be a declaration)
	  - Line or next_line contains '{' (start of function body)
	  - The identifier is not a C keyword
	  - Line does not start with '#' (preprocessor)
*/
uint8_t Looks_Like_Function_Definition(char const* line, char const* next_line)
{
	char const* paren;
	char const* p;
	char const* id_end;
	char const* id_start;
	char trimmed[FE_MAX_LINE_LENGTH];
	size_t len;

	if (line == NULL)
	{
		return FE_FALSE;
	}

	/* Copy and trim, strip trailing // comments */
	strncpy(trimmed, line, FE_MAX_LINE_LENGTH - 1);
	trimmed[FE_MAX_LINE_LENGTH - 1] = '\0';
	{
		char* comment_pos = strstr(trimmed, "//");
		if (comment_pos != NULL)
		{
			*comment_pos = '\0';
		}
	}
	Trim_Whitespace(trimmed);
	len = strlen(trimmed);

	if (len == 0)
	{
		return FE_FALSE;
	}

	/* Skip preprocessor directives */
	if (trimmed[0] == '#')
	{
		return FE_FALSE;
	}

	/* Skip lines that end with ';' (declarations / prototypes) */
	if (trimmed[len - 1] == ';')
	{
		return FE_FALSE;
	}

	/* Skip assignments (contains '=' before '(') */
	{
		char* eq_pos = strchr(trimmed, '=');
		char* paren_pos = strchr(trimmed, '(');
		if (eq_pos != NULL && (paren_pos == NULL || eq_pos < paren_pos))
		{
			return FE_FALSE;
		}
	}

	/* Must have '(' for parameter list */
	paren = strchr(trimmed, '(');
	if (paren == NULL)
	{
		return FE_FALSE;
	}

	/* Walk backward from '(' to find the identifier */
	p = paren - 1;
	while (p >= trimmed && Is_Whitespace(*p))
	{
		p--;
	}

	if (p < trimmed || !Is_Alphanumeric(*p))
	{
		return FE_FALSE;
	}

	id_end = p + 1;
	while (p >= trimmed && Is_Alphanumeric(*p))
	{
		p--;
	}
	id_start = p + 1;

	/* The identifier must not be a C keyword */
	{
		char name_buf[FE_MAX_FUNC_NAME_LENGTH];
		size_t name_len = (size_t)(id_end - id_start);
		if (name_len == 0 || name_len >= FE_MAX_FUNC_NAME_LENGTH)
		{
			return FE_FALSE;
		}
		memcpy(name_buf, id_start, name_len);
		name_buf[name_len] = '\0';

		if (Is_C_Keyword(name_buf))
		{
			return FE_FALSE;
		}
	}

	/* There must be a return type before the function name */
	if (id_start == trimmed)
	{
		return FE_FALSE;
	}

	/* Check for '{' on this line or the next line */
	if (strchr(trimmed, '{') != NULL)
	{
		return FE_TRUE;
	}

	if (next_line != NULL)
	{
		char next_stripped[FE_MAX_LINE_LENGTH];
		char* nc;
		strncpy(next_stripped, next_line, FE_MAX_LINE_LENGTH - 1);
		next_stripped[FE_MAX_LINE_LENGTH - 1] = '\0';
		nc = strstr(next_stripped, "//");
		if (nc != NULL)
		{
			*nc = '\0';
		}
		if (strchr(next_stripped, '{') != NULL)
		{
			return FE_TRUE;
		}
	}

	return FE_FALSE;
}


/*
	Extract_Function_Name

	Given a line that looks like a function definition,
	extract the function name and return type.
*/
FE_STATUS Extract_Function_Name(char const* line, char const* next_line, FUNCTION_INFO* func_info)
{
	char const* paren;
	char const* p;
	char const* id_end;
	char const* id_start;
	char trimmed[FE_MAX_LINE_LENGTH];
	size_t name_len;
	size_t ret_len;

	(void)next_line;

	if (line == NULL || func_info == NULL)
	{
		return FE_ERROR_PARSE;
	}

	strncpy(trimmed, line, FE_MAX_LINE_LENGTH - 1);
	trimmed[FE_MAX_LINE_LENGTH - 1] = '\0';
	Trim_Whitespace(trimmed);

	paren = strchr(trimmed, '(');
	if (paren == NULL)
	{
		return FE_ERROR_PARSE;
	}

	/* Walk backward from '(' to find the identifier */
	p = paren - 1;
	while (p >= trimmed && Is_Whitespace(*p))
	{
		p--;
	}

	if (p < trimmed || !Is_Alphanumeric(*p))
	{
		return FE_ERROR_PARSE;
	}

	id_end = p + 1;
	while (p >= trimmed && Is_Alphanumeric(*p))
	{
		p--;
	}
	id_start = p + 1;

	/* Extract function name */
	name_len = (size_t)(id_end - id_start);
	if (name_len == 0 || name_len >= FE_MAX_FUNC_NAME_LENGTH)
	{
		return FE_ERROR_PARSE;
	}
	memcpy(func_info->name, id_start, name_len);
	func_info->name[name_len] = '\0';

	/* Extract return type (everything before the function name, trimmed) */
	ret_len = (size_t)(id_start - trimmed);
	if (ret_len >= FE_MAX_FUNC_NAME_LENGTH)
	{
		ret_len = FE_MAX_FUNC_NAME_LENGTH - 1;
	}
	memcpy(func_info->return_type, trimmed, ret_len);
	func_info->return_type[ret_len] = '\0';
	Trim_Whitespace(func_info->return_type);

	return FE_SUCCESS;
}


/*
	Function List Management
*/

FE_STATUS Init_Function_List(FUNCTION_LIST* func_list, uint64_t initial_capacity)
{
	if (func_list == NULL)
	{
		return FE_ERROR_MEMORY;
	}

	func_list->functions = (FUNCTION_INFO*)malloc((size_t)initial_capacity * sizeof(FUNCTION_INFO));
	if (func_list->functions == NULL)
	{
		return FE_ERROR_MEMORY;
	}

	func_list->count = 0;
	func_list->capacity = initial_capacity;

	return FE_SUCCESS;
}


FE_STATUS Add_Function_To_List(FUNCTION_LIST* func_list, FUNCTION_INFO const* func_info)
{
	FUNCTION_INFO* new_buf;
	uint64_t new_capacity;

	if (func_list == NULL || func_info == NULL)
	{
		return FE_ERROR_MEMORY;
	}

	if (func_list->count >= func_list->capacity)
	{
		new_capacity = func_list->capacity * 2;
		new_buf = (FUNCTION_INFO*)realloc(
			func_list->functions,
			(size_t)new_capacity * sizeof(FUNCTION_INFO)
		);
		if (new_buf == NULL)
		{
			return FE_ERROR_MEMORY;
		}
		func_list->functions = new_buf;
		func_list->capacity = new_capacity;
	}

	memcpy(&func_list->functions[func_list->count], func_info, sizeof(FUNCTION_INFO));
	func_list->count++;

	return FE_SUCCESS;
}


void Free_Function_List(FUNCTION_LIST* func_list)
{
	if (func_list != NULL)
	{
		if (func_list->functions != NULL)
		{
			free(func_list->functions);
			func_list->functions = NULL;
		}
		func_list->count = 0;
		func_list->capacity = 0;
	}
}


/*
	Parse_C_Source_File

	Reads a C source file line-by-line and extracts function definitions.
*/
FE_STATUS Parse_C_Source_File(char const* file_path, FUNCTION_LIST* func_list)
{
	FILE* fp;
	char line_buf[FE_MAX_LINE_LENGTH];
	char next_buf[FE_MAX_LINE_LENGTH];
	char* got_line;
	char* got_next;
	uint64_t line_number;
	uint8_t in_block_comment;
	FUNCTION_INFO info;
	FE_STATUS status;

	if (file_path == NULL || func_list == NULL)
	{
		return FE_ERROR_FILE;
	}

	fp = fopen(file_path, "r");
	if (fp == NULL)
	{
		fprintf(stderr, "Error: Cannot open file '%s'\n", file_path);
		return FE_ERROR_FILE;
	}

	line_number = 0;
	in_block_comment = FE_FALSE;

	got_line = fgets(line_buf, FE_MAX_LINE_LENGTH, fp);
	line_number = 1;

	while (got_line != NULL)
	{
		/* Peek at next line */
		got_next = fgets(next_buf, FE_MAX_LINE_LENGTH, fp);

		/* Basic block comment tracking */
		if (in_block_comment)
		{
			if (strstr(line_buf, "*/") != NULL)
			{
				in_block_comment = FE_FALSE;
			}
			goto advance;
		}

		if (strstr(line_buf, "/*") != NULL && strstr(line_buf, "*/") == NULL)
		{
			in_block_comment = FE_TRUE;
			goto advance;
		}

		/* Skip single-line comments */
		{
			char* comment_pos = strstr(line_buf, "//");
			if (comment_pos != NULL)
			{
				/* Check if the entire meaningful content is a comment */
				char temp[FE_MAX_LINE_LENGTH];
				size_t prefix_len = (size_t)(comment_pos - line_buf);
				memcpy(temp, line_buf, prefix_len);
				temp[prefix_len] = '\0';
				Trim_Whitespace(temp);
				if (temp[0] == '\0')
				{
					goto advance;
				}
			}
		}

		/* Check if this looks like a function definition */
		if (Looks_Like_Function_Definition(line_buf, got_next != NULL ? next_buf : NULL))
		{
			memset(&info, 0, sizeof(info));
			status = Extract_Function_Name(line_buf, got_next != NULL ? next_buf : NULL, &info);
			if (status == FE_SUCCESS)
			{
				info.line_number = line_number;
				Add_Function_To_List(func_list, &info);
			}
		}

advance:
		/* Move next line into current */
		if (got_next != NULL)
		{
			memcpy(line_buf, next_buf, FE_MAX_LINE_LENGTH);
			got_line = line_buf;
		}
		else
		{
			got_line = NULL;
		}
		line_number++;
	}

	fclose(fp);
	return FE_SUCCESS;
}


/*
	Write_Function_List

	Writes the extracted function names to a text file.
*/
FE_STATUS Write_Function_List(char const* output_file, FUNCTION_LIST const* func_list)
{
	FILE* fp;
	uint64_t i;

	if (output_file == NULL || func_list == NULL)
	{
		return FE_ERROR_FILE;
	}

	fp = fopen(output_file, "w");
	if (fp == NULL)
	{
		fprintf(stderr, "Error: Cannot open output file '%s'\n", output_file);
		return FE_ERROR_FILE;
	}

	fprintf(fp, "Function List (%llu functions found)\n", (unsigned long long)func_list->count);
	fprintf(fp, "========================================\n\n");

	for (i = 0; i < func_list->count; i++)
	{
		fprintf(fp, "%-40s  %-20s  line %llu\n",
			func_list->functions[i].name,
			func_list->functions[i].return_type,
			(unsigned long long)func_list->functions[i].line_number
		);
	}

	fclose(fp);
	return FE_SUCCESS;
}
