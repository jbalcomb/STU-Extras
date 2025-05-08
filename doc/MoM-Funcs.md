


UU_LBX_DOSAlloc                                seg007
UU_DOS_MemAlloc                                seg007
UU_DOS_MemFree                                 seg007
Check_Allocation                               seg007
farpeekb                                       seg007
farpeekw                                       seg007
PEEK_Dword                                     seg007
farpokeb                                       seg007
farpokew                                       seg007
UU_POKE_Dword                                  seg007

Near_Allocate_Mark                             seg008
Near_Allocate_Undo                             seg008
Near_Allocate_Reset                            seg008
Near_Allocate_First                            seg008
Near_Allocate_Next                             seg008
Near_Allocation_Error                          seg008
Allocate_Space                                 seg008
Allocate_Space_No_Header__LbxHeader            seg008
Allocate_Space_No_Header__EntrySize            seg008
Mark_Block                                     seg008
Release_Block                                  seg008
Allocate_First_Block                           seg008
Reset_First_Block                              seg008
Allocate_Next_Block                            seg008
Get_Free_Blocks                                seg008
SA_MK_FP0                                      seg008
UU_combine_32                                  seg008
UU_LBX_Alloc_Free                              seg008
Allocation_Error                               seg008

lbx_open                                       seg009
lbx_close                                      seg009
lbx_seek                                       seg009
UU_DISK_GetFileSize                            seg009
lbx_read_sgmt                                  seg009
lbx_read_ofst                                  seg009
String_Copy_Far                                seg009

LBX_Load                                       seg010
LBX_Reload                                     seg010
LBX_Reload_Next                                seg010
LBX_Load_Data                                  seg010
UU_LBXR_LoadReplace                            seg010
UU_LBXR_LoadAppend                             seg010
UU_LBXE_LoadSingle2                            seg010
UU_LBXE_LoadReplace2                           seg010
UU_LBXE_LoadAppend2                            seg010
LBX_Load_Entry                                 seg010
LBX_Load_Library_Data                          seg010
LBX_Load_Data_Static                           seg010
LBX_GetEntryData                               seg010
UU_LBX_SetPath                                 seg010
Error_Handler                                  seg010
File_Name_Base                                 seg010
RAM_SetMinKB                                   seg010





Fill                                           seg016
VGA_PutPixel                                   seg016
Line                                           seg016
VGA_DrawPatternLine                            seg016
UU_VGA_CreateColorWave                         seg016
UU_VGA_DiagColumns                             seg016
RNG_Direct_LFSR                                seg016
Tint_Fill                                      seg016
Gray_Scale_Fill                                seg016
UU_VGA_Columns                                 seg016
UU_VGA_ScrambleRect                            seg016



Print_Paragraph                                seg019     00000001 0000018B R F . . B T .
UU_VGA_WndDrawText                             seg019     00000195 00000100 R F . . B T .
UU_GUI_CreateHelpLinks                         seg019     00000295 0000030B R F . . B T .
Mark_Paragraph                                 seg019     000005A8 00000330 R F . . B T .
Get_Paragraph_Max_Height                       seg019     000008D8 0000005E R F . . B T .
VGA_GetTextWidth                               seg019     00000936 00000125 R F . . B T .
Reset_Paragraph_Exclusions                     seg019     00000A64 0000000B R F . . B T .
VGA_AddFloatingBlock                           seg019     00000A6F 00000044 R F . . B T .
UU_VGA_AddFltIMGSpace                          seg019     00000AB3 00000074 R F . . B T .
Remove_Paragraph_Marks                         seg019     00000B27 00000026 R F . . B T .
Get_Font_Height                                seg019     00000B4D 00000021 R F . . B T .
UU_VGA_WidestGlyph                             seg019     00000B6E 0000004B R F . . B T .
LBX_DrawText                                   seg019     00000BB9 00000198 R F . . B T .
VGA_TextDraw_Init                              seg019     00000D59 00000075 R F . . B T .
Load_Palette                                   seg020     0000000E 000000E2 R F . . B T .
Update_Remap_Gray_Palette                      seg020     000000F0 0000001D R F . . B T .
Calculate_Remap_Colors                         seg020     0000010D 0000007E R F . . B T .
Update_Remap_Color_Range                       seg020     0000018B 000000A0 R F . . B T .
Set_Palette_Changes                            seg020     0000022B 00000023 R F . . B T .
Clear_Palette_Changes                          seg020     0000024E 00000022 R F . . B T .
UU_VGA_ClearDAC                                seg020     00000270 00000035 R F . . B T .
UU_VGA_SaveDAC                                 seg020     000002A5 0000001F R F . . B T .
UU_VGA_RestoreDAC                              seg020     000002C4 0000002C R F . . B T .
UU_VGA_LoadDAC                                 seg020     000002F0 00000048 R F . . B T .
Reset_Cycle_Palette_Color                      seg020     00000338 0000000B R F . . B T .
Cycle_Palette_Color                            seg020     00000343 000001CC R F . . B T .
Update_Cycle                                   seg020     0000050F 00000094 R F . . B T .
UU_VGA_CreateHues                              seg020     000005A3 000000B8 R F . . B T .



