

/*
WIZARDS.EXE


Unit Type Names

dseg:21D3
2B673
38C73
cnst_Dwarf db 'Dwarf',0
...
...
...
dseg:267B
cnst_Nagas db 'Nagas',0
2BB21
39121
dseg:2681


dseg:2681 42 65 61 73 74 6D 65 6E 00                      cnst_Beastmen db 'Beastmen',0
...
...
...
dseg:26D8 54 72 6F 6C 6C 00                               cnst_Troll db 'Troll',0
2BB7E
3917E
dseg:26DE



Race Names
¿ only for 'Hall Of Fame' ?

2EE70





*/

/*
SimTex Race Name Abbreviatoins

Type         |BA|BE|DE|DR|DW|GN|HF|HE|HM|KL|LZ|NO|OC|TR|


BA  
BE  Beastmen
DE  Dark Elf
DR  Drak Elf
DW  Dwarven
GN  Gnoll
HF  Halfling
HE  High Elf
HM  High Men
KL  Klackon
LZ  Lizardmen
NO  Nomad
OC  Orc
TR  Troll


dseg:2681 42 65 61 73 74 6D 65 6E 00                      cnst_Beastmen db 'Beastmen',0
dseg:268A 44 61 72 6B 20 45 6C 66 00                      cnst_DarkElf db 'Dark Elf',0
dseg:2693 44 77 61 72 76 65 6E 00                         cnst_Dwarven db 'Dwarven',0
dseg:269B 47 6E 6F 6C 6C 00                               cnst_Gnoll db 'Gnoll',0
dseg:26A1 48 61 6C 66 6C 69 6E 67 00                      cnst_Halfling db 'Halfling',0
dseg:26AA 48 69 67 68 20 45 6C 66 00                      cnst_HighElf db 'High Elf',0
dseg:26B3 48 69 67 68 20 4D 65 6E 00                      cnst_HighMen db 'High Men',0
dseg:26BC 4B 6C 61 63 6B 6F 6E 00                         cnst_Klackon db 'Klackon',0
dseg:26C4 4C 69 7A 61 72 64 6D 61 6E 00                   cnst_Lizardman db 'Lizardman',0
dseg:26CE 4E 6F 6D 61 64 00                               cnst_Nomad db 'Nomad',0
dseg:26D4 4F 72 63 00                                     cnst_Orc db 'Orc',0
dseg:26D8 54 72 6F 6C 6C 00                               cnst_Troll db 'Troll',0



*/

/*
C:\STU\__XethNyrrow-Discord-MoMDosClassic-202310181535\MOM_1_2.TXT


---------------------------
Version 1.2 Chart Revisions
---------------------------

Building Chart:

                                RACES
Type         |BA|BE|DE|DR|DW|GN|HF|HE|HM|KL|LZ|NO|OC|TR|
--------------------------------------------------------
ALCHEMIST'S  | Y| Y| Y| Y| Y| N| Y| Y| Y| N| N| Y| Y| N|
--------------------------------------------------------
...
...
...


Units Chart:

                                RACES
TYPE          |BA|BE|DE|DR|DW|GN|HF|HE|HM|KL|LZ|NO|OC|TR|
---------------------------------------------------------
AIRSHIP       | N| N| N| Y| N| N| N| N| N| N| N| N| N| N|
---------------------------------------------------------
...
...
...


The following is a complete list of unrest modifiers according
to a wizard's capitol race (ex. If you have a capitol race of
barbarians, you will have 1 additional unrest in a city of dwarves
you have conquered).

      BA    BE    DE    DR    DW    GN    HF
BA:    0    +1    +1    +1    +1    +1    +1
BE:   +1     0    +2    +2    +2     0    +1
DE:   +1    +2     0    +2    +3    +2    +2
DR:   +1    +2    +2     0    +2    +1    +1
DW:   +1    +2    +3    +2     0    +1     0
GN:   +1     0    +2    +1    +1     0     0
HF:   +1    +1    +2    +1     0     0     0
HE:   +1    +2    +4    +1    +3    +2     0
HM:   +1    +1    +2    +1     0    +1     0
KL:   +2    +2    +2    +2    +2    +2    +2
LZ:   +1    +1    +2    +1    +1    +1    +1
NO:    0    +1    +2    +1    +1    +1     0
OC:    0    +1    +2    +1     0     0     0
TR:   +1    +2    +3    +2     0     0    +1

      HE    HM    KL    LZ    NO    OC    TR
BA:   +1    +1    +2    +1     0     0    +1
BE:   +2    +1    +2    +1    +1    +1    +2
DE:   +4    +2    +2    +2    +2    +2    +3
DR:   +1    +1    +2    +1    +1    +1    +2
DW:   +3     0    +2    +1     0    +3    +4
GN:   +1    +1    +2    +1    +1     0     0
HF:    0     0    +2     0     0     0     0
HE:    0     0    +2    +1     0    +2    +3
HM:    0     0    +2    +1     0     0    +1
KL:   +2    +2    -2    +2    +2    +2    +2
LZ:   +1    +1    +2     0    +1    +1    +1
NO:    0     0    +2    +1     0     0    +1
OC:    0     0    +2    +1     0     0     0
TR:   +1    +1    +2    +1    +1     0     0

*/

/*

char unit_type_name_000[] = "Dwarf";
...
...
...
char unit_type_name_119[] = "Nagas";

char * _unit_type_names[] =
{
    unit_type_name_000,
    ...
    ...
    ...
    unit_type_name_119
};

struct s_UNIT_TYPE _unit_type_table[] =
{
    {&_unit_type_names[0], 5, 0, -1, 0, 0, 4, 10, 4, 100, 2, 4, 10, 0, 0x0000, 10, 1, 0, 1, 0, 0, 0x0041, 0x0000, 0x0000, 0x0000, 0x0000, 0},
    ...
    ...
    ...
    {&_unit_type_names[119],   4,  0,  -1,   0,   1, 3, 7, 2, 120, 2, 17, 6, 0, 0x0000, 6, 1, 0, 2, 0, 4, 0x0004, 0x0000, 0x0000, 0x0001, 0x0006, 0},
};


dseg:019C

D3 21 05 00 FF 00 00 04 0A 04 64 00 02 04 0A 00 00 00 0A 01 00 01 00 00 41 00 00 00 00 00 00 00 00 00 00 00 

D9 21 05 03 64 00 00 03 06 04 64 00+


_unit_type_table
s_UNIT_TYPE <offset cnst_Dwarf,   5, 0, 0FFh, 0, 0,   4,  10,   4,   100,   2, R_Dwarf, < 0Ah>, 0, 0,  10,   1, 0,   1, 0, 0, M_Cavalry or M_Mntnr, 0, 0, 0, 0, 0>; 0
s_UNIT_TYPE <offset cnst_Barbarian,

dseg:019C 00 00 00 00 D9 21 05 03 64 00 00 03 06 04 64 00
s_UNIT_TYPE <offset cnst_Barbarian,   5,   3,  64h, 0, 0,   3,   6,   4,   100,   2, 0, < 11h>, 0,      1,   9,   1, 0,   1, 0, 0, M_Cavalry, 0, 0, 0, 0, 0>; 1 ; "Dwarf"

dseg:21D3 44 77 61 72 66 00                               cnst_Dwarf db 'Dwarf',0                 ; DATA XREF: dseg:_unit_type_tableo
dseg:21D9 42 61 72 62 61 72 69 61 6E 00                   cnst_Barbarian db 'Barbarian',0         ; DATA XREF: dseg:_unit_type_tableo

uint16_t
offset, from DSEG, to Unit Type Name


*/
