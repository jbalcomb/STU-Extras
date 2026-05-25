


IDA Pro v5.5
    ¿ Options / Settings ?
    File > Produce file >
    Create MAP file...
    Create ASM file...
    Create INC file...
    Create LST file...
c:/python314/python.exe C:\STU\devel\STU-Extras\Piethawn\Piethawn\ida_dasm_export\ida55_copy_outputs.py
    From:   C:\STU_DASM\IDA55_MOM131\
    To:     C:\STU\devel\STU-Extras\Piethawn\Piethawn\in\
    MAGIC.MAP, MAGIC.ASM, MAGIC.INC, MAGIC.LST, WIZARDS.MAP, WIZARDS.ASM, WIZARDS.INC, WIZARDS.LST

c:/python314/python.exe C:\STU\devel\STU-Extras\Piethawn\Piethawn\ida_dasm_export\ida55_clean_export.py
c:/python314/python.exe C:\STU\devel\STU-Extras\Piethawn\Piethawn\ida_dasm_export\ida55_copy_outputs.py

delete stale .asm files from out directory
redo segment and procedure extractions
reconcile procedure name changes - apply function renames to .c files, MGC & WZD function todo, and Gemini translation checklist
check and update MGC & WZD function completion and Gemini translation checklist




# Dasm/Rasm Side-Project TODO Refresh

Last refreshed: 2026-04-26.

This file is the working scratchpad for turning IDA 5.5 assembly dumps into C89 candidate translations with Gemini. The live automation is in:

- `rasm_named_gemini.py`: run Gemini for a specific function name, whether or not it is marked `__WIP` or `__STUB`.
- `rasm_todo_gemini.py`: build a STUB/WIP queue from the external ReMoM todo file if present, or synthesize one from local dumps and `out/`.
- `rasm_random_todo_gemini.py`: pick one pending STUB/WIP item at random and run it.
- `Rasm-Gemini.cmd`: PowerShell/Gemini wrapper that performs API preflight, sends one ASM file to Gemini, and writes the generated `.c`.
- `gemini_api_status.py`: quick backend reachability check used by the wrapper.

## Current Snapshot

- The hand-maintained checklist below currently has 107 checked `rasm_named_gemini.py` runs and 40 unchecked commands.
- The automated TODO queue currently reports 237 entries and 141 pending items via `rasm_todo_gemini.py list`.
- The local `out/MAGIC` and `out/WIZARDS` trees currently contain 403 `__WIP`/`__STUB` ASM inputs.
- The local `out/MAGIC` and `out/WIZARDS` trees currently contain 65 generated `__WIP`/`__STUB` C outputs.
- The active focus near the current editor state is `AI_Set_Move_Or_Goto_Target`, with generated files under `out/WIZARDS/ovr158/`.

## Normal Usage

Run one named function:

```powershell
c:/python314/python.exe .\rasm_named_gemini.py AI_Set_Move_Or_Goto_Target
```

List matches before running when a name may exist in both games or multiple segments:

```powershell
c:/python314/python.exe .\rasm_named_gemini.py AI_Set_Move_Or_Goto_Target --list-matches
```

Force regeneration if the `.c` output already exists:

```powershell
c:/python314/python.exe .\rasm_named_gemini.py City_Screen__WIP --force
```

Dry-run any named command before spending Gemini quota:

```powershell
c:/python314/python.exe .\rasm_named_gemini.py AI_UNIT_Move --dry-run
```

Run the raw ASM wrapper directly:

```powershell
.\Rasm-Gemini.cmd out\WIZARDS\ovr158\AI_Set_Move_Or_Goto_Target.asm
```

## Queue Usage

List the next TODO queue entries:

```powershell
c:/python314/python.exe .\rasm_todo_gemini.py list
```

Run the next pending queue entry:

```powershell
c:/python314/python.exe .\rasm_todo_gemini.py next
```

Run one queue item by exact STUB/WIP name:

```powershell
c:/python314/python.exe .\rasm_todo_gemini.py run AITP_CombatSpell__STUB
```

Pick a random pending item:

```powershell
c:/python314/python.exe .\rasm_random_todo_gemini.py
```

Useful filters:

```powershell
c:/python314/python.exe .\rasm_todo_gemini.py list --game WIZARDS --status WIP --limit 50
c:/python314/python.exe .\rasm_todo_gemini.py next --prefer-game WIZARDS
c:/python314/python.exe .\rasm_random_todo_gemini.py --status STUB --seed 1234
```

## Gemini Health Checks

Check API/backend status directly:

```powershell
c:/python314/python.exe .\gemini_api_status.py
```

Skip wrapper preflight for one run only when you already know the backend check is the problem:

