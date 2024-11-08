
#include "UnitType.hpp"



// /* 00 */  char Name[];
void Get_UnitType_Name(uint8_t unit_type_idx, char ** unittype_name)
{
    struct s_UNITTYPE* rvr_utt;
    rvr_utt = (struct s_UNITTYPE*)(unit_type_table + unit_type_idx);
    *unittype_name = (char*)(wzd_dseg + rvr_utt->Name_Offset);
}

// /* 02 */  int8_t   Melee;
int8_t Get_UnitType_Melee(uint8_t unit_type_idx)
{
    //struct s_UNITTYPE* rvr_utt;
    //rvr_utt = (struct s_UNITTYPE*)(unit_type_table + unit_type_idx);
    //return rvr_utt->Melee;
    return unit_type_table[unit_type_idx].melee;
}

// /* 03 */  int8_t   Ranged;
void Get_UnitType_Ranged(uint8_t unit_type_idx, int8_t* unittype_mbr_val)
{
    struct s_UNITTYPE* rvr_utt;
    rvr_utt = (struct s_UNITTYPE*)(unit_type_table + unit_type_idx);
    *unittype_mbr_val = rvr_utt->ranged;
}

// /* 04 */  int8_t   Ranged_Type;
void Get_UnitType_RangedType(uint8_t unit_type_idx, int8_t* unittype_mbr_val)
{
    struct s_UNITTYPE* rvr_utt;
    rvr_utt = (struct s_UNITTYPE*)(unit_type_table + unit_type_idx);
    *unittype_mbr_val = rvr_utt->ranged_type;
}

// /* 0D */  int8_t   Race;
void Get_UnitType_Race(uint8_t unit_type_idx, int8_t * unittype_mbr_val)
{
    struct s_UNITTYPE* rvr_utt;
    rvr_utt = (struct s_UNITTYPE*)(unit_type_table + unit_type_idx);
    *unittype_mbr_val = rvr_utt->race_idx;
}

// /* 17 */  int8_t   Spec_Att_Attr;
void Get_UnitType_SpecAttAttr(uint8_t unit_type_idx, int8_t* unittype_mbr_val)
{
    struct s_UNITTYPE* rvr_utt;
    rvr_utt = (struct s_UNITTYPE*)(unit_type_table + unit_type_idx);
    *unittype_mbr_val = rvr_utt->spec_att_attr;
}

// /* 18 */  uint16_t Move_Flags;
void Get_UnitType_MoveFlags(uint8_t unit_type_idx, uint16_t* unittype_mbr_val)
{
    struct s_UNITTYPE* rvr_utt;
    rvr_utt = (struct s_UNITTYPE*)(unit_type_table + unit_type_idx);
    *unittype_mbr_val = rvr_utt->move_flags;
}

// /* 1A */  uint16_t Attribs_1;
void Get_UnitType_Attribs1(uint8_t unit_type_idx, uint16_t* unittype_mbr_val)
{
    struct s_UNITTYPE* rvr_utt;
    rvr_utt = (struct s_UNITTYPE*)(unit_type_table + unit_type_idx);
    *unittype_mbr_val = rvr_utt->attribs_1;
}

// /* 1C */  uint16_t Attribs_2;
void Get_UnitType_Attribs2(uint8_t unit_type_idx, uint16_t* unittype_mbr_val)
{
    struct s_UNITTYPE* rvr_utt;
    rvr_utt = (struct s_UNITTYPE*)(unit_type_table + unit_type_idx);
    *unittype_mbr_val = rvr_utt->attribs_2;
}

// /* 1E */  uint16_t Abilities;
void Get_UnitType_Abilities(uint8_t unit_type_idx, uint16_t* unittype_mbr_val)
{
    struct s_UNITTYPE* rvr_utt;
    rvr_utt = (struct s_UNITTYPE*)(unit_type_table + unit_type_idx);
    *unittype_mbr_val = rvr_utt->abilities;
}

// /* 20 */  uint16_t Attack_Flags;
void Get_UnitType_AttackFlags(uint8_t unit_type_idx, uint16_t* unittype_mbr_val)
{
    struct s_UNITTYPE* rvr_utt;
    rvr_utt = (struct s_UNITTYPE*)(unit_type_table + unit_type_idx);
    *unittype_mbr_val = rvr_utt->attack_flags;
}



void Print_UnitTypeName(uint8_t unit_type_idx)
{
    struct s_UNITTYPE* rvr_utt;
    uint16_t wzd_ds_ofs_unit_type_name;
    char* unittype_name;

    rvr_utt = (struct s_UNITTYPE*)(unit_type_table + unit_type_idx);

    wzd_ds_ofs_unit_type_name = rvr_utt->Name_Offset;

    unittype_name = (char *)(wzd_dseg + wzd_ds_ofs_unit_type_name);

    printf("unittype_name: %s\n", unittype_name);


    // printf("%04X\n", unit_type_table[unit_type_idx]->name);

}

void Print_UnitType(uint8_t unit_type_idx)
{

}
