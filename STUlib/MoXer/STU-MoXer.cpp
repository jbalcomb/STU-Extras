// STU-MoXer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#define _CRT_SECURE_NO_WARNINGS 1
#pragma warning(disable : 4996)
// Error	C4996	'itoa': The POSIX name for this item is deprecated.Instead, use the ISO Cand C++ conformant name : _itoa. See online help for details.

#include "MoX_TYPE.H"

#include <STU-Utilities/FC_Array.H>

#include <iostream>     /* ? */

#include <windows.h>

/*

HRESULT     windows.h   winnt.h
LPCWSTR     windows.h   winnt.h
LPBYTE      windows.h   minwindef.h
DWORD       windows.h   minwindef.h

_tprintf    tchar.h

GetLastError()  errhandlingapi.h

*/


// #include ""FC_Array.H"
// #include <STU-Utilities/STU-Utilities.hpp>
#include <STU-Utilities/FC_File_WinAPI.hpp>



/*
    Manifest Constants for Unit Types
    Character Arrays for Unit Type Names
    Array of Pointers to Character Arrays for Unit Type Names
    Unit Type Structures for Unit Types
    Array of Unit Type Structures for Unit Types

*/

/*
    Manifest Constants for Unit Types

    FIGURES__.LBX
    16 files
    1728 entries
    8 entries per unit type
    216 distinct entry names
    ? duplicates ? e.g., DJINN3, SKYDRAKE

*/

// sizeof: 20h  32d
struct s_LBX_STR_TBL_REC
{
    char name[9];
    char description[23];
};

// sizeof: 24h  36d
struct s_UNIT_TYPE
{
    /* 00 */ uint16_t Name;                   /* ; offset (00036AA0) */
    /* 02 */ int8_t Melee;                  /* ; base 10 */
    /* 03 */ int8_t Ranged;                 /* ; base 10 */
    /* 04 */ int8_t Ranged_Type;
    /* 05 */ int8_t Ammo;                   /* ; base 10 */
    /* 06 */ int8_t To_Hit;                 /* ; base 10 */
    /* 07 */ int8_t Defense;                /* ; base 10 */
    /* 08 */ int8_t Resist;                 /* ; base 10 */
    /* 09 */ int8_t Move_Halves;            /* ; base 10 */
    /* 0A */ int16_t Cost;                  /* ; base 10 */
    /* 0C */ int8_t Upkeep;                 /* ; base 10 */
    /* 0D */ int8_t Race;                   /* ; enum Race_Code */
    /* 0E */ int8_t Bldng1_or_Portrait;     /* ; enum BLD_TYPE */
    /* 0F */ int8_t Bldng2_or_HeroType;     /* ; enum BLD_TYPE */
    // /* 10 */ SAMB_ptr pict_seg;             /* ; segment pointers to LBX_Alloc_Space headers for reserved EMM file links */
    /* 10 */ uint16_t pict_seg;
    /* 12 */ int8_t Hits;                   /* ; base 10 */
    /* 13 */ int8_t Sight;                  /* ; base 10 */
    /* 14 */ int8_t Transport;              /* ; base 10 */
    /* 15 */ int8_t Figures;                /* ; base 10 */
    /* 16 */ int8_t Construction;           /* ; base 10 */
    /* 17 */ int8_t Spec_Att_Attr;          /* ; base 10 */
    /* 18 */ uint16_t Move_Flags;            /* ; bitfield enum MOVEFLAGS */
    /* 1A */ uint16_t Attribs_1;             /* ; bitfield enum ATTRIB_1 */
    /* 1C */ uint16_t Attribs_2;             /* ; bitfield enum ATTRIB_2 */
    /* 1E */ uint16_t Abilities;             /* ; bitfield enum ABL_FLAGS */
    /* 20 */ uint16_t Attack_Flags;          /* ; bitfield enum ATK_FLAGS */
    /* 22 */ int16_t Sound;
};

struct s_UNIT_TYPE* mom_unit_types;

// Unit Type Records / Structure Definitions
#define UNIT_TYPE_STRUCTURE_COUNT 198
uint16_t unit_type_structures_ofst = 0x019C;  // Offset from DSEG in WIZARDS.EXE
uint8_t * _unit_types_structures;
char szUnitTypeStructures[65536];

// Unit Type Names
#define UNIT_TYPE_NAME_COUNT 120
uint16_t unit_type_name_ofst = 0x21D3;  // Offset from DSEG in WIZARDS.EXE
uint8_t * _unit_types_names;
char szUnitTypeNames[65536];

struct s_UNIT_TYPE_NAME_OFFSET
{
    uint16_t ofst;
    uint16_t idx;
};
struct s_UNIT_TYPE_NAME_OFFSET _unit_type_names_offsets[UNIT_TYPE_NAME_COUNT];

// Unit Type Manifest Constants / Abbreviations
char _figures_names[1728][9];
char _distinct_figures_names[216][9];




WCHAR szFileName[MAX_PATH] = L"WZD_DSEG.BIN";

WCHAR szFiguresFileName_01[MAX_PATH] = L"FIGURES1.LBX";
WCHAR szFiguresFileName_02[MAX_PATH] = L"FIGURES2.LBX";
WCHAR szFiguresFileName_03[MAX_PATH] = L"FIGURES3.LBX";
WCHAR szFiguresFileName_04[MAX_PATH] = L"FIGURES4.LBX";
WCHAR szFiguresFileName_05[MAX_PATH] = L"FIGURES5.LBX";
WCHAR szFiguresFileName_06[MAX_PATH] = L"FIGURES6.LBX";
WCHAR szFiguresFileName_07[MAX_PATH] = L"FIGURES7.LBX";
WCHAR szFiguresFileName_08[MAX_PATH] = L"FIGURES8.LBX";
WCHAR szFiguresFileName_09[MAX_PATH] = L"FIGURES9.LBX";
WCHAR szFiguresFileName_10[MAX_PATH] = L"FIGURE10.LBX";
WCHAR szFiguresFileName_11[MAX_PATH] = L"FIGURE11.LBX";
WCHAR szFiguresFileName_12[MAX_PATH] = L"FIGURE12.LBX";
WCHAR szFiguresFileName_13[MAX_PATH] = L"FIGURE13.LBX";
WCHAR szFiguresFileName_14[MAX_PATH] = L"FIGURE14.LBX";
WCHAR szFiguresFileName_15[MAX_PATH] = L"FIGURE15.LBX";
WCHAR szFiguresFileName_16[MAX_PATH] = L"FIGURE16.LBX";



