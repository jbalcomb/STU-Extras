#include <idc.idc>

static log_line(handle, message)
{
  fprintf(handle, "%s\n", message);
}

static sync_completion(log_handle, old_name, desired_color)
{
  auto ea;
  auto current_color;
  ea = LocByName(old_name);
  if (ea == BADADDR) {
    Message("Missing symbol: %s\n", old_name);
    log_line(log_handle, form("missing,%s", old_name));
    return;
  }
  current_color = GetColor(ea, CIC_FUNC);
  if (current_color == desired_color) {
    log_line(log_handle, form("already-set,%s,%08X", old_name, current_color));
    return;
  }
  if (!SetColor(ea, CIC_FUNC, desired_color)) {
    Message("SetColor failed: %s\n", old_name);
    log_line(log_handle, form("setcolor-failed,%s,%08X,%08X", old_name, current_color, desired_color));
    return;
  }
  log_line(log_handle, form("updated,%s,%08X,%08X", old_name, current_color, desired_color));
}

static main()
{
  auto log_path;
  auto log_handle;
  auto update_count;
  update_count = 142;
  log_path = GetIdbPath() + ".completion-sync.log";
  log_handle = fopen(log_path, "w");
  if (log_handle == 0) {
    Warning("Could not open log file: %s", log_path);
    return;
  }
  Message("Applying completion color 00FFFF80 to MAGIC...\n");
  log_line(log_handle, form("game,%s", "MAGIC"));
  log_line(log_handle, form("completed_color,%s", "00FFFF80"));
  log_line(log_handle, form("update_count,%d", update_count));
  sync_completion(log_handle, "Add_Button_Field", 0x00FFFF80);
  sync_completion(log_handle, "Add_Button_Info", 0x00FFFF80);
  sync_completion(log_handle, "Add_Continuous_String_Input_Field", 0x00FFFF80);
  sync_completion(log_handle, "Add_Input_Field", 0x00FFFF80);
  sync_completion(log_handle, "Add_Multi_Hot_Key_Field", 0x00FFFF80);
  sync_completion(log_handle, "Add_Picture_Field", 0x00FFFF80);
  sync_completion(log_handle, "Add_Picture_To_Bitmap", 0x00FFFF80);
  sync_completion(log_handle, "Add_String_List_Field", 0x00FFFF80);
  sync_completion(log_handle, "Auto_Input", 0x00FFFF80);
  sync_completion(log_handle, "Check_Default_Video_Page", 0x00FFFF80);
  sync_completion(log_handle, "Check_Help_List", 0x00FFFF80);
  sync_completion(log_handle, "Check_Release_Version", 0x00FFFF80);
  sync_completion(log_handle, "Clear_Bitmap_Pixels", 0x00FFFF80);
  sync_completion(log_handle, "Clear_Bitmap_Region", 0x00FFFF80);
  sync_completion(log_handle, "Clear_Palette_Changes", 0x00FFFF80);
  sync_completion(log_handle, "Clipped_Dot", 0x00FFFF80);
  sync_completion(log_handle, "Clipped_Draw_Frame", 0x00FFFF80);
  sync_completion(log_handle, "Clipped_Fill", 0x00FFFF80);
  sync_completion(log_handle, "Clipped_Line", 0x00FFFF80);
  sync_completion(log_handle, "Clipped_Line_Base", 0x00FFFF80);
  sync_completion(log_handle, "Clipped_Multi_Colored_Line", 0x00FFFF80);
  sync_completion(log_handle, "Clipped_Print_Centered", 0x00FFFF80);
  sync_completion(log_handle, "Clipped_Print_Centered_Far", 0x00FFFF80);
  sync_completion(log_handle, "Clipped_Print_Far", 0x00FFFF80);
  sync_completion(log_handle, "Clipped_Print_Integer", 0x00FFFF80);
  sync_completion(log_handle, "Clipped_Print_Integer_Right", 0x00FFFF80);
  sync_completion(log_handle, "Clipped_Print_Long", 0x00FFFF80);
  sync_completion(log_handle, "Clipped_Print_Long_Right", 0x00FFFF80);
  sync_completion(log_handle, "Clipped_Print_Right", 0x00FFFF80);
  sync_completion(log_handle, "Clipped_Print_Right_Far", 0x00FFFF80);
  sync_completion(log_handle, "Copy_Bitmap_To_Bitmap", 0x00FFFF80);
  sync_completion(log_handle, "Cos", 0x00FFFF80);
  sync_completion(log_handle, "Create_Blank_Picture", 0x00FFFF80);
  sync_completion(log_handle, "Create_Multi_Frame_Blank_Bitmap", 0x00FFFF80);
  sync_completion(log_handle, "DBG_Close_ERROR_LOG", 0x00FFFF80);
  sync_completion(log_handle, "DBG_Open_ERROR_LOG", 0x00FFFF80);
  sync_completion(log_handle, "Delete_Structure", 0x00FFFF80);
  sync_completion(log_handle, "Delta_XY_With_Wrap", 0x00FFFF80);
  sync_completion(log_handle, "Disable_Mouse_Wait", 0x00FFFF80);
  sync_completion(log_handle, "Disp_PSTR", 0x00FFFF80);
  sync_completion(log_handle, "Disp_PSTR_Wrapper", 0x00FFFF80);
  sync_completion(log_handle, "Dot", 0x00FFFF80);
  sync_completion(log_handle, "Draw_File_Animation", 0x00FFFF80);
  sync_completion(log_handle, "Draw_Input_Box_Popup", 0x00FFFF80);
  sync_completion(log_handle, "Draw_Picture_ASM", 0x00FFFF80);
  sync_completion(log_handle, "EMMDATAH_Restore_Last_Offset", 0x00FFFF80);
  sync_completion(log_handle, "EMMDATAH_Save_Last_Offset", 0x00FFFF80);
  sync_completion(log_handle, "Enable_Mouse_Wait", 0x00FFFF80);
  sync_completion(log_handle, "Enlarge_Bitmap", 0x00FFFF80);
  sync_completion(log_handle, "Exit_With_Value", 0x00FFFF80);
  sync_completion(log_handle, "FLIC_Draw", 0x00FFFF80);
  sync_completion(log_handle, "Find_Shortest_Path", 0x00FFFF80);
  sync_completion(log_handle, "Get_Angle", 0x00FFFF80);
  sync_completion(log_handle, "Get_Base_Angle", 0x00FFFF80);
  sync_completion(log_handle, "Get_Bitmap_Actual_Size", 0x00FFFF80);
  sync_completion(log_handle, "Get_Current_Font_Style", 0x00FFFF80);
  sync_completion(log_handle, "Get_Current_Highlight_Color", 0x00FFFF80);
  sync_completion(log_handle, "Get_Current_Normal_Color", 0x00FFFF80);
  sync_completion(log_handle, "Get_Current_Special_Color", 0x00FFFF80);
  sync_completion(log_handle, "Get_File_Animation_Frame", 0x00FFFF80);
  sync_completion(log_handle, "Get_Font_Vertical_Spacing", 0x00FFFF80);
  sync_completion(log_handle, "Get_Full_Store_Flag", 0x00FFFF80);
  sync_completion(log_handle, "Get_List_Field", 0x00FFFF80);
  sync_completion(log_handle, "Get_Paragraph_Max_Height", 0x00FFFF80);
  sync_completion(log_handle, "Get_Paragraph_Max_Width", 0x00FFFF80);
  sync_completion(log_handle, "Get_Random_Seed", 0x00FFFF80);
  sync_completion(log_handle, "Gradient_Fill", 0x00FFFF80);
  sync_completion(log_handle, "Input_Box_Popup", 0x00FFFF80);
  sync_completion(log_handle, "Line", 0x00FFFF80);
  sync_completion(log_handle, "Load_File_Animation_Frame", 0x00FFFF80);
  sync_completion(log_handle, "Load_MAGIC_SET", 0x00FFFF80);
  sync_completion(log_handle, "Load_Palette_From_Animation", 0x00FFFF80);
  sync_completion(log_handle, "Mark_Block", 0x00FFFF80);
  sync_completion(log_handle, "Mark_Paragraph", 0x00FFFF80);
  sync_completion(log_handle, "Mask_Bitmap_Pixels", 0x00FFFF80);
  sync_completion(log_handle, "Mouse_Button", 0x00FFFF80);
  sync_completion(log_handle, "Mouse_Button_Handler", 0x00FFFF80);
  sync_completion(log_handle, "Mouse_Movement_Handler", 0x00FFFF80);
  sync_completion(log_handle, "Mouse_Wait", 0x00FFFF80);
  sync_completion(log_handle, "No_Print_Alias", 0x00FFFF80);
  sync_completion(log_handle, "Open_File_Animation", 0x00FFFF80);
  sync_completion(log_handle, "PSTR", 0x00FFFF80);
  sync_completion(log_handle, "PSTRL", 0x00FFFF80);
  sync_completion(log_handle, "PSTRM", 0x00FFFF80);
  sync_completion(log_handle, "PSTRS", 0x00FFFF80);
  sync_completion(log_handle, "PSTRU", 0x00FFFF80);
  sync_completion(log_handle, "Path_Wrap", 0x00FFFF80);
  sync_completion(log_handle, "Print_Centered_Far", 0x00FFFF80);
  sync_completion(log_handle, "Print_Far", 0x00FFFF80);
  sync_completion(log_handle, "Print_Full", 0x00FFFF80);
  sync_completion(log_handle, "Print_Integer", 0x00FFFF80);
  sync_completion(log_handle, "Print_Integer_Centered", 0x00FFFF80);
  sync_completion(log_handle, "Print_Integer_Right", 0x00FFFF80);
  sync_completion(log_handle, "Print_Long", 0x00FFFF80);
  sync_completion(log_handle, "Print_Long_Right", 0x00FFFF80);
  sync_completion(log_handle, "Print_Message", 0x00FFFF80);
  sync_completion(log_handle, "Print_Right", 0x00FFFF80);
  sync_completion(log_handle, "Print_Right_Far", 0x00FFFF80);
  sync_completion(log_handle, "Push_Field_Down", 0x00FFFF80);
  sync_completion(log_handle, "Reduce_Bitmap", 0x00FFFF80);
  sync_completion(log_handle, "Release_Block", 0x00FFFF80);
  sync_completion(log_handle, "Release_Version", 0x00FFFF80);
  sync_completion(log_handle, "Remap_Draw_Picture_ASM", 0x00FFFF80);
  sync_completion(log_handle, "Remove_Paragraph_Marks", 0x00FFFF80);
  sync_completion(log_handle, "Replace_Color", 0x00FFFF80);
  sync_completion(log_handle, "Reset_Cycle_Palette_Color", 0x00FFFF80);
  sync_completion(log_handle, "Reset_System_Mouse", 0x00FFFF80);
  sync_completion(log_handle, "Scale_Bitmap", 0x00FFFF80);
  sync_completion(log_handle, "Screen_Flic_Capture", 0x00FFFF80);
  sync_completion(log_handle, "Set_Default_Game_Settings", 0x00FFFF80);
  sync_completion(log_handle, "Set_File_Animation_Frame", 0x00FFFF80);
  sync_completion(log_handle, "Set_Font_LF", 0x00FFFF80);
  sync_completion(log_handle, "Set_Font_Spacing", 0x00FFFF80);
  sync_completion(log_handle, "Set_Font_Spacing_Width", 0x00FFFF80);
  sync_completion(log_handle, "Set_Font_Style_Outline", 0x00FFFF80);
  sync_completion(log_handle, "Set_Font_Style_Outline_Heavy", 0x00FFFF80);
  sync_completion(log_handle, "Set_Font_Style_Shadow_Down", 0x00FFFF80);
  sync_completion(log_handle, "Set_Font_Style_Shadow_Heavy", 0x00FFFF80);
  sync_completion(log_handle, "Set_Font_Style_Shadow_Up", 0x00FFFF80);
  sync_completion(log_handle, "Set_Mouse_Position", 0x00FFFF80);
  sync_completion(log_handle, "Set_Palette_Changes", 0x00FFFF80);
  sync_completion(log_handle, "Set_Pointer_Position", 0x00FFFF80);
  sync_completion(log_handle, "Set_Random_Seed", 0x00FFFF80);
  sync_completion(log_handle, "Settings_Screen", 0x00FFFF80);
  sync_completion(log_handle, "Settings_Screen_Draw", 0x00FFFF80);
  sync_completion(log_handle, "Setup_Input_Box_Popup", 0x00FFFF80);
  sync_completion(log_handle, "Sin", 0x00FFFF80);
  sync_completion(log_handle, "Swap_Short", 0x00FFFF80);
  sync_completion(log_handle, "Tint_Fill", 0x00FFFF80);
  sync_completion(log_handle, "Tint_Palette", 0x00FFFF80);
  sync_completion(log_handle, "Trim", 0x00FFFF80);
  sync_completion(log_handle, "UU_DBG_SelectDialog", 0x00FFFF80);
  sync_completion(log_handle, "UU_DBG_SetSelectSetting", 0x00FFFF80);
  sync_completion(log_handle, "UU_EMM_LBX_Load2Hnd", 0x00FFFF80);
  sync_completion(log_handle, "UU_Get_Input_Delay", 0x00FFFF80);
  sync_completion(log_handle, "UU_Interlaced_Fill", 0x00FFFF80);
  sync_completion(log_handle, "UU_Legacy_Startup", 0x00FFFF80);
  sync_completion(log_handle, "UU_VGA_DrawDblRect", 0x00FFFF80);
  sync_completion(log_handle, "UU_VGA_DrawRect", 0x00FFFF80);
  sync_completion(log_handle, "UU_VGA_WndDrawRect", 0x00FFFF80);
  sync_completion(log_handle, "VGA_GetDACBlock", 0x00FFFF80);
  sync_completion(log_handle, "VGA_ReadScreenLine", 0x00FFFF80);
  fclose(log_handle);
  Message("Wrote completion sync log to %s\n", log_path);
}
