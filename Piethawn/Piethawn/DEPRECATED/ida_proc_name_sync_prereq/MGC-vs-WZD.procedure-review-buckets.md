# Procedure Comparison Review Buckets

Left database: `MGC`
Right database: `WZD`
Pairing rule: `mapped_segment_and_procedure_ordinal`

## Counts

- `same_normalized_hash_different_name`: 20 - Same normalized body hash, different name
- `same_mnemonic_hash_different_normalized_hash`: 211 - Same mnemonic hash, different normalized hash
- `same_name_different_normalized_hash`: 145 - Same name, different normalized body hash
- `different_name_different_normalized_hash`: 189 - Different name, different normalized body hash
- `same_name_same_normalized_hash`: 103 - same name same normalized hash

## Review Buckets

### Same normalized body hash, different name

Count: 20

- `seg012:10` `EMM_GetPageFrame` -> `EMM_Get_Page_Frame` (seg012:0246 -> seg012:024A)
- `seg013:5` `UU_EMM_LoadLBX2Hnd` -> `RP_EMM_LoadLBX2Hnd` (seg013:03EF -> seg013:03F3)
- `seg013:6` `UU_EMM_LoadLBX2Hnd2` -> `RP_EMM_LoadLBX2Hnd2` (seg013:0408 -> seg013:040C)
- `seg013:23` `UU_EMM_VGAWrite_P5` -> `RP_EMM_VGAWrite_P5` (seg013:15B7 -> seg013:15BB)
- `seg013:24` `UU_EMM_VGARead_P5` -> `RP_EMM_VGARead_P5` (seg013:15E2 -> seg013:15E6)
- `seg016:7` `RNG_Direct_LFSR` -> `rnd_bitfiddle__1oom` (seg016:04C9 -> seg016:04CD)
- `seg020:10` `UU_Set_Palette_Colors` -> `NX_VGA_LoadDAC` (seg020:02EC -> seg020:02F0)
- `seg022:13` `RP_MEM_Clear_Near` -> `UU_MEM_Clear_Near` (seg022:0229 -> seg022:022D)
- `seg022:14` `RP_MEM_Set_Near` -> `UU_MEM_Set_Near` (seg022:0250 -> seg022:0254)
- `seg022:26` `RP_MEM_TestBit_Near` -> `MEM_TestBit_Near` (seg022:055D -> seg022:0561)
- `seg034:64` `UU_GUI_GetControlCount` -> `Get_Field_Counter` (seg034:6B07 -> seg036:6B05)
- `seg034:68` `UU_GUI_GetDelay` -> `UU_Get_Input_Delay` (seg034:6C39 -> seg036:6C37)
- `seg034:84` `UU_CTRL_Clear_Skip1` -> `UU_GUI_ExtClear` (seg034:9551 -> seg036:954F)
- `_AIL_:38` `_AIL_start_digi_play` -> `_AIL_start_digital_playback` (_AIL_:0C88 -> _AIL_:0C86)
- `_AIL_:39` `_AIL_stop_digi_play` -> `_AIL_stop_digital_playback` (_AIL_:0C8E -> _AIL_:0C8C)
- `_AIL_:47` `_AIL_register_seq` -> `_AIL_register_sequence` (_AIL_:0CBE -> _AIL_:0CBC)
- `_AIL_:49` `_AIL_t_cache_size` -> `_AIL_default_timbre_cache_size` (_AIL_:0CCA -> _AIL_:0CC8)
- `_AIL_:50` `_AIL_define_t_cache` -> `_AIL_define_timbre_cache` (_AIL_:0CD0 -> _AIL_:0CCE)
- `_AIL_:51` `_AIL_timbre_req` -> `_AIL_timbre_request` (_AIL_:0CD6 -> _AIL_:0CD4)
- `_AIL_:62` `UU_AIL_set_rel_volume` -> `_AIL_set_rel_volume` (_AIL_:0D18 -> _AIL_:0D16)

### Same mnemonic hash, different normalized hash

Count: 211