void Generate_Unit_Type_Data(void);
void Load_Unit_Type_Names(void);
void Load_Unit_Type_Structures(void);
void Build_Unit_Type_Names(void);
void Build_Unit_Type_Structures(void);
void Build_Unit_Type_Names_Offsets(void);
// NOGO void Replace_Unit_Type_Name_Offsets(void);
void Export_UNIT_DATA_H(void);

void Program_RunTime_Information(void);
void Print_WZD_DSEG(void);
void Print_MoM_Unit_Types(void);

void Load_Figures_Names(void);
void Figures_String_Table(void);

void Load_Terrain(void);




/*
char unit_name_000[] = "Dwarf";
...
char unit_name_119[] = "Nagas";

char * _unit_names[] =
{
    unit_name_000,
    ...
    unit_name_119
}
*/

int main()
{

    // Generate_Unit_Type_Data();

    /*
    Load_Figures_Names();
    // print_strings(_distinct_figures_names, 20);
    uint64_t itr_strings;
    for (itr_strings = 0; itr_strings < 216; itr_strings++)
    {
        // printf("strings[%u]: %s\n", itr_strings, _distinct_figures_names[itr_strings]);
        if (strlen(_distinct_figures_names[itr_strings]) <= 6)
        {
            printf("#define %s \t\t %u\n", _distinct_figures_names[itr_strings], itr_strings);
        }
        else
        {
            printf("#define %s \t %u\n", _distinct_figures_names[itr_strings], itr_strings);
        }
    }
    */


    Load_Terrain();

}