```powershell
$env:RASM_GEMINI_SKIP_API_STATUS = '1'
.\Rasm-Gemini.cmd out\MAGIC\seg022\PageFlip_GrowOut__WIP.asm
Remove-Item Env:RASM_GEMINI_SKIP_API_STATUS
```

The project-local `.gemini/settings.json` is expected to keep `context.includeDirectoryTree=false` and `ui.errorVerbosity=low`, which prevents Gemini CLI from flooding failures with the full repo tree.

## Suggested Next TODO Focus

- Finish the unchecked RNG helpers: `RNG_WeightedPick32`, `UU_RNG_HighestPick16`, `UU_RNG_HighestPick32`.
- Finish combat movement path helpers: `BU_GetInstaMoveType__WIP`, `Combat_Move_Path_Find`, `Make_Move_Path`, `Init_MovePathMap`, `Update_MovePathMap`, `TILE_ExtendRange`, `Move_Path_Find`.
- Finish remaining combat/AI STUBs: `AITP_CombatSpell__STUB`, `AI_Spell_Select__STUB`, `CRP_NEWG_CreatePathGrid__STUB`.
- Continue the overland AI chain around `AI_UNIT_Move`, `AI_Magic_Strategy__WIP`, `AI_Continent_Eval__WIP`, and related target-value helpers.
- Revisit the draw-frame comparison notes for `Draw_File_Animation_Frame__WIP`, `Remap_Draw_File_Animation_Frame__NOP`, `FLIC_Draw_Frame`, and `FLIC_Remap_Draw_Frame`.

## Raw Working Notes

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



[x] c:/python314/python.exe .\rasm_named_gemini.py Battle_Unit_Action__WIP
[x] c:/python314/python.exe .\rasm_named_gemini.py Battle_Unit_Attack__WIP
[x] c:/python314/python.exe .\rasm_named_gemini.py Move_Battle_Unit

[x] c:/python314/python.exe .\rasm_named_gemini.py BU_SetCityMovement__WIP
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
[x] c:/python314/python.exe .\rasm_named_gemini.py AITP_CombatSpell__STUB

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
[x] c:/python314/python.exe .\rasm_named_gemini.py EMM_EMMDATAH_AllocFirst
[] c:/python314/python.exe .\rasm_named_gemini.py EMM_EMMDATAH_AllocNext
[x] c:/python314/python.exe .\rasm_named_gemini.py UU_EMM_LBX_Load2Hnd

[x] c:/python314/python.exe .\rasm_named_gemini.py LBX_Load_Entry
[x] c:/python314/python.exe .\rasm_named_gemini.py EMM_LBX_EntryLoader
[x] c:/python314/python.exe .\rasm_named_gemini.py LBX_Load_Library_Data
[x] c:/python314/python.exe .\rasm_named_gemini.py EMM_LBX_RecLoader
[x] c:/python314/python.exe .\rasm_named_gemini.py LBX_Load_Data_Static
[x] c:/python314/python.exe .\rasm_named_gemini.py EMM_LBXR_DirectLoad
[x] c:/python314/python.exe .\rasm_named_gemini.py LBX_GetEntryData__WIP

[x] c:/python314/python.exe .\rasm_named_gemini.py Load_Font_File
[x] c:/python314/python.exe .\rasm_named_gemini.py VGA_TextDraw_Init

[x] c:/python314/python.exe .\rasm_named_gemini.py Draw_File_Animation__HACK

Draw_File_Animation_Frame_ASM
[x] c:/python314/python.exe .\rasm_named_gemini.py Draw_File_Animation_Frame__WIP
Remap_Draw_File_Animation_Frame_ASM
[] c:/python314/python.exe .\rasm_named_gemini.py Remap_Draw_File_Animation_Frame__NOP

FLIC_Draw_Frame
FLIC_Remap_Draw_Frame

Regarding the remapped color drawing, aren't Remap_Draw_File_Animation_Frame_ASM in seg025 and FLIC_Remap_Draw_Frame in seg029 equivalent?



Compare Draw_File_Animation_Frame_ASM and FLIC_Draw_Frame. make a list of similarities and a list of differences. create a new file with your notes and a translation of both to C89. also, write a summary fo seg025 and seg029.
...
...more like...
Compare the code-paths for Draw_File_Animation() |-> Draw_File_Animation_Frame_ASM() and FLIC_Draw() |-> FLIC_Draw_Frame.
...
or, the focus should have been just on the draw code



[x] c:/python314/python.exe .\rasm_named_gemini.py AI_Research_Picker__STUB
[x] c:/python314/python.exe .\rasm_named_gemini.py Get_Weighted_Choice_MGC
[x] c:/python314/python.exe .\rasm_named_gemini.py Get_Weighted_Choice
[x] c:/python314/python.exe .\rasm_named_gemini.py Get_Weighted_Choice_Long
[] c:/python314/python.exe .\rasm_named_gemini.py RNG_WeightedPick32
[] c:/python314/python.exe .\rasm_named_gemini.py UU_RNG_HighestPick16
[] c:/python314/python.exe .\rasm_named_gemini.py UU_RNG_HighestPick32