UU_VGA_GetPointInDir                           seg023     00000002 000000FB R F . . B T .
TILE_GetDistance                               seg023     000000FD 0000004B R F . . B T .
Delta_XY_With_Wrap                             seg023     00000148 00000061 R F . . B T .
UU_Tile_PlotLine                               seg023     000001A9 00000181 R F . . B T .
TILE_PlotMapLine                               seg023     0000032A 0000023C R F . . B T .



VGA_FILEH_Draw                                 seg025     0000000E 000001C7 R F . . B T .
VGA_MapNextEMMPage                             seg025     000001D5 0000002F R . . . . T .
VGA_FILEH_Draw_R                               seg025     00000204 000001E7 R F . . B T .



FLIC_Draw_Frame                                seg029     0000000A 00000100 R F . . B T .
FLIC_Remap_Draw_Frame                          seg029     0000010A 000001E6 . F . . B T .
FLIC_Draw_Frame_EMM                            seg029     000002FC 000001C3 . F . . B T .
VGA_MapNextEMMPages                            seg029     000004BF 00000027 R . . . . T .
FLIC_Draw_Frame_EMM_Remap                      seg029     000004E6 000001E7 R F . . B T .



Clipped_Draw_Frame                             seg031     00000012 00000235 R F . . B T .
Clipped_Remap_Draw_Frame                       seg031     00000247 000002EA R F . . B T .

EMS_Clipped_Draw                               seg032     00000010 000002F8 R F . . B T .
EMS_Clipped_Draw_Map4Pages                     seg032     00000308 0000002C R . . . . T .
EMS_Clipped_Remap_Draw                         seg032     00000334 000002F1 R F . . B T .



MATH_Inverse_Tangent                           seg034     000001F5 00000059 R F . . B T .
MATH_Inverse_Tan_QI                            seg034     0000024E 000000AD R . . . . T .
MATH_Int_Cos                                   seg034     000002FB 0000008E R F . . B T .
MATH_Int_Sin                                   seg034     00000389 0000008E R F . . B T .
VGA_DrawTexture                                seg034     00000433 0000017C R F . . B T .
VGA_DrawTexture_R                              seg034     000005AF 000001E5 R F . . B T .

Set_Mouse_List                                 seg035     0000006C 00000044 R F . . B T .
Check_Mouse_Shape                              seg035     000000B0 0000005A R F . . B T .
Get_Pointer_Image_Number                       seg035     0000010A 0000000B R F . . . T .
Get_Pointer_Offset                             seg035     00000115 0000000B R F . . . T .
Mouse_Button                                   seg035     00000120 00000026 R F . . . T .
MOUSE_WaitRelease                              seg035     00000146 00000020 R F . . . T .
Init_Mouse_Driver                              seg035     00000166 00000096 R F . . B T .
RP_MOUSE_SetUsable                             seg035     000001FC 0000000E R F . . . T .
MOUSE_Reset                                    seg035     0000020A 00000048 R F . . . T .
Mouse_Movement_Handler                         seg035     00000252 00000035 R F . . . T .
Mouse_Button_Handler                           seg035     00000287 00000035 R F . . . T .
MOUSE_INT_Handler                              seg035     000002BC 000000E0 R F . . B T .
Save_Mouse_State                               seg035     0000039C 00000018 R F . . . T .
Restore_Mouse_State                            seg035     000003B4 00000012 R F . . . T .
UU_MOUSE_Usable                                seg035     000003C6 00000015 R F . . . T .
Keyboard_Status                                seg035     000003DB 00000020 R F . . . T .
Read_Key                                       seg035     000003FB 0000025F R F . . . T .
Pointer_X                                      seg035     0000065A 0000000B R F . . . T .
Pointer_Y                                      seg035     00000665 0000000B R F . . . T .
Set_Pointer_Position                           seg035     00000670 0000002B R F . . B T .
Set_Mouse_Position                             seg035     0000069B 00000016 R F . . B T .
MOUSE_GetClickX                                seg035     000006B1 0000000B R F . . . T .
MOUSE_GetClickY                                seg035     000006BC 0000000B R F . . . T .
MD_GetClickedBtns                              seg035     000006C7 0000000B R F . . . T .
Mouse_Buffer                                   seg035     000006D2 00000011 R F . . . T .
Mouse_Buffer2                                  seg035     000006E3 00000011 R F . . . T .
RP_MOUSE_SetSecClick                           seg035     000006F4 0000001E R F . . B T .
UU_MOUSE_GenerateClick                         seg035     00000712 0000002A R F . . B T .
MOUSE_SaveClick                                seg035     0000073C 0000003A R F . . B T .
Save_Mouse_On_Page                             seg035     00000776 000000D3 R F . . B T .
Save_Mouse_Off_Page                            seg035     00000849 000000DE R F . . B T .
Save_Mouse_On_Page_                            seg035     00000927 000000B7 R F . . B T .
Copy_Mouse_Off_To_Mouse_Back                   seg035     000009DE 0000001B R F . . . T .
Restore_Mouse_On_Page                          seg035     000009F9 00000090 R F . . B T .
Restore_Mouse_Off_Page                         seg035     00000A89 00000094 R F . . B T .
UU_VGA_RestoreToFrame3                         seg035     00000B1D 00000085 R F . . B T .
Draw_Mouse_On_Page                             seg035     00000BA2 00000026 R F . . B T .
Draw_Mouse_Off_Page                            seg035     00000BC8 00000026 R F . . B T .
Draw_Mouse_On_Page_                            seg035     00000BEE 000000D1 R F . . B T .
Draw_Mouse_Off_Page_                           seg035     00000CBF 000000D5 R F . . B T .
UU_GUI_ClearClickBuffer                        seg035     00000D94 0000001E R F . . . T .
UU_GUI_ClickBuffReset                          seg035     00000DB2 00000007 R F . . . T .
UU_GUI_SetClickBuffVar1                        seg035     00000DB9 00000007 R F . . . T .
UU_GUI_ClearBufferClick                        seg035     00000DC0 0000001E R F . . . T .
UU_GUI_FindEmptyClick                          seg035     00000DDE 00000033 R F . . . T .
UU_GUI_GetClickBuffVar1                        seg035     00000E11 00000004 R F . . . T .
UU_GUI_AddBufferClick                          seg035     00000E15 0000004E R F . . B T .
UU_GUI_ProcBufferClick                         seg035     00000E63 00000079 R F . . . T .



_find_proc                                     _AIL_      000003BA 00000038 R F L . . T .
_call_driver                                   _AIL_      000003F2 00000018 R F L . . T .
_API_timer                                     _AIL_      0000040A 000000D1 R F L . . T .
_init_DDA_arrays                               _AIL_      000004DB 0000003C R F L . . T .
_bios_caller                                   _AIL_      00000517 00000007 R F L . . T .
_hook_timer_process                            _AIL_      0000051E 0000004C R F L . . T .
_unhook_timer_proc                             _AIL_      0000056A 00000030 R F L . . T .
_set_PIT_divisor                               _AIL_      0000059A 00000029 R F L . B T .
_set_PIT_period                                _AIL_      000005C3 0000002A R F L . B T .
_ul_divide                                     _AIL_      000005ED 0000004B R F L . B T .
_program_timers                                _AIL_      00000638 00000095 R F L . . T .
_AIL_startup                                   _AIL_      000006CD 0000004B R F L . . T .
_AIL_shutdown                                  _AIL_      00000718 00000065 R F L . B T .
_AIL_register_timer                            _AIL_      0000077D 000000CB R F L . B T .
_AIL_release_tmr_hnd                           _AIL_      00000848 00000049 R F L . B T .
_AIL_release_all_tmrs                          _AIL_      00000891 0000001E R F L . . T .
_AIL_start_timer                               _AIL_      000008AF 00000028 R F L . B T .
_AIL_start_all_timers                          _AIL_      000008D7 0000001E R F L . . T .
_AIL_stop_timer                                _AIL_      000008F5 00000028 R F L . B T .
_AIL_stop_all_timers                           _AIL_      0000091D 0000001E R F L . . T .
_AIL_set_timer_prd                             _AIL_      0000093B 00000055 R F L . B T .
_AIL_set_timer_freq                            _AIL_      00000990 00000041 R F L . B T .
_AIL_set_timer_divsr                           _AIL_      000009D1 0000003C R F L . B T .
_AIL_int_divisor                               _AIL_      00000A0D 00000014 R F L . . T .
_AIL_register_driver                           _AIL_      00000A21 000000E3 R F L . B T .
_AIL_release_drv_hnd                           _AIL_      00000B04 00000047 R F L . B T .
_AIL_describe_driver                           _AIL_      00000B4B 00000026 R F L . B T .
_AIL_detect_device                             _AIL_      00000B71 00000006 R F L . . T .
_AIL_init_driver                               _AIL_      00000B77 000000B2 R F L . B T .
_AIL_shutdown_drvr                             _AIL_      00000C29 00000033 R F L . . T .
UU_AIL_index_VOC_blk                           _AIL_      00000C5C 00000006 R F L . . T .
UU_AIL_reg_snd_buffer                          _AIL_      00000C62 00000006 R F L . . T .
UU_AIL_format_snd_buff                         _AIL_      00000C68 00000006 R F L . . T .
UU_AIL_snd_buff_status                         _AIL_      00000C6E 00000006 R F L . . T .
_AIL_play_VOC_file                             _AIL_      00000C74 00000006 R F L . . T .
_AIL_format_VOC_file                           _AIL_      00000C7A 00000006 R F L . . T .
UU_AIL_VOC_play_status                         _AIL_      00000C80 00000006 R F L . . T .
_AIL_start_digi_play                           _AIL_      00000C86 00000006 R F L . . T .
_AIL_stop_digi_play                            _AIL_      00000C8C 00000006 R F L . . T .
UU_AIL_pause_digi_play                         _AIL_      00000C92 00000006 R F L . . T .
UU_AIL_resm_digi_play                          _AIL_      00000C98 00000006 R F L . . T .
UU_AIL_set_digi_volume                         _AIL_      00000C9E 00000006 R F L . . T .
UU_AIL_get_digi_volume                         _AIL_      00000CA4 00000006 R F L . . T .
UU_AIL_set_digi_panpot                         _AIL_      00000CAA 00000006 R F L . . T .
UU_AIL_get_digi_panpot                         _AIL_      00000CB0 00000006 R F L . . T .
_AIL_state_tbl_size                            _AIL_      00000CB6 00000006 R F L . . T .
_AIL_register_seq                              _AIL_      00000CBC 00000006 R F L . . T .
_AIL_release_seq_hnd                           _AIL_      00000CC2 00000006 R F L . . T .
_AIL_t_cache_size                              _AIL_      00000CC8 00000006 R F L . . T .
_AIL_define_t_cache                            _AIL_      00000CCE 00000006 R F L . . T .
_AIL_timbre_req                                _AIL_      00000CD4 00000006 R F L . . T .
_AIL_install_timbre                            _AIL_      00000CDA 00000006 R F L . . T .
UU_AIL_protect_timbre                          _AIL_      00000CE0 00000006 R F L . . T .
UU_AIL_unprotect_t                             _AIL_      00000CE6 00000006 R F L . . T .
UU_AIL_timbre_status                           _AIL_      00000CEC 00000006 R F L . . T .
_AIL_start_sequence                            _AIL_      00000CF2 00000006 R F L . . T .
_AIL_stop_sequence                             _AIL_      00000CF8 00000006 R F L . . T .
UU_AIL_resume_seq                              _AIL_      00000CFE 00000006 R F L . . T .
_AIL_sequence_status                           _AIL_      00000D04 00000006 R F L . . T .
UU_AIL_relative_volume                         _AIL_      00000D0A 00000006 R F L . . T .
UU_AIL_relative_tempo                          _AIL_      00000D10 00000006 R F L . . T .
_AIL_set_rel_volume                            _AIL_      00000D16 00000006 R F L . . T .
UU_AIL_set_rel_tempo                           _AIL_      00000D1C 00000006 R F L . . T .
UU_AIL_beat_count                              _AIL_      00000D22 00000006 R F L . . T .
UU_AIL_measure_cnt                             _AIL_      00000D28 00000006 R F L . . T .
UU_AIL_branch_index                            _AIL_      00000D2E 00000006 R F L . . T .
UU_AIL_ctrl_value                              _AIL_      00000D34 00000006 R F L . . T .
UU_AIL_set_ctrl_value                          _AIL_      00000D3A 00000006 R F L . . T .
UU_AIL_channel_notes                           _AIL_      00000D40 00000006 R F L . . T .
UU_AIL_send_cv_msg                             _AIL_      00000D46 00000006 R F L . . T .
UU_AIL_send_sysex_msg                          _AIL_      00000D4C 00000006 R F L . . T .
UU_AIL_write_display                           _AIL_      00000D52 00000006 R F L . . T .
UU_AIL_install_cb                              _AIL_      00000D58 00000006 R F L . . T .
UU_AIL_cancel_cb                               _AIL_      00000D5E 00000006 R F L . . T .
UU_AIL_lock_channel                            _AIL_      00000D64 00000006 R F L . . T .
UU_AIL_map_seq_channel                         _AIL_      00000D6A 00000006 R F L . . T .
UU_AIL_release_channel                         _AIL_      00000D70 00000006 R F L . . T .
UU_AIL_true_seq_chan                           _AIL_      00000D76 00000006 R F L . . T .
_AIL_unhook                                    _AIL_      00000E7A 00000037 R F L . . T .
_AIL_hook                                      _AIL_      00000EB1 00000037 R F L . . T .
UU_sub_ptr                                     _AIL_      00000EE8 00000047 R F L . B T .
_block_type                                    _AIL_      00000F2F 0000000F R F L . . T .
_marker_num                                    _AIL_      00000F3E 00000017 R F L . . T .
_get_sample_rate                               _AIL_      00000F55 0000003B R F L . B T .
_DP_preformat                                  _AIL_      00000F90 000000B3 R F L . B T .
_CB_set_vect                                   _AIL_      00001043 0000001A R F L . B T .
_CB_play_VOC                                   _AIL_      0000105D 0000001F R F L . . T .
_CB_play_buffer                                _AIL_      0000107C 00000034 R F L . . T .
_next_block                                    _AIL_      000010B0 00000055 R F L . . T .
_next_buffer                                   _AIL_      00001105 00000027 R F L . . T .
_process_block                                 _AIL_      0000112C 00000157 R F L . . T .
_process_buffer                                _AIL_      00001283 00000053 R F L . B T .
_DMA_transfer                                  _AIL_      000012D6 0000002B R F L . B T .
_xfer_chunk                                    _AIL_      00001301 000000D7 R F L . B T .
_DP_describe_driver                            _AIL_      000013D8 00000080 R F L . B T .
_DP_detect_device                              _AIL_      00001458 0000000E R F L . B T .
_DP_init_driver                                _AIL_      00001466 00000065 R F L . B T .
_DP_shutdown_driver                            _AIL_      000014CB 00000033 R F L . B T .
UU_DP_index_VOC_blk                            _AIL_      000014FE 00000126 R F L . B T .
_DP_register_sb                                _AIL_      00001624 0000005F R F L . B T .
UU_DP_get_sb_status                            _AIL_      00001683 00000015 R F L . B T .
_DP_play_VOC_file                              _AIL_      00001698 000000A6 R F L . B T .
_DP_format_VOC_file                            _AIL_      0000173E 000000EE R F L . B T .
UU_DP_format_sb                                _AIL_      0000182C 0000003D R F L . B T .
_DP_start_d_pb                                 _AIL_      00001869 00000048 R F L . B T .
_DP_stop_d_bp                                  _AIL_      000018B1 00000044 R F L . B T .
UU_DP_get_VOC_status                           _AIL_      000018F5 0000000F R F L . B T .



fxn_o53p01                                     ovr053
fxn_o53p02                                     ovr053
City_Screen                                    ovr054
City_Screen_Draw                               ovr054
IDK_CityScrn_Draw                              ovr054
City_Screen_Add_Fields                         ovr054
City_Add_Fields_Buildings                      ovr054
City_Screen_Add_Fields_Production_Window       ovr054
City_Screen_Draw_Buttons                       ovr054
IDK_BuilingCompletedMessage                    ovr054
NameStartingCity_Dialog_Popup                  ovr054
Draw_NameStartingCity_Background               ovr054
IDK_CityScreen_s49DF0                          ovr054
IDK_CityScreen_DrawUnitPictures                ovr054
City_Screen_Load                               ovr054
Allocate_Reduced_Map_Load_Unit_Figure          ovr054
CTY_GetBldngGFXSize                            ovr054
City_Can_Buy_Product                           ovr054
City_Screen_Draw_Map                           ovr054



Main_Screen                                    ovr057
MainScreen_Add_Fields                          ovr057
Add_Unit_Action_Fields                         ovr057
Main_Screen_Draw_Unit_Action_Buttons           ovr057
Add_Game_Button_Fields                         ovr057
Main_Screen_Draw_Game_Buttons                  ovr057
Add_Unit_Window_Fields                         ovr057
Main_Screen_Draw                               ovr057
Main_Screen_Reset                              ovr057
Move_Stack_DirKey                              ovr057



o62p01_Empty_pFxn                              ovr062
Select_Unit_Stack                              ovr062
Sort_Unit_Stack                                ovr062
Build_Unit_Stack                               ovr062
OVL_BringIntoView                              ovr062
OVL_TileOffScrnEdge                            ovr062
o62p07_Empty_pFxn                              ovr062
Stack_Action                                   ovr062
EarthGateTeleport                              ovr062
USW_FullDisplay                                ovr062



CityList_Screen                                ovr065
CityList_Draw_Reduced_Map                      ovr065
CityList_Screen_Load                           ovr065
CityList_Screen_Draw                           ovr065
Build_City_List                                ovr065
Update_Cities_List                             ovr065
CityList_Add_List_Fields                       ovr065
CityList_Set_List_Item_Count                   ovr065
ArmyList_Screen                                ovr066
ArmyList_Screen_Draw                           ovr066
ArmyList_Add_List_Fields                       ovr066
IDK_ArmyList_s59569                            ovr066
ArmyList_Draw_Reduced_Map                      ovr066
Get_List_Armies_Index                          ovr066
Build_Army_List                                ovr066
Build_Army_List_Counts                         ovr066
ArmyList_Set_List_Item_Count                   ovr066
ArmyList_Screen_Load                           ovr066
NX_IDK_ArmlyIst_PrintParagraph                 ovr066
ArmyList_Hero_Portrait_Coords                  ovr066
Draw_Maps                                      ovr067
IDK_CheckSet_MapDisplay_XY                     ovr067
sub_59DF7                                      ovr067
Set_Draw_Active_Stack_Always                   ovr067
Set_Draw_Active_Stack_Never                    ovr067
Reset_Draw_Active_Stack                        ovr067
Draw_Active_Unit_Stack                         ovr067
Reduced_Map_Set_Dims                           ovr067
Draw_World_Window                              ovr067
Set_Entities_On_Map_Window                     ovr067
IDK_Nodes_Entities_s5A53C                      ovr067
OVL_DrawMapSection                             ovr067
Set_Unit_Draw_Priority                         ovr067
Reset_Stack_Draw_Priority                      ovr067
IsPassableTower                                ovr067
Center_Map                                     ovr068
City_Center_Map                                ovr068
Reduced_Map_Coords                             ovr068
Redraw_Map_Unexplored_Area                     ovr068
RP_OVL_DrawCities2                             ovr068
TILE_Explore                                   ovr068
TILE_ExploreCorners                            ovr068
RP_TILE_ExploreMatch                           ovr068
List_Screen_Draw_Reduced_Map                   ovr068



Unit_List_Window_Pup                           ovr079
Draw_Unit_List_Window_Pup                      ovr079
IDK_ULW_Add_Fields                             ovr079



Square_Has_Lair                                ovr083
EZ_Clear                                       ovr083
sub_6D490                                      ovr083
EZ_Resolve                                     ovr083
OVL_EZ_Confirm_Entry                           ovr083
EZ_TreasureDialog                              ovr083
EZ_GoldManaReward                              ovr083
STR_ListSeparator                              ovr083
STR_GetIndefinite                              ovr083
EZ_GenerateRewards                             ovr083
OVL_EZ_Confirm_Dlg                             ovr083
OVL_DrawEZConfirm                              ovr083
EZ_SpecialTreasure                             ovr083
EZ_GetBookRealm                                ovr083



Clear_Square_Scouted_Flags                     ovr092
Check_Square_Scouted                           ovr092
Set_Square_Scouted                             ovr092
UU_Clear_Square_Scouted                        ovr092
Update_Scouted_And_Contacted                   ovr092
Set_Square_Scouted_Flags                       ovr092
UNIT_ContactHuman                              ovr092
Check_Square_Explored                          ovr092

WIZ_Conquer                                    ovr093
WIZ_Conquest                                   ovr093
GAME_DrawConqstAnim                            ovr093
GAME_DrawConquerors                            ovr093
GAME_Conqest_Scene2                            ovr093
GAME_DrawZappedWiz                             ovr093
GAME_DrawZapping                               ovr093
sub_79907                                      ovr093
GAME_IsWon                                     ovr093
GAME_PlayVictoryAnim                           ovr093
GAME_Draw_WIN_Anim                             ovr093
GAME_LimboFallAnim                             ovr093
GAME_DrawLimboFall                             ovr093
WIZ_Banishment                                 ovr093
GAME_ReturnDialog                              ovr093
GAME_DrawReturnAnim                            ovr093



Move_Units                                     ovr095
Move_Units_Draw                                ovr095
AI_ContactWizards                              ovr095
G_STK_OvlObstacles                             ovr095
STK_GetLandlubbers                             ovr095
G_STK_SetPatrol                                ovr095



STK_EvaluatePath                               ovr097
sub_7E597                                      ovr097
Square_Has_City                                ovr097
Units_Moves                                    ovr097
RP_CTY_CheckSpellWard                          ovr097
CTY_CheckSpellWard                             ovr097
OVL_SpellWardError                             ovr097
Print_Moves_String                             ovr097



GAME_NextHumanStack                            ovr101



