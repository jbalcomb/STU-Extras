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
  update_count = 49;
  log_path = GetIdbPath() + ".completion-sync.log";
  log_handle = fopen(log_path, "w");
  if (log_handle == 0) {
    Warning("Could not open log file: %s", log_path);
    return;
  }
  Message("Applying completion color 00FFFF80 to WIZARDS...\n");
  log_line(log_handle, form("game,%s", "WIZARDS"));
  log_line(log_handle, form("completed_color,%s", "00FFFF80"));
  log_line(log_handle, form("update_count,%d", update_count));
  sync_completion(log_handle, "Allocate_Data_Space", 0x00FFFF80);
  sync_completion(log_handle, "Allocate_Space", 0x00FFFF80);
  sync_completion(log_handle, "Allocate_Space_No_Header__LbxHeader", 0x00FFFF80);
  sync_completion(log_handle, "Allocation_Error", 0x00FFFF80);
  sync_completion(log_handle, "Copy_Mouse_Off_To_Mouse_Back", 0x00FFFF80);
  sync_completion(log_handle, "Draw_Mouse_Off_Page", 0x00FFFF80);
  sync_completion(log_handle, "Draw_Mouse_Off_Page_", 0x00FFFF80);
  sync_completion(log_handle, "Draw_Mouse_On_Page", 0x00FFFF80);
  sync_completion(log_handle, "Draw_Mouse_On_Page_", 0x00FFFF80);
  sync_completion(log_handle, "EMM_CheckHandleOpen", 0x00FFFF80);
  sync_completion(log_handle, "EMM_Detect_Device", 0x00FFFF80);
  sync_completion(log_handle, "EMM_Get_Handle", 0x00FFFF80);
  sync_completion(log_handle, "EMM_LBXR_DirectLoad", 0x00FFFF80);
  sync_completion(log_handle, "EMM_Load_LBX_File", 0x00FFFF80);
  sync_completion(log_handle, "EMM_Load_LBX_File_1", 0x00FFFF80);
  sync_completion(log_handle, "EMM_MapnRead", 0x00FFFF80);
  sync_completion(log_handle, "EMM_Startup", 0x00FFFF80);
  sync_completion(log_handle, "Error_Handler", 0x00FFFF80);
  sync_completion(log_handle, "FLIC_Get_CurrentFrame", 0x00FFFF80);
  sync_completion(log_handle, "File_Name_Base", 0x00FFFF80);
  sync_completion(log_handle, "Get_Free_Blocks", 0x00FFFF80);
  sync_completion(log_handle, "Get_String_Width", 0x00FFFF80);
  sync_completion(log_handle, "Hall_Of_Fame_Screen", 0x00FFFF80);
  sync_completion(log_handle, "Interpret_Keyboard_Input", 0x00FFFF80);
  sync_completion(log_handle, "Interpret_Mouse_Input", 0x00FFFF80);
  sync_completion(log_handle, "LBX_Load", 0x00FFFF80);
  sync_completion(log_handle, "LBX_Load_Data", 0x00FFFF80);
  sync_completion(log_handle, "LBX_Load_Data_Static", 0x00FFFF80);
  sync_completion(log_handle, "LBX_Reload", 0x00FFFF80);
  sync_completion(log_handle, "LBX_Reload_Next", 0x00FFFF80);
  sync_completion(log_handle, "Load_Screen", 0x00FFFF80);
  sync_completion(log_handle, "Load_Screen_Draw", 0x00FFFF80);
  sync_completion(log_handle, "Mouse_Buffer", 0x00FFFF80);
  sync_completion(log_handle, "RAM_Set_Minimum", 0x00FFFF80);
  sync_completion(log_handle, "Restore_Mouse_Off_Page", 0x00FFFF80);
  sync_completion(log_handle, "Restore_Mouse_On_Page", 0x00FFFF80);
  sync_completion(log_handle, "SA_MK_FP0", 0x00FFFF80);
  sync_completion(log_handle, "Save_Mouse_Off_Page", 0x00FFFF80);
  sync_completion(log_handle, "Save_Mouse_On_Page", 0x00FFFF80);
  sync_completion(log_handle, "Save_Mouse_On_Page_", 0x00FFFF80);
  sync_completion(log_handle, "Scan_Input", 0x00FFFF80);
  sync_completion(log_handle, "Toggle_Pages", 0x00FFFF80);
  sync_completion(log_handle, "VGA_TextDraw_Init", 0x00FFFF80);
  sync_completion(log_handle, "_fstrcpy", 0x00FFFF80);
  sync_completion(log_handle, "lbx_close", 0x00FFFF80);
  sync_completion(log_handle, "lbx_open", 0x00FFFF80);
  sync_completion(log_handle, "lbx_read_ofst", 0x00FFFF80);
  sync_completion(log_handle, "lbx_read_sgmt", 0x00FFFF80);
  sync_completion(log_handle, "lbx_seek", 0x00FFFF80);
  fclose(log_handle);
  Message("Wrote completion sync log to %s\n", log_path);
}
