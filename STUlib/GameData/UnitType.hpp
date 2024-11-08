#pragma once

#include "../STUlib.hpp"



// /* 00 */  char Name[];
void Get_UnitType_Name(uint8_t unit_type_idx, char** unittype_name);

// /* 02 */  int8_t   Melee;
void Get_UnitType_Melee(uint8_t unit_type_idx, int8_t* unittype_mbr_val);

// /* 03 */  int8_t   Ranged;
void Get_UnitType_Ranged(uint8_t unit_type_idx, int8_t* unittype_mbr_val);

// /* 04 */  int8_t   Ranged_Type;
void Get_UnitType_RangedType(uint8_t unit_type_idx, int8_t* unittype_mbr_val);

// /* 0D */  int8_t   Race;
void Get_UnitType_Race(uint8_t unit_type_idx, int8_t* unittype_race);

// /* 17 */  int8_t   Spec_Att_Attr;
void Get_UnitType_SpecAttAttr(uint8_t unit_type_idx, int8_t* unittype_mbr_val);

// /* 18 */  uint16_t Move_Flags;
void Get_UnitType_MoveFlags(uint8_t unit_type_idx, uint16_t* unittype_mbr_val);

// /* 1A */  uint16_t Attribs_1;
void Get_UnitType_Attribs1(uint8_t unit_type_idx, uint16_t* unittype_mbr_val);

// /* 1C */  uint16_t Attribs_2;
void Get_UnitType_Attribs2(uint8_t unit_type_idx, uint16_t* unittype_mbr_val);

// /* 1E */  uint16_t Abilities;
void Get_UnitType_Abilities(uint8_t unit_type_idx, uint16_t* unittype_mbr_val);

// /* 20 */  uint16_t Attack_Flags;
void Get_UnitType_AttackFlags(uint8_t unit_type_idx, uint16_t* unittype_mbr_val);



void Print_UnitType(uint8_t unit_type_idx);

void Print_UnitTypeName(uint8_t unit_type_idx);
