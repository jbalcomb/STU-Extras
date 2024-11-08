#define _CRT_SECURE_NO_WARNINGS

// #include <cstring>
// #include <string.h>
#include <stdio.h>      /* FILE; fclose(), fopen(), fread(), fseek(), printf(); */
#include <stdint.h>
#include <stdlib.h>     /* itoa() */
#include <string.h>     /* strcat(), strcpy() */



char proc_line_1[] = "void Dedu_UTT_RangedType(void)";


char proc_name[1][2][11] =
{
    {"Dedu_UTT", "RangedType"}
};


void proggen_unittype(void)
{
    char buffer[4096];
    FILE* output_file_pointer;
    char ouput_filename[] = "DeduRedux.cpp";


    sprintf(buffer, "void %s_%s(void)\n{\n}\n", proc_name[0][0], proc_name[0][1]);

    printf("%s", buffer);



    output_file_pointer = fopen(ouput_filename, "w");

    if (output_file_pointer == NULL) {
        printf("FAILURE: Could not open output file %s.\n", ouput_filename);
        goto CleanUp;
    }

    fprintf(output_file_pointer, "/*\n");
    fprintf(output_file_pointer, "    Generated from Deduce-ProcGen\n");
    fprintf(output_file_pointer, "*/\n");

    fprintf(output_file_pointer, "\n");
    fprintf(output_file_pointer, "\n");
    fprintf(output_file_pointer, "\n");

    fprintf(output_file_pointer, "#include <stdint.h>\n");

    fprintf(output_file_pointer, "\n");
    fprintf(output_file_pointer, "\n");
    fprintf(output_file_pointer, "\n");


    fprintf(output_file_pointer, "};\n");


CleanUp:
    if (NULL != output_file_pointer)
    {
        fclose(output_file_pointer);
    }

}