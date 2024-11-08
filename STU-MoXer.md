
MoM

Code/Data Generator

Unit Names
Unit Data
FIGURES__.LBX
Manifest Constants
MGC DSEG
WZD DSEG





## Load Terrain Sprite Map
NOTE:   TERRAIN_000 is *special*
        The data does not start at the offset specificed for the LBX Entry.
So, ...
Load Entire File and offset at 0x0300
Load LBX Entry and offset at 0xC0
The way it works in-game is using TERRAIN_001 to index, 
  which treats index 2 as the 1st entry, 
  with the entire file loaded into EMM
? ~== Load_LBX_File() ... vs. Load_LBX_Entry() and Load_LBX_Record() ?
...
Generalization?  ? |-> 4C_Files ?
    just use C std?
    alloc and dealloc?
    file size?
...
    Palette?
...
    Back to the idea of just loading all of the files?
    or just embedding all of the data?
    or finding better boundaries for the API/STGE?





## Generate Unit Type Data

### Unit Type Names
    load WZD_DSEG
    sprintf names
    sprintf array of names

### Unit Type Structures
    load WZD_DSEG
    sprintf array of structures


char unit_name_000[] = "Dwarf";
...
char unit_name_119[] = "Nagas";

char * _unit_names[] =
{
    unit_name_000,
    ...
    unit_name_119
}




Load WZD DSEG
Load Unit Type Names
Load Unit Type Structures
Load Figures Names





unit type structures
unit type names
FIGURES string table names

build array of unit type names
build array of unit type structures
build manifest constants of FIGURES string table names

?!? Unit Data vs. Unit Type Data !?!


struct s_UNIT
    char * name

_unit_type[0]
    { unit_type_names[0], ... }

PoI?
    So, ...
        maybe, you want to know which static figure picture to draw?
            get the type from _UNITS[].type
            index into _unit_types[] for pict_seg
            type is set at creation/addition of unit to _UNITS
            *don't need to know the unit type here*







extract string tables from FIGURES__.LBX
    FIGURES1.LBX, ..., FIGURES16.LBX

extract name from string table entry
    42 52 41 58 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
    BRAX����������������������������

? 8 entries per unit ?
? entry counts ~== 120 ? ? 120 / 8 = 15 ? 15 units per LBX ?
120  FIGURES1.LBX, ..., FIGURES13.LBX
 24  FIGURES14.LBX  DJINN3, SKYDRAKE, NAGA
120  FIGURES15.LBX
 24  FIGURES16.LBX  AIRELEM2, DJINN3, SKYDRAKE

(120 * 13) + 24 + 120 + 24 = 1728 / 8 = 216






## MAGIC.EXE Data-Segment
MGC DSEG



## WIZARDS.EXE Data-Segment
WZD DSEG
WZD_DSEG.BIN
WZD_DSEG.C/.H

    IDA
        Fst:  0x36AA0
        Lst:    0x454F3

    EXE
        Begin:  0x294A0
        End:    0x37F00
        0x37F00 - 0x294A0 = 0xEA60   60,000 bytes

StartOffset(h): 00000000, EndOffset(h): 0000EA5F, Length(h): 0000EA60
