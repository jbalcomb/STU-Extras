

prep IDA settings
IDA dasm output file



REM From PowerShell:
REM   .\Rasm-Gemini.cmd out\MAGIC\seg022\PageFlip_GrowOut__WIP.asm
REM
REM Examples:
REM   .\Rasm-Gemini.cmd out\MAGIC\seg022\PageFlip_GrowOut__WIP.asm
REM   .\Rasm-Gemini.cmd out\MAGIC\seg022\PageFlip_GrowOut__WIP.asm out\MAGIC\seg022\PageFlip_GrowOut__WIP.c
REM   .\Rasm-Gemini.cmd out\WIZARDS\ovr114\AI_BU_AssignAction__WIP.asm


"--model",



[x] c:/python314/python.exe .\rasm_named_gemini.py Get_Weighted_Choice
[] c:/python314/python.exe .\rasm_named_gemini.py RNG_WeightedPick32
[] c:/python314/python.exe .\rasm_named_gemini.py UU_RNG_HighestPick16
[] c:/python314/python.exe .\rasm_named_gemini.py UU_RNG_HighestPick32

[x] c:/python314/python.exe .\rasm_named_gemini.py Battle_Unit_Action__WIP
[x] c:/python314/python.exe .\rasm_named_gemini.py Battle_Unit_Attack__WIP
[x] c:/python314/python.exe .\rasm_named_gemini.py Move_Battle_Unit

[] c:/python314/python.exe .\rasm_named_gemini.py BU_SetCityMovement__WIP
[] c:/python314/python.exe .\rasm_named_gemini.py BU_GetInstaMoveType__WIP

[] c:/python314/python.exe .\rasm_named_gemini.py Combat_Move_Path_Find

[x] c:/python314/python.exe .\rasm_named_gemini.py BU_Teleport
[x] c:/python314/python.exe .\rasm_named_gemini.py BU_TunnelTo


[x] c:/python314/python.exe .\rasm_named_gemini.py AI_BU_SelectAction__WIP
[x] c:/python314/python.exe .\rasm_named_gemini.py AI_BU_AssignAction__WIP
[x] c:/python314/python.exe .\rasm_named_gemini.py AI_BU_ProcessAction__WIP
[x] c:/python314/python.exe .\rasm_named_gemini.py AI_MoveBattleUnits__WIP
[x] c:/python314/python.exe .\rasm_named_gemini.py AI_CMB_PlayTurn__WIP
[x] c:/python314/python.exe .\rasm_named_gemini.py CMB_ProgressTurnFlow__WIP

[x] c:/python314/python.exe .\rasm_named_gemini.py BU_IsVisible
[x] c:/python314/python.exe .\rasm_named_gemini.py AI_BU_GetAttackValue__STUB
[] c:/python314/python.exe .\rasm_named_gemini.py AITP_CombatSpell__STUB

[x] c:/python314/python.exe .\rasm_named_gemini.py CMB_CE_Refresh__WIP

[x] c:/python314/python.exe .\rasm_named_gemini.py Combat_Cast_Spell__WIP

[x] c:/python314/python.exe .\rasm_named_gemini.py CMB_ComposeBackgrnd__WIP

[x] c:/python314/python.exe .\rasm_named_gemini.py Check_For_Winner__WIP
[x] c:/python314/python.exe .\rasm_named_gemini.py AI_FightorFlight__STUB
[x] c:/python314/python.exe .\rasm_named_gemini.py AI_Raze_Decision
[x] c:/python314/python.exe .\rasm_named_gemini.py STK_CaptureCity
[x] c:/python314/python.exe .\rasm_named_gemini.py CTY_RampageVictory
[x] c:/python314/python.exe .\rasm_named_gemini.py CTY_GetConquerGold
[x] c:/python314/python.exe .\rasm_named_gemini.py BU_MoveConfused

[x] c:/python314/python.exe .\rasm_named_gemini.py CMB_GetUndeadCreator

[x] c:/python314/python.exe .\rasm_named_gemini.py BU_SetCityMovement__WIP
[x] c:/python314/python.exe .\rasm_named_gemini.py City_Screen__WIP --force

