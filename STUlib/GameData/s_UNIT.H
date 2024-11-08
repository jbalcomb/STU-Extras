#pragma once

#include <stdint.h>



// sizeof:  20h  32d
#pragma pack (2)
struct s_UNIT
{
    /* 00 */  int8_t  world_x;
    /* 01 */  int8_t  world_y;
    /* 02 */  int8_t  world_plane;
    /* 03 */  int8_t  owner_idx;    /* set to ST_UNDEFINED when a unit dies */
    /* 04 */  int8_t  HalfMoves_Max;
    /* 05 */  uint8_t type;         /* Unit Type Index |-> Unit Type Table */
    /* 06 */  int8_t  Hero_Slot;    /* index into _players[owner_idx].Heroes[] */
    /* 07 */  int8_t  Finished;     /* FALSE/TRUE; Unit is out of Movement Points or is Busy performing a Unit Action (Go-To, Build Road, Purify, etc.) */
    /* 08 */  int8_t  HMoves;       /* Movement Points, as half-points */
    /* 09 */  int8_t  dst_wx;       /* Go-To Destination World X */
    /* 0A */  int8_t  dst_wy;       /* Go-To Destination World Y */
    /* 0B */  int8_t  Status;
    /* 0C */  int8_t  Level;
    /* 0D */  /* PAD_BYTE  int8_t  Unused_0Dh; */
    /* 0E */  int16_t XP;
    /* 10 */  int8_t  Move_Failed;
    /* 11 */  int8_t  Damage;
    /* 12 */  int8_t  Draw_Priority;
    /* 13 */  /* PAD_BYTE  int8_t  Unknown_13h; */
    /* 14 */  int16_t In_Tower;     /* FALSE/TRUE;  Unit is in a Tower of Wizardry */
    /* 16 */  int8_t  Sight_Range;
    /* 17 */  int8_t  Mutations;    // ; enum MUT_FLAGS
    /* 18 */  int16_t Enchants_LO;  // ; enum UE_FLAGS_L
    /* 1A */  int16_t Enchants_HI;  // ; enum UE_FLAGS_H
    /* 1C */  int8_t  Rd_Constr_Left;
    /* 1D */  int8_t  Rd_From_X;
    /* 1E */  int8_t  Rd_From_Y;
    /* 1F */  /* PAD_BYTE  int8_t  Unknown_1Fh; */  /* padding - align to 2 bytes */
    /* 20 */
};
#pragma pack()   // n defaults to 8; equivalent to /Zp8
