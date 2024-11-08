
#pragma once

#include <stdint.h>
typedef unsigned char* SAMB_ptr;
typedef unsigned char* byte_ptr;

/*
    Unit Type - Data Structure

sizeof:  24h  36d

*/
struct s_UNIT_TYPE
{
    /* 00 */ char* Name;                   /* ; offset (00036AA0) */
    /* 02 */ int8_t Melee;                  /* ; base 10 */
    /* 03 */ int8_t Ranged;                 /* ; base 10 */
    /* 04 */ int8_t Ranged_Type;
    /* 05 */ int8_t Ammo;                   /* ; base 10 */
    /* 06 */ int8_t To_Hit;                 /* ; base 10 */
    /* 07 */ int8_t Defense;                /* ; base 10 */
    /* 08 */ int8_t Resist;                 /* ; base 10 */
    /* 09 */ int8_t Move_Halves;            /* ; base 10 */
    /* 0A */ int16_t Cost;                  /* ; base 10 */
    /* 0C */ int8_t Upkeep;                 /* ; base 10 */
    /* 0D */ int8_t Race;                   /* ; enum Race_Code */
    /* 0E */ int8_t Bldng1_or_Portrait;     /* ; enum BLD_TYPE */
    /* 0F */ int8_t Bldng2_or_HeroType;     /* ; enum BLD_TYPE */
    /* 10 */ SAMB_ptr pict_seg;             /* ; segment pointers to LBX_Alloc_Space headers for reserved EMM file links */
    /* 12 */ int8_t Hits;                   /* ; base 10 */
    /* 13 */ int8_t Sight;                  /* ; base 10 */
    /* 14 */ int8_t Transport;              /* ; base 10 */
    /* 15 */ int8_t Figures;                /* ; base 10 */
    /* 16 */ int8_t Construction;           /* ; base 10 */
    /* 17 */ int8_t Spec_Att_Attr;          /* ; base 10 */
    /* 18 */ int16_t Move_Flags;            /* ; enum MOVEFLAGS */
    /* 1A */ int16_t Attribs_1;             /* ; enum ATTRIB_1 */
    /* 1C */ int16_t Attribs_2;             /* ; enum ATTRIB_2 */
    /* 1E */ int16_t Abilities;             /* ; enum ABL_FLAGS */
    /* 20 */ int16_t Attack_Flags;          /* ; enum ATK_FLAGS */
    /* 22 */ int16_t Sound;
};

