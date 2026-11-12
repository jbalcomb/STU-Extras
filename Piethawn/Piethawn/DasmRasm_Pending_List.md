
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\seg021\VGA_SlideColors__STUB.asm
[] .\Rasm-Agy.cmd out\WIZARDS\seg021\VGA_SlideColors__STUB.asm --force

C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr141\VGA_SlideColors__STUB.asm
[] .\Rasm-Agy.cmd out\WIZARDS\ovr141\VGA_SlideColors__STUB.asm --force





## Raw Working Notes

prep IDA settings
IDA dasm output file





Regarding the remapped color drawing, aren't Remap_Draw_File_Animation_Frame_ASM in seg025 and FLIC_Remap_Draw_Frame in seg029 equivalent?

Compare Draw_File_Animation_Frame_ASM and FLIC_Draw_Frame. make a list of similarities and a list of differences. create a new file with your notes and a translation of both to C89. also, write a summary fo seg025 and seg029.
...
...more like...
Compare the code-paths for Draw_File_Animation() |-> Draw_File_Animation_Frame_ASM() and FLIC_Draw() |-> FLIC_Draw_Frame.
...
or, the focus should have been just on the draw code



C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr161\Map_Square_Is_Embarkable.c
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr161\Map_Square_Is_Embarkable.asm
Right, your intuition is mostly correct: regular land terrain does not count as embarkable in this function.
The behavior in Terrain.c:1335 is:
Embarkable is only two shoreline ID bands:
    0x62 to 0xA1
    0xD9 to 0xE8
Everything else is non-embarkable, including water tiles outside those bands.
So to your specific question, water that does not count as embarkable includes:
    Ocean and deep ocean types
    The single lake tile tt_Lake
    Lake-with-river-outlet tiles 0xC5 to 0xC8
    Early shore band 0x02 to 0x61
    Shore-with-river tiles 0xC9 to 0xD8
    River/4-way-river tiles 0xE9 and above
    Shore2 and Shore3 ranges (including high shoreline/ocean-anim ranges)
In short: this is a strict subset of sailable water, not all water.
    _Shore10111000  = 0x62, /* 98   start of range for invalid river outflow */
    _Shore10101111  = 0xA1, /* 161  end   of range for invalid river outflow */
    _Shore00001R10  = 0xD9,
    _Shore000R0000  = 0xE8,



CITYCALC.c
// WZD o142p32
CTY_Food_Breakdown
CTY_Gold_Breakdown
CTY_Prod_Breakdown
CTY_Rsrc_Breakdown
CTY_Pwr_Breakdown



Next_Turn_Proc()
    |-> Next_Turn_Calc()



PageFlip_FX
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\seg001\PageFlip_FX.asm



Items.c
AI_Process_Item_Pool__STUB
// WZD o109p03
// drake178: AI_ProcessItems()
// AI_Process_Item_Pool__STUB()
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr109\AI_ProcessItems__STUB.asm
[x] .\Rasm-Agy.cmd out\WIZARDS\ovr109\AI_ProcessItems__STUB.asm

// WZD o109p04
// drake178: AI_EquipHeroFirst()
// AI_EquipHeroFirst()
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr109\AI_EquipHeroFirst.asm
[x] .\Rasm-Agy.cmd out\WIZARDS\ovr109\AI_EquipHeroFirst.asm

// WZD o109p05
// drake178: AI_EquipRemainder()
// AI_EquipRemainder()
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr109\AI_EquipRemainder.asm
[x] .\Rasm-Agy.cmd out\WIZARDS\ovr109\AI_EquipRemainder.asm





## Overland Spells
Spell_Animation_Load_Graphics
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr134\Spell_Animation_Load_Graphics.asm
[x] .\Rasm-Agy.cmd out\WIZARDS\ovr134\Spell_Animation_Load_Graphics.asm





// WZD s96p01
Move_Units() |-> Combat()
Lair_Combat() |-> ... |-> Combat()
int16_t Combat(int16_t attacker_player_idx, int16_t defender_player_idx, int16_t troop_count, int16_t troops[])
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr096\Combat__WIP.asm
[x] .\Rasm-Agy.cmd out\WIZARDS\ovr096\Combat__WIP.asm

Lair_Make_Guardians()
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr096\Lair_Make_Guardians.asm
[x] .\Rasm-Agy.cmd out\WIZARDS\ovr096\Lair_Make_Guardians.asm

Update_Defender_Hostility()
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr096\Update_Defender_Hostility.asm
[x] .\Rasm-Agy.cmd out\WIZARDS\ovr096\Update_Defender_Hostility.asm

Lair_Combat_Do()
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr096\Lair_Combat_Do.asm
[x] .\Rasm-Agy.cmd out\WIZARDS\ovr096\Lair_Combat_Do.asm





Auto_Move_Unit
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr114\Auto_Move_Unit.asm
[x] .\Rasm-Agy.cmd out\WIZARDS\ovr114\Auto_Move_Unit.asm

Battle_Unit_Attack
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr122\Battle_Unit_Attack.asm
[x] .\Rasm-Agy.cmd out\WIZARDS\ovr122\Battle_Unit_Attack.asm --force



AI_CombatSpellList
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr139\AI_CombatSpellList.asm
[x] .\Rasm-Agy.cmd out\WIZARDS\ovr139\AI_CombatSpellList.asm
AI_SelectCmbtSpell
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr139\AI_SelectCmbtSpell.asm
[] .\Rasm-Agy.cmd out\WIZARDS\ovr139\AI_SelectCmbtSpell.asm
AI_EvaluateCmbtSpell
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr139\AI_EvaluateCmbtSpell.asm
[] .\Rasm-Agy.cmd out\WIZARDS\ovr139\AI_EvaluateCmbtSpell.asm
AI_SetCombatRealms
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr139\AI_SetCombatRealms.asm
[] .\Rasm-Agy.cmd out\WIZARDS\ovr139\AI_SetCombatRealms.asm
AI_UnitThreatRealms
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr139\AI_UnitThreatRealms.asm
[] .\Rasm-Agy.cmd out\WIZARDS\ovr139\AI_UnitThreatRealms.asm

ovr111 p02-p07 and p09  (AITP_DarknessLight, AITP_Healing, AITP_WarpWood, AITP_WarpCreature, UU15_AITP_Disintegrate, AITP_DispelMagic, AITP_HolyWord)
ovr112 p08-p12          (AITP_EarthToMud, AITP_Disrupt, AITP_CracksCall, UU_AITP_WordofRecall, AITP_RecallHero)

AITP_DarknessLight
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr111\AITP_DarknessLight.asm
[x] .\Rasm-Agy.cmd out\WIZARDS\ovr111\AITP_DarknessLight.asm
AITP_Healing
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr111\AITP_Healing.asm
[x] .\Rasm-Agy.cmd out\WIZARDS\ovr111\AITP_Healing.asm
AITP_WarpWood
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr111\AITP_WarpWood.asm
[x] .\Rasm-Agy.cmd out\WIZARDS\ovr111\AITP_WarpWood.asm
AITP_WarpCreature
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr111\AITP_WarpCreature.asm
[x] .\Rasm-Agy.cmd out\WIZARDS\ovr111\AITP_WarpCreature.asm
UU15_AITP_Disintegrate
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr111\UU15_AITP_Disintegrate.asm
[x] .\Rasm-Agy.cmd out\WIZARDS\ovr111\UU15_AITP_Disintegrate.asm
AITP_DispelMagic
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr111\AITP_DispelMagic.asm
[x] .\Rasm-Agy.cmd out\WIZARDS\ovr111\AITP_DispelMagic.asm
AITP_HolyWord
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr111\AITP_HolyWord.asm
[x] .\Rasm-Agy.cmd out\WIZARDS\ovr111\AITP_HolyWord.asm

AITP_EarthToMud
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr112\AITP_EarthToMud.asm
[x] .\Rasm-Agy.cmd out\WIZARDS\ovr112\AITP_EarthToMud.asm
AITP_Disrupt
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr112\AITP_Disrupt.asm
[x] .\Rasm-Agy.cmd out\WIZARDS\ovr112\AITP_Disrupt.asm
AITP_CracksCall
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr112\AITP_CracksCall.asm
[x] .\Rasm-Agy.cmd out\WIZARDS\ovr112\AITP_CracksCall.asm
UU_AITP_WordofRecall
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr112\UU_AITP_WordofRecall.asm
[x] .\Rasm-Agy.cmd out\WIZARDS\ovr112\UU_AITP_WordofRecall.asm
AITP_RecallHero
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr112\AITP_RecallHero.asm
[x] .\Rasm-Agy.cmd out\WIZARDS\ovr112\AITP_RecallHero.asm





// WZD s96p04
// drake178: sub_7DE08()
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr096\UU_IDK_CMB_s7DE08.asm
[x] .\Rasm-Agy.cmd out\WIZARDS\ovr096\UU_IDK_CMB_s7DE08.asm

// WZD s96p06
// UU_IDK_Main_Screen_Draw()
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr096\UU_IDK_Main_Screen_Draw.asm
[x] .\Rasm-Agy.cmd out\WIZARDS\ovr096\UU_IDK_Main_Screen_Draw.asm

C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr098\Get_Battle_Unit_Move_Sound_Buffer.asm
[x] .\Rasm-Agy.cmd out\WIZARDS\ovr098\Get_Battle_Unit_Move_Sound_Buffer.asm

C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr098\NIU_Who_Has_More_Leadership.asm
[x] .\Rasm-Agy.cmd out\WIZARDS\ovr098\NIU_Who_Has_More_Leadership.asm

// WZD o116p12
// sub_9A43E()
NX_Unit_Has_Spell_As_Enchantment_Or_Item_Power
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr116\NX_Unit_Has_Spell_As_Enchantment_Or_Item_Power.asm
[x] .\Rasm-Agy.cmd out\WIZARDS\ovr116\NX_Unit_Has_Spell_As_Enchantment_Or_Item_Power.asm

// WZD o122p05
// BU_GetATKImmFlags()  ==>  Battle_Unit_Attack_Immunities?
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr122\BU_GetATKImmFlags.asm
[x] .\Rasm-Agy.cmd out\WIZARDS\ovr122\BU_GetATKImmFlags.asm

// WZD dseg:709C
// drake178: RP_CMB_NoCombatMap
// WZD ovr153p01
// drake178: UU_sub_DC990()
void NX_IDK_CombatInit_Tactical(int16_t wx, int16_t wy, int16_t wp)
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr153\NX_IDK_CombatInit_Tactical.asm
[x] .\Rasm-Agy.cmd out\WIZARDS\ovr153\NX_IDK_CombatInit_Tactical.asm


AI_RestrictToCity__WIP 
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr124\AI_RestrictToCity__WIP.asm
[] .\Rasm-Agy.cmd out\WIZARDS\ovr124\AI_RestrictToCity__WIP.asm

// WZD o114p09
int16_t Auto_Move_Unit(int16_t battle_unit_idx, int16_t Dest_X, int16_t Dest_Y, int16_t target_battle_unit_idx, int16_t Max_X, int16_t Max_Y);
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr114\Auto_Move_Unit.asm
[] .\Rasm-Agy.cmd out\WIZARDS\ovr114\Auto_Move_Unit.asm --force