- `seg002:1` `Mark_Time` -> `Mark_Time` (seg002:0002 -> seg002:0006)
- `seg002:2` `Time_Passed_` -> `Time_Passed_` (seg002:0018 -> seg002:001C)
- `seg002:4` `RP_CLK_ExtWait` -> `UU_CLK_ExtWait` (seg002:005E -> seg002:0062)
- `seg002:5` `Wait_For_Vsync_` -> `VGA_WaitSync` (seg002:00BF -> seg002:00C3)
- `seg003:1` `RP_Return_Ignore` -> `UU_Return_Ignore` (seg003:0000 -> seg003:0004)
- `seg003:2` `RP_DISK_GetSpace` -> `UU_Botched_DISK_GetSpace` (seg003:0003 -> seg003:0007)
- `seg003:3` `RP_DISK_GetDefaultDrive` -> `UU_DISK_GetDefaultDrive` (seg003:005D -> seg003:0061)
- `seg003:6` `UU_DISK_GetFileDateTime` -> `UU_DISK_GetFileDateTime` (seg003:00EA -> seg003:00EE)
- `seg003:7` `UU_DISK_CheckDrive` -> `UU_DISK_CheckDrive` (seg003:0138 -> seg003:013C)
- `seg003:8` `UU_DISK_GetCurrentDir` -> `UU_DISK_GetCurrentDir` (seg003:016F -> seg003:0173)
- `seg004:1` `DIR` -> `DIR` (seg004:000C -> seg004:0000)
- `seg004:2` `Disk_Free` -> `Disk_Free` (seg004:008C -> seg004:0080)
- `seg005:4` `GAME_EXE_Swap` -> `GAME_EXE_Swap` (seg005:01B6 -> seg005:01AA)
- `seg005:7` `RP_VGA_TextModeString` -> `UU_VGA_TextModeString` (seg005:0365 -> seg005:0359)
- `seg006:1` `Reset_Video` -> `Reset_Video` (seg006:000E -> seg006:0002)
- `seg006:3` `UU_DOS_PrintString` -> `Print_Message` (seg006:0041 -> seg006:0035)
- `seg006:4` `dos_memfree_kb` -> `Dos_Free` (seg006:006A -> seg006:005E)
- `seg006:5` `DBG_Open_ERROR_LOG` -> `DBG_Open_ERROR_LOG` (seg006:0088 -> seg006:007C)
- `seg006:6` `DBG_Close_ERROR_LOG` -> `DBG_Close_ERROR_LOG` (seg006:0091 -> seg006:0085)
- `seg007:5` `farpeekb` -> `farpeekb` (seg007:00A5 -> seg007:00A9)
- `seg007:7` `farpeekdw` -> `farpeekdw` (seg007:00CB -> seg007:00CF)
- `seg007:8` `farpokeb` -> `farpokeb` (seg007:00E4 -> seg007:00E8)
- `seg007:9` `farpokew` -> `farpokew` (seg007:00F9 -> seg007:00FD)
- `seg008:1` `Near_Allocate_Mark` -> `Near_Allocate_Mark` (seg008:0007 -> seg008:000B)
- `seg008:2` `Near_Allocate_Undo` -> `Near_Allocate_Undo` (seg008:0012 -> seg008:0016)
- ... 186 more

### Same name, different normalized body hash

Count: 145

- `seg002:3` `Release_Time` -> `Release_Time` (seg002:0028 -> seg002:002C)
- `seg003:5` `LOF` -> `LOF` (seg003:0082 -> seg003:0086)
- `seg005:1` `Exit_With_Size` -> `Exit_With_Size` (seg005:000E -> seg005:0002)
- `seg005:2` `Exit_With_Message` -> `Exit_With_Message` (seg005:0145 -> seg005:0139)
- `seg006:2` `Quit_With_Message` -> `Quit_With_Message` (seg006:0014 -> seg006:0008)
- `seg007:4` `Check_Allocation` -> `Check_Allocation` (seg007:007E -> seg007:0082)
- `seg007:6` `farpeekw` -> `farpeekw` (seg007:00B9 -> seg007:00BD)
- `seg008:4` `Near_Allocate_First` -> `Near_Allocate_First` (seg008:002E -> seg008:0032)
- `seg008:5` `Near_Allocate_Next` -> `Near_Allocate_Next` (seg008:0065 -> seg008:0069)
- `seg008:6` `Near_Allocation_Error` -> `Near_Allocation_Error` (seg008:00A1 -> seg008:00A5)
- `seg008:7` `Allocate_Space` -> `Allocate_Space` (seg008:00EB -> seg008:00EF)
- `seg008:8` `Allocate_Space_No_Header__LbxHeader` -> `Allocate_Space_No_Header__LbxHeader` (seg008:018A -> seg008:018E)
- `seg008:11` `Release_Block` -> `Release_Block` (seg008:024C -> seg008:0250)
- `seg008:12` `Allocate_First_Block` -> `Allocate_First_Block` (seg008:027A -> seg008:027E)
- `seg008:14` `Allocate_Next_Block` -> `Allocate_Next_Block` (seg008:0378 -> seg008:037C)
- `seg008:15` `Get_Free_Blocks` -> `Get_Free_Blocks` (seg008:0449 -> seg008:044D)
- `seg008:16` `SA_MK_FP0` -> `SA_MK_FP0` (seg008:047C -> seg008:0480)
- `seg008:19` `Allocation_Error` -> `Allocation_Error` (seg008:04EF -> seg008:04F3)
- `seg009:2` `lbx_close` -> `lbx_close` (seg009:0022 -> seg009:0026)
- `seg009:3` `lbx_seek` -> `lbx_seek` (seg009:0036 -> seg009:003A)
- `seg009:4` `lbx_lof` -> `lbx_lof` (seg009:005F -> seg009:0063)
- `seg009:6` `lbx_read_ofst` -> `lbx_read_ofst` (seg009:00B5 -> seg009:00B9)
- `seg010:1` `LBX_Load` -> `LBX_Load` (seg010:0006 -> seg010:000A)
- `seg010:2` `LBX_Reload` -> `LBX_Reload` (seg010:0022 -> seg010:0026)
- `seg010:4` `LBX_Load_Data` -> `LBX_Load_Data` (seg010:005C -> seg010:0060)
- ... 120 more

