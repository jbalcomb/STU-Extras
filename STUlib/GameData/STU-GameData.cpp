
#include "../STUlib.hpp"


#include <cjson/cJSON.h>

char* create_monitor(void);
char* create_monitor_with_helpers(void);
int supports_full_hd(const char* const monitor);


// STU_DIR
// SDL_DIR
// IMGUI_DIR
// STB_DIR


#define MAGIC_EXE   "MAGIC.EXE"
#define MAGIC       "MAGIC.EXE"
#define MGC         "MAGIC.EXE"
#define WIZARDS_EXE "WIZARDS.EXE"
#define WIZARDS     "WIZARDS.EXE"
#define WZD         "WIZARDS.EXE"

#define SZ_B              1     // SizeOf      Byte
#define SZ_KB          1024     // SizeOf  KiloByte  1024 *    1 =       1
#define SZ_MB       1048576     // SizeOf  MegaByte  1024 * 1024 = 1048576
#define SZ_PR            16     // SizeOf  Paragraph   16 *    1 =      16
#define SZ_PR_B          16     // SizeOf  Paragraph   16 *    1 =      16
// #define SZ_PR_KB
// #define SZ_PR_MB
// 1048576 / 16 = 65536

// char * lbx_name

// Good-To-Know:
// Size Of: Short Int vs. Int
// Size Of: Long Int vs. Int
// Size Of: pointer
// sizeof(void*)
// CWD
// CPU Architecutre
// SIMD
// Open GL
// _PAGESIZE_  PAGESIZE
// STACKSIZE


// MAGIC.EXE

// Borland C++ - Copyright 1991 Borland Intl.�
// 42 6F 72 6C 61 6E 64 20 43 2B 2B 20 2D 20 43 6F 70 79 72 69 67 68 74 20 31 39 39 31 20 42 6F 72 6C 61 6E 64 20 49 6E 74 6C 2E 00
// Offset: 0x24514
// Length: 0x2B

// EoF
// Offset: 0x4A0A0


// WIZARDS.EXE

// Borland C++ - Copyright 1991 Borland Intl.�
// 42 6F 72 6C 61 6E 64 20 43 2B 2B 20 2D 20 43 6F 70 79 72 69 67 68 74 20 31 39 39 31 20 42 6F 72 6C 61 6E 64 20 49 6E 74 6C 2E 00
// Offset: 0x294A4
// Length: 0x2B


// dseg:2080 00 10                                           __ovrbuffer dw 4096                     ; DATA XREF: __OvrPrepare+19r ...
// dseg:2080                                                                                         ; declared in overlay.lib\OVRBUFF
// dseg:2080  0002B520  00038B20

// dseg:2082 41 6C 63 68 65 6D 79                            cnst_Alchemy db 'Alchemy'               ; DATA XREF: dseg:wizard_abilities_nameso

// Alchemy�Warlord�Chaos Mastery�Nature Mastery�Sorcery Mastery�Infernal Power�Divine Power�Sage Master�Channeler�Myrran�Archmage�Mana Focusing�Node Mastery�Famous�Runemaster�Conjurer�Charismatic�Artificer�
// Offset: 0x2B522
// Length: 0xCB
// Count: 18

// ¿ DSEG ? ¿ ALL strings ?
// Offset: 0x2B522

// ¿ Unit (Type) Names ?
// Offset: 0x2B673


// ¿ Race (Type) Names ?
// Offset: 0x2EE70
// Barbarians�Beastmen�Dark Elves�Draconians�Dwarves�Gnolls�Halflings�High Elves�High Men�Klackons�Lizardmen�Nomads�Orcs�Trolls�

// Barbarians�
// Offset: 0x2EE70
// Length: 0xB

// Beastmen�Dark Elves�Draconians�Dwarves�Gnolls�Halflings�High Elves�High Men�Klackons�Lizardmen�Nomads�Orcs�Trolls�


// EoF
// Offset: 0xF4710



constexpr int data_set_count = 2;
struct s_DATA_SET_ELEMENT
{
    int offset;
    int length;
    int type;
};
struct s_DATA_SET_ELEMENT data_set_definition[data_set_count] =
{
    {0x294A4, 0x2B, 0xC},
    {0x2B522, 0xCB, 0xCA}
};




int old_main()
{

    // char file_name[(1 * 1024 * 1024)];  // One Million Bytes!!
    // char file_data_buffer[(1 * 1024 * 1024)];  // One Million Bytes!!
    char file_name[MAX_PATH];
    char file_data_buffer[4096];
    
    FILE * file_pointer;
    FILE* output_file_pointer;

    size_t itr;  // data_set_count
    size_t itr_len;  // data set element length
    // char* rvr;

    cJSON* data_set_json = NULL;
    cJSON* cstring_array = NULL;
    char* data_set_json_string = NULL;

    strcpy(file_name, "WIZARDS");
    strcat(file_name, ".EXE");

    file_pointer = fopen(file_name, "rb");

    if (NULL == file_pointer)
    {
        fprintf(stderr, "Can not open file.");
        return -1;
    }

    fread(file_data_buffer, 1, sizeof(file_data_buffer), file_pointer);

    data_set_json = cJSON_CreateObject();
    cJSON_AddStringToObject(data_set_json, "name", "John Doe");
    cJSON_AddNumberToObject(data_set_json, "age", 30);
    cJSON_AddStringToObject(data_set_json, "email", "john.doe@example.com");

    for (itr = 0; itr < data_set_count; itr++)
    {

        fseek(file_pointer, data_set_definition[itr].offset, 0);
        fread(file_data_buffer, 1, data_set_definition[itr].length, file_pointer);

        switch (data_set_definition[itr].type)
        {
        case 0xC:  /* C-String */
        {
            fprintf(stdout, "%s\n", file_data_buffer);
            cJSON_AddStringToObject(data_set_json, "cstring", file_data_buffer);
        } break;
        case 0xCA:  /* Array of C-String */
        {
            cstring_array = cJSON_AddArrayToObject(data_set_json, "cstring_array");
            for (itr_len = 0; itr_len < data_set_definition[itr].length; itr_len++)
            {
                // rvr = file_data_buffer;
                // while (rvr++ != '\0')
                // {
                //     fprintf(stdout, "%s\n", file_data_buffer);
                // }
                // if (*(file_data_buffer + itr_len) == '\0') { *(file_data_buffer + itr_len) = ','; }
                if (file_data_buffer[itr_len] == '\0') { file_data_buffer[itr_len] = ','; }
            }
            file_data_buffer[itr_len] = '\0';
            fprintf(stdout, "%s\n", file_data_buffer);
            cJSON_AddStringToObject(data_set_json, "name", file_data_buffer);
            
        } break;
        }

    }

    // convert the cJSON object to a JSON string 
    data_set_json_string = cJSON_Print(data_set_json);

    // write the JSON string to a file 
    output_file_pointer = fopen("WIZARDS.json", "w");
    if (output_file_pointer == NULL) {
        printf("Error: Unable to open the file.\n");
        return 1;
    }
    printf("%s\n", data_set_json_string);
    fputs(data_set_json_string, output_file_pointer);


    // free the JSON string and cJSON object 
    cJSON_free(data_set_json_string);
    cJSON_Delete(data_set_json);


    if (NULL != output_file_pointer)
    {
        fclose(output_file_pointer);
    }
    if (NULL != file_pointer)
    {
        fclose(file_pointer);
    }


    return 0;
}