// WZD ovr154p18
void Combat_Cache_Read(void)
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr154\Combat_Cache_Read.asm
[] .\Rasm-Agy.cmd out\WIZARDS\ovr154\Combat_Cache_Read.asm --force
// WZD ovr154p19
void Combat_Cache_Write(void)
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr154\Combat_Cache_Write.asm
[] .\Rasm-Agy.cmd out\WIZARDS\ovr154\Combat_Cache_Write.asm --force






// WZD s16p09
// drake178: VGA_Grayscale_Rect()
// MoO2  Module: graphics  Gray_Scale_Fill()
// VGA_Grayscale_Rect
// Gray_Scale_Fill

// WZD s16p10
// drake178: UU_VGA_Columns()
// MoO2  
// UU_VGA_Columns

// WZD s16p11
// drake178: UU_VGA_ScrambleRect()
// MoO2  
// UU_VGA_ScrambleRect



// WZD o97p03
int16_t Square_Has_City(int16_t wx, int16_t wy, int16_t wp)
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr097\Square_Has_City.asm
[x] .\Rasm-Agy.cmd out\WIZARDS\ovr097\Square_Has_City.asm

// WZD o63p14
int16_t City_At_Square__2(int16_t wx, int16_t wy, int16_t wp)
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr063\City_At_Square__2.asm
[x] .\Rasm-Agy.cmd out\WIZARDS\ovr063\City_At_Square__2.asm

// WZD o61p08
void Build_Moveable_Stack(int16_t * troop_count, int16_t troops[])
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr061\Build_Moveable_Stack.asm
[] .\Rasm-Agy.cmd out\WIZARDS\ovr061\Build_Moveable_Stack.asm

// WZD o61p02
int16_t Move_Stack(int16_t move_x, int16_t move_y, int16_t player_idx, int16_t * map_x, int16_t * map_y, int16_t * map_p)
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr061\Move_Stack.asm
[] .\Rasm-Agy.cmd out\WIZARDS\ovr061\Move_Stack.asm

// WZD o59p13
void Units_In_Tower(int16_t troop_count, int16_t troops[], int16_t map_p)
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr059\Units_In_Tower.asm
[x] .\Rasm-Agy.cmd out\WIZARDS\ovr059\Units_In_Tower.asm

// WZD o134p04
int16_t Cast_Spell_Overland_Do(int16_t player_idx, int16_t spell_idx, int16_t spellbook_page_spell_index)
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr134\Cast_Spell_Overland_Do.asm
[x] .\Rasm-Agy.cmd out\WIZARDS\ovr134\Cast_Spell_Overland_Do.asm

// WZD o54p01
void City_Screen__WIP(void)
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr054\City_Screen__WIP.asm
[x] .\Rasm-Agy.cmd out\WIZARDS\ovr054\City_Screen__WIP.asm

// WZD o56p07
// drake178: BLD_GetAllowsList()
void Building_Allows_List__WIP(int16_t bldg_idx, int16_t * allows_list_count, int16_t allows_list[])
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr056\Building_Allows_List__WIP.asm
[x] .\Rasm-Agy.cmd out\WIZARDS\ovr056\Building_Allows_List__WIP.asm

// WZD o56p12
// drake178: BLD_AllowedByRace()
int16_t BLD_AllowedByRace__WIP_TRUE(int16_t UU_Bld1, int16_t Building)
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr056\BLD_AllowedByRace__WIP_TRUE.asm
[x] .\Rasm-Agy.cmd out\WIZARDS\ovr056\BLD_AllowedByRace__WIP_TRUE.asm

// WZD o76p08
void Cartographer_Screen__WIP(void)
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr076\Cartographer_Screen__WIP.asm
[x] .\Rasm-Agy.cmd out\WIZARDS\ovr076\Cartographer_Screen__WIP.asm

// WZD o76p08
void Cartographer_Screen__WIP(void)
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr076\Cartographer_Screen_Draw__WIP.asm
[] .\Rasm-Agy.cmd out\WIZARDS\ovr076\Cartographer_Screen_Draw__WIP.asm