Set_Main_Screen_Help_List                      ovr104
Set_ArmyList_Screen_Help                       ovr104
Set_Items_Screen_Help_List                     ovr104
Set_CityList_Screen_Help                       ovr104
Set_Magic_Screen_Help_List                     ovr104
Set_City_Screen_Help_List                      ovr104
Mirror_Screen_Load_Help                        ovr104
HLP_Load_Cartographr                           ovr104
sub_855F8                                      ovr104
sub_8562B                                      ovr104
Set_Alchemy_Screen_Help_List                   ovr104
sub_85691                                      ovr104
HLP_Load_USW                                   ovr104
IDK_LoadHelp_EnemyCityScap_s85711              ovr104
sub_85744                                      ovr104
GUI_SetFullHelp                                ovr104
Outpost_Screen_Help                            ovr104
sub_857BE                                      ovr104
sub_857F1                                      ovr104
sub_85824                                      ovr104
HLP_Load_CombatScrn                            ovr104
Set_TaxCollector_Window_Help_List              ovr104
Set_Advisor_Screen_Help_List                   ovr104
HLP_Load_HallofFame                            ovr104



Hall_Of_Fame_Screen                            ovr106
Hall_Of_Fame_Screen_Draw                       ovr106

GAME_OVER                                      ovr107
GAME_DrawScoreBrkdwn                           ovr107

o108p01_Empty_pFxn                             ovr108
o108p02_Empty_pFxn                             ovr108
o108p03_Empty_pFxn                             ovr108



CMB_Disenchant                                 ovr131
TILE_DispelMagic                               ovr131
CMB_ApplyCracksCall                            ovr131
CMB_BattlefieldSpell                           ovr131
WIZ_FlameStrike                                ovr131
WIZ_HolyWord                                   ovr131
WIZ_DeathSpell                                 ovr131
CMB_PlaySpellAnim                              ovr131
CMB_RaiseDead                                  ovr131
CMB_AnimateDead                                ovr131



BU_WarpCreature                                ovr133
WIZ_Wrack                                      ovr133
WIZ_CallLightning                              ovr133
BU_LifeDrain                                   ovr133
CMB_WallRise_Anim                              ovr133
CMB_CounterMessage                             ovr133
WIZ_DispelAttempt                              ovr133
TILE_CracksCall                                ovr133
TILE_BoltFromAbove                             ovr133
TILE_CombatSpellAnim                           ovr133
BU_Teleport                                    ovr133
BU_TunnelTo                                    ovr133
BU_CombatSummon                                ovr133
TILE_LightningBolt                             ovr133
WIZ_CreateVortex                               ovr133
CMB_SetVortexCursor                            ovr133
CMB_VortexPlayerMove                           ovr133
CMB_ProcessVortices                            ovr133
CMB_VortexMovement                             ovr133
CMB_CallChaos                                  ovr133
WIZ_CallChaos                                  ovr133



GAME_AutoSave                                  ovr141



o146p01_Empty_pFxn                             ovr146
o146p02_Empty_pFxn                             ovr146
o146p03_Empty_pFxn                             ovr146
o146p04_Empty_pFxn                             ovr146
o146p05_Empty_pFxn                             ovr146
o146p06_Empty_pFxn                             ovr146

Overland_Pathfinder                            ovr147

UU_OVL_Return_2                                ovr148
UU_OVL_GetLinePath                             ovr148
STK_GetPath                                    ovr148
TILE_ExtendRange                               ovr148
sub_D601B                                      ovr148
STK_SetOvlMoveMap                              ovr148
OVL_ClearUnitPath                              ovr148
OVL_StoreLongPath                              ovr148

Confirmation_Box                               ovr149
Confirmation_Box_Draw                          ovr149
GUI_WarningType1                               ovr149
GUI_WarningType0                               ovr149
GUI_ShowRedMessage                             ovr149
GUI_DrawRedMessage                             ovr149
Selection_Box                                  ovr149
Selection_Box_Draw                             ovr149
Selection_Box_Coords                           ovr149
GUI_LoadNotifyGfx                              ovr149
GUI_Notify_Dialog                              ovr149
GUI_DrawNotifyDialog                           ovr149
GUI_Familiar_Notify                            ovr149
GUI_DrawFamiliarNtfy                           ovr149
HLP_NoResizeExpand                             ovr149
HLP_DrawExpanding                              ovr149
G_USW_DrawEHLines                              ovr149
HLP_DrawExpandLines                            ovr149
ITEM_GetViewDrawPos                            ovr149
ITEM_ResetViewGrow                             ovr149
Save_ScreenSeg                                 ovr149
Restore_ScreenSeg                              ovr149

