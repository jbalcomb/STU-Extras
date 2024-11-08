#pragma once

#include <stdint.h>



// sizeof:  24h  36d
#pragma pack (2)
struct s_UNITTYPE
{
    /* 00 */  /* char** Name; */                    /* offset (00036AA0) */
    uint16_t Name_Offset;
    /* 02 */  int8_t   melee;                   /* base 10 */
    /* 03 */  int8_t   ranged;                  /* base 10 */
    /* 04 */  int8_t   ranged_type;
    /* 05 */  int8_t   ammo;                    /* base 10 */
    /* 06 */  int8_t   to_hit;                  /* base 10 */
    /* 07 */  int8_t   defense;                 /* base 10 */
    /* 08 */  int8_t   resistance;                  /* base 10 */
    /* 09 */  int8_t   move2_points;            /* base 10 */
    /* 0A */  int16_t  cost;                    /* gold or mana */
    /* 0C */  int8_t   upkeep;                  /* base 10 */
    /* 0D */  int8_t   race_idx;                /* enum Race_Code */
    /* 0E */  // int8_t   Bldng1_or_Portrait;      /* enum BLD_TYPE */
    union
    {
        int8_t   required_building_1_idx;
        int8_t   hero_portrait;
    };
    /* 0F */  // int8_t   Bldng2_or_HeroType;      /* enum BLD_TYPE */
    union
    {
        int8_t   required_building_2_idx;
        int8_t   hero_type;
    };
    /* 10 */  /* SAMB_ptr pict_seg; */                /* segment pointers to LBX_Alloc_Space headers for reserved EMM file links */
    uint16_t pict_seg;
    /* 12 */  int8_t   hits;                    /* base 10 */
    /* 13 */  int8_t   scout_range;                   /* base 10 */
    /* 14 */  int8_t   carry_capacity;               /* base 10 */
    /* 15 */  int8_t   figure_count;                 /* base 10 */
    /* 16 */  int8_t   construction;            /* base 10 */
    /* 17 */  int8_t   spec_att_attr;           /* base 10 */
    /* 18 */  uint16_t move_flags;              /* enum MOVEFLAGS */  /* TODO: check the data-type - UnitHasFlying() uses `byte ptr` and `CBW` */
    /* 1A */  uint16_t attribs_1;               /* enum ATTRIB_1 */
    /* 1C */  uint16_t attribs_2;               /* enum ATTRIB_2 */
    /* 1E */  uint16_t abilities;               /* enum ABL_FLAGS */
    /* 20 */  uint16_t attack_flags;            /* enum ATK_FLAGS */
    /* 22 */  int16_t  sound;
    /* 24 */
};
#pragma pack()   // n defaults to 8; equivalent to /Zp8




// sizeof:  24h  36d
struct s_OLD_UNITTYPE
{
    /* 00 */  char** Name;                    /* offset (00036AA0) */
    /* 02 */  int8_t   Melee;                   /* base 10 */
    /* 03 */  int8_t   Ranged;                  /* base 10 */
    /* 04 */  int8_t   Ranged_Type;
    /* 05 */  int8_t   Ammo;                    /* base 10 */
    /* 06 */  int8_t   To_Hit;                  /* base 10 */
    /* 07 */  int8_t   Defense;                 /* base 10 */
    /* 08 */  int8_t   Resist;                  /* base 10 */
    /* 09 */  int8_t   Move_Halves;             /* base 10 */
    /* 0A */  int16_t  Cost;                    /* base 10 */
    /* 0C */  int8_t   Upkeep;                  /* base 10 */
    /* 0D */  int8_t   Race;                    /* enum Race_Code */
    /* 0E */  int8_t   Bldng1_or_Portrait;      /* enum BLD_TYPE */
    /* 0F */  int8_t   Bldng2_or_HeroType;      /* enum BLD_TYPE */
    // /* 10 */  SAMB_ptr pict_seg;                /* segment pointers to LBX_Alloc_Space headers for reserved EMM file links */
    uint16_t pict_seg;  // 16-bit Far Pointer
    /* 12 */  int8_t   Hits;                    /* base 10 */
    /* 13 */  int8_t   Sight;                   /* base 10 */
    /* 14 */  int8_t   Transport;               /* base 10 */
    /* 15 */  int8_t   Figures;                 /* base 10 */
    /* 16 */  int8_t   Construction;            /* base 10 */
    /* 17 */  int8_t   Spec_Att_Attr;           /* base 10 */
    /* 18 */  uint16_t Move_Flags;              /* enum MOVEFLAGS */  /* TODO: check the data-type - UnitHasFlying() uses `byte ptr` and `CBW` */
    /* 1A */  uint16_t Attribs_1;               /* enum ATTRIB_1 */
    /* 1C */  uint16_t Attribs_2;               /* enum ATTRIB_2 */
    /* 1E */  uint16_t Abilities;               /* enum ABL_FLAGS */
    /* 20 */  uint16_t Attack_Flags;            /* enum ATK_FLAGS */
    /* 22 */  int16_t  Sound;
};