// WZD o76p10
void Cartograph_Screen_Draw_Map__WIP(int16_t flag)
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr076\Cartograph_Screen_Draw_Map__WIP.asm
[x] .\Rasm-Agy.cmd out\WIZARDS\ovr076\Cartograph_Screen_Draw_Map__WIP.asm

// WZD o150p17
void Cartograph_Screen_Draw_Map_Terrain__NOWORKIE(int16_t cartograph_plane, SAMB_ptr cartograph_seg)
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr150\Cartograph_Screen_Draw_Map_Terrain__NOWORKIE.asm
[x] .\Rasm-Agy.cmd out\WIZARDS\ovr150\Cartograph_Screen_Draw_Map_Terrain__NOWORKIE.asm

// WZD o150p18
// sub_DAFF4()
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr150\sub_DAFF4.asm
[x] .\Rasm-Agy.cmd out\WIZARDS\ovr150\sub_DAFF4.asm
// WZD s161p24
int16_t Square_Is_Sailable(int16_t wx, int16_t wy, int16_t wp)
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr161\Square_Is_Sailable.asm
[x] .\Rasm-Agy.cmd out\WIZARDS\ovr161\Square_Is_Sailable.asm

// WZD o76p05
void IDK_Powergraph_Draw_Turn_Data(int16_t player_idx, int16_t turns, int16_t month)
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr076\IDK_Powergraph_Draw_Turn_Data.asm
[x] .\Rasm-Agy.cmd out\WIZARDS\ovr076\IDK_Powergraph_Draw_Turn_Data.asm