Reset_Map_Draw                                 ovr150
OVL_MapStateSave                               ovr150
OVL_MapStateRestore                            ovr150
Draw_Map_Window                                ovr150
Draw_Map_Terrain                               ovr150
Cycle_Map_Animations                           ovr150
Draw_Map_Unexplored_Area                       ovr150
Draw_Map_Cities                                ovr150
Draw_Map_Towers                                ovr150
Draw_Map_Lairs                                 ovr150
Draw_Map_Nodes                                 ovr150
Draw_Map_Biota                                 ovr150
Draw_Map_Minerals                              ovr150
Draw_Map_Roads                                 ovr150
Draw_Map_Units                                 ovr150
Draw_Reduced_Map                               ovr150
IDK_Prepare_TileMap_sDAB1A                     ovr150
sub_DAFF4                                      ovr150

UU_SPATH_Arbitrary                             ovr151
UU_SPATH_Segmented                             ovr151
UU_SPATH_15by15                                ovr151
UU_SPATH_Segments                              ovr151
UU_SPATH_GetBorderTiles                        ovr151

Allocate_Data_Space                            ovr152



AI_SetUnitOrders                               ovr158
G_AI_RallyFill                                 ovr158
AI_FillGarrisons                               ovr158
AI_ProcessOcean                                ovr158
G_AI_ProcessTransports                         ovr158
AI_ProcessRoamers                              ovr158
G_AI_HomeRallyFill                             ovr158
AI_PullForMainWar                              ovr158
G_AI_RallyOrFerry                              ovr158
AI_CreateTargetList                            ovr158
AI_AddTarget                                   ovr158
AI_AssignStackTarget                           ovr158
AI_CONTX_Regroup                               ovr158
AI_FindClosestEnemy                            ovr158
AI_OppMoveOverride                             ovr158
AI_Balance_Upkeep                              ovr158
AI_ShiftOffHomePlane                           ovr158
AI_Transport_Sailoff                           ovr158
AI_GarrBuilderPush                             ovr158
sub_EFBD6                                      ovr158
sub_EFC92                                      ovr158
AI_ProcessMelders                              ovr158
AI_ProcessSettlers                             ovr158
AI_ProcessPurifiers                            ovr158
AI_ProcessRoadBuilds                           ovr158
AI_UNIT_SetMoveOrder                           ovr158
AI_UNIT_SettleOrder                            ovr158
AI_UNIT_RoadBldOrder                           ovr158
AI_UNIT_SeekTransport                          ovr158
AI_UNIT_SetMeldOrder                           ovr158
AI_UNIT_PurifyOrder                            ovr158
TILE_CheckEnemyUnits                           ovr158
AI_TILE_CheckEnemies                           ovr158
AI_CanSettleOffPlane                           ovr158
AI_SendToColonize                              ovr158
AI_SeekTransportFrom                           ovr158
AI_CheckOtherPlane                             ovr158
AI_SingleCont_Reeval                           ovr158

Draw_Help_Entry                                ovr159
Draw_Help_Entry_Draw                           ovr159
EMM_Map_VGAFILEH1_5                            ovr159



sub_F48F0                                      ovr162
sub_F49E3                                      ovr162
sub_F4A9E                                      ovr162
sub_F4B96                                      ovr162
sub_F4C89                                      ovr162
sub_F4D32                                      ovr162
sub_F4E28                                      ovr162
sub_F5025                                      ovr162
sub_F514D                                      ovr162
sub_F528F                                      ovr162
sub_F5432                                      ovr162
AI_STK_SetTarget                               ovr162
sub_F57AF                                      ovr162
sub_F58D2                                      ovr162
sub_F59DA                                      ovr162
sub_F5B56                                      ovr162
sub_F5D73                                      ovr162
sub_F5EBF                                      ovr162
AI_STK_MoveTransports                          ovr162
sub_F60F4                                      ovr162
TILE_AI_FindLoadTile                           ovr162
sub_F6316                                      ovr162
TILE_AI_FindEmptyLnd                           ovr162
sub_F64C2                                      ovr162
sub_F671D                                      ovr162
sub_F689A                                      ovr162
sub_F68C9                                      ovr162
sub_F69A6                                      ovr162
sub_F6CAB                                      ovr162
sub_F6E1A                                      ovr162
sub_F6EBF                                      ovr162
TILE_NextFreeLand                              ovr162
EVNT_MakeRampageList                           ovr162
AI_TILE_CanLeave                               ovr162
AI_Continent_Reeval                            ovr162
AI_SetEnemyStrMaps                             ovr162
AI_Pick_Action_Conts                           ovr162
WIZ_UNIT_ResetOrders                           ovr162
