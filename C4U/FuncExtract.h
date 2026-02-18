/*
    FuncExtract - Function Name Extractor for C Source Files

    Purpose:
        Parse C source code files and extract function names
        for documentation and analysis purposes.

    Features:
        - Parse multiple C source files
        - Extract function definitions (not declarations)
        - Handle various C function syntax patterns
        - Output to text file

    Limitations:
        - Does not handle preprocessor directives comprehensively
        - May not detect all edge cases of C syntax
        - Focuses on standard function definitions

*/

#ifndef FUNCEXTRACT_H
#define FUNCEXTRACT_H

#include <stdint.h>
#include <stdio.h>


/*
    Constants
*/
#define FE_FALSE            0
#define FE_TRUE             1
#define FE_NULL             0

#define FE_SUCCESS          0
#define FE_ERROR_FILE       1
#define FE_ERROR_MEMORY     2
#define FE_ERROR_PARSE      3

#define FE_MAX_LINE_LENGTH      4096
#define FE_MAX_FUNC_NAME_LENGTH 256
#define FE_MAX_FUNCTIONS        10000


/*
    Data Structures
*/
typedef struct s_FUNCTION_INFO
{
    char name[FE_MAX_FUNC_NAME_LENGTH];
    uint64_t line_number;
    char return_type[FE_MAX_FUNC_NAME_LENGTH];
} FUNCTION_INFO;

typedef struct s_FUNCTION_LIST
{
    FUNCTION_INFO* functions;
    uint64_t count;
    uint64_t capacity;
} FUNCTION_LIST;

typedef uint8_t FE_STATUS;


/*
    Function Prototypes
*/
#ifdef __cplusplus
extern "C" {
#endif

/* Core parsing functions */
FE_STATUS Parse_C_Source_File(char const* file_path, FUNCTION_LIST* func_list);
FE_STATUS Extract_Function_Name(char const* line, char const* next_line, FUNCTION_INFO* func_info);
FE_STATUS Write_Function_List(char const* output_file, FUNCTION_LIST const* func_list);

/* Utility functions */
FE_STATUS Init_Function_List(FUNCTION_LIST* func_list, uint64_t initial_capacity);
FE_STATUS Add_Function_To_List(FUNCTION_LIST* func_list, FUNCTION_INFO const* func_info);
void Free_Function_List(FUNCTION_LIST* func_list);
int Add_Two_Numbers(int a, int b);
int Subtract_Two_Numbers(int a, int b);

/* String processing helpers */
uint8_t Is_Whitespace(char c);
uint8_t Is_Alpha(char c);
uint8_t Is_Alphanumeric(char c);
void Trim_Whitespace(char* str);
uint8_t Is_C_Keyword(char const* word);
uint8_t Looks_Like_Function_Definition(char const* line, char const* next_line);

#ifdef __cplusplus
}
#endif

#endif /* FUNCEXTRACT_H */
