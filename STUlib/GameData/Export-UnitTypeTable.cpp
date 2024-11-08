
#include "Export-UnitTypeTable.hpp"



/*
WIZARDS.EXE

dseg:019C
2963C
36C3C


dseg:1D74
2B214
38814


38814 - 36C3C = 1BD8
7128

sizeof(s_UNIT)
// sizeof:  24h  36d

7128 / 36 = 198


38C73 - 2B673 = D600  54,784


dseg:21D3
0002B673
00038C73


0x0002963C

*/
/*
Unit Type Table

Unit Prototype + Race

120 Unit Prototype Names

198 Unit Type Records

loop through 198 s_UNIT structures

capture offset in DSEG to Unit Type Name

Dasm shows using _UNITS[].type to index Unit Type Table

Manual references "prototype", in regards to Hero Units

there is some odd code somewhere for getting at Unit Types that have a Magic Realm for their Race

*/

/*
dseg:21D9 42 61 72 62 61 72 69 61 6E 00                   cnst_Barbarian db 'Barbarian', 0; DATA XREF : dseg:_unit_type_tableo ...
dseg:2681 42 65 61 73 74 6D 65 6E 00                      cnst_Beastmen db 'Beastmen', 0; DATA XREF : dseg:_unit_race_tableo
dseg:268A 44 61 72 6B 20 45 6C 66 00                      cnst_DarkElf db 'Dark Elf', 0; DATA XREF : dseg:_unit_race_tableo
dseg:2693 44 77 61 72 76 65 6E 00                         cnst_Dwarven db 'Dwarven', 0; DATA XREF : dseg:_unit_race_tableo
dseg:269B 47 6E 6F 6C 6C 00                               cnst_Gnoll db 'Gnoll', 0; DATA XREF : dseg:_unit_race_tableo
dseg:26A1 48 61 6C 66 6C 69 6E 67 00                      cnst_Halfling db 'Halfling', 0; DATA XREF : dseg:_unit_race_tableo
dseg:26AA 48 69 67 68 20 45 6C 66 00                      cnst_HighElf db 'High Elf', 0; DATA XREF : dseg:_unit_race_tableo
dseg:26B3 48 69 67 68 20 4D 65 6E 00                      cnst_HighMen db 'High Men', 0; DATA XREF : dseg:_unit_race_tableo
dseg:26BC 4B 6C 61 63 6B 6F 6E 00                         cnst_Klackon db 'Klackon', 0; DATA XREF : dseg:_unit_race_tableo
dseg:26C4 4C 69 7A 61 72 64 6D 61 6E 00                   cnst_Lizardman db 'Lizardman', 0; DATA XREF : dseg:_unit_race_tableo
dseg:26CE 4E 6F 6D 61 64 00                               cnst_Nomad db 'Nomad', 0; DATA XREF : dseg:_unit_race_tableo
dseg:26D4 4F 72 63 00                                     cnst_Orc db 'Orc', 0; DATA XREF : dseg:_unit_race_tableo
dseg:26D8 54 72 6F 6C 6C 00                               cnst_Troll db 'Troll', 0; DATA XREF : dseg:_unit_race_tableo
*/
char race_name_00[] = "NONE";
char race_name_01[] = "Barbarian";
char race_name_02[] = "Beastmen";
char race_name_03[] = "Dark Elf";
char race_name_04[] = "Dwarven";
char race_name_05[] = "Gnoll";
char race_name_06[] = "Halfling";
char race_name_07[] = "High Elf";
char race_name_08[] = "High Men";
char race_name_09[] = "Klackon";
char race_name_10[] = "Lizardman";
char race_name_11[] = "Nomad";
char race_name_12[] = "Orc";
char race_name_13[] = "Troll";
char race_name_14[] = "ZERO";
char race_name_15[] = "Arcane";
char race_name_16[] = "Nature";
char race_name_17[] = "Sorcery";
char race_name_18[] = "Chaos";
char race_name_19[] = "Life";
char race_name_20[] = "Death";

char * race_names[] =
{
    race_name_00,
    race_name_01,
    race_name_02,
    race_name_03,
    race_name_04,
    race_name_05,
    race_name_06,
    race_name_07,
    race_name_08,
    race_name_09,
    race_name_10,
    race_name_11,
    race_name_12,
    race_name_13,
    race_name_14,
    race_name_15,
    race_name_16,
    race_name_17,
    race_name_18,
    race_name_19,
    race_name_20
};

// char race_names[21][10] = { "NONE", "Barbarian", "Beastmen", "Dark Elf", "Dwarven", "Gnoll", "Halfling", "High Elf", "High Men", "Klackon", "Lizardman", "Nomad", "Orc", "Troll", "ZERO", "Arcane", "Nature", "Sorcery", "Chaos", "Life", "Death" };

