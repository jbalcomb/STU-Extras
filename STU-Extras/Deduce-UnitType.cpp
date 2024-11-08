/*

all values for all members of s_UNITTYPE

by data-type

¿ ALL vs. Hero, Normal, Summoned ?

*/

#include "Deduce-UnitType.hpp"



void Dedu_UTT_RangedType(void)
{
    // uint8_t unit_type_idx;
    uint8_t itr_unit_types;
    int8_t unittype_mbr_i8;
    int8_t unittype_mbr_i8_arr[NUM_UNIT_TYPES];
    uint8_t* unique_unittype_mbr_i8_arr;
    uint64_t unique_unittype_mbr_i8_arr_count;
    uint64_t itr_unique_unittype_mbr_i8_arr_count;

    for (itr_unit_types = 0; itr_unit_types < NUM_UNIT_TYPES; itr_unit_types++)
    {
        Get_UnitType_RangedType(itr_unit_types, &unittype_mbr_i8);
        unittype_mbr_i8_arr[itr_unit_types] = unittype_mbr_i8;
    }

    unique_array_uint8(unittype_mbr_i8_arr, NUM_UNIT_TYPES, &unique_unittype_mbr_i8_arr, &unique_unittype_mbr_i8_arr_count);
    Bubble_Sort_uint8(unique_unittype_mbr_i8_arr, unique_unittype_mbr_i8_arr_count);

    for (itr_unique_unittype_mbr_i8_arr_count = 0; itr_unique_unittype_mbr_i8_arr_count < unique_unittype_mbr_i8_arr_count; itr_unique_unittype_mbr_i8_arr_count++)
    {
        printf("%u", unique_unittype_mbr_i8_arr[itr_unique_unittype_mbr_i8_arr_count]);
        if (itr_unique_unittype_mbr_i8_arr_count < (unique_unittype_mbr_i8_arr_count - 1))
        {
            printf(", ");
        }
    }

}

void Dedu_UTT_Races(void)
{
    // uint8_t unit_type_idx;
    uint8_t itr_unit_types;
    int8_t unittype_race;
    int8_t unittype_races[NUM_UNIT_TYPES];
    uint8_t* unique_unittype_races;
    uint64_t unique_unittype_races_count;
    uint64_t itr_unique_unittype_races_count;

    for (itr_unit_types = 0; itr_unit_types < NUM_UNIT_TYPES; itr_unit_types++)
    {
        Get_UnitType_Race(itr_unit_types, &unittype_race);
        unittype_races[itr_unit_types] = unittype_race;
    }

    unique_array_uint8(unittype_races, NUM_UNIT_TYPES, &unique_unittype_races, &unique_unittype_races_count);
    Bubble_Sort_uint8(unique_unittype_races, unique_unittype_races_count);

    for (itr_unique_unittype_races_count = 0; itr_unique_unittype_races_count < unique_unittype_races_count; itr_unique_unittype_races_count++)
    {
        printf("%u", unique_unittype_races[itr_unique_unittype_races_count]);
        if (itr_unique_unittype_races_count < (unique_unittype_races_count - 1))
        {
            printf(", ");
        }
    }

}

// /* 17 */  int8_t   Spec_Att_Attr;
void Dedu_UTT_SpecAttAttr(void)
{
    // uint8_t unit_type_idx;
    uint8_t itr_unit_types;
    int8_t unittype_race;
    int8_t unittype_races[NUM_UNIT_TYPES];
    uint8_t* unique_unittype_races;
    uint64_t unique_unittype_races_count;
    uint64_t itr_unique_unittype_races_count;

    for (itr_unit_types = 0; itr_unit_types < NUM_UNIT_TYPES; itr_unit_types++)
    {
        Get_UnitType_SpecAttAttr(itr_unit_types, &unittype_race);
        unittype_races[itr_unit_types] = unittype_race;
    }

    unique_array_uint8(unittype_races, NUM_UNIT_TYPES, &unique_unittype_races, &unique_unittype_races_count);
    Bubble_Sort_uint8(unique_unittype_races, unique_unittype_races_count);

    for (itr_unique_unittype_races_count = 0; itr_unique_unittype_races_count < unique_unittype_races_count; itr_unique_unittype_races_count++)
    {
        printf("%u", unique_unittype_races[itr_unique_unittype_races_count]);
        if (itr_unique_unittype_races_count < (unique_unittype_races_count - 1))
        {
            printf(", ");
        }
    }

}

// /* 18 */  uint16_t Move_Flags;
void Dedu_UTT_MoveFlags(void)
{
    // uint8_t unit_type_idx;
    uint8_t itr_unit_types;
    uint16_t unittype_val;
    uint16_t unittype_arr[NUM_UNIT_TYPES];
    uint16_t* set_arr;
    uint64_t set_cnt;
    uint64_t itr_set_cnt;

    for (itr_unit_types = 0; itr_unit_types < NUM_UNIT_TYPES; itr_unit_types++)
    {
        Get_UnitType_MoveFlags(itr_unit_types, &unittype_val);
        unittype_arr[itr_unit_types] = unittype_val;
    }

    unique_array_uint16(unittype_arr, NUM_UNIT_TYPES, &set_arr, &set_cnt);
    Bubble_Sort_uint16(set_arr, set_cnt);

    for (itr_set_cnt = 0; itr_set_cnt < set_cnt; itr_set_cnt++)
    {
        printf("%u", set_arr[itr_set_cnt]);
        if (itr_set_cnt < (set_cnt - 1))
        {
            printf(", ");
        }
    }

}

