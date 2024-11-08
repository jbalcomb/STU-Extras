#pragma once

//#include <Windows.h>
//// winbase.h  GetCurrentDirectory()
//
//#include <iostream>
//#include <malloc.h>     /* malloc() */
//#include <stdio.h>      /* FILE; fclose(), fopen(), fread(), fseek(), printf(); */
//#include <stdlib.h>     /* itoa() */
//#include <string.h>     /* strcat(), strcpy() */
//
//
//
//#include "WZD_OFS.H"
//#include "s_UNITTYPE.H"
//#include "s_UNIT.H"
//
//#include "../../ReMoM/src/MoX_TYPE.H"
//#include <STU-Utilities/STU-Utilities.hpp>



/*
    NUM_
    OFS_ & LEN_
    ptr/rvr decl, defn, impl

    .H per struct

    hierarchy / order of



*/

/*
MAGIC.EXE

dseg:26E7  cnst_HighMen db 'High Men',0
dseg:3C58  cnst_Race08 db 'High Men',0


dseg:26B5 42 65 61 73 74 6D 65 6E 00                      cnst_Beastmen db 'Beastmen',0           ; DATA XREF: dseg:TBL_Raceso
dseg:26BE 44 61 72 6B 20 45 6C 66 00                      cnst_DarkElf db 'Dark Elf',0            ; DATA XREF: dseg:TBL_Raceso
dseg:26C7 44 77 61 72 76 65 6E 00                         cnst_Dwarven db 'Dwarven',0             ; DATA XREF: dseg:TBL_Raceso
dseg:26CF 47 6E 6F 6C 6C 00                               cnst_Gnoll db 'Gnoll',0                 ; DATA XREF: dseg:TBL_Raceso
dseg:26D5 48 61 6C 66 6C 69 6E 67 00                      cnst_Halfling db 'Halfling',0           ; DATA XREF: dseg:TBL_Raceso
dseg:26DE 48 69 67 68 20 45 6C 66 00                      cnst_HighElf db 'High Elf',0            ; DATA XREF: dseg:TBL_Raceso
dseg:26E7 48 69 67 68 20 4D 65 6E 00                      cnst_HighMen db 'High Men',0            ; DATA XREF: dseg:TBL_Raceso
dseg:26F0 4B 6C 61 63 6B 6F 6E 00                         cnst_Klackon db 'Klackon',0             ; DATA XREF: dseg:TBL_Raceso
dseg:26F8 4C 69 7A 61 72 64 6D 61 6E 00                   cnst_Lizardman db 'Lizardman',0         ; DATA XREF: dseg:TBL_Raceso
dseg:2702 4E 6F 6D 61 64 00                               cnst_Nomad db 'Nomad',0                 ; DATA XREF: dseg:TBL_Raceso
dseg:2708 4F 72 63 00                                     cnst_Orc db 'Orc',0                     ; DATA XREF: dseg:TBL_Raceso
dseg:270C 54 72 6F 6C 6C 00                               cnst_Troll db 'Troll',0                 ; DATA XREF: dseg:TBL_Raceso


dseg:3BEE 0A 3C 15 3C 1E 3C 29 3C 34 3C 3C 3C 43 3C 4D 3C+TBL_HoF_RaceStrings@ dw offset cnst_Race00,offset cnst_Race01,offset cnst_Race02,offset cnst_Race03,offset cnst_Race04,offset cnst_Race05,offset cnst_Race06,offset cnst_Race07,offset cnst_Race08,offset cnst_Race09,offset cnst_Race0A,offset cnst_Race0B,offset cnst_Race0C,offset cnst_Race0D
dseg:3BEE 58 3C 61 3C 6A 3C 74 3C 7B 3C 80 3C                                                     ; DATA XREF: Draw_Hall_Of_Fame_Screen+1CBr
dseg:3BEE                                                                                         ; "Barbarians"
dseg:3C0A 42 61 72 62 61 72 69 61 6E 73 00                cnst_Race00 db 'Barbarians',0           ; DATA XREF: dseg:TBL_HoF_RaceStrings@o
dseg:3C15 42 65 61 73 74 6D 65 6E 00                      cnst_Race01 db 'Beastmen',0             ; DATA XREF: dseg:TBL_HoF_RaceStrings@o
dseg:3C1E 44 61 72 6B 20 45 6C 76 65 73 00                cnst_Race02 db 'Dark Elves',0           ; DATA XREF: dseg:TBL_HoF_RaceStrings@o
dseg:3C29 44 72 61 63 6F 6E 69 61 6E 73 00                cnst_Race03 db 'Draconians',0           ; DATA XREF: dseg:TBL_HoF_RaceStrings@o
dseg:3C34 44 77 61 72 76 65 73 00                         cnst_Race04 db 'Dwarves',0              ; DATA XREF: dseg:TBL_HoF_RaceStrings@o
dseg:3C3C 47 6E 6F 6C 6C 73 00                            cnst_Race05 db 'Gnolls',0               ; DATA XREF: dseg:TBL_HoF_RaceStrings@o
dseg:3C43 48 61 6C 66 6C 69 6E 67 73 00                   cnst_Race06 db 'Halflings',0            ; DATA XREF: dseg:TBL_HoF_RaceStrings@o
dseg:3C4D 48 69 67 68 20 45 6C 76 65 73 00                cnst_Race07 db 'High Elves',0           ; DATA XREF: dseg:TBL_HoF_RaceStrings@o
dseg:3C58 48 69 67 68 20 4D 65 6E 00                      cnst_Race08 db 'High Men',0             ; DATA XREF: dseg:TBL_HoF_RaceStrings@o
dseg:3C61 4B 6C 61 63 6B 6F 6E 73 00                      cnst_Race09 db 'Klackons',0             ; DATA XREF: dseg:TBL_HoF_RaceStrings@o
dseg:3C6A 4C 69 7A 61 72 64 6D 65 6E 00                   cnst_Race0A db 'Lizardmen',0            ; DATA XREF: dseg:TBL_HoF_RaceStrings@o
dseg:3C74 4E 6F 6D 61 64 73 00                            cnst_Race0B db 'Nomads',0               ; DATA XREF: dseg:TBL_HoF_RaceStrings@o
dseg:3C7B 4F 72 63 73 00                                  cnst_Race0C db 'Orcs',0                 ; DATA XREF: dseg:TBL_HoF_RaceStrings@o
dseg:3C80 54 72 6F 6C 6C 73 00                            cnst_Race0D db 'Trolls',0               ; DATA XREF: dseg:TBL_HoF_RaceStrings@o


dseg:2207 44 77 61 72 66 00                               cnst_Dwarf db 'Dwarf',0                 ; DATA XREF: dseg:TBL_Unit_Typeso

*/

/*

Race

Race Id

Race Name

Race Attributes




'CityList Screen'

Get Unit Type Name
Get Unit Race Name

_UNITS[].type
_UNITS[].race


JSON Race Data is Array of Objects

JSON Object is Race (Type), with Race Id

Race Id is enum of values from Unit Type Data
~ {0, ..., 20}

Rade Id to Race Name is 
char race_names[21][10] = { "NONE", "Barbarian", "Beastmen", "Dark Elf", "Dwarven", "Gnoll", "Halfling", "High Elf", "High Men", "Klackon", "Lizardman", "Nomad", "Orc", "Troll", "ZERO", "Arcane", "Nature", "Sorcery", "Chaos", "Life", "Death" };


just need to get from value in unit race structure member to string for unit race name



*/