### Different name, different normalized body hash

Count: 189

- `seg005:3` `UU_Exit_With_Value` -> `Exit_With_Value` (seg005:0171 -> seg005:0165)
- `seg005:5` `Update_MemFreeWorst_KB` -> `Check_Free` (seg005:0251 -> seg005:0245)
- `seg005:6` `RP_VGA_B800Dump` -> `UU_VGA_B800Dump` (seg005:0269 -> seg005:025D)
- `seg007:1` `UU_DOS_Allocate_Space` -> `UU_SAMB_DOS_Allocate_Memory` (seg007:000A -> seg007:000E)
- `seg007:2` `UU_DOS_MemBlk_Alloc` -> `UU_DOS_MemAlloc` (seg007:0042 -> seg007:0046)
- `seg007:3` `UU_DOS_MemBlk_Free` -> `UU_DOS_MemFree` (seg007:0061 -> seg007:0065)
- `seg007:10` `UU_farpokedw` -> `farpokedw` (seg007:010E -> seg007:0112)
- `seg008:9` `SA_Allocate_MemBlk_EntrySize` -> `Allocate_Space_No_Header__EntrySize` (seg008:01D4 -> seg008:01D8)
- `seg008:13` `UU_SA_Alloc_Reset` -> `Reset_First_Block` (seg008:0343 -> seg008:0347)
- `seg008:17` `UU_SA_MK_FP_Norm` -> `UU_combine_32` (seg008:049F -> seg008:04A3)
- `seg008:18` `UU_SA_Deallocate_Space` -> `UU_LBX_Alloc_Free` (seg008:04CE -> seg008:04D2)
- `seg010:10` `LBX_Load_Library` -> `LBX_Load_Entry` (seg010:011D -> seg010:0121)
- `seg010:11` `LBX_Load_Record` -> `LBX_Load_Library_Data` (seg010:0533 -> seg010:0537)
- `seg010:13` `LBX_GetEntryData` -> `LBX_Get_Entry_Data` (seg010:0D2A -> seg010:0D2E)
- `seg012:6` `EMS_GetHandleName` -> `EMM_Get_Handle_Name` (seg012:00F8 -> seg012:00FC)
- `seg012:7` `EMM_ReleaseHandle` -> `EMM_Release_Handle` (seg012:012E -> seg012:0132)
- `seg012:11` `EMM_Map4` -> `EMM_MapFourPages` (seg012:0251 -> seg012:0255)
- `seg013:2` `UU_EMM_Load_LBX_0` -> `EMM_Load_LBX_File_0` (seg013:01B6 -> seg013:01BA)
- `seg013:11` `EMM_LBX_Load_Entry` -> `EMM_LBX_EntryLoader` (seg013:0877 -> seg013:087B)
- `seg013:12` `EMM_LBX_Load_FLIC_Header` -> `EMM_LBX_Load_Picture_Header` (seg013:0B15 -> seg013:0B19)
- `seg013:13` `EMM_LBX_Load_Record` -> `EMM_LBX_RecLoader` (seg013:0D16 -> seg013:0D1A)
- `seg013:16` `EMM_ReleaseAll__STUB` -> `EMM_Release_All` (seg013:11D6 -> seg013:11DA)
- `seg013:26` `EMM_EMMDATAH_AllocFirst` -> `EMMDATAH_Allocate_First` (seg013:1620 -> seg013:1624)
- `seg013:27` `EMM_EMMDATAH_AllocNext` -> `EMMDATAH_Allocate_Next` (seg013:16CA -> seg013:16CE)
- `seg013:30` `RP_EMM_LBX_Load2Hnd` -> `UU_EMM_LBX_Load2Hnd` (seg013:1788 -> seg013:178C)
- ... 164 more

