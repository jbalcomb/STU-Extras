#pragma once

#include "../../ReMoM/src/MoX_TYPE.H"



//// sizeof: 18h  24d
//struct s_RACE
//{
//    union
//    {
//        /* 0x00 */ int16_t wzd_dseg_ofs_race_name;
//        struct
//        {
//            /* 0x00 */ int16_t Name;  // offset
//            /* 0x02 */ int16_t CantBuild_Count;
//            /* 0x04 */ int16_t CantBuild[7];  // enum BLD_TYPE
//            /* 0x12 */ int16_t Outpost_Growth_Pcnt;
//            /* 0x14 */ int16_t Growth_Mod;
//            /* 0x16 */ int16_t HouseType;
//            /* 0x18 */
//        };
//    };
//};

// sizeof: 18h  24d
#pragma pack (2)
struct s_RACE
{
    /* 0x00 */ int16_t Name;  // offset
    /* 0x02 */ int16_t CantBuild_Count;
    /* 0x04 */ int16_t CantBuild[7];  // enum BLD_TYPE
    /* 0x12 */ int16_t Outpost_Growth_Pcnt;
    /* 0x14 */ int16_t Growth_Mod;
    /* 0x16 */ int16_t HouseType;
    /* 0x18 */
};
#pragma pack()   // n defaults to 8; equivalent to /Zp8