#define BRAX             0
#define GUNTHER          1
#define ZALDRON          2
// #define B'SHAN           3
#define BSHAN           3
#define RAKIR            4
#define VALANA           5
#define BAHGTRU          6
#define SERENA           7
#define SHURI            8
#define THERIA           9
#define GREYFAIR         10
#define TAKI             11
#define REYWIND          12
#define MALLEUS          13
#define TUMU             14
#define JAER             15
#define MARCUS           16
#define FANG-1           17
#define MORGANA          18
#define AUREUS           19
#define SHIN_BO          20
#define SPYDER           21
#define SHALLA           22
#define YRAMRAG          23
#define X                24
#define AERIE            25
#define DETHSTRY         26
#define ELANA            27
#define ROLAND           28
#define MORTU            29
#define ALORRA           30
#define HAROLD           31
#define RAVASHAK         32
#define WARRAX           33
#define TORIN            34
#define TRIREME          35
#define GALLEY           36
#define CATAPULT         37
#define WARSHIP          38
#define BRBSPEAR         39
#define BRBSWORD         40
#define BRBBOW           41
#define BRBCAV           42
#define BRBSHAMN         43
#define SETTLERS         44
#define BRBBRSRK         45
#define BSTSPEAR         46
#define BSTSWORD         47
#define BSTHLBRD         48
#define BSTBOW           49
#define BSTPRST          50
#define BSTMAGE          51
#define BSTENGNR         52
#define SETTLERS         53
#define CENTAUR          54
#define MANTICOR         55
#define MINOTAUR         56
#define DRKSPEAR         57
#define DRKSWORD         58
#define DRKHALB          59
#define DKELFCAV         60
#define DKPRIEST         61
#define SETTLERS         62
#define NITEBLAD         63
#define DKWARLOC         64
#define NITEMARE         65
#define DRASPEAR         66
#define DRASWORD         67
#define DRAHALBE         68
#define DRABOW           69
#define DRASHAMA         70
#define DRAMAGIC         71
#define DRAENGIN         72
#define SETTLERS         73
#define DOOMDRAK         74
#define AIRSHIP          75
#define DWARSWOR         76
#define DWARHALB         77
#define DWARENGI         78
#define DWARFHAM         79
#define STMCANON         80
#define GOLEM            81
#define SETTLERS         82
#define GNOSPEAR         83
#define GNOSWORD         84
#define GNOHALBE         85
#define GNOBOW           86
#define SETTLERS         87
#define WOLFRDR3         88
#define HALSPEAR         89
#define HALSWORD         90
#define HALBOW           91
#define HALSHAMA         92
#define SETTLERS         93
#define HALSLING         94
#define HE2SPEAR         95
#define HE1SWORD         96
#define HE1HALB          97
#define HIELFCAV         98
#define ELFMAGIC         99
#define SETTLERS         100
#define HE2LONGB         101
#define ELFKNIGH         102
#define PEGASUS2         103
#define HMSPEAR          104
#define HMSWORD          105
#define HMBOW            106
#define HIMENCAV         107
#define HMPRIES2         108
#define HMMAGE           109
#define HMENGINE         110
#define SETTLERS         111
#define HMPIKE           112
#define HMPALADI         113
#define KLSPEAR          114
#define KLSWORD          115
#define KLHLBRD          116
#define KLENGNR          117
#define SETTLERS         118
#define STAGBTL          119
#define LIZSPEAR         120
#define LIZSWORD         121
#define LIZHALBE         122
#define LIZBOW           123
#define LIZSHAMA         124
#define SETTLERS         125
#define DRTURTLE         126
#define NOMSPEAR         127
#define NOMSWORD         128
#define NOMBOW           129
#define NMPRIEST         130
#define NOMMAGE          131
#define SETTLERS         132
#define NMBOWCAV         133
#define NOMPIKE          134
#define NOMRANGE         135
#define GRIFFON          136
#define ORCSPEAR         137
#define ORCSWORD         138
#define ORCHALB          139
#define ORCBOW           140
#define ORCCAV           141
#define ORPRIEST         142
#define ORCMAGIC         143
#define ORCENGIN         144
#define SETTLERS         145
#define WVRNRIDR         146
#define TRLSPEAR         147
#define TRLSWORD         148
#define TRLHLBRD         149
#define TRLSHAMN         150
#define SETTLERS         151
#define TROLLWA2         152
#define WMAMMOT3         153
#define SPIRIT           154
#define HELHOUND         155
#define GARGOYLE         156
#define FIEGIANT         157
#define FIREELEM         158
#define BEHOLDE7         159
#define CHIMERA          160
#define DOOMBAT          161
#define EFREET2          162
#define HYDRA2           163
#define GREATDRA         164
#define SKELETO2         165
#define GHOUL            166
#define BSTALKER         167
#define WEREWOLF         168
#define DEMON            169
#define WRAITH           170
#define SHADOWDE         171
#define DTHKNITE         172
#define DEMONLOR         173
#define ZOMBIE           174
#define UNICORN          175
#define GUARDIAN         176
#define ANGEL3           177
#define ARCANGEL         178
#define WARBEAR          179
#define SPRITE           180
#define COKATRIC         181
#define BASILISK         182
#define GNTSPIDR         183
#define STNGIANT         184
#define COLOSSUS         185
#define GORGON2          186
#define EARTHELE         187
#define BEHEMOTH         188
#define GRTWYRM3         189
#define FLOTISLE         190
#define PNTBEAST         191
#define PNTWARRI         192
#define STMGIANT         193
#define AIRELEM2         194
#define DJINN3           195
#define SKYDRAKE         196
#define NAGA             197
#define WARBEAR          198
#define SPRITE           199
#define COKATRIC         200
#define BASILISK         201
#define GNTSPIDR         202
#define STNGIANT         203
#define COLOSSUS         204
#define GORGON2          205
#define EARTHELE         206
#define BEHEMOTH         207
#define GRTWYRM3         208
#define FLOTISLE         209
#define PNTBEAST         210
#define PNTWARRI         211
#define STMGIANT         212
#define AIRELEM2         213
#define DJINN3           214
#define SKYDRAKE         215