[x] c:/python314/python.exe .\rasm_named_gemini.py Generate_Home_City__WIP
[x] c:/python314/python.exe .\rasm_named_gemini.py Generate_Neutral_Cities__WIP

[x] c:/python314/python.exe .\rasm_named_gemini.py CMB_SpawnStoneWall
[x] c:/python314/python.exe .\rasm_named_gemini.py CMB_SpawnDarkWall
[x] c:/python314/python.exe .\rasm_named_gemini.py CMB_SpawnFireWall
[x] c:/python314/python.exe .\rasm_named_gemini.py CMB_SpawnStructures

[x] c:/python314/python.exe .\rasm_named_gemini.py Allocate_Data_Space
[x] c:/python314/python.exe .\rasm_named_gemini.py EMM_GetHandle
[x] c:/python314/python.exe .\rasm_named_gemini.py EMM_MakeNamedHandle
[x] c:/python314/python.exe .\rasm_named_gemini.py EMM_GetPageCount
[x] c:/python314/python.exe .\rasm_named_gemini.py EMM_MapnRead
[x] c:/python314/python.exe .\rasm_named_gemini.py EMM_MapnWrite
the EMMDATAH allocator
[] c:/python314/python.exe .\rasm_named_gemini.py EMM_EMMDATAH_AllocFirst
[] c:/python314/python.exe .\rasm_named_gemini.py EMM_EMMDATAH_AllocNext



[x] c:/python314/python.exe .\rasm_named_gemini.py AI_Research_Picker__STUB
[x] c:/python314/python.exe .\rasm_named_gemini.py Get_Weighted_Choice_MGC
[x] c:/python314/python.exe .\rasm_named_gemini.py Get_Weighted_Choice
[x] c:/python314/python.exe .\rasm_named_gemini.py Get_Weighted_Choice_Long



[x] c:/python314/python.exe .\rasm_named_gemini.py NEWG_FinalizeTables
[x] c:/python314/python.exe .\rasm_named_gemini.py AI_WIZ_StrategyReset__WIP
[x] c:/python314/python.exe .\rasm_named_gemini.py Init_Players
[x] c:/python314/python.exe .\rasm_named_gemini.py Player_Base_Casting_Skill

[] c:/python314/python.exe .\rasm_named_gemini.py AI_OVL_SplCat_Picker

[x] c:/python314/python.exe .\rasm_named_gemini.py AI_Next_Turn__WIP
[x] c:/python314/python.exe .\rasm_named_gemini.py AI_SetUnitOrders__WIP
[x] c:/python314/python.exe .\rasm_named_gemini.py AI_MoveUnits__WIP
[x] c:/python314/python.exe .\rasm_named_gemini.py AI_UNIT_Move__WIP
[x] c:/python314/python.exe .\rasm_named_gemini.py RdBd_UNIT_MoveStack__WIP
[x] c:/python314/python.exe .\rasm_named_gemini.py Build_RoadBuilder_Stack
[x] c:/python314/python.exe .\rasm_named_gemini.py OVL_ClearUnitPath
[x] c:/python314/python.exe .\rasm_named_gemini.py OVL_StoreLongPath

[x] c:/python314/python.exe .\rasm_named_gemini.py Cast_Spell_Overland__WIP

[] c:/python314/python.exe .\rasm_named_gemini.py Make_Move_Path
[] c:/python314/python.exe .\rasm_named_gemini.py Init_MovePathMap
[] c:/python314/python.exe .\rasm_named_gemini.py Update_MovePathMap
[] c:/python314/python.exe .\rasm_named_gemini.py TILE_ExtendRange
[] c:/python314/python.exe .\rasm_named_gemini.py Move_Path_Find

[x] c:/python314/python.exe .\rasm_named_gemini.py AI_SetNeutralFarmers
[x] c:/python314/python.exe .\rasm_named_gemini.py AI_SetNeutralTargets
[x] c:/python314/python.exe .\rasm_named_gemini.py AI_GetNeutralStacks
[x] c:/python314/python.exe .\rasm_named_gemini.py AI_STK_SetTarget
[x] c:/python314/python.exe .\rasm_named_gemini.py AI_TILE_CanLeave

