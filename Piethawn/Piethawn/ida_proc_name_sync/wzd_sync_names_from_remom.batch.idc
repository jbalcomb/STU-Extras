#include <idc.idc>

static sync_magic_proc_name(old_name, new_name)
{
    auto ea, current_name;

    ea = LocByName(old_name);
    if ( ea == BADADDR )
    {
        ea = LocByName(new_name);
        if ( ea != BADADDR )
        {
            Message("already-synced,%s,%s,%a\n", old_name, new_name, ea);
            return 1;
        }
        Message("not-found,%s,%s\n", old_name, new_name);
        return 0;
    }

    current_name = GetFunctionName(ea);
    if ( current_name != old_name )
    {
        Message("name-mismatch,%s,%s,%s,%a\n", old_name, new_name, current_name, ea);
        return 0;
    }

    if ( LocByName(new_name) != BADADDR )
    {
        Message("collision,%s,%s\n", old_name, new_name);
        return 0;
    }

    if ( !MakeNameEx(ea, new_name, SN_CHECK|SN_NOWARN) )
    {
        Message("rename-failed,%s,%s,%a\n", old_name, new_name, ea);
        return 0;
    }

    Message("renamed,%s,%s,%a\n", old_name, new_name, ea);
    return 1;
}

static main(void)
{
    auto ok, attempted;

    ok = 0;
    attempted = 0;
    Batch(1);

    attempted = attempted + 1; ok = ok + sync_magic_proc_name("zzzMain_Screen_Load_Pictures", "Main_Screen_Load_Pictures");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("zzzCity_Screen_Load_Pictures", "City_Screen_Load_Pictures");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("fxn_o52p15", "Load_TERRSTAT");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("City_Screen__WIP", "City_Screen");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("City_Screen_Draw__WIP", "City_Screen_Draw");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("City_Screen_Add_Fields", "City_Screen_Add_Fields__WIP");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("NameStartingCity_Dialog_Popup", "Change_Home_City_Name_Popup");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("City_Building_Is_Currently_Required", "City_Building_Not_Needed_For_Product");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("sub_4BEF0", "Player_Gold_Covers_Production_Cost__NIU");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("City_Draw_City_Enchantments", "City_Screen_Draw_City_Enchantments");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Resource_Breakdown__STUB", "Resource_Breakdown");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Resource_Breakdown_Draw__STUB", "Resource_Breakdown_Draw");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("sub_4E507", "Draw_Stat_Icons_10_And_1");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Building_Allows_List__WIP", "Building_Allows_List");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Main_Screen_Add_Unit_Window_Fields", "Add_Unit_Window_Fields");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("zMove_Stack_DirKey", "Move_Stack_DirKey");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Play_Background_Music__STUB", "Play_Background_Music");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Next_Turn_Proc__WIP", "Next_Turn_Proc");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("City_At_Square__2", "Square_Has_City_Dupe");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Allocate_Reduced_Map__2", "Allocate_Reduced_Map");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("zzzCombat_Unit_Statistics_Window", "Combat_Unit_Statistics_Window");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Production_Screen_Set_Unit_View", "UV_Setup_ProdScr");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("GrandVizier_Window", "Grand_Vizier_Window");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("IDK_Diplo_Add_Spell_Or_Gold_To_Message", "Diplomacy_Add_Incentive_To_Message");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Get_Main_Diplomacy_Choices_", "Get_Main_Diplomacy_Choices");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("IDK_Npc_Target_Player__STUB", "Diplomacy_Choose_Other_Wizard");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Limit_Temporary_Peace_Modifier", "Limit_Treaty_Modifiers");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Change_Relations__WIP", "Change_Relations");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Diplomacy_Offer_Tribute__STUB", "Diplomacy_Offer_Tribute");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("j_NPC_Required_Alliances_", "NPC_Required_Alliances");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("sub_74E38", "Army_Strength_Random_Difference__NIU");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("sub_74F4A", "Population_Random_Difference__NIU");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Prod_Init_Battle_Unit", "Prod_Load_Battle_Unit");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Unit_Figure_Draw", "Draw_Unit_Figure");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("zzzCombat_Screen", "Combat_Screen");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("UU_BU_LoadFigureGFX", "NIU_Battle_Unit_Load_Figure_Pictures");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("UU_IDK_CMB_s7DE08", "NIU_Enemy_Stack_Combat");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("UU_IDK_Main_Screen_Draw", "NIU_Combat_Redraw_Main_Screen");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Units_Moves", "Army_Moves2");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("RP_CTY_CheckSpellWard__STUB", "Stack_Blocked_By_Spell_Ward");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("CTY_CheckSpellWard__STUB", "City_Spell_Ward_Check");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("BU_GetRangedCursor__WIP", "Ranged_Mouse_Image");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Battle_Unit_Movement_Icon", "Battle_Unit_Movement_Mode");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("BU_CheckFlight__WIP", "Battle_Unit_Is_Airborne");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("STK_ComposeFleeLost__STUB", "Build_Flee_Loss_Message");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Combat_Screen_Draw_Buttons", "Tactical_Combat_Draw_Buttons");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Combat_Grid_Entities__WIP", "Combat_Grid_Entities");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Do_Build_Outpost", "Active_Army_Do_Settle");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("STK_SettleTile", "Army_Do_Settle");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("AI_UNIT_SeekTransprt", "AI_Unit_Army_Do_Ferry");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Combat_Node_type", "Combat_Node_Type");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("BU_CreateImage__SEGRAX", "Battle_Unit_Compose_Bitmap");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("zCombat_Cast_Spell_Error", "Combat_Cast_Spell_Error");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("CMB_LoadResources__WIP", "Combat_Screen_Load_Resources");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("CMB_SetNearAllocs__WIP", "Allocate_Combat_Near_Buffers");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("zCombat_Cast_Spell_With_Caster", "Combat_Cast_Spell_With_Caster");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Combat_Figure_Effect__WIP", "Combat_Figure_Effect");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("BU_SetBaseAnims__WIP", "Battle_Unit_Set_Animation_Flags");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("AI_ProcessItems__STUB", "AI_Process_Item_Pool");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("AI_EquipHeroFirst", "AI_Equip_Hero_Best_Items");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("AI_EquipRemainder", "AI_Equip_Hero_Empty_Slots");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("UU15_AITP_Disintegrate", "AITP_Disintegrate");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Cast_Spell_On_Battle_Unit", "Combat_Cast_Apply_Spell_Effect");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("CMB_ComposeBookBG__WIP", "Combat_Compose_Spellbook_Background");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Do_Legal_Spell_Check__WIP", "Do_Legal_Spell_Check");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("UU_AITP_WordofRecall", "AITP_Word_Of_Recall");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Apply_Battle_Unit_Damage_From_Spell", "Compute_Battle_Unit_Damage_From_Spell");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("BU_ApplyDamage__WIP__SEGRAX", "Battle_Unit_Commit_Damage");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("BU_MeleeWallCheck", "Check_Attack_Melee_City_Wall");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("GUI_DrawNearMessage", "Combat_Spell_Counter_Message_Box_Draw");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("AI_GetCombatRallyPt", "AI_Stage_Point_For_Unit");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("BU_Apply_Item_Powers", "Battle_Unit_Item_Stats");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("BU_Apply_Item_Enchantments", "Item_Powers_To_Unit_Enchantments");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("BU_Apply_Item_Attack_Specials", "Item_Powers_To_Attack_Attributes");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("BU_Init_Hero_Unit", "Battle_Unit_Hero_Skill_Stats");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("BU_Apply_Battlefield_Effects__WIP", "Battle_Unit_Special_Stats");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("NX_Unit_Has_Spell_As_Enchantment_Or_Item_Power", "NIU_Unit_Has_Spell_Enchantment");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("UU_IDK_turn_to_cast", "UU_IDK_turn_to_cast__STUB");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("SBK_DrawSpellDecode__STUB", "Learn_Spell_Animation_Decode_Draw");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Apprentice_Screen__WIP", "Apprentice_Screen");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Push_Off_Square_With_Message", "Evict_Unit_With_Message");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Evict_Weakest_From_Full_Stack", "Evict_Weakest_Unit");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("CMB_AttackRoll__SEGRAX", "Combat_Roll_Damage_Dealt");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("CMB_DefenseRoll__SEGRAX", "Combat_Roll_Damage_Blocked");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("End_Of_Combat__WIP", "End_Of_Combat");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Ranged_Attack_Strength", "Total_Ranged_Attack_Strength");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("AI_RestrictToCity__WIP", "Update_Move_Map_City_Perimeter_Restrictions");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Get_Player_Mode", "AI_Player_Mode");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("BU_IsCombatSummon", "Battle_Unit_Is_Summoned_Creature");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("STK_CaptureCity", "Combat_City_Capture");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Retreat_Check", "AI_Retreat_Check");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Cast_DeathWish", "Cast_Death_Wish");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Cast_GreatUnsummoning", "Cast_Great_Unsummoning");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Cast_Disenchant", "Combat_Cast_Disenchant");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Cast_Dispel_Magic", "Combat_Cast_Dispel");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("CTY_ChaosRift", "Apply_Chaos_Rift");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("CTY_StreamOfLife", "Apply_Stream_Of_Life");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("CTY_GaiasBlessing", "Apply_Gaias_Blessing");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("CTY_NightshadeDispel", "Apply_Nightshade_Dispel");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Cast_Lycanthropy", "Cast_Lycantrophy");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("CMB_CounterMessage__STUB", "Combat_Spell_Counter_Message");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("WIZ_DispelAttempt__STUB", "Combat_Spell_Dispel_Attempt");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("TILE_BoltFromAbove", "Combat_Spell_Animation_Bolt");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Combat_Spell_Animation_Generic__WIP", "Combat_Spell_Animation_Default");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("BU_Teleport", "Battle_Unit_Teleport");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("BU_TunnelTo", "Battle_Unit_Tunnel");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Cast_Call_Chaos__WIP", "Cast_Call_Chaos");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Apply_Call_Chaos__WIP", "Apply_Call_Chaos");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Overland_Cast_Disenchant", "Cast_Disenchant");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Players_Apply_Upkeeps", "Players_Apply_Upkeeps__WIP");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("WIZ_RemoveCEs", "Remove_City_Enchantments_For_Upkeep");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("WIZ_RemoveGEs", "Remove_Overland_Enchantments_For_Upkeep");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("WIZ_RemoveUEs", "Remove_Unit_Enchantments_For_Upkeep");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("sub_C538E", "Human_Gold_Upkeep_Shortfall__NIU");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("sub_C53FA", "Human_Mana_Upkeep_Shortfall__NIU");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Apply_City_Changes", "gd_dump_cities");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Diplomacy_Growth_For_Enchantments__WIP", "Diplomacy_Growth_For_Enchantments");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("empty_fxn_o142p03", "o142p03_empty_function");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("LD_MAP_TFUnk40_Eval", "Record_Unknown_40H_Squares");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("WIZ_Get_Nation_Size", "Empire_Population_Points");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Compute_Base_Map_Square_Values", "Compute_Base_Values_For_Map_Square");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("CTY_Food_Breakdown", "City_Food_Breakdown");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("CTY_Gold_Breakdown", "City_Gold_Breakdown");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("CTY_Prod_Breakdown", "City_Production_Breakdown");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("CTY_Rsrc_Breakdown", "City_Research_Breakdown");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("CTY_Pwr_Breakdown", "City_Power_Breakdown");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("TILE_GenerateOre", "Generate_Ore");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("EVNT_TargetDepletion__STUB", "Depletion_Event");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("EVNT_FindNewMineral__STUB", "New_Mineral_Event");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("TILE_ReplaceMinerals", "Replace_Minerals");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("AI_Landmass_Strength_Ratios", "AI_Landmass_Values_And_Strengths");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("UU_OVL_Return_2", "o148p01_empty_function");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("UU_OVL_GetLinePath", "Move_Path_Straight_Line__NIU");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("TILE_ExtendRange", "Move_Path_Search_Box");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("sub_D601B", "Move_Path_Build__NIU");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("HLP_NoResizeExpand__TODO", "Draw_Expanding_Box");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("HLP_DrawExpanding__TODO", "Draw_Expanding_Box_Picture");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("G_USW_DrawEHLines", "Redraw_Expanding_Box_Lines");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("HLP_DrawExpandLines__TODO", "Draw_Expanding_Box_Lines");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("ITEM_GetViewDrawPos", "Expanding_Picture_Position");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("AI_OVL_PickSummon", "AI_Select_Spell_Group_Summon");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("AI_OVL_PickRealmSupr", "AI_Select_Spell_Group_Suppression");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("AI_OVL_PickGlobal", "AI_Select_Spell_Group_Global");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Disp_PSTR_Known_Spells", "DBG_Disp_PSTR_Known_Spells");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("ATPI_Node", "AITP_Node");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Map_Square_Is_Legal_For_Floating_Island", "Square_Is_Legal_For_Floating_Island");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("AI_Find_Nearest_Target_Unit", "AI_Stacks_Target_Nearest_Hostile_Stack");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("AI_Order_Meld", "AI_Stacks_Order_Meld");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("AI_Ferry_Add_Location", "AI_Stacks_Ferry_Add_Location");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("AI_Reevaluate_Continent", "gd_dump_ai_continents");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Draw_Help_Entry", "Draw_Help_Entry__WIP");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Draw_Help_Dummy_AF", "Draw_Help_Entry_Draw");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Draw_Help_Map_VGAFILEH", "EMM_Map4Pages_VGAFILEH1234");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Loaded_Game_Update__ovr160", "Loaded_Game_Update");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Map_Square_Production_Bonus", "Square_Production_Bonus");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Map_Square_Gold_Bonus", "Square_Gold_Bonus");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("TILE_GetSilverGold", "Square_Silver_Gold_Income");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("TILE_GetGoldOreGold", "Square_Gold_Ore_Gold_Income");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("TILE_GetGemGold", "Square_Gems_Gold_Income");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Map_Square_Magic_Power", "Square_Magic_Power");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("TILE_GetQuorkPower", "Square_Quork_Power");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("TILE_GetCrysxPower", "Square_Crysx_Power");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Terrain_Unit_Cost_Reduction", "Square_Unit_Cost_Reduction");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("AI_Evaluate_Continents", "gd_ci_get");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("AI_Evaluation_Map", "gd_dump_ai_evaluation_map");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("DBG_Open_ERROR_LOG", "DBG_Open_ERROR_LOG__STUB");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("DBG_Close_ERROR_LOG", "DBG_Close_ERROR_LOG__STUB");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Allocate_Space_No_Header__LbxHeader", "Allocate_Space_No_Header");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("UU_LBXR_LoadReplace", "LBX_Reload_Data");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("UU_LBXR_LoadAppend", "LBX_Reload_Next_Data");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("EMM_Detect_Device", "EMM_DetectDevice");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("EMM_Number_Of_Handles", "EMM_GetHandleCount");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("EMM_Get_Free_Page_Count", "EMM_GetFreePageCnt");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("EMM_Get_Handle_Name", "EMM_GetHandleName");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("EMM_Release_Handle", "EMM_ReleaseHandle__SUTB");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("EMM_Get_Page_Frame", "EMM_GetPageFrame");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("EMM_Release_All", "EMM_ReleaseAll__STUB");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("EMM_Get_Handle", "EMM_GetHandle");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("EMMDATAH_Allocate_First", "EMM_EMMDATAH_AllocFirst");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("EMMDATAH_Allocate_Next", "EMM_EMMDATAH_AllocNext");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("EMMDATAH_Save_Last_Offset", "UU_EMM_Data_Mark");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("EMMDATAH_Restore_Last_Offset", "UU_EMM_Data_Undo");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("EMM_Error_Message", "EMM_GetLowString");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("PSTRM", "ST_PSTRM");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Disp_PSTR_Wrapper", "DBG_Disp_PSTR");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("PSTR", "ST_PSTR");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Print_Character_ASM_", "Print_Character_ASM");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Print_Character_No_Alias_ASM_", "Print_Character_No_Alias_ASM");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("UU_VGA_WndDrawText", "Print_Paragraph_Aligned__NIU");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("UU_GUI_CreateHelpLinks", "Add_Paragraph_Link_Fields__NIU");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Cycle_Palette_Color__STUB", "Cycle_Palette_Color");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("VGA_SlideColors__STUB", "Rotate_Palette_Colors");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("NX_VGA_ColorWave", "Wave_Palette_Colors__NIU");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Copy_Memory_Less", "Copy_Memory_Less_Than");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Test_Bitfield_Near", "Test_Bit_Field_Near");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Draw_Frame_ASM", "FLIC_Draw_Frame");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Remap_Draw_Frame_ASM", "FLIC_Remap_Draw_Frame");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("VGA_WndDrawRotateImg", "Draw_Bitmap_Rotated");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("VGA_WndDrawImageRect", "Draw_Bitmap_Animation_Block");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("VGA_RotateRect", "Get_Rotated_Pixels");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Clear_Bitmap_Pixels", "Clear_Memory_Far");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("VGA_DrawTexture", "Texture_Map_Block");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("VGA_DrawTexture_R", "Remap_Texture_Map_Block");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("RP_MOUSE_SetUsable", "RP_Mouse_SetUsable");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Disable_Cancel", "Disable_Cancel__rnsync_tmp");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Enable_Cancel", "Disable_Cancel");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Disable_Cancel__rnsync_tmp", "Enable_Cancel");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("UU_GUI_KeyWaitLoop", "Read_Character");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("UU_GUI_CreateHelpLink", "Add_Click_Link_Field__NIU");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("NX_UU_GUI_NoSelectColor", "Set_Unselectable_Text_Color__NIU");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("UU_Get_Input_Delay", "Get_Input_Delay");

    Message("wzd-name-sync-summary,attempted=%d,ok=%d,failed=%d\n", attempted, ok, attempted - ok);
    if ( !SaveBase("", 0) )
    {
        Message("wzd-name-sync-save-failed\n");
        Batch(0);
        Exit(2);
    }
    Message("wzd-name-sync-saved\n");
    Batch(0);
    if ( ok != attempted )
        Exit(1);
    Exit(0);
}