// /* 1A */  uint16_t Attribs_1;
void Dedu_UTT_Attribs1(void)
{
    // uint8_t unit_type_idx;
    uint8_t itr_unit_types;
    uint16_t unittype_val;
    uint16_t unittype_arr[NUM_UNIT_TYPES];
    uint16_t* set_arr;
    uint64_t set_cnt;
    uint64_t itr_set_cnt;

    for (itr_unit_types = 0; itr_unit_types < NUM_UNIT_TYPES; itr_unit_types++)
    {
        Get_UnitType_Attribs1(itr_unit_types, &unittype_val);
        unittype_arr[itr_unit_types] = unittype_val;
    }

    unique_array_uint16(unittype_arr, NUM_UNIT_TYPES, &set_arr, &set_cnt);
    Bubble_Sort_uint16(set_arr, set_cnt);

    for (itr_set_cnt = 0; itr_set_cnt < set_cnt; itr_set_cnt++)
    {
        printf("%u", set_arr[itr_set_cnt]);
        if (itr_set_cnt < (set_cnt - 1))
        {
            printf(", ");
        }
    }

}

// /* 1C */  uint16_t Attribs_2;
void Dedu_UTT_Attribs2(void)
{
    // uint8_t unit_type_idx;
    uint8_t itr_unit_types;
    uint16_t unittype_val;
    uint16_t unittype_arr[NUM_UNIT_TYPES];
    uint16_t* set_arr;
    uint64_t set_cnt;
    uint64_t itr_set_cnt;

    for (itr_unit_types = 0; itr_unit_types < NUM_UNIT_TYPES; itr_unit_types++)
    {
        Get_UnitType_Attribs2(itr_unit_types, &unittype_val);
        unittype_arr[itr_unit_types] = unittype_val;
    }

    unique_array_uint16(unittype_arr, NUM_UNIT_TYPES, &set_arr, &set_cnt);
    Bubble_Sort_uint16(set_arr, set_cnt);

    for (itr_set_cnt = 0; itr_set_cnt < set_cnt; itr_set_cnt++)
    {
        printf("%u", set_arr[itr_set_cnt]);
        if (itr_set_cnt < (set_cnt - 1))
        {
            printf(", ");
        }
    }

}

// /* 1E */  uint16_t Abilities;
void Dedu_UTT_Abilities(void)
{
    // uint8_t unit_type_idx;
    uint8_t itr_unit_types;
    uint16_t unittype_val;
    uint16_t unittype_arr[NUM_UNIT_TYPES];
    uint16_t* set_arr;
    uint64_t set_cnt;
    uint64_t itr_set_cnt;

    for (itr_unit_types = 0; itr_unit_types < NUM_UNIT_TYPES; itr_unit_types++)
    {
        Get_UnitType_Abilities(itr_unit_types, &unittype_val);
        unittype_arr[itr_unit_types] = unittype_val;
    }

    unique_array_uint16(unittype_arr, NUM_UNIT_TYPES, &set_arr, &set_cnt);
    Bubble_Sort_uint16(set_arr, set_cnt);

    for (itr_set_cnt = 0; itr_set_cnt < set_cnt; itr_set_cnt++)
    {
        printf("%u", set_arr[itr_set_cnt]);
        if (itr_set_cnt < (set_cnt - 1))
        {
            printf(", ");
        }
    }

}

// /* 20 */  uint16_t Attack_Flags;
void Dedu_UTT_AttackFlags(void)
{
    // uint8_t unit_type_idx;
    uint8_t itr_unit_types;
    uint16_t unittype_val;
    uint16_t unittype_arr[NUM_UNIT_TYPES];
    uint16_t* set_arr;
    uint64_t set_cnt;
    uint64_t itr_set_cnt;

    for (itr_unit_types = 0; itr_unit_types < NUM_UNIT_TYPES; itr_unit_types++)
    {
        Get_UnitType_AttackFlags(itr_unit_types, &unittype_val);
        unittype_arr[itr_unit_types] = unittype_val;
    }

    unique_array_uint16(unittype_arr, NUM_UNIT_TYPES, &set_arr, &set_cnt);
    Bubble_Sort_uint16(set_arr, set_cnt);

    for (itr_set_cnt = 0; itr_set_cnt < set_cnt; itr_set_cnt++)
    {
        printf("%u", set_arr[itr_set_cnt]);
        if (itr_set_cnt < (set_cnt - 1))
        {
            printf(", ");
        }
    }

}