[x] c:/python314/python.exe .\rasm_named_gemini.py EVNT_GenerateRaiders
[x] c:/python314/python.exe .\rasm_named_gemini.py EVNT_RampageMonsters
[x] c:/python314/python.exe .\rasm_named_gemini.py AI_ExcessNeutrals
[x] c:/python314/python.exe .\rasm_named_gemini.py AI_StasisDisband
[x] c:/python314/python.exe .\rasm_named_gemini.py TILE_NextFreeLand__WIP
[x] c:/python314/python.exe .\rasm_named_gemini.py EVNT_MakeRampageList

[] c:/python314/python.exe .\rasm_named_gemini.py CRP_NEWG_CreatePathGrid__STUB

Army_Movement_Modes

[] c:/python314/python.exe .\rasm_named_gemini.py Create_Unit__WIP

[] c:/python314/python.exe .\rasm_named_gemini.py AI_Evaluate_Hostility
[] c:/python314/python.exe .\rasm_named_gemini.py AI_Magic_Strategy__WIP
[] c:/python314/python.exe .\rasm_named_gemini.py Player_Hostile_Opponents
[] c:/python314/python.exe .\rasm_named_gemini.py AI_Player_Calculate_Target_Values
[] c:/python314/python.exe .\rasm_named_gemini.py AI_Continent_Eval__WIP
[] c:/python314/python.exe .\rasm_named_gemini.py AI_Pick_Action_Conts__WIP
[] c:/python314/python.exe .\rasm_named_gemini.py Allocate_AI_Data
[] c:/python314/python.exe .\rasm_named_gemini.py Player_Hostile_Opponents
[] c:/python314/python.exe .\rasm_named_gemini.py AI_Player_Calculate_Target_Values
[] c:/python314/python.exe .\rasm_named_gemini.py AI_Spell_Select__STUB
[] c:/python314/python.exe .\rasm_named_gemini.py AI_Update_Magic_Power
[] c:/python314/python.exe .\rasm_named_gemini.py AI_Sanity_Check_Overland_Enchantments
[] c:/python314/python.exe .\rasm_named_gemini.py AI_Update_Gold_And_Mana_Reserves
[] c:/python314/python.exe .\rasm_named_gemini.py AI_Update_Gold_Income_And_Food_Income
[] c:/python314/python.exe .\rasm_named_gemini.py Player_All_Colony_Autobuild
[] c:/python314/python.exe .\rasm_named_gemini.py AI_SetEnemyStrMaps__WIP
[] c:/python314/python.exe .\rasm_named_gemini.py AI_Continent_Reeval__WIP
[] c:/python314/python.exe .\rasm_named_gemini.py AI_SetUnitOrders__WIP
[] c:/python314/python.exe .\rasm_named_gemini.py AI_Kill_Excess_Settlers_And_Engineers

[] c:/python314/python.exe .\rasm_named_gemini.py WIZ_NextIdleStack


Module: AIDUDES
[x] c:/python314/python.exe .\rasm_named_gemini.py AI_SetEnemyStrMaps__WIP
[x] c:/python314/python.exe .\rasm_named_gemini.py EMM_Map_CONTXXX__WIP
[x] c:/python314/python.exe .\rasm_named_gemini.py CONTX_CreateChains__WIP
[x] c:/python314/python.exe .\rasm_named_gemini.py CONTX_CreateLChains__WIP

[x] c:/python314/python.exe .\rasm_named_gemini.py AI_Continent_Reeval__WIP

[x] c:/python314/python.exe .\rasm_named_gemini.py Get_Input
[x] c:/python314/python.exe .\rasm_named_gemini.py RP_GUI_KeyInputOnly


EVNT_GenerateRaiders
EVNT_RampageMonsters
AI_MoveUnits

AI_Next_Turn__WIP




The remaining __STUB references are all active call sites — functions like AITP_CombatSpell__STUB, Stop_All_Sounds__STUB, STK_ComposeFleeLost__STUB, etc. that haven't been implemented yet. Those are different from the __WIP__OLD dead code — they're live stubs that still need to be filled in.