[x] c:/python314/python.exe .\rasm_named_gemini.py NEWG_FinalizeTables
[x] c:/python314/python.exe .\rasm_named_gemini.py AI_WIZ_StrategyReset__WIP
[x] c:/python314/python.exe .\rasm_named_gemini.py Init_Players
[x] c:/python314/python.exe .\rasm_named_gemini.py Player_Base_Casting_Skill

[x] c:/python314/python.exe .\rasm_named_gemini.py AI_OVL_SplCat_Picker

[x] c:/python314/python.exe .\rasm_named_gemini.py AI_Next_Turn__WIP
[x] c:/python314/python.exe .\rasm_named_gemini.py AI_SetUnitOrders__WIP
[] c:/python314/python.exe .\rasm_named_gemini.py 
[x] c:/python314/python.exe .\rasm_named_gemini.py AI_sEFBD6__WIP
[x] c:/python314/python.exe .\rasm_named_gemini.py AI_sEFC92__WIP

[x] c:/python314/python.exe .\rasm_named_gemini.py AI_PullForMainWar__WIP
[x] c:/python314/python.exe .\rasm_named_gemini.py G_AI_HomeRallyFill__WIP
[x] c:/python314/python.exe .\rasm_named_gemini.py AI_Survey_Excess_Units
[x] c:/python314/python.exe .\rasm_named_gemini.py AI_Survey_Excess_Units_In_Stack  C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr158\AI_Survey_Excess_Units.c
[x] c:/python314/python.exe .\rasm_named_gemini.py G_AI_RallyFill__WIP              C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr158\G_AI_RallyFill__WIP.c
[x] c:/python314/python.exe .\rasm_named_gemini.py AI_FillGarrisons__WIP            C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr158\AI_FillGarrisons__WIP.c
[x] c:/python314/python.exe .\rasm_named_gemini.py AI_ProcessOcean__WIP             C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr158\AI_ProcessOcean__WIP.c
[x] c:/python314/python.exe .\rasm_named_gemini.py G_AI_ProcessTransports__WIP      C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr158\G_AI_ProcessTransports__WIP.c


[] c:/python314/python.exe .\rasm_named_gemini.py AI_SendToColonize__WIP


[x] c:/python314/python.exe .\rasm_named_gemini.py AI_Choose_War_Landmass

[x] c:/python314/python.exe .\rasm_named_gemini.py AI_SingleCont_Reeval__WIP

[x] c:/python314/python.exe .\rasm_named_gemini.py AI_Set_Move_Or_Goto_Target
[] c:/python314/python.exe .\rasm_named_gemini.py AI_UNIT_Move

AI_Set_Move_Or_Goto_Target
(AI_UNIT_Move in SETTLE.c?)


    |-> AI_Set_Unit_Orders()
        |-> AI_Disband_To_Balance_Budget()
        |-> AI_Shift_Off_Home_Plane()
        |-> AI_Move_Out_Boats()
        |-> AI_Find_Opportunity_City_Target()
            Per Plane:
                Per Landmass:
                |-> AI_Stacks_Init_Build_Target_Order()
                |-> AI_GarrBuilderPush__WIP()

                |-> AI_Do_Meld()
                |-> AI_Do_Settle()
                |-> AI_Do_Purify()
                |-> AI_Do_RoadBuild()
                |-> AI_Build_Target_List()
                |-> AI_ProcessRoamers__WIP()

[x] c:/python314/python.exe .\rasm_named_gemini.py AI_Set_Unit_Orders
[] c:/python314/python.exe .\rasm_named_gemini.py AI_Disband_To_Balance_Budget
[] c:/python314/python.exe .\rasm_named_gemini.py AI_Shift_Off_Home_Plane
[] c:/python314/python.exe .\rasm_named_gemini.py AI_Move_Out_Boats
[] c:/python314/python.exe .\rasm_named_gemini.py AI_Find_Opportunity_City_Target
[x] c:/python314/python.exe .\rasm_named_gemini.py AI_Stacks_Init_Build_Target_Order

[x] c:/python314/python.exe .\rasm_named_gemini.py AI_GarrBuilderPush__WIP --force
[] c:/python314/python.exe .\rasm_named_gemini.py AI_Do_Meld
[] c:/python314/python.exe .\rasm_named_gemini.py AI_Do_Settle
[] c:/python314/python.exe .\rasm_named_gemini.py AI_Do_Purify
[] c:/python314/python.exe .\rasm_named_gemini.py AI_Do_RoadBuild
[x] c:/python314/python.exe .\rasm_named_gemini.py AI_Build_Target_List
[x] c:/python314/python.exe .\rasm_named_gemini.py AI_ProcessRoamers__WIP
[x] c:/python314/python.exe .\rasm_named_gemini.py AI_AssignStackTarget__WIP