/*

    ? |-> Load_File("TERRAIN.LBX");

*/
void Load_Terrain(void)
{
    WCHAR wc_figures_filename[MAX_PATH] = { 0 };
    LPBYTE paFileData;  // pointer to array of byte of data
    DWORD cbFileData;   // count of bytes of data  (AKA count of array elements of 8-bit bytes)
    int16_t entry_count;
    uint32_t entry_offset_begin;
    uint32_t entry_offset_end;
    uint32_t entry_length;
    uint8_t baito;
    void* terrain_lbx_000;
    int16_t itr_terrain;

    wcscpy_s(wc_figures_filename, L"TERRAIN");
    wcscat_s(wc_figures_filename, L".LBX");

    WinAPI_Load_File_W(wc_figures_filename, &paFileData, &cbFileData);


    baito = paFileData[0];


    entry_count = ( (int16_t)paFileData[0] ) | ( ( (int16_t)paFileData[1] ) << 8);
    printf("entry_count: %d\n", entry_count);

    /*
        ~== LBX_Load_Entry()

        entry_offset_begin
        entry_offset_end
        entry_length = entry_offset_end - entry_offset_begin
        NOTE: Maximum Length is DOS/EMM Memory Page Size - 64 KB / 4092 PR

        ? cast pointer void/untyped pointer ?

    */
    // entry_start = ( GET_4B_OFS( (lbxload_lbx_header), ( 8 + ((entry_num) * 4)    ) ) );
    // entry_end   = ( GET_4B_OFS( (lbxload_lbx_header), ( 8 + ((entry_num) * 4) + 4) ) );
    entry_offset_begin = (int16_t)paFileData[8] | ((int16_t)paFileData[9] << 8) | ((int16_t)paFileData[10] << 16) | ((int16_t)paFileData[11] << 24);
    entry_offset_end = (int16_t)paFileData[12] | ((int16_t)paFileData[13] << 8) | ((int16_t)paFileData[14] << 16) | ((int16_t)paFileData[15] << 24);
    entry_length = entry_offset_end - entry_offset_begin;
    printf("entry_offset_begin: %08X\n", entry_offset_begin);
    printf("entry_offset_end: %08X\n", entry_offset_end);
    printf("entry_length: %u\n", entry_length);
// entry_count: 3
// entry_offset_begin: 00000240
// entry_offset_end: 000A5480
// entry_length: 676416
    // ---seek to 576  (0x0240)---
    // ---read 676416---
    // 192 bytes of padding, from 0x0240 to 0x0300
    // 676416 - 192 = 676224
    // 676224 / 384 = 1761



    
    terrain_lbx_000 = VirtualAlloc(0, entry_length, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    // terrain pict/bitm
    // 0x180  384
    // 676416 / 384  = 1761.5

    // ? which are terrain types vs subsequent frames of animated terrain types ?

    // 14 00    12 00    0A DE    00 00 00 00 00 00 00 00 00 00
    // width    height   magsig   ?

    // baito = paFileData[768];
    baito = *(paFileData + 768);
    printf("baito: %02X\n", baito);

    for (itr_terrain = 0; itr_terrain < 1761; itr_terrain++)
    {
        baito = *(paFileData + 0x0300 + (itr_terrain * 384));
        if (baito != 0x14)  // width = 20
        {
            printf("FAILURE: [%d]  %02X  @  %08X\n", itr_terrain, baito, (0x0300 + (itr_terrain * 384)));
        }
    }

}

void Generate_Unit_Type_Data(void)
{
    int itr_unit_type_names;


    Load_Unit_Type_Names();
    Load_Unit_Type_Structures();

    Build_Unit_Type_Names();       // populate szUnitTypeNames 
    // printf("%s", szUnitTypeNames);

    // NOTE: depends on Build_Unit_Type_Names_Offsets()
    // Build_Unit_Type_Structures();  // populate szUnitTypeStructures
    // printf("%s", szUnitTypeStructures);

    Build_Unit_Type_Names_Offsets();
    // for (itr_unit_type_names = 0; itr_unit_type_names < UNIT_TYPE_NAME_COUNT; itr_unit_type_names++)
    // {
    //     printf("_unit_type_names_offsets[%d]: [%d],[%04X]\n", itr_unit_type_names, _unit_type_names_offsets[itr_unit_type_names].idx, _unit_type_names_offsets[itr_unit_type_names].ofst);
    // }

    Build_Unit_Type_Structures();
    // printf("%s", szUnitTypeStructures);

    Export_UNIT_DATA_H();

}

/*
    WZD_DSEG.BIN

    LPBYTE paFileData
        LPBYTE: minwindef.h  typedef BYTE far            *LPBYTE;
        BYTE:   minwindef.h  typedef unsigned char       BYTE;
        "LP": Long pointer;  "L": Long;  "P": Pointer
        "pa":   "p": Pointer;  "a": Array
    DWORD cbFileData
        DWORD   minwindef.h  typedef unsigned long       DWORD;
        "cb": Count of bytes
    "sz": Zero terminate string  (AKA C-style string; NULL terminated) ... terminator, token, sentinal value, blah, blah, non-sized vs. sized ...
*/
/*
    4C / FC / C4
    Files for C
    Load File:
        file path & name
        pointer for array of bytes
        pointer for count of bytes
        ...
        /type/ for file path name
            ?, multi-byte, wide, unicode
            Per MS, ~== _A || _W
            :poop: ..., MBCS, ..., TCHAR, ... :poop:

*/
void Load_WZD_DSEG(void)
{
    LPBYTE paFileData;
    DWORD cbFileData;
    WinAPI_Load_File_W(szFileName, &paFileData, &cbFileData);
}


void Program_RunTime_Information(void)
{
    TCHAR tszBuffer[MAX_PATH];
    DWORD dwRet;
    dwRet = GetCurrentDirectory(MAX_PATH, tszBuffer);
    // if(dwRet == 0) { OutputDebugStringA("FAILURE: GetCurrentDirectory(): function failed\n"); }
    // if(dwRet > MAX_PATH) { OutputDebugStringA("FAILURE: GetCurrentDirectory(): buffer too short\n"); }
    char Buffer[MAX_PATH];
    // sprintf(Buffer, "%s\n", tszBuffer);
    // Severity	Code	Description	Project	File	Line	Suppression State
    // Error	C4996	'sprintf': This function or variable may be unsafe.Consider using sprintf_s instead.To disable deprecation, use _CRT_SECURE_NO_WARNINGS.See online help for details.STU - MoXer	C : \devel\STU - MoXer\STU - MoXer\STU - MoXer.cpp	42
    sprintf_s(Buffer, "%s\n", tszBuffer);
    // // OutputDebugStringA(Buffer);
    // // OutputDebugStringA(tszBuffer);  // TCHAR * vs. LPCSTR
    OutputDebugString(tszBuffer);
    // // "J:\STU\devel\STU-MoM_Rasm\data"
    wprintf(L"tszBuffer: %s\n", tszBuffer);
    // C:\devel\STU-MoXer\STU-MoXer
}







void Print_WZD_DSEG(void)
{

    LPBYTE paBmpFileData;
    DWORD cbBmpFileData;
    WinAPI_Load_File_W(szFileName, &paBmpFileData, &cbBmpFileData);

    wprintf(L"paBmpFileData[0]: %04X\n", paBmpFileData[0]);
    wprintf(L"paBmpFileData[1]: %04X\n", paBmpFileData[1]);
    wprintf(L"paBmpFileData[2]: %04X\n", paBmpFileData[2]);
    wprintf(L"paBmpFileData[3]: %04X\n", paBmpFileData[3]);

    wprintf(L"paBmpFileData[4]: %04X\n", paBmpFileData[4]);
    wprintf(L"paBmpFileData[5]: %04X\n", paBmpFileData[5]);
    wprintf(L"paBmpFileData[6]: %04X\n", paBmpFileData[6]);
    wprintf(L"paBmpFileData[7]: %04X\n", paBmpFileData[7]);

    wprintf(L"paBmpFileData[4]: %c\n", paBmpFileData[4]);
    wprintf(L"paBmpFileData[5]: %c\n", paBmpFileData[5]);
    wprintf(L"paBmpFileData[6]: %c\n", paBmpFileData[6]);
    wprintf(L"paBmpFileData[7]: %c\n", paBmpFileData[7]);

    LPBYTE paMomUnitData;
    paMomUnitData = (paBmpFileData + 0x019C);  // 0x019C: offset, in DSEG, to _unit_type_table

    wprintf(L"paMomUnitData[0]: %04X\n", paMomUnitData[0]);
    wprintf(L"paMomUnitData[1]: %04X\n", paMomUnitData[1]);
    wprintf(L"paMomUnitData[2]: %04X\n", paMomUnitData[2]);
    wprintf(L"paMomUnitData[3]: %04X\n", paMomUnitData[3]);

}

void Print_MoM_Unit_Types(void)
{

    LPBYTE paBmpFileData;
    DWORD cbBmpFileData;
    WinAPI_Load_File_W(szFileName, &paBmpFileData, &cbBmpFileData);

    mom_unit_types = (struct s_UNIT_TYPE*)(paBmpFileData + 0x019C + (0x24 * 0));
    // printf("mom_unit_types->Name: %04X\n", mom_unit_types->Name);  // 0x21D: offset, in DSEG, to _unit_names_table

    printf("mom_unit_types->Name: %04X\n", mom_unit_types->Name);
    printf("mom_unit_types->Melee: %02X\n", mom_unit_types->Melee);
    printf("mom_unit_types->Ranged: %02X\n", mom_unit_types->Ranged);
    printf("mom_unit_types->Ranged_Type: %02X\n", mom_unit_types->Ranged_Type);
    printf("mom_unit_types->Ammo: %02X\n", mom_unit_types->Ammo);
    printf("mom_unit_types->To_Hit: %02X\n", mom_unit_types->To_Hit);
    printf("mom_unit_types->Defense: %02X\n", mom_unit_types->Defense);
    printf("mom_unit_types->Resist: %02X\n", mom_unit_types->Resist);
    printf("mom_unit_types->Move_Halves: %02X\n", mom_unit_types->Move_Halves);
    printf("mom_unit_types->Cost: %02X\n", mom_unit_types->Cost);
    printf("mom_unit_types->Upkeep: %02X\n", mom_unit_types->Upkeep);
    printf("mom_unit_types->Race: %02X\n", mom_unit_types->Race);
    printf("mom_unit_types->Bldng1_or_Portrait: %02X\n", mom_unit_types->Bldng1_or_Portrait);
    printf("mom_unit_types->Bldng2_or_HeroType: %02X\n", mom_unit_types->Bldng2_or_HeroType);
    printf("mom_unit_types->pict_seg: %04X\n", mom_unit_types->pict_seg);
    printf("mom_unit_types->Hits: %02X\n", mom_unit_types->Hits);
    printf("mom_unit_types->Sight: %02X\n", mom_unit_types->Sight);
    printf("mom_unit_types->Transport: %02X\n", mom_unit_types->Transport);
    printf("mom_unit_types->Figures: %02X\n", mom_unit_types->Figures);
    printf("mom_unit_types->Construction: %02X\n", mom_unit_types->Construction);
    printf("mom_unit_types->Spec_Att_Attr: %02X\n", mom_unit_types->Spec_Att_Attr);
    printf("mom_unit_types->Move_Flags: %04X\n", mom_unit_types->Move_Flags);
    printf("mom_unit_types->Attribs_1: %04X\n", mom_unit_types->Attribs_1);
    printf("mom_unit_types->Attribs_2: %04X\n", mom_unit_types->Attribs_2);
    printf("mom_unit_types->Abilities: %04X\n", mom_unit_types->Abilities);
    printf("mom_unit_types->Attack_Flags: %04X\n", mom_unit_types->Attack_Flags);
    printf("mom_unit_types->Sound: %04X\n", mom_unit_types->Sound);



    printf("\n");
    printf("struct s_UNIT _unit_data[] =");
    printf("\n");
    printf("{");
    printf("\n");


    printf("{");

    printf("%04X,", mom_unit_types->Name);
    printf("%d,", mom_unit_types->Melee);
    printf("%d,", mom_unit_types->Ranged);
    printf("%02X,", mom_unit_types->Ranged_Type);
    printf("%d,", mom_unit_types->Ammo);
    printf("%d,", mom_unit_types->To_Hit);
    printf("%d,", mom_unit_types->Defense);
    printf("%d,", mom_unit_types->Resist);
    printf("%d,", mom_unit_types->Move_Halves);
    printf("%d,", mom_unit_types->Cost);
    printf("%d,", mom_unit_types->Upkeep);
    printf("%d,", mom_unit_types->Race);
    printf("%d,", mom_unit_types->Bldng1_or_Portrait);
    printf("%d,", mom_unit_types->Bldng2_or_HeroType);
    printf("%04X,", mom_unit_types->pict_seg);
    printf("%d,", mom_unit_types->Hits);
    printf("%d,", mom_unit_types->Sight);
    printf("%d,", mom_unit_types->Transport);
    printf("%d,", mom_unit_types->Figures);
    printf("%d,", mom_unit_types->Construction);
    printf("%d,", mom_unit_types->Spec_Att_Attr);
    printf("%04X,", mom_unit_types->Move_Flags);
    printf("%04X,", mom_unit_types->Attribs_1);
    printf("%04X,", mom_unit_types->Attribs_2);
    printf("%04X,", mom_unit_types->Abilities);
    printf("%04X,", mom_unit_types->Attack_Flags);
    printf("%d", mom_unit_types->Sound);

    printf("}, ");
    printf("\n");


    printf("};");
    printf("\n");


}


void Load_Figures_Names(void)
{
    int itr_figures;
    char figures_number[_MAX_ITOSTR_BASE10_COUNT] = {0};
    WCHAR wc_figures_number[_MAX_ITOSTR_BASE10_COUNT] = { 0 };
    char figures_filename[MAX_PATH] = {0};
    WCHAR wc_figures_filename[MAX_PATH] = { 0 };
    WCHAR szFileName[MAX_PATH] = {0};
    int16_t entry_count;
    uint16_t string_table_ofst;
    uint64_t itr_figures_names;
    uint64_t itr_distinct_figures_names;
    struct s_LBX_STR_TBL_REC* string_table_record;
    uint64_t itr_entry_count;
    char curr_entry_name[9];

    strcpy(curr_entry_name, "ZZZZZZZZ");
    itr_figures_names = 0;
    itr_distinct_figures_names = 0;

    for (itr_figures = 1; itr_figures <= 16; itr_figures++)
    {
        // itoa(itr_figures, figures_number, 10);
        // Severity	Code	Description	Project	File	Line	Suppression State
        // Error	C4996	'itoa': The POSIX name for this item is deprecated.Instead, use the ISO Cand C++ conformant name : _itoa.See online help for details.STU - MoXer	C : \devel\STU - MoXer\STU - MoXer\Unit_Data.cpp	15
        // _itoa(itr_figures, figures_number, 10);
        // Severity	Code	Description	Project	File	Line	Suppression State
        // Error	C4996	'_itoa': This function or variable may be unsafe.Consider using _itoa_s instead.To disable deprecation, use _CRT_SECURE_NO_WARNINGS.See online help for details.STU - MoXer	C : \devel\STU - MoXer\STU - MoXer\STU - MoXer.cpp	217
        _itoa_s(itr_figures, figures_number, 10);
        // wprintf(L"figures_number: %s\n", figures_number);
        strcpy_s(figures_filename, figures_number);
        // printf("figures_filename: %s\n", figures_filename);
        if (itr_figures < 10)
        {
            strcpy_s(figures_filename, "FIGURES");
        }
        else
        {
            strcpy_s(figures_filename, "FIGURE");
        }
        strcat_s(figures_filename, figures_number);
        strcat_s(figures_filename, ".LBX");
        // printf("figures_filename: %s\n", figures_filename);

        _itow_s(itr_figures, wc_figures_number, 10);
        // wprintf(L"wc_figures_number: %s\n", wc_figures_number);
        if (itr_figures < 10)
        {
            wcscpy_s(wc_figures_filename, L"FIGURES");
        }
        else
        {
            wcscpy_s(wc_figures_filename, L"FIGURE");
        }
        wcscat_s(wc_figures_filename, wc_figures_number);
        wcscat_s(wc_figures_filename, L".LBX");
        // wprintf(L"wc_figures_filename: %s\n", wc_figures_filename);

        LPBYTE paFileData;
        DWORD cbFileData;
        // LoadFile(szFileName, &paFileData, &cbFileData);
        WinAPI_Load_File_W(wc_figures_filename, &paFileData, &cbFileData);

        entry_count = (int16_t)paFileData[0] | ((int16_t)paFileData[1] << 8);
        // printf("entry_count: %d\n", entry_count);

        string_table_ofst = 0x0200;
        string_table_record = (struct s_LBX_STR_TBL_REC*)(paFileData + string_table_ofst);

        for (itr_entry_count = 0; itr_entry_count < entry_count; itr_entry_count++)
        {
            // Unit Type Manifest Constants / Abbreviations
            // char _figures_names[1728][9];

            // printf("string_table_record[%u].name: %s\n", itr_entry_count, string_table_record[itr_entry_count].name);
            strcpy(_figures_names[itr_figures_names], string_table_record[itr_entry_count].name);

            // strcpy(curr_entry_name, string_table_record[itr_entry_count].name);
            // // strcpy(next_entry_name, string_table_record[itr_entry_count+1].name);
            if (strcmp(curr_entry_name, string_table_record[itr_entry_count].name) != 0)
            {
                strcpy(curr_entry_name, string_table_record[itr_entry_count].name);
                printf("curr_entry_name: %s\n", curr_entry_name);

                printf("itr_distinct_figures_names: %u\n", itr_distinct_figures_names);
                strcpy(_distinct_figures_names[itr_distinct_figures_names], string_table_record[itr_entry_count].name);
                itr_distinct_figures_names++;

            }

            itr_figures_names++;
        }


        delete paFileData;
    }
}

void Figures_String_Table(void)
{
    LPBYTE paFileData;
    DWORD cbFileData;
    WCHAR szFileName[MAX_PATH] = L"FIGURES1.LBX";
    int16_t entry_count;
    uint16_t string_table_ofst;
    // char* entry_name;
    // char* entry_description;
    struct s_LBX_STR_TBL_REC* string_table_entry;

    WinAPI_Load_File_W(szFileName, &paFileData, &cbFileData);

    wprintf(L"paFileData[0]: %02X\n", paFileData[0]);
    wprintf(L"paFileData[1]: %02X\n", paFileData[1]);

    entry_count = 0xFFFF;
    wprintf(L"entry_count: %04X\n", entry_count);
    wprintf(L"entry_count: %d\n", entry_count);
    // entry_count = (int16_t)paFileData[0];
    // entry_count = (int16_t)paFileData[1];
    // entry_count = (int16_t)paFileData[0] | 0xFF;
    // entry_count = (int16_t)paFileData[0] | 0x11 << 8;
    // entry_count = (int16_t)paFileData[0] << 8 | 0x11;
    entry_count = (int16_t)paFileData[0] | ((int16_t)paFileData[1] << 8);
    wprintf(L"entry_count: %04X\n", entry_count);
    wprintf(L"entry_count: %d\n", entry_count);

    // SizeOf Entry Data Offset Table: entry_count * sizeof(uint16_t)
    wprintf(L"SizeOf Entry Data Offset Table: %d\n", entry_count * sizeof(uint16_t));

    // Header Size ~== 1st Entry Data Offset 0x00001100

    // Offset of String Table = SizeOf LBX_HEADER + SizeOf Entry Data Offset Table
    string_table_ofst = 8 + (entry_count * sizeof(uint16_t));
    wprintf(L"string_table_ofst: %04X\n", string_table_ofst);
    // 0x00000200
    string_table_ofst = 0x0200;
    // entry_name = (paFileData + string_table_ofst + 0);
    // entry_description = (paFileData + string_table_ofst + 8 + 1);
    printf("entry_name: %s\n", (paFileData + string_table_ofst + 0));
    printf("entry_description: %s\n", (paFileData + string_table_ofst + 9));

    string_table_entry = (struct s_LBX_STR_TBL_REC* )(paFileData + string_table_ofst + 0);
    printf("string_table_entry->name: %s\n", string_table_entry->name);

    delete paFileData;
}


void Load_Unit_Type_Names(void)
{
    LPBYTE paFileData;
    DWORD cbFileData;
    WCHAR szFileName[MAX_PATH] = L"WZD_DSEG.BIN";

    WinAPI_Load_File_W(szFileName, &paFileData, &cbFileData);

    _unit_types_names = (uint8_t *)(paFileData + unit_type_name_ofst);

    // delete paFileData;
}

void Load_Unit_Type_Structures(void)
{
    LPBYTE paFileData;
    DWORD cbFileData;
    WCHAR szFileName[MAX_PATH] = L"WZD_DSEG.BIN";

    WinAPI_Load_File_W(szFileName, &paFileData, &cbFileData);

    _unit_types_structures = (uint8_t*)(paFileData + unit_type_structures_ofst);

    // delete paFileData;
}

void Build_Unit_Type_Names(void)
{
    char * szTextFile;
    char szTextLine[256];
    int itr_unit_type_names;
    char unit_type_name_number[1 + 10 + 1];
    char* rvr_unit_type_names;
    char tmp_unit_type_name[256];
    char* rvr_unit_type_name;
    int itr_unit_type_name;

    szTextFile = szUnitTypeNames;

    rvr_unit_type_names = (char *)&_unit_types_names[0];

    strcpy(szTextFile, "");

    sprintf(szTextLine, "\n");
    strcat(szTextFile, szTextLine);

    /*
    char unit_name_000[] = "Dwarf";
    ...
    char unit_name_119[] = "Nagas";
    */
    for (itr_unit_type_names = 0; itr_unit_type_names < 120; itr_unit_type_names++)
    {
        sprintf(szTextLine, "%s", "char unit_type_name_");
        strcat(szTextFile, szTextLine);

        itoa(itr_unit_type_names, unit_type_name_number, 10);
        // left pad zeros
        if (strlen(unit_type_name_number) == 1)
        {
            sprintf(szTextLine, "%s", "00");
            strcat(szTextFile, szTextLine);
        }
        if (strlen(unit_type_name_number) == 2)
        {
            sprintf(szTextLine, "%s", "0");
            strcat(szTextFile, szTextLine);
        }

        sprintf(szTextLine, "%s", unit_type_name_number);
        strcat(szTextFile, szTextLine);

        sprintf(szTextLine, "%s", "[] = \"");
        strcat(szTextFile, szTextLine);


        /*
        rvr_unit_type_name = &tmp_unit_type_name[0];
        while (*rvr_unit_type_names != '\0')
        {
            *rvr_unit_type_name = *rvr_unit_type_names;
            rvr_unit_type_names++;
            rvr_unit_type_name++;
        }
        rvr_unit_type_names++;
        */
        itr_unit_type_name = 0;
        while (*rvr_unit_type_names != '\0') {
            tmp_unit_type_name[itr_unit_type_name] = *rvr_unit_type_names;
            rvr_unit_type_names++;
            itr_unit_type_name++;
        }
        tmp_unit_type_name[itr_unit_type_name] = '\0';
        rvr_unit_type_names++;
        sprintf(szTextLine, "%s", tmp_unit_type_name);
        strcat(szTextFile, szTextLine);

        sprintf(szTextLine, "%s", "\";");
        strcat(szTextFile, szTextLine);

        sprintf(szTextLine, "\n");
        strcat(szTextFile, szTextLine);
    }

    sprintf(szTextLine, "\n");
    strcat(szTextFile, szTextLine);
    sprintf(szTextLine, "\n");
    strcat(szTextFile, szTextLine);

    /*
    char* _unit_names[] =
    {
        unit_name_000,
        ...
        unit_name_119
    }
    */

    sprintf(szTextLine, "%s", "char * _unit_type_names[] =");
    strcat(szTextFile, szTextLine);
    sprintf(szTextLine, "\n");
    strcat(szTextFile, szTextLine);
    sprintf(szTextLine, "%s", "{");
    strcat(szTextFile, szTextLine);
    sprintf(szTextLine, "\n");
    strcat(szTextFile, szTextLine);

    for (itr_unit_type_names = 0; itr_unit_type_names < 120; itr_unit_type_names++)
    {
        sprintf(szTextLine, "    ");
        strcat(szTextFile, szTextLine);

        sprintf(szTextLine, "%s", "unit_type_name_");
        strcat(szTextFile, szTextLine);

        itoa(itr_unit_type_names, unit_type_name_number, 10);
        // left pad zeros
        if (strlen(unit_type_name_number) == 1)
        {
            sprintf(szTextLine, "%s", "00");
            strcat(szTextFile, szTextLine);
        }
        if (strlen(unit_type_name_number) == 2)
        {
            sprintf(szTextLine, "%s", "0");
            strcat(szTextFile, szTextLine);
        }
        sprintf(szTextLine, "%s", unit_type_name_number);
        strcat(szTextFile, szTextLine);

        if (itr_unit_type_names < 120 - 1)
        {
            sprintf(szTextLine, "%s", ",");
            strcat(szTextFile, szTextLine);
        }

        sprintf(szTextLine, "\n");
        strcat(szTextFile, szTextLine);
    }

    sprintf(szTextLine, "%s", "};");
    strcat(szTextFile, szTextLine);
    sprintf(szTextLine, "\n");
    strcat(szTextFile, szTextLine);
}


void Build_Unit_Type_Structures(void)
{
    char * szTextFile;
    char szTextLine[256];
    struct s_UNIT_TYPE* unit_type_structure;
    int itr_unit_types;
    int itr_unit_type_names;
    uint16_t unit_type_structure_name_idx;

    szTextFile = szUnitTypeStructures;


    strcpy(szTextFile, "");
    sprintf(szTextLine, "\n");
    strcat(szTextFile, szTextLine);
    sprintf(szTextLine, "%s", "struct s_UNIT_TYPE _unit_type_table[] =");
    strcat(szTextFile, szTextLine);
    sprintf(szTextLine, "\n");
    strcat(szTextFile, szTextLine);
    sprintf(szTextLine, "%s", "{");
    strcat(szTextFile, szTextLine);
    sprintf(szTextLine, "\n");
    strcat(szTextFile, szTextLine);


    for (itr_unit_types = 0; itr_unit_types < UNIT_TYPE_STRUCTURE_COUNT; itr_unit_types++)
    {
        sprintf(szTextLine, "%s", "{");
        strcat(szTextFile, szTextLine);

        unit_type_structure = (struct s_UNIT_TYPE*)(_unit_types_structures + (sizeof(struct s_UNIT_TYPE) * itr_unit_types));

        // sprintf(
        //     szTextLine,
        //     "0x%04X, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, 0x%04X, %d, %d, %d, %d, %d, %d, 0x%04X, 0x%04X, 0x%04X, 0x%04X, 0x%04X, %d",
        //     unit_type_structure->Name, unit_type_structure->Melee, unit_type_structure->Ranged, unit_type_structure->Ranged_Type, unit_type_structure->Ammo, unit_type_structure->To_Hit, unit_type_structure->Defense, unit_type_structure->Resist, unit_type_structure->Move_Halves, unit_type_structure->Cost, unit_type_structure->Upkeep, unit_type_structure->Race, unit_type_structure->Bldng1_or_Portrait, unit_type_structure->Bldng2_or_HeroType, unit_type_structure->pict_seg, unit_type_structure->Hits, unit_type_structure->Sight, unit_type_structure->Transport, unit_type_structure->Figures, unit_type_structure->Construction, unit_type_structure->Spec_Att_Attr, unit_type_structure->Move_Flags, unit_type_structure->Attribs_1, unit_type_structure->Attribs_2, unit_type_structure->Abilities, unit_type_structure->Attack_Flags, unit_type_structure->Sound
        // );

        // unit_type_structure->Name with _unit_names_table[unit_type_idx]
        // loop through _unit_type_names_offsets, match offset value
        for (itr_unit_type_names = 0; itr_unit_type_names < UNIT_TYPE_NAME_COUNT; itr_unit_type_names++)
        {
            if (unit_type_structure->Name == _unit_type_names_offsets[itr_unit_type_names].ofst)
            {
                unit_type_structure_name_idx = _unit_type_names_offsets[itr_unit_type_names].idx;
            }
        }

        // sprintf(
        //     szTextLine,
        //     "&_unit_type_names[%d], %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, 0x%04X, %d, %d, %d, %d, %d, %d, 0x%04X, 0x%04X, 0x%04X, 0x%04X, 0x%04X, %d",
        //     unit_type_structure_name_idx, unit_type_structure->Melee, unit_type_structure->Ranged, unit_type_structure->Ranged_Type, unit_type_structure->Ammo, unit_type_structure->To_Hit, unit_type_structure->Defense, unit_type_structure->Resist, unit_type_structure->Move_Halves, unit_type_structure->Cost, unit_type_structure->Upkeep, unit_type_structure->Race, unit_type_structure->Bldng1_or_Portrait, unit_type_structure->Bldng2_or_HeroType, unit_type_structure->pict_seg, unit_type_structure->Hits, unit_type_structure->Sight, unit_type_structure->Transport, unit_type_structure->Figures, unit_type_structure->Construction, unit_type_structure->Spec_Att_Attr, unit_type_structure->Move_Flags, unit_type_structure->Attribs_1, unit_type_structure->Attribs_2, unit_type_structure->Abilities, unit_type_structure->Attack_Flags, unit_type_structure->Sound
        // );

        // hard-code SAMB_ptr unit_type_structure->pict_seg to NULL
        sprintf(
            szTextLine,
            "&_unit_type_names[%d], %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, 0x%04X, %d, %d, %d, %d, %d, %d, 0x%04X, 0x%04X, 0x%04X, 0x%04X, 0x%04X, %d",
            unit_type_structure_name_idx, unit_type_structure->Melee, unit_type_structure->Ranged, unit_type_structure->Ranged_Type, unit_type_structure->Ammo, unit_type_structure->To_Hit, unit_type_structure->Defense, unit_type_structure->Resist, unit_type_structure->Move_Halves, unit_type_structure->Cost, unit_type_structure->Upkeep, unit_type_structure->Race, unit_type_structure->Bldng1_or_Portrait, unit_type_structure->Bldng2_or_HeroType, 0, unit_type_structure->Hits, unit_type_structure->Sight, unit_type_structure->Transport, unit_type_structure->Figures, unit_type_structure->Construction, unit_type_structure->Spec_Att_Attr, unit_type_structure->Move_Flags, unit_type_structure->Attribs_1, unit_type_structure->Attribs_2, unit_type_structure->Abilities, unit_type_structure->Attack_Flags, unit_type_structure->Sound
        );
        strcat(szTextFile, szTextLine);

        sprintf(szTextLine, "%s", "}, ");
        strcat(szTextFile, szTextLine);
        sprintf(szTextLine, "\n");
        strcat(szTextFile, szTextLine);
    }


    sprintf(szTextLine, "%s", "};");
    strcat(szTextFile, szTextLine);
    sprintf(szTextLine, "\n");
    strcat(szTextFile, szTextLine);

}


/*
    get the Unit Type Name Array offset values from the Unit Type Structures Array
    unique() & sort()
    build array of offset and idx
    ...
    elsewhere, replace value with unit_type_names[unit_type_idx]
*/
void Build_Unit_Type_Names_Offsets(void)
{
    struct s_UNIT_TYPE* unit_type_structure;
    int itr_unit_types;
    int itr_unit_type_names;
    uint16_t* unique_unit_type_name_ofst;
    uint64_t unique_unit_type_name_ofst_count;


    uint16_t unit_type_name_ofst[UNIT_TYPE_STRUCTURE_COUNT];
    struct s_UNIT_TYPE* _unit_type_table;

    unit_type_structure = (struct s_UNIT_TYPE*)_unit_types_structures;

    for (itr_unit_types = 0; itr_unit_types < UNIT_TYPE_STRUCTURE_COUNT; itr_unit_types++)
    {
        unit_type_name_ofst[itr_unit_types] = unit_type_structure[itr_unit_types].Name;
    }

    // print_array_uint16(unit_type_name_ofst, unit_type_count);

    unique_array_uint16(unit_type_name_ofst, UNIT_TYPE_STRUCTURE_COUNT, &unique_unit_type_name_ofst, &unique_unit_type_name_ofst_count);

    // print_array_uint16(unique_unit_type_name_ofst, unique_unit_type_name_ofst_count);

    // if (NULL != unique_unit_type_name_ofst)
    // {
    //     free(unique_unit_type_name_ofst);
    // }

    // uint16_t* sorted_unique_unit_type_name_ofst;
    // uint64_t sorted_unique_unit_type_name_ofst_count;
    // sorted_unique_array_uint16(unit_type_name_ofst, unit_type_count, &unique_unit_type_name_ofst, &unique_unit_type_name_ofst_count);

    Bubble_Sort_uint16(unique_unit_type_name_ofst, unique_unit_type_name_ofst_count);
    // DEBUG: swapped: array[30] > array[31]
    // swap_count: 1
    // array[30]: 0x22D5
    // array[31]: 0x22CE

    //print_array_uint16(unique_unit_type_name_ofst, unique_unit_type_name_ofst_count);


    for (itr_unit_type_names = 0; itr_unit_type_names < UNIT_TYPE_NAME_COUNT; itr_unit_type_names++)
    {
        _unit_type_names_offsets[itr_unit_type_names].idx = itr_unit_type_names;
        _unit_type_names_offsets[itr_unit_type_names].ofst = unique_unit_type_name_ofst[itr_unit_type_names];
    }


    if (NULL != unique_unit_type_name_ofst)
    {
        free(unique_unit_type_name_ofst);
    }

}

void Replace_Unit_Type_Name_Offsets(void)
{
    struct s_UNIT_TYPE* unit_type_structures;
    int itr_unit_type_structures;
    int itr_unit_type_names;

    unit_type_structures = (struct s_UNIT_TYPE*)_unit_types_structures;


    for (itr_unit_type_structures = 0; itr_unit_type_structures < UNIT_TYPE_STRUCTURE_COUNT; itr_unit_type_structures++)
    {
        for (itr_unit_type_names = 0; itr_unit_type_names < UNIT_TYPE_NAME_COUNT; itr_unit_type_names++)
        {
            if (unit_type_structures[itr_unit_type_structures].Name == _unit_type_names_offsets[itr_unit_type_names].ofst)
            {
                // unit_type_structures[itr_unit_type_structures].Name == 
            }
        }
    }

}


void Export_UNIT_DATA_H(void)
{
    HANDLE hFile;
    // DWORD dwBytesToWrite = (DWORD)strlen(szTextFile);
    char* szTextFile;
    DWORD dwBytesToWrite;
    DWORD dwBytesWritten = 0;
    BOOL bErrorFlag = FALSE;
    WCHAR szFileName[MAX_PATH] = L"__UNIT_DATA.H";


    hFile = CreateFile(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) { wprintf(L"Terminal failure: Unable to open file \"%s\" for write.\n", szFileName); }

    szTextFile = szUnitTypeNames;
    dwBytesToWrite = (DWORD)strlen(szTextFile);
    wprintf(L"Writing %d bytes to %s.\n", dwBytesToWrite, szFileName);
    bErrorFlag = WriteFile(hFile, szTextFile, dwBytesToWrite, &dwBytesWritten, NULL);

    szTextFile = szUnitTypeStructures;
    dwBytesToWrite = (DWORD)strlen(szTextFile);
    wprintf(L"Writing %d bytes to %s.\n", dwBytesToWrite, szFileName);
    bErrorFlag = WriteFile(hFile, szTextFile, dwBytesToWrite, &dwBytesWritten, NULL);

    CloseHandle(hFile);



    /*
        HANDLE hFile;
        char DataBuffer[] = "This is some test data to write to the file.";
        DWORD dwBytesToWrite = (DWORD)strlen(DataBuffer);
        DWORD dwBytesWritten = 0;
        BOOL bErrorFlag = FALSE;
        // LPCWSTR lpFileName;
        WCHAR szFileName[MAX_PATH] = L"MsWinApi_WriteTextTest.txt";

        hFile = CreateFile(szFileName,              // name of the write
                            GENERIC_WRITE,          // open for writing
                            0,                      // do not share
                            NULL,                   // default security
                            CREATE_NEW,             // create new file only
                            FILE_ATTRIBUTE_NORMAL,  // normal file
                            NULL);                  // no attr. template

        if (hFile == INVALID_HANDLE_VALUE)
        {
            // DisplayError(TEXT("CreateFile"));
            // _tprintf(TEXT("Terminal failure: Unable to open file \"%s\" for write.\n"), argv[1]);
            wprintf(L"Terminal failure: Unable to open file \"%s\" for write.\n", szFileName);
            // return;
        }

        // _tprintf(TEXT("Writing %d bytes to %s.\n"), dwBytesToWrite, argv[1]);
        wprintf(L"Writing %d bytes to %s.\n", dwBytesToWrite, szFileName);

        bErrorFlag = WriteFile(
            hFile,           // open file handle
            DataBuffer,      // start of data to write
            dwBytesToWrite,  // number of bytes to write
            &dwBytesWritten, // number of bytes that were written
            NULL);            // no overlapped structure

        if (FALSE == bErrorFlag)
        {
            // DisplayError(TEXT("WriteFile"));
            // printf("Terminal failure: Unable to write to file.\n");
            wprintf(L"Terminal failure: Unable to write to file.\n");
        }
        else
        {
            if (dwBytesWritten != dwBytesToWrite)
            {
                // This is an error because a synchronous write that results in
                // success (WriteFile returns TRUE) should write all data as
                // requested. This would not necessarily be the case for
                // asynchronous writes.
                // printf("Error: dwBytesWritten != dwBytesToWrite\n");
                wprintf(L"Error: dwBytesWritten != dwBytesToWrite\n");
            }
            else
            {
                // _tprintf(TEXT("Wrote %d bytes to %s successfully.\n"), dwBytesWritten, argv[1]);
                wprintf(L"Wrote %d bytes to %s successfully.\n", dwBytesWritten, szFileName);
            }
        }

        CloseHandle(hFile);
    */
}