### same name same normalized hash

Count: 103

- `seg003:4` `UU_DISK_DeleteFile` -> `UU_DISK_DeleteFile` (seg003:006C -> seg003:0070)
- `seg013:9` `UU_EMM_CustLoadLBX2` -> `UU_EMM_CustLoadLBX2` (seg013:062A -> seg013:062E)
- `seg016:5` `UU_VGA_CreateColorWave` -> `UU_VGA_CreateColorWave` (seg016:0385 -> seg016:0389)
- `seg017:6` `Set_Outline_Color` -> `Set_Outline_Color` (seg017:00EB -> seg017:00EF)
- `seg022:4` `Clear_Screens` -> `Clear_Screens` (seg022:0033 -> seg022:0037)
- `seg022:5` `Set_Random_Seed` -> `Set_Random_Seed` (seg022:0074 -> seg022:0078)
- `seg022:28` `UU_MEM_ClearBit_Near` -> `UU_MEM_ClearBit_Near` (seg022:05BE -> seg022:05C2)
- `seg022:29` `UU_DBG_SetSelectSetting` -> `UU_DBG_SetSelectSetting` (seg022:05E8 -> seg022:05EC)
- `seg028:8` `UU_LBX_IMG_CReplaceRect` -> `UU_LBX_IMG_CReplaceRect` (seg028:06AB -> seg030:06B9)
- `seg028:31` `UU_LBX_IMG_FullGScale` -> `UU_LBX_IMG_FullGScale` (seg028:284E -> seg030:285C)
- `seg028:32` `UU_LBX_IMG_ExtGScaleEC` -> `UU_LBX_IMG_ExtGScaleEC` (seg028:286E -> seg030:287C)
- `seg031:4` `EMM_MapNextIMGPages2` -> `EMM_MapNextIMGPages2` (seg031:022A -> seg033:0228)
- `seg033:3` `Get_Pointer_Image_Number` -> `Get_Pointer_Image_Number` (seg033:010C -> seg035:010A)
- `seg033:4` `Get_Pointer_Offset` -> `Get_Pointer_Offset` (seg033:0117 -> seg035:0115)
- `seg033:13` `Save_Mouse_State` -> `Save_Mouse_State` (seg033:039E -> seg035:039C)
- `seg033:14` `Restore_Mouse_State` -> `Restore_Mouse_State` (seg033:03B6 -> seg035:03B4)
- `seg033:16` `Keyboard_Status` -> `Keyboard_Status` (seg033:03DD -> seg035:03DB)
- `seg033:25` `Mouse_Buffer` -> `Mouse_Buffer` (seg033:06D4 -> seg035:06D2)
- `seg033:26` `Mouse_Buffer2` -> `Mouse_Buffer2` (seg033:06E5 -> seg035:06E3)
- `seg033:33` `Copy_Mouse_Off_To_Mouse_Back` -> `Copy_Mouse_Off_To_Mouse_Back` (seg033:09E0 -> seg035:09DE)
- `seg033:41` `UU_GUI_ClearClickBuffer` -> `UU_GUI_ClearClickBuffer` (seg033:0D96 -> seg035:0D94)
- `seg033:42` `UU_GUI_ClickBuffReset` -> `UU_GUI_ClickBuffReset` (seg033:0DB4 -> seg035:0DB2)
- `seg033:43` `UU_GUI_SetClickBuffVar1` -> `UU_GUI_SetClickBuffVar1` (seg033:0DBB -> seg035:0DB9)
- `seg033:44` `UU_GUI_ClearBufferClick` -> `UU_GUI_ClearBufferClick` (seg033:0DC2 -> seg035:0DC0)
- `seg033:46` `UU_GUI_GetClickBuffVar1` -> `UU_GUI_GetClickBuffVar1` (seg033:0E13 -> seg035:0E11)
- ... 78 more