[x] c:/python314/python.exe .\rasm_named_gemini.py AI_PullForMainWar__WIP --force
[x] c:/python314/python.exe .\rasm_named_gemini.py G_AI_RallyFill__WIP --force
[x] c:/python314/python.exe .\rasm_named_gemini.py G_AI_HomeRallyFill__WIP --force
[x] c:/python314/python.exe .\rasm_named_gemini.py G_AI_RallyOrFerry__WIP --force
[x] c:/python314/python.exe .\rasm_named_gemini.py AI_SeekTransportFrom__WIP --force
[x] c:/python314/python.exe .\rasm_named_gemini.py AI_Order_SeekTransport --force
[x] c:/python314/python.exe .\rasm_named_gemini.py AI_FillGarrisons__WIP --force


[x] c:/python314/python.exe .\rasm_named_gemini.py AI_Stacks_Target_Nearest_Hostile_Stack
[] c:/python314/python.exe .\rasm_named_gemini.py AI_Find_Opportunity_City_Target

[] c:/python314/python.exe .\rasm_named_gemini.py AI_Disband_To_Balance_Budget


[] c:/python314/python.exe .\rasm_named_gemini.py AI_CanSettleOffPlane__STUB




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

[x] c:/python314/python.exe .\rasm_named_gemini.py CRP_NEWG_CreatePathGrid__STUB

Army_Movement_Modes

[x] c:/python314/python.exe .\rasm_named_gemini.py Create_Unit__WIP

[] c:/python314/python.exe .\rasm_named_gemini.py Allocate_AI_Data
[x] c:/python314/python.exe .\rasm_named_gemini.py AI_Evaluate_Hostility
[x] c:/python314/python.exe .\rasm_named_gemini.py AI_Magic_Strategy__WIP
[x] c:/python314/python.exe .\rasm_named_gemini.py Player_Hostile_Opponents
[x] c:/python314/python.exe .\rasm_named_gemini.py AI_Player_Calculate_Target_Values
[x] c:/python314/python.exe .\rasm_named_gemini.py AI_Continent_Eval__WIP

[x] c:/python314/python.exe .\rasm_named_gemini.py AI_Choose_War_Landmass --force
[x] c:/python314/python.exe .\rasm_named_gemini.py AI_Continent_Reeval__WIP --force
[x] c:/python314/python.exe .\rasm_named_gemini.py AI_SingleCont_Reeval__WIP --force
[] c:/python314/python.exe .\rasm_named_gemini.py AI_ProcessRoamers__WIP
G_AI_ProcessTransports__WIP

[] c:/python314/python.exe .\rasm_named_gemini.py Loaded_Game_Update__seg001

[] c:/python314/python.exe .\rasm_named_gemini.py AI_Spell_Select__STUB
[] c:/python314/python.exe .\rasm_named_gemini.py AI_Update_Magic_Power
[] c:/python314/python.exe .\rasm_named_gemini.py AI_Sanity_Check_Overland_Enchantments
[] c:/python314/python.exe .\rasm_named_gemini.py AI_Update_Gold_And_Mana_Reserves
[] c:/python314/python.exe .\rasm_named_gemini.py AI_Update_Gold_Income_And_Food_Income
[] c:/python314/python.exe .\rasm_named_gemini.py Player_All_Colony_Autobuild
[x] c:/python314/python.exe .\rasm_named_gemini.py AI_SetEnemyStrMaps__WIP
[x] c:/python314/python.exe .\rasm_named_gemini.py AI_SetUnitOrders__WIP
[] c:/python314/python.exe .\rasm_named_gemini.py AI_Kill_Excess_Settlers_And_Engineers

[] c:/python314/python.exe .\rasm_named_gemini.py WIZ_NextIdleStack


Module: AIDUDES
[x] c:/python314/python.exe .\rasm_named_gemini.py AI_SetEnemyStrMaps__WIP
[x] c:/python314/python.exe .\rasm_named_gemini.py EMM_Map_CONTXXX__WIP
[x] c:/python314/python.exe .\rasm_named_gemini.py CONTX_CreateChains__WIP
[x] c:/python314/python.exe .\rasm_named_gemini.py CONTX_CreateLChains__WIP
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr145\CONTX_CreateChains__WIP.c
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\ovr145\CONTX_CreateLChains__WIP.c

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
[x] c:/python314/python.exe .\rasm_named_gemini.py Combat_Grid_Entity_Create__WIP

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
