
#include "../STUlib.hpp"



uint16_t wzd_ds_ofs_unit_type_name[NUM_UNIT_TYPES];
// Get_Unit_Type_Name_Offset() - Offset being the offset to the c-string in the WIZARDS.EXE Data Segment



int GameData()
{
    // uint8_t unit_type_idx;
    // uint8_t itr_unit_types;
    // int8_t unittype_race;

    Load_MAGIC_EXE();
    Load_WIZARDS_EXE();
    Load_SAVE_GAM("SAVETEST.GAM");
    
    // unit_type_idx = 146;

    //// Print_UnitType(146);
    //// Print_UnitTypeName(unit_type_idx);
    //for (itr_unit_types = 0; itr_unit_types < NUM_UNIT_TYPES; itr_unit_types++)
    //{
    //    Print_UnitTypeName(itr_unit_types);
    //}
    

    //Print_UnitTypeName(unit_type_idx);
    //Get_UnitType_Race(unit_type_idx, &unittype_race);
    //printf("unittype_race: %d\n", unittype_race);
    

    
    // Dedu_UTT_RangedType();
    // Dedu_UTT_Races();
    // Dedu_UTT_SpecAttAttr();
    // Dedu_UTT_MoveFlags();
    // Dedu_UTT_Attribs1();
    // Dedu_UTT_Attribs2();
    // Dedu_UTT_Abilities();
    // Dedu_UTT_AttackFlags();



    return 0;
}