enum Race
{
    e_RaceName_NONE,
    e_RaceName_Barbarian,
    e_RaceName_Beastmen,
    e_RaceName_DarkElf,
    e_RaceName_Dwarven,
    e_RaceName_Gnoll,
    e_RaceName_Halfling,
    e_RaceName_HighElf,
    e_RaceName_HighMen,
    e_RaceName_Klackon,
    e_RaceName_Lizardman,
    e_RaceName_Nomad,
    e_RaceName_Orc,
    e_RaceName_Troll,
    e_RaceName_ZERO,
    e_RaceName_Arcane,
    e_RaceName_Nature,
    e_RaceName_Sorcery,
    e_RaceName_Chaos,
    e_RaceName_Life,
    e_RaceName_Death
};

//#define RC_NONE 0
//#define RC_BA 1
//#define RC_BE 2



struct s_UNITTYPE unit_prototype_table[NUM_UNIT_TYPES];



bool Export_Unit_Type_Table(void)
{
    FILE* output_file_pointer;
    struct s_OLD_UNITTYPE* rvr_unit_type;
    uint16_t unit_type_name_data_segment_offset;
    uint16_t itr;

    output_file_pointer = fopen("UnitTypeTable.H", "w");

    if (output_file_pointer == NULL) {
        printf("FAILURE: Could not open output file.\n");
        return false;
    }

    //for (itr = 0; itr < 198; itr++)
    //{
    //    unit_type_name_data_segment_offset = GET_2B_OFS((char*)rvr_unit_type_table, (itr * 36));
    //    printf("char unit_type_name_%d[] = \"%s\";\n", itr, (char*)(wzd_buf + WZD_OFS_DSEG + unit_type_name_data_segment_offset));
    //}



    fprintf(output_file_pointer, "/*\n");
    fprintf(output_file_pointer, "    Generated from WIZARDS.EXE\n");
    fprintf(output_file_pointer, "*/\n");

    fprintf(output_file_pointer, "\n");
    fprintf(output_file_pointer, "\n");
    fprintf(output_file_pointer, "\n");

    fprintf(output_file_pointer, "/*\n");
    fprintf(output_file_pointer, "    Unit Type Table\n");
    fprintf(output_file_pointer, "*/\n");

    fprintf(output_file_pointer, "\n");
    fprintf(output_file_pointer, "\n");
    fprintf(output_file_pointer, "\n");

    fprintf(output_file_pointer, "struct s_UNIT_TYPE _unit_type_table[] =\n");
    
    // {&_unit_type_names[0],     5,   0,  -1,   0,   0,   4,  10,   4, 100,   2,   4,  10,   0, 0x0000, 10,   1,   0,   1,   0,    0, 0x0041, 0x0000, 0x0000, 0x0000, 0x0000, 0}, 

    // fprintf(output_file_pointer, "{&_unit_type_names[0],     5,   0,  -1,   0,   0,   4,  10,   4, 100,   2,   4,  10,   0, 0x0000, 10,   1,   0,   1,   0,    0, 0x0041, 0x0000, 0x0000, 0x0000, 0x0000, 0}, \n");

    // 21D3  Dwarf
    // dseg:019C
    // Name, mel, rng, rt, amm, hit, def, res, mov, cos,  upk, rac, bl1, bl2, pict, hp, sco, tra, fig, con, spec,  mvfl,  att1, att2, abl,  atfl, snd
    // D321  05   00   FF  00   00   04   0A   04   6400  02   04   0A   00   0000  0A  01   00   01   00   00     4100   0000  0000  0000  0000  0000
    //rvr_unit_type = (struct s_UNIT_PROTOTYPE*)&unit_type_table[0];
    //unit_type_name_data_segment_offset = GET_2B_OFS((char*)rvr_unit_type, 0);
    //                             Name,      mel,  rng,  rt,  amm,  hit,  def, res,  mov, cos,  upk,  rac, bl1,  bl2, pict,  hp, sco, tra, fig, con, spec,  mvfl,  att1,  att2,   abl,  atfl, snd
    //fprintf(output_file_pointer, "{\"%s\",     %d,   %d,  %d,   %d,   %d,   %d,  %d,   %d,  %d,   %d,   %d,  %d,   %d, %02X,  %d,  %d,  %d,  %d,  %d,   %d,  %04X,  %04X,  %04X,  %04X,  %04X,  %d}, \n",
    //    (char*)(wzd_buf + WZD_OFS_DSEG + unit_type_name_data_segment_offset),
    //    rvr_unit_type->Melee,
    //    rvr_unit_type->Ranged,
    //    rvr_unit_type->Ranged_Type,
    //    rvr_unit_type->Ammo,
    //    rvr_unit_type->To_Hit,
    //    rvr_unit_type->Defense,
    //    rvr_unit_type->Resist,
    //    rvr_unit_type->Move_Halves,
    //    rvr_unit_type->Cost,
    //    rvr_unit_type->Upkeep,
    //    rvr_unit_type->Race,
    //    rvr_unit_type->Bldng1_or_Portrait,
    //    rvr_unit_type->Bldng2_or_HeroType,
    //    rvr_unit_type->pict_seg,
    //    rvr_unit_type->Hits,
    //    rvr_unit_type->Sight,
    //    rvr_unit_type->Transport,
    //    rvr_unit_type->Figures,
    //    rvr_unit_type->Construction,
    //    rvr_unit_type->Spec_Att_Attr,
    //    rvr_unit_type->Move_Flags,
    //    rvr_unit_type->Attribs_1,
    //    rvr_unit_type->Attribs_2,
    //    rvr_unit_type->Abilities,
    //    rvr_unit_type->Attack_Flags,
    //    rvr_unit_type->Sound
    //);

    for (itr = 0; itr < NUM_UNIT_TYPES; itr++)
    {
        // rvr_unit_type = (struct s_UNIT_PROTOTYPE*)&unit_type_table[1];
        rvr_unit_type = (struct s_OLD_UNITTYPE*)(unit_type_table + (itr * sizeof(s_UNITTYPE)));

        unit_type_name_data_segment_offset = GET_2B_OFS((char*)rvr_unit_type, 0);

        //                             Name,      mel,  rng,  rt,  amm,  hit,  def, res,  mov, cos,  upk,  rac, bl1,  bl2,   pict,  hp, sco, tra, fig, con, spec,    mvfl,    att1,    att2,     abl,    atfl, snd
        fprintf(output_file_pointer, "/*  %d  %d  %s  %s  */\n", itr, rvr_unit_type->Race, race_names[rvr_unit_type->Race], (char*)(wzd_buf + WZD_OFS_DSEG + unit_type_name_data_segment_offset));
        fprintf(output_file_pointer, "/* { Name,      mel,  rng,  rt,  amm,  hit,  def, res,  mov, cos,  upk,  rac, bl1,  bl2,   pict,  hp, sco, tra, fig, con, spec,    mvfl,    att1,    att2,     abl,    atfl, snd } */\n");
        fprintf(output_file_pointer, "{\"%s\",     %d,   %d,  %d,   %d,   %d,   %d,  %d,   %d,  %d,   %d,   %d,  %d,   %d, 0x%02X,  %d,  %d,  %d,  %d,  %d,   %d,  0x%04X,  0x%04X,  0x%04X,  0x%04X,  0x%04X,  %d}, \n",
            (char*)(wzd_buf + WZD_OFS_DSEG + unit_type_name_data_segment_offset),
            rvr_unit_type->Melee,
            rvr_unit_type->Ranged,
            rvr_unit_type->Ranged_Type,
            rvr_unit_type->Ammo,
            rvr_unit_type->To_Hit,
            rvr_unit_type->Defense,
            rvr_unit_type->Resist,
            rvr_unit_type->Move_Halves,
            rvr_unit_type->Cost,
            rvr_unit_type->Upkeep,
            rvr_unit_type->Race,
            rvr_unit_type->Bldng1_or_Portrait,
            rvr_unit_type->Bldng2_or_HeroType,
            rvr_unit_type->pict_seg,
            rvr_unit_type->Hits,
            rvr_unit_type->Sight,
            rvr_unit_type->Transport,
            rvr_unit_type->Figures,
            rvr_unit_type->Construction,
            rvr_unit_type->Spec_Att_Attr,
            rvr_unit_type->Move_Flags,
            rvr_unit_type->Attribs_1,
            rvr_unit_type->Attribs_2,
            rvr_unit_type->Abilities,
            rvr_unit_type->Attack_Flags,
            rvr_unit_type->Sound
        );

    }

    // for (itr = 0; itr < NUM_UNIT_TYPES; itr++)
    //for (itr = 0; itr < 3; itr++)
    //{
    //    fprintf(output_file_pointer, "/* %d */  %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %04X, %d, %d, %d, %d, %d, %04X, %04X, %04X, %04X, %04X, %04X, %04X\n",
    //        itr,
    //        /* wzd_utt_buf->Name, */
    //        unit_type_table[itr]->Melee,
    //        unit_type_table[itr]->Ranged,
    //        unit_type_table[itr]->Ranged_Type,
    //        unit_type_table[itr]->Ammo,
    //        unit_type_table[itr]->To_Hit,
    //        unit_type_table[itr]->Defense,
    //        unit_type_table[itr]->Resist,
    //        unit_type_table[itr]->Move_Halves,
    //        unit_type_table[itr]->Cost,
    //        unit_type_table[itr]->Upkeep,
    //        unit_type_table[itr]->Race,
    //        unit_type_table[itr]->Bldng1_or_Portrait,
    //        unit_type_table[itr]->Bldng2_or_HeroType,
    //        unit_type_table[itr]->pict_seg,
    //        unit_type_table[itr]->Hits,
    //        unit_type_table[itr]->Sight,
    //        unit_type_table[itr]->Transport,
    //        unit_type_table[itr]->Figures,
    //        unit_type_table[itr]->Construction,
    //        unit_type_table[itr]->Spec_Att_Attr,
    //        unit_type_table[itr]->Move_Flags,
    //        unit_type_table[itr]->Attribs_1,
    //        unit_type_table[itr]->Attribs_2,
    //        unit_type_table[itr]->Abilities,
    //        unit_type_table[itr]->Attack_Flags,
    //        unit_type_table[itr]->Sound
    //    );
    //}


fprintf(output_file_pointer, "};\n");



if (NULL != output_file_pointer)
{
    fclose(output_file_pointer);
}

    return true;
}