// WZD s30p35
// VGA_FILEH_LoadFirst()
// MoO2  Module: file_ani  Open_File_Animation()
/*
MoO2
Module: file_ani
    function (0 bytes) Open_File_Animation
void Open_File_Animation__HACK(char * file_name, int16_t entry_num)
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\seg030\Open_File_Animation.asm
[x] .\Rasm-Agy.cmd out\WIZARDS\seg030\Open_File_Animation.asm

// WZD s30p36
// drake178: VGA_FILEH_DrawFrame()
// Draw_File_Animation()
void Draw_File_Animation__HACK(void)
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\seg030\Draw_File_Animation__HACK.asm
[x] .\Rasm-Agy.cmd out\WIZARDS\seg030\Draw_File_Animation.asm

// WZD s30p37
// drake178: VGA_FILEH_Loader()
// void Load_File_Animation_Frame__STUB(int32_t offset, int32_t size, int16_t logical_page)
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\seg030\Load_File_Animation_Frame.asm
[x] .\Rasm-Agy.cmd out\WIZARDS\seg030\Load_File_Animation_Frame.asm

// WZD s30p38
// drake178: VGA_FILEH_GetFrame()
int16_t Get_File_Animation_Frame(void)
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\seg030\Get_File_Animation_Frame.asm
[x] .\Rasm-Agy.cmd out\WIZARDS\seg030\Get_File_Animation_Frame.asm

// WZD s25p01
void Draw_File_Animation_Frame_ASM(void)
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\seg025\Draw_File_Animation_Frame_ASM.asm
[x] .\Rasm-Agy.cmd out\WIZARDS\seg025\Draw_File_Animation_Frame_ASM.asm

// WZD s25p02
void Map_File_Animation_Frame_ASM(void)
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\seg025\Map_File_Animation_Frame_ASM.asm
[x] .\Rasm-Agy.cmd out\WIZARDS\seg025\Map_File_Animation_Frame_ASM.asm

// WZD s25p03
void Remap_Draw_File_Animation_Frame_ASM(void)
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\seg025\Remap_Draw_File_Animation_Frame_ASM.asm
[x] .\Rasm-Agy.cmd out\WIZARDS\seg025\Remap_Draw_File_Animation_Frame_ASM.asm

// WZD o143p13
int16_t EVNT_TargetDepletion__STUB(int16_t player_idx, int16_t * wx, int16_t * wy, int16_t * wp, int16_t * terrain_special)
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr143\EVNT_TargetDepletion__STUB.asm
[x] .\Rasm-Agy.cmd out\WIZARDS\ovr143\EVNT_TargetDepletion__STUB.asm

// WZD o143p14
int16_t EVNT_FindNewMineral__STUB(int16_t player_idx, int16_t * wx, int16_t * wy, int16_t * wp, int16_t * terrain_special)
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr143\EVNT_FindNewMineral__STUB.asm
[x] .\Rasm-Agy.cmd out\WIZARDS\ovr143\EVNT_FindNewMineral__STUB.asm

// WZD o143p15
void TILE_ReplaceMinerals__STUB(int16_t wx, int16_t wy, int16_t wp, int16_t terain_special)
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr143\TILE_ReplaceMinerals.asm
[x] .\Rasm-Agy.cmd out\WIZARDS\ovr143\TILE_ReplaceMinerals.asm

// WZD o143p08
// TILE_AdjustMapFlow()
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr143\TILE_AdjustMapFlow.asm
[x] .\Rasm-Agy.cmd out\WIZARDS\ovr143\TILE_AdjustMapFlow.asm

// WZD o143p09
// TILE_AdjacentFlow()
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr143\TILE_AdjacentFlow.asm
[x] .\Rasm-Agy.cmd out\WIZARDS\ovr143\TILE_AdjacentFlow.asm

// WZD o143p10
// TILE_GetRoughType()
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr143\TILE_GetRoughType.asm
[x] .\Rasm-Agy.cmd out\WIZARDS\ovr143\TILE_GetRoughType.asm

// WZD o143p11
void TILE_GenerateOre__STUB(int16_t wx, int16_t wy, int16_t wp)
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr143\TILE_GenerateOre.asm
[x] .\Rasm-Agy.cmd out\WIZARDS\ovr143\TILE_GenerateOre.asm

// WZD o143p05
// RP_TILE_MakeDesert()
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr143\Set_Terrain_Type_Desert.asm
[x] .\Rasm-Agy.cmd out\WIZARDS\ovr143\Set_Terrain_Type_Desert.asm

// WZD o89p09
// drake178: sub_759DE()
// IDK_Drown_Units()
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr089\IDK_Drown_Units.asm
[x] .\Rasm-Agy.cmd out\WIZARDS\ovr089\IDK_Drown_Units.asm

// WZD o89p10
void UNIT_GetDependants__WIP(int16_t unit_idx, int16_t * troop_count, int16_t troops[])
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr089\UNIT_GetDependants__WIP.asm
[x] .\Rasm-Agy.cmd out\WIZARDS\ovr089\UNIT_GetDependants__WIP.asm

// WZD o68p05
// RP_OVL_DrawCities2()
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr068\DEPR_MainScr_Draw_Map_Cities.asm
[x] .\Rasm-Agy.cmd out\WIZARDS\ovr068\DEPR_MainScr_Draw_Map_Cities.asm





Here's the list: 36 items, 42 IDA functions in your format. #43, #335, #338, #339 and #408 each cover several functions, and #346/#397 and #377/#391 share one. Each block also links the item and its ReMoM location. Where ReMoM has no body, that's the comment line naming the function, or the prototype in EMS.h for the EMS ones. Release_Time, gfopen, Mouse_Button and Clipped_Draw_Frame are SDL or stdio replacements, not reconstructions. FLIC_Load_Palette is missing only its EMS branch, and Mirror_Screen_Draw only its help-slot stores.


All 42 asm files exist. The same list is saved as C:\Users\jbalcomb\AppData\Local\Temp\claude\c00932cd21261fca\b8775e42-472b-43e5-a9eb-08f89368ed55\scratchpad\missing36.md.



#333 -- ReMoM: Timer.c:75
// WZD s02p03
// Release_Time()
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\seg002\Release_Time.asm
[] .\Rasm-Agy.cmd out\WIZARDS\seg002\Release_Time.asm

#338 -- ReMoM: Input.c:1754
// WZD s36p21
// RP_GUI_KeyInputOnly()
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\seg036\RP_GUI_KeyInputOnly.asm
[] .\Rasm-Agy.cmd out\WIZARDS\seg036\RP_GUI_KeyInputOnly.asm --force



There are three lists of open work: 34 open rows in the BQ triage table, about 40 open follow-ups, and 182 open entries in stub_wip_todo.md. Seven of the 34 rows are now stale, because this session reconstructed their code but the rows still say pending.

3. Engine and MoX rows, all pending (25)
Waits and timing:
#333 Wait_Ticks;
#383 Wait_Ticks_Or_Input.
DOS and file I/O:
#334 DOS_File_Open;
#384 DOS_Drive_Ready.
EMS / LBX loaders:
#335 EMS_LBX_Load_Entry;
#374 EMS_Cache_LBX_Range_Core;
#399 Pic_EMS_Next_Pages;
#459 EMS_Cache_LBX_File;
#460 EMS_LBX_Uncache;
#461 EMS_LBX_Reload_Into_Handle.
Input:
#338 UI_Poll_Input_Keyboard;
#485 UI_Text_List_Select_Scrolling.
Picture drawing:
#339 Pic_Blit_Clipped_Planar;
#378 Pic_Draw_Warped_Core;
#386 Pic_Load_Palette;
#394 Pic_Keep_Rect;
#396 Pic_Draw_Rect_Outline.
Window zoom:
#377 and #391 Window_Zoom_Draw;
#470 Window_Zoom_Open.
Sound: #346 and #397 Sound_Fatal_Error.
Other:
#388 Paragraph_Add_Hotkey_Regions;
#407 Engine_Init_Short;
#488 Wizard_Info_Draw.
#378 is the one you set aside for a full 1:1 review of Draw_Bitmap_Animation_Block.

4. Open follow-ups

The follow-ups table has about 40 rows. The 8 that have been checked against the asm:

the area-dispel battle_units[99] overrun;
the _ITEMS[-1] read in Choose_Target_And_Action;
the uninitialised Melee_Unit_List[0];
the byte store that should be a word in Modifier_Diplomacy_Adjustments;
Retreat_From_Combat returns void where the asm returns 0;
Draw_Bitmap_Animation_Block;
the Set_Font_Style third colour set, which needs a DOSBox measurement;
the road-connection and mouse-page GUI checks.
The rest haven't been checked against the asm.

Items carried over from earlier in the session:

OVL_SpellWardError;
the incentive block in DIPL_sub_72DB6;
the REBUG comment for the moved block at DIPLOMAC.c:5664.
5. Functions: stub_wip_todo.md has 182 entries unchecked and 237 checked. The biggest groups:

AI overland: AI_Set_Unit_Orders, AI_Next_Turn, the CONTX_* chain builders, and the AITP_* overland targeters.
Combat AI (L117-L140): implemented, but each still needs its done-done review.
City screen: the Cityscape_Roads_* and City_Screen_*__WIP functions.
Astrologer: the WIZ_Get_Astr_* stubs.
The bq-base squash still waits on sections 1–3 at least.



Still open: after that, the move code reads and writes _UNITS[-1]. I haven't checked whether that is undefined behaviour in ReMoM.

#259 (Gaia's Blessing): added an OGBUG note at Spells132.c:1509. I checked ovr161/Square_Is_Volcano.asm: it does cmp cx, tt_ChaosNode / jz → TRUE, so a Chaos Node square counts as a volcano.



Can't-fail branch: Resource_Breakdown_Draw's grey "negative food" branch can only fire if a breakdown value is negative, and I didn't check whether any can be.