The one __WIP__OLD hit on CMBTAI.c:968 is just a comment, not a function definition.

Looks clean. The dead code is out.

[x] c:/python314/python.exe .\rasm_named_gemini.py BU_SetVisibility__WIP
[x] c:/python314/python.exe .\rasm_named_gemini.py BU_MeleeWallCheck__WIP

[x] c:/python314/python.exe .\rasm_named_gemini.py Generate_Combat_Map__WIP
[] c:/python314/python.exe .\rasm_named_gemini.py Combat_Grid_Entity_Create__WIP

[x] c:/python314/python.exe .\rasm_named_gemini.py BU_ApplyDamage__WIP__SEGRAX

[x] c:/python314/python.exe .\rasm_named_gemini.py UNIT_SummonToBattle__SEGRAX
[x] c:/python314/python.exe .\rasm_named_gemini.py BU_UnitLoadToBattle__SEGRAX




[x] c:/python314/python.exe .\rasm_named_gemini.py AI_SetBasicAttacks__WIP
[x] c:/python314/python.exe .\rasm_named_gemini.py BU_SortSlowestFirst__WIP
[x] c:/python314/python.exe .\rasm_named_gemini.py AI_GetCombatRallyPt__WIP
// WZD o114p01
void AI_SetBasicAttacks__WIP(int16_t player_idx);
// WZD o114p02
void AI_BU_ProcessAction__WIP__OLD(int16_t battle_unit_idx, int16_t Rally_X, int16_t Rally_Y);
void AI_BU_ProcessAction(int16_t battle_unit_idx, int16_t rally_cgx, int16_t rally_cgy);
// WZD o114p03
void BU_SortSlowestFirst__WIP(int16_t Melee_Unit_List[], int16_t Melee_Unit_Count);
// WZD o114p04
void AI_GetCombatRallyPt__WIP(int16_t battle_unit_idx, int16_t * Rally_X, int16_t * Rally_Y);
// WZD o114p05
void AI_MoveBattleUnits__WIP__OLD(int16_t player_idx);
void AI_MoveBattleUnits(int16_t player_idx);
// WZD o114p06
int16_t AI_BU_AssignAction__WIP__OLD(int16_t battle_unit_idx, int16_t no_spells_flag);
int16_t AI_BU_AssignAction(int16_t battle_unit_idx, int16_t no_spells_flag);
// WZD o114p07
int16_t AI_BU_SelectAction__WIP__OLD(int16_t battle_unit_idx, int16_t * selected_action, int16_t has_ranged_attack);
int16_t AI_BU_SelectAction(int16_t battle_unit_idx, int16_t * selected_action, int16_t has_ranged_attack);
// WZD o114p08
void Do_Auto_Unit_Turn(int16_t battle_unit_idx, int16_t dst_cgx, int16_t dst_cgy, int16_t target_battle_unit_idx, int16_t rally_cgx, int16_t rally_cgy);
// WZD o114p09
int16_t Auto_Move_Unit(int16_t battle_unit_idx, int16_t Dest_X, int16_t Dest_Y, int16_t target_battle_unit_idx, int16_t Max_X, int16_t Max_Y);

[x] c:/python314/python.exe .\rasm_named_gemini.py WIZ_GetLastRangedStr__WIP

[x] c:/python314/python.exe .\rasm_named_gemini.py Assign_Combat_Grids






c:/python314/python.exe .\rasm_named_gemini.py Strategic_Combat__STUB





Items.c
OVL_MosaicFlip__STUB
// WZD o109p03
// drake178: AI_ProcessItems()
// AI_Process_Item_Pool__STUB()
// WZD o109p04
// drake178: AI_EquipHeroFirst()
// AI_EquipHeroFirst()
// WZD o109p05
// drake178: AI_EquipRemainder()
// AI_EquipRemainder()


// WZD o158p21
// drake178: sub_EFC92()
/*
evaluates the excess units in the stack, and adds
them to the two global offensive? stacks if they are
stronger than the current selections
*/
/*

*/
void AI_sEFC92__WIP(int16_t CX_ID, int16_t Count, int16_t Excess)
