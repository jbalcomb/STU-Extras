

MAGIC.EXE
ovr050



MoO2
    Module: NEWGAME
    Module: RACESEL

MoO2  Module: NEWGAME
    `_combat_button_strings
    _gal_size_button_strings
    _color_ken
    _gal_size_anims
    _gal_age_button_strings
    _civ_button_strings
    _opp_button_strings
    _hack_msg
    _anims
    _difficulty_anims
    _diff_button_strings
    _tech_level_anims
    _button_seg
    _cancel_button_seg
    _num_players_anims
    _gal_age_anims
    _fields
    _newgame_accept_button_seg
    _background_seg
    _gal_age_button_variable
    _xenon_attacks_button_variable
    _combat_button_variable
    _gal_size_button_variable
    _civ_button_variable
    _diff_button_variable
    _random_events_button_variable
    _g_x
    _g_y
    Load_New_Game_Strings_()
    Reload_Newgame_Screen_()
    Draw_Pictures_()
    Draw_Newgame_Screen_()
    Update_Fields_()
    Add_Fields_()
    Temps_To_Globals_()
    Globals_To_Temps_()
    Newgame_Screen_()
    _strings_been_loaded
    _opp_button_variable



New Game

'NEW GAME'

does ESC cancel/go back/previous page
from menu, hotkey 'N'

### New Game Screen #1
Game Options
    Difficulty
    Opponents
    Land Size
    Magic
"Cancel" button
"OK" button

does fade in
on cancel, does fade out
does button down offset, but no highlight/frame change


### New Game Screen #2













NewGame_Screen_1





start                        seg000    00000000 00000163 R F L . . . .
__cleanup                    seg000    00000163 00000013 R F L . . T .
__checknull                  seg000    00000176 00000028 R F L . . T .
__terminate                  seg000    0000019E 00000009 . F L . B T .
ZeroDivision                 seg000    000001A7 00000009 . . L . . T .
SaveVectors                  seg000    000001B0 00000043 R . L . . T .
__restorezero                seg000    000001F3 0000002D R F L . . T .
Initialize                   seg000    00000220 00000044 R . L . . T .
Cleanup                      seg000    00000264 00000041 R . L . . T .
ErrorDisplay                 seg000    000002A5 00000008 R . L . . T .
_abort                       seg000    000002AD 00000017 . . L . . T .
abs                          seg000    000002C8 0000000E R F L . B T .
UU_atexit                    seg000    000002D6 0000002D R F L . B T .
__DOScmd                     seg000    00000303 000000A9 R . L . B T .
__DOSenv                     seg000    000003AC 000000FD R . L . B T .
RP__dos_getfileattr          seg000    000004A9 0000001D R F L . B T .
RP__dos_setfileattr          seg000    000004C6 0000001B R F L . B T .
_dos_findfirst               seg000    000004E1 00000033 R F L . B T .
RP__dos_findnext             seg000    00000514 0000002D R F L . B T .
_dos_getdrive                seg000    00000541 00000013 R F L . B T .
RP__dos_setdrive             seg000    00000554 00000017 R F L . B T .
_dummy                       seg000    0000056B 00000001 R F L . . T .
___exit                      seg000    0000056C 00000057 R . L . B T .
_exit                        seg000    000005C3 0000000F . F L . B T .
__exit                       seg000    000005D2 00000012 . F L . B T .
__cexit                      seg000    000005E4 0000000E R F L . . T .
RP__c_exit                   seg000    000005F2 0000000C R F L . . T .
LXMUL@                       seg000    000005FE 00000017 R F L . . T .
SCOPY@                       seg000    00000615 0000001C R F L . B T .
N_LDIV@                      seg000    00000631 00000003 R . L . . T .
LDIV@                        seg000    00000634 00000004 R F L . . T .
N_LUDIV@                     seg000    00000638 00000003 R . L . . T .
N_LMOD@                      seg000    00000640 00000003 R . L . . T .
LMOD@                        seg000    00000643 00000005 R F L . . T .
N_LUMOD@                     seg000    00000648 00000003 R . L . . T .
_DIV_common                  seg000    0000064E 00000091 R F L . B T .
N_LXLSH@                     seg000    000006DF 00000003 R . L . . T .
LXLSH@                       seg000    000006E2 0000001E R F L . . T .
N_LXURSH@                    seg000    00000700 00000003 R . L . . T .
LXURSH@                      seg000    00000703 0000001E R F L . . T .
N_PADD@                      seg000    00000721 00000003 R . L . . T .
N_PSUB@                      seg000    00000750 00000003 R . L . . T .
__IOerror                    seg000    00000781 00000039 R . L . B T .
__DOSerror                   seg000    000007BA 00000012 R . L . B T .
isalnum                      seg000    000007CC 00000012 R F L . B T .
RP_isascii                   seg000    000007DE 00000013 R F L . B T .
RP_isalpha                   seg000    000007F1 00000012 R F L . B T .
RP_iscntrl                   seg000    00000803 00000012 R F L . B T .
RP_isdigit                   seg000    00000815 00000012 R F L . B T .
RP_isgraph                   seg000    00000827 00000019 R F L . B T .
RP_islower                   seg000    00000840 00000012 R F L . B T .
RP_isprint                   seg000    00000852 00000019 R F L . B T .
RP_ispunct                   seg000    0000086B 00000012 R F L . B T .
RP_isspace                   seg000    0000087D 00000012 R F L . B T .
RP_isupper                   seg000    0000088F 00000012 R F L . B T .
RP_isxdigit                  seg000    000008A1 00000012 R F L . B T .
RP_getc                      seg000    000008B3 00000021 R F L . B T .
RP_getchar                   seg000    000008D4 0000001F R F L . . T .
RP_putc                      seg000    000008F3 00000028 R F L . B T .
RP_putchar                   seg000    0000091B 0000002A R F L . B T .
UU_feof                      seg000    00000945 00000010 R F L . B T .
UU_ferror                    seg000    00000955 00000010 R F L . B T .
isatty                       seg000    00000965 00000011 R F L . B T .
__longtoa                    seg000    00000976 0000007D R . L . B T .
__utoa                       seg000    000009F3 0000001D R . L . B T .
lseek                        seg000    00000A10 00000029 R F L . B T .
CRP___mkname                 seg000    00000A39 0000003F R . L . B T .
RP___tmpnam                  seg000    00000A78 00000043 R . L . B T .
N_LXMUL@                     seg000    00000ABB 00000017 R . L . . T .
N_PCMP@                      seg000    00000AD2 00000021 R . L . . T .
_read                        seg000    00000AF3 0000002D R F L . B T .
CopyUpr                      seg000    00000B20 00000026 R . L . B T .
_getcurdir                   seg000    00000B46 00000019 R F L . B T .
CheckFile                    seg000    00000B5F 000000C1 R . L . B T .
__search_env                 seg000    00000C20 0000015E R . L . B T .
__searchpath                 seg000    00000D7E 00000014 R . L . B T .
RP__searchenv                seg000    00000D92 00000033 R F L . B T .
RP__searchstr                seg000    00000DC5 00000033 R F L . B T .
_setupio                     seg000    00000DF8 000000A5 R . L . . T .
toupper                      seg000    00000E9D 0000002C R F L . B T .
CRP_unlink                   seg000    00000EC9 00000016 R F L . B T .
_stpcpy                      seg000    00000EDF 00000029 R F L . B T .
__brk                        seg000    00000F08 00000022 R . L . B T .
__sbrk                       seg000    00000F2A 00000031 R . L . B T .
RP_brk                       seg000    00000F5B 0000000C R F L . B T .
RP_sbrk                      seg000    00000F67 00000010 R F L . B T .
coreleft                     seg000    00000F77 00000010 R F L . . T .
_Loader                      seg000    00001086 00000061 R . L . . T .
__exec                       seg000    000010EE 0000022F R . L . B T .
execl                        seg000    0000131D 0000001A R F L . B T .
fFreeLastBlock               seg000    00001343 00000063 R . L . . T .
fFreeInnerBlock              seg000    000013A6 0000005B R . L . . T .
fJoinFreeBlocks              seg000    00001401 00000016 R . L . . T .
fPullFreeBlock               seg000    00001417 00000029 R . L . . T .
fInsertFreeBlock             seg000    00001440 00000037 R . L . . T .
_farfree                     seg000    00001477 00000029 R F L . B T .
fCreateHeap                  seg000    000014A0 00000064 R . L . . T .
fExtendHeap                  seg000    00001504 0000005A R . L . . T .
fAllocPartBlock              seg000    0000155E 00000023 R . L . . T .
_farmalloc                   seg000    00001581 0000007D R F L . B T .
RP_fExpandBlock              seg000    000015FE 0000007C R . L . . T .
RP_fShrinkBlock              seg000    0000167A 00000064 R . L . . T .
RP_farrealloc                seg000    000016DE 0000007A R F L . B T .
normalize                    seg000    00001758 00000076 R . L . B T .
_brk                         seg000    000017CE 0000003F R . L . B T .
_sbrk                        seg000    0000180D 0000008B R . L . B T .
ioctl                        seg000    00001898 00000027 R F L . B T .
_LoadProg                    seg000    000018BF 00000097 R . L . B T .
_free                        seg000    00001956 0000001D R F L . . T .
nFreeLastBlock               seg000    00001973 0000003A R . L . . T .
nFreeInnerBlock              seg000    000019AD 0000002E R . L . . T .
nJoinFreeBlocks              seg000    000019DB 0000000B R . L . . T .
nPullFreeBlock               seg000    000019E6 0000001C R . L . . T .
nInsertFreeBlock             seg000    00001A02 00000023 R . L . . T .
_malloc                      seg000    00001A25 00000060 R F L . . T .
nCreateHeap                  seg000    00001A85 00000040 R . L . . T .
nExtendHeap                  seg000    00001AC5 00000029 R . L . . T .
nAllocPartBlock              seg000    00001AEE 00000019 R . L . . T .
RP_nExpandBlock              seg000    00001B07 00000035 R . L . B T .
RP_nShrinkBlock              seg000    00001B3C 00000042 R . L . . T .
RP_realloc                   seg000    00001B7E 00000053 R F L . B T .
__setargv                    seg000    00001BD3 00000093 R . L . . T .
NextChar                     seg000    00001C66 00000028 R . L . . T .
setblock                     seg000    00001CEA 0000001C R F L . B T .
__setenvp                    seg000    00001D06 0000005D R . L . . T .
_chmod                       seg000    00001D63 0000001B R F L . B T .
close                        seg000    00001D7E 00000028 R F L . B T .
_close                       seg000    00001DA6 0000001E R F L . B T .
fcloseall                    seg000    00001DC4 00000052 R F L . B T .
eof                          seg000    00001E16 00000077 R F L . B T .
fclose                       seg000    00001E8D 00000081 R F L . B T .
fflush                       seg000    00001F0E 00000081 R F L . B T .
flushall                     seg000    00001F8F 0000003A R F L . B T .
CheckOpenType                seg000    00001FC9 000000BA R . L . B T .
__openfp                     seg000    00002083 0000009C R . L . B T .
__getfp                      seg000    0000211F 0000002B R . L . . T .
fopen                        seg000    0000214A 0000001F R F L . B T .
_fgetn                       seg000    00002169 000000D7 R . L . B T .
fread                        seg000    00002240 0000004B R F L . B T .
Displacement                 seg000    0000228B 0000005D R . L . B T .
fseek                        seg000    000022E8 00000068 R F L . B T .
RP_ftell                     seg000    00002350 000000C7 R F L . B T .
fwrite                       seg000    00002417 0000004C R F L . B T .
FlushOutStreams              seg000    00002463 00000029 R . L . . T .
_ffill                       seg000    0000248C 00000059 R . L . B T .
_fgetc                       seg000    000024E5 00000012 R F L . B T .
UU___Nfgetc@                 seg000    000024F7 00000003 R . L . . T .
fgetc@                       seg000    000024FA 000000AF R F L . B T .
UU_fgetchar                  seg000    000025A9 0000000A R F L . . T .
getenv                       seg000    000025B3 0000005F R F L . B T .
itoa                         seg000    00002612 00000028 R F L . B T .
CRP_ultoa                    seg000    0000263A 0000001A R F L . B T .
ltoa                         seg000    00002654 00000025 R F L . B T .
memcpy                       seg000    00002679 0000001F R F L . B T .
dosCreat                     seg000    00002698 00000019 R . L . B T .
dosWriteNone                 seg000    000026B1 00000012 R . L . B T .
open                         seg000    000026C3 00000158 R F L . B T .
_open                        seg000    0000281B 0000004D R F L . B T .
_fputc                       seg000    00002868 00000019 R F L . B T .
fputc                        seg000    00002881 00000136 R F L . B T .
RP_fputchar                  seg000    000029B7 00000012 R F L . B T .
__fputn                      seg000    000029C9 000001A8 R . L . B T .
__read                       seg000    00002B71 000000CE R F L . B T .
_setvbuf                     seg000    00002C3F 000000D8 R F L . B T .
CopyIt                       seg000    00002D17 0000003F R . L . B T .
DotFound                     seg000    00002D56 00000047 R . L . B T .
_fnsplit                     seg000    00002DAD 0000015C R F L . B T .
UU__splitpath                seg000    00002F25 0000001B R F L . B T .
strcat                       seg000    00002F40 00000039 R F L . B T .
strcpy                       seg000    00002F79 00000022 R F L . B T .
stricmp                      seg000    00002F9B 00000040 R F L . B T .
strlen                       seg000    00002FDB 0000001A R F L . B T .
strncpy                      seg000    00002FF5 0000002C R F L . B T .
strupr                       seg000    00003021 00000020 R F L . B T .
__write                      seg000    00003041 0000010E R F L . B T .
_write                       seg000    0000314F 0000003A R F L . B T .
_xfclose                     seg000    00003189 00000028 R F L . . T .
_xfflush                     seg000    000031B1 00000023 R F L . . T .
__OverlayHalt                seg000    000031EB 0000001D . . L . . T .
__FAR_RETURN                 seg000    00003208 00000001 R F L . . T .
_main                        seg001    00000009 000003D0 R F . . B T .
SND_Empty_Fn                 seg001    0000043A 00000005 R F . . B T .
Menu_Screen_Control          seg001    0000043F 000000E1 R F . . B T .
Load_MGC_Resources           seg001    0000052A 000000C0 R F . . B T .
Main_Menu_Screen             seg001    000005EA 000004D0 R F . . B T .
Main_Menu_Screen_Draw        seg001    00000ABA 000001DA R F . . B T .
Load_TERRSTAT                seg001    00000C94 0000002D R F . . B T .
Load_SPELLDAT                seg001    00000CC1 0000002D R F . . B T .
UU_HLP_Load_MainMenu         seg001    00000CEE 00000033 R F . . B . .
Mark_Time                    seg002    00000002 00000016 R F . . . T .
UU_CLK_Timer                 seg002    00000018 00000010 R F . . . T .
Release_Time                 seg002    00000028 00000036 R F . . B T .
RP_CLK_ExtWait               seg002    0000005E 00000061 R F . . B T .
Wait_For_Vsync_              seg002    000000BF 00000010 R F . . . T .
RP_Return_Ignore             seg003    00000000 00000003 R F . . . T .
RP_DISK_GetSpace             seg003    00000003 0000005A R F . . B T .
RP_DISK_GetDefaultDrive      seg003    0000005D 0000000F R F . . . T .
UU_DISK_DeleteFile           seg003    0000006C 00000016 R F . . B T .
LOF                          seg003    00000082 00000068 R F . . B T .
UU_DISK_GetFileDateTime      seg003    000000EA 0000004E R F . . B T .
UU_DISK_CheckDrive           seg003    00000138 00000037 R F . . B T .
UU_DISK_GetCurrentDir        seg003    0000016F 0000002D R F . . B T .
DIR                          seg004    0000000C 00000080 R F . . B T .
UU_DISK_GetFreeSpace         seg004    0000008C 00000022 R F . . . T .
Exit_With_Size               seg005    0000000E 00000137 R F . . B T .
Exit_With_Message            seg005    00000145 0000002C R F . . B T .
UU_Exit_With_Value           seg005    00000171 00000045 R F . . B T .
GAME_EXE_Swap                seg005    000001B6 0000009B R F . . B T .
Update_MemFreeWorst_KB       seg005    00000251 00000018 R F . . B T .
RP_VGA_B800Dump              seg005    00000269 000000FC . F . . B T .
RP_VGA_TextModeString        seg005    00000365 00000058 R F . . B T .
VGA_SetTextMode              seg006    0000000E 00000006 R F . . . T .
QuitToDOS                    seg006    00000014 0000002D R F . . B T .
UU_DOS_PrintString           seg006    00000041 00000029 R F . . B T .
dos_memfree_kb               seg006    0000006A 0000001E R F . . . T .
CRP_Empty_Exit_Fn2           seg006    00000088 00000009 R F . . . T .
CRP_Empty_Exit_Fn3           seg006    00000091 00000009 R F . . . T .
UU_DOS_Allocate_Space        seg007    0000000A 00000038 R F . . B T .
UU_DOS_MemBlk_Alloc          seg007    00000042 0000001F R F . . B T .
UU_DOS_MemBlk_Free           seg007    00000061 0000001D R F . . B T .
Check_Allocation             seg007    0000007E 00000027 R F . . B T .
farpeekb                     seg007    000000A5 00000014 R F . . B T .
farpeekw                     seg007    000000B9 00000012 R F . . B T .
farpeekdw                    seg007    000000CB 00000019 R F . . B T .
farpokeb                     seg007    000000E4 00000015 R F . . B T .
farpokew                     seg007    000000F9 00000015 R F . . B T .
UU_farpokedw                 seg007    0000010E 00000019 R F . . B T .
Near_Allocate_Mark           seg008    00000007 0000000B R F . . B T .
Near_Allocate_Undo           seg008    00000012 0000000B R F . . B T .
UU_Near_Allocate_Reset       seg008    0000001D 00000011 R F . . B T .
Near_Allocate_First          seg008    0000002E 00000037 R F . . B T .
Near_Allocate_Next           seg008    00000065 0000003C R F . . B T .
Near_Allocation_Error        seg008    000000A1 0000004A R F . . B T .
Allocate_Space               seg008    000000EB 0000009F R F . . B T .
SA_Allocate_MemBlk_LbxHeader seg008    0000018A 0000004A R F . . B T .
SA_Allocate_MemBlk_EntrySize seg008    000001D4 0000004A R F . . B T .
SA_Alloc_Mark                seg008    0000021E 0000002E R F . . B T .
SA_Alloc_Undo                seg008    0000024C 0000002E R F . . B T .
Allocate_First_Block         seg008    0000027A 000000C9 R F . . B T .
UU_SA_Alloc_Reset            seg008    00000343 00000035 R F . . B T .
Allocate_Next_Block          seg008    00000378 000000D1 R F . . B T .
SA_Get_Free_Blocks           seg008    00000449 00000033 R F . . B T .
SA_MK_FP0                    seg008    0000047C 00000023 R F . . B T .
UU_SA_MK_FP_Norm             seg008    0000049F 0000002F R F . . B T .
UU_SA_Deallocate_Space       seg008    000004CE 00000021 R F . . B T .
Allocation_Error             seg008    000004EF 0000019A R F . . B T .
lbx_open                     seg009    00000002 00000020 R F . . B T .
lbx_close                    seg009    00000022 00000014 R F . . B T .
lbx_seek                     seg009    00000036 00000029 R F . . B T .
UU_lbx_size                  seg009    0000005F 00000029 R F . . B T .
lbx_read_sgmt                seg009    00000088 0000002D R F . . B T .
lbx_read_ofst                seg009    000000B5 00000028 R F . . B . .
farstrcpy                    seg009    000000DD 00000039 R F . . B T .
LBX_Load                     seg010    00000006 0000001C R F . . B T .
LBX_Reload                   seg010    00000022 0000001D R F . . B T .
LBX_Reload_Next              seg010    0000003F 0000001D R F . . B T .
LBX_Load_Data                seg010    0000005C 00000022 R F . . B T .
RP_LBXR_LoadReplace          seg010    0000007E 00000023 R F . . B T .
RP_LBXR_LoadAppend           seg010    000000A1 00000023 R F . . B T .
UU_LBXE_LoadSingle2          seg010    000000C4 0000001D R F . . B T .
UU_LBXE_LoadReplace2         seg010    000000E1 0000001E R F . . B T .
UU_LBXE_LoadAppend2          seg010    000000FF 0000001E R F . . B T .
LBX_Load_Library             seg010    0000011D 00000416 R F . . B T .
LBX_Load_Record              seg010    00000533 000004C8 R F . . B T .
LBX_Load_Data_Static         seg010    000009FB 0000032F R F . . B T .
LBX_GetEntryData             seg010    00000D2A 00000219 R F . . B . .
UU_LBX_SetPath               seg010    00000F43 00000013 R F . . B T .
Error_Handler                seg010    00000F56 0000019F . F . . B T .
File_Name_Base               seg010    00001115 0000003D R F . . B T .
RAM_SetMinKB                 seg010    00001152 0000000B R F . . B T .
DISK_OpenCreate              seg011    0000000D 00000077 R F . . B T .
DISK_ReadFile                seg011    00000084 0000002F R F . . B T .
DISK_WriteFile               seg011    000000B3 0000002F R F . . B T .
DISK_FileSeek                seg011    000000E2 00000016 R F . . B T .
DISK_FileClose               seg011    000000F8 0000000D R F . . B T .
EMM_Init                     seg012    00000008 00000058 R F . . . T .
EMM_GetActiveHandleCount     seg012    00000060 0000001C R F . . . T .
EMM_GetFreePageCount         seg012    0000007C 0000001C R F . . . T .
EMS_GetHandlePageCount       seg012    00000098 00000024 R F . . B T .
EMS_MakeNamedHandle          seg012    000000BC 0000003C R F . . B T .
EMS_GetHandleName            seg012    000000F8 00000036 R F . . B T .
EMM_ReleaseHandle            seg012    0000012E 0000001A R F . . B T .
EMM_MapnRead                 seg012    00000148 0000007F R F . . B T .
EMM_MapnWrite                seg012    000001C7 0000007F R F . . B T .
EMM_GetPageFrame             seg012    00000246 0000000B R F . . . T .
EMM_Map4                     seg012    00000251 0000003B R F . . B T .
EMM_MapMulti4                seg012    0000028C 0000003B R F . . B T .
EMS_Startup                  seg013    00000007 000001AF R F . . B T .
UU_EMM_Load_LBX_0            seg013    000001B6 00000012 R F . . B T .
EMM_Load_LBX_File_1          seg013    000001C8 00000013 R F . . B T .
EMM_Load_LBX_File            seg013    000001DB 00000214 R F . . B T .
UU_EMM_LoadLBX2Hnd           seg013    000003EF 00000019 R F . . B T .
UU_EMM_LoadLBX2Hnd2          seg013    00000408 0000001A R F . . B T .
UU_EMM_LoadLBX_Fail          seg013    00000422 000001E3 R F . . B T .
UU_EMM_CustLoadLBX           seg013    00000605 00000025 R F . . B T .
UU_EMM_CustLoadLBX2          seg013    0000062A 00000026 R F . . B T .
UU_EMM_CustomLBXLoad         seg013    00000650 00000227 R F . . B T .
EMM_LBX_Load_Entry           seg013    00000877 0000029E R F . . B T .
EMM_LBX_Load_FLIC_Header     seg013    00000B15 00000201 R F . . B T .
EMM_LBX_Load_Record          seg013    00000D16 000002C2 R F . . B . .
EMM_LBXR_DirectLoad          seg013    00000FD8 000001A9 R F . . B . .
EMM_CheckHandleOpen          seg013    00001181 00000055 R F . . B T .
EMS_Shutdown                 seg013    000011D6 00000036 R F . . B T .
EMM_Release_Resd             seg013    0000120C 00000035 R F . . B T .
UU_EMM_ReleaseHnd            seg013    00001241 000000EB R F . . B T .
UU_Get_EMM_OK                seg013    0000132C 0000000A R F . . B T .
EMM_GetHandle                seg013    00001336 0000023D R F . . B T .
UU_EMM_VGAWrite_fP5          seg013    00001573 00000022 R F . . B T .
UU_EMM_VGARead_fP5           seg013    00001595 00000022 R F . . B T .
UU_EMM_VGAWrite_P5           seg013    000015B7 0000002B R F . . B T .
UU_EMM_VGARead_P5            seg013    000015E2 0000002B R F . . B T .
EMM_Map4_EMMDATAH            seg013    0000160D 00000013 R F . . B T .
EMM_EMMDATAH_AllocFirst      seg013    00001620 000000AA R F . . B T .
EMM_EMMDATAH_AllocNext       seg013    000016CA 000000A8 R F . . B T .
UU_EMM_Data_Mark             seg013    00001772 0000000B R F . . B T .
UU_EMM_Data_Undo             seg013    0000177D 0000000B R F . . B T .
RP_EMM_LBX_Load2Hnd          seg013    00001788 00000163 R F . . B T .
EMS_SetMinKB                 seg013    000018EB 0000000B R F . . B T .
EMS_BuildEmsErrStr           seg013    000018F6 00000047 R F . . B T .
Init_Drivers                 seg014    0000000D 0000009D R F . . B T .
UU_Legacy_Startup            seg014    000000AA 0000005A R F . . B T .
Load_Font_File               seg014    00000104 000000C8 R F . . B T .
Set_Window                   seg014    000001CC 00000057 R F . . B T .
Reset_Window                 seg014    00000223 0000001D R F . . B T .
VGA_WndDrawFilldRect         seg014    00000240 0000007C R F . . B T .
UU_VGA_WndPutPixel           seg014    000002BC 0000003E R F . . B T .
VGA_WndDrawLine              seg014    000002FA 00000028 R F . . B T .
UU_VGA_WndDrawPtrnLine       seg014    00000322 00000029 R F . . B T .
VGA_WndDrawLineBase          seg014    0000034B 000002C6 R F . . B T .
UU_VGA_DrawBiColorRect       seg014    00000611 00000061 R F . . B T .
UU_VGA_DrawRect              seg014    00000672 00000078 R F . . B T .
UU_VGA_WndDrawRect           seg014    000006EA 00000074 R F . . B T .
VGA_DrawDblRect              seg014    0000075E 000000BB R F . . B T .
VGA_RectangleFX              seg014    00000819 000000B7 R F . . B T .
DOS_PrintString              seg014    000008F3 00000011 R F . . B T .
Set_Video_Mode               seg015    00000004 00000044 R F . . . T .
Fill                         seg016    0000001C 00000104 R F . . B T .
Dot                          seg016    00000120 0000003A R F . . B T .
Line                         seg016    0000015A 00000109 R F . . B T .
VGA_DrawPatternLine          seg016    00000263 00000122 R F . . B T .
UU_VGA_CreateColorWave       seg016    00000385 00000037 R F . . . T .
RP_VGA_DiagColumns           seg016    000003BC 0000010D R F . . B T .
RNG_Direct_LFSR              seg016    000004C9 00000038 R F . . . T .
VGA_Shade_Rect               seg016    00000502 000001FD R F . . B T .
VGA_Grayscale_Rect           seg016    000006FF 0000022D R F . . B T .
RP_VGA_Columns               seg016    0000092C 000000EA R F . . B T .
RP_VGA_ScrambleRect          seg016    00000A16 00000174 R F . . B T .
Set_Font_Style1              seg017    0000000A 0000002D R F . . B T .
VGA_SetFont_Style2           seg017    00000037 0000002D R F . . B T .
UU_VGA_SetFont_Style3        seg017    00000064 0000002D R F . . B T .
UU_VGA_SetFont_Style4        seg017    00000091 0000002D R F . . B T .
UU_VGA_SetFont_Style5        seg017    000000BE 0000002D R F . . B T .
Set_Outline_Color            seg017    000000EB 0000000B R F . . B T .
VGA_SetVertSpacing           seg017    000000F6 00000028 R F . . B T .
UU_VGA_SetSpaceWidth         seg017    0000011E 00000018 R F . . B T .
UU_VGA_SetHorzSpacing        seg017    00000136 00000018 R F . . B T .
UU_VGA_BotchedFColor         seg017    0000014E 0000002B R F . . B T .
Set_Font_Colors_15           seg017    00000179 00000044 R F . . B T .
VGA_SetAAColor               seg017    000001BD 00000077 R F . . B T .
VGA_SaveAAColors             seg017    00000234 0000006E R F . . B T .
VGA_RestoreAAColors          seg017    000002A2 0000007D R F . . B T .
VGA_DrawFarString            seg017    0000031F 0000002C R F . . B T .
UU_VGA_DrawCenteredFar       seg017    0000034B 0000002C R F . . B T .
UU_VGA_DrawRtAlgFar          seg017    00000377 0000002C R F . . B T .
UU_VGA_WndDrawFarString      seg017    000003A3 0000002C R F . . B T .
UU_VGA_WndDrawCntrdFar       seg017    000003CF 0000002C R F . . B T .
UU_VGA_WndDrawRtAlgFar       seg017    000003FB 0000002C R F . . B T .
UU_VGA_DrawNumber            seg017    00000427 00000037 R F . . B T .
UU_VGA_DrawLongNumber        seg017    0000045E 0000003A R F . . B T .
Print_Right                  seg017    00000498 00000035 R F . . B T .
Print_Centered               seg017    000004CD 0000003B R F . . B T .
Print_Integer_Right          seg017    00000508 00000036 R F . . B T .
UU_VGA_DrawCntrdNumber       seg017    0000053E 00000036 R F . . B T .
UU_VGA_DrawRtAlgLongN        seg017    00000574 00000039 R F . . B T .
UU_VGA_WndDrawNumber         seg017    000005AD 00000037 R F . . B T .
UU_VGA_WndDrawLongN          seg017    000005E4 0000003A R F . . B T .
UU_VGA_WndDrawRtAligned      seg017    0000061E 00000035 R F . . B T .
UU_VGA_WndDrawCentered       seg017    00000653 0000003B R F . . B T .
UU_VGA_WndDrawRtAlgNum       seg017    0000068E 00000036 R F . . B T .
UU_VGA_WndDrawRtAlgLong      seg017    000006C4 00000039 R F . . B T .
VGA_DrawJustified            seg017    000006FD 00000022 R F . . B T .
Print                        seg017    0000071F 00000019 R F . . B T .
Print_Display                seg017    00000738 000001F1 R F . . B T .
VGA_DrawString               seg017    00000929 000001A3 R F . . B T .
UU_VGA_DisableAAPixels       seg017    00000AF1 0000000B R F . . B T .
FNT_FontColorSelect_0        seg017    00000AFC 0000000E R F . . B T .
FNT_FontColorSelect_0or1     seg017    00000B0A 00000027 R F . . B T .
FNT_FontColorSelect_0or2     seg017    00000B31 00000027 R F . . B T .
FNT_FontColorSelect          seg017    00000B58 00000059 R F . . B T .
UU_DBG_TblDrawValue          seg017    00000BB1 0000009D R F . . B T .
UU_DBG_TblCellWrapper        seg017    00000C4E 00000024 R F . . B T .
UU_DBG_TblDrawString         seg017    00000C72 0000003A R F . . B T .
UU_DBG_TblDrawS16            seg017    00000CAC 0000003D R F . . B T .
UU_DBG_TblDrawS32            seg017    00000CE9 00000040 R F . . B T .
UU_DBG_TblDrawU32            seg017    00000D29 0000004B R F . . B T .
UU_DBG_DrawTableCell         seg017    00000D74 00000148 R F . . B T .
UU_VGA_WndDrawSString        seg017    00000EBC 000001BA R F . . B T .
UU_VGA_WndDrawString         seg017    00001076 000000F5 R F . . B T .
UU_VGA_WndDrawChar           seg017    0000118F 00000122 R F . . B T .
VGA_GetFontIndex             seg017    000012B1 00000015 R F . . B T .
VGA_GetFontColor1            seg017    000012C6 00000015 R F . . B T .
VGA_GetFontColor2            seg017    000012DB 00000015 R F . . B T .
UU_VGA_GetFontColor3         seg017    000012F0 00000015 R F . . B T .
UU_STR_CopyToNearLBX         seg017    00001305 0000001D R F . . B T .
VGA_GetVertSpacing           seg017    00001322 00000034 R F . . B T .
STR_TrimWhiteSpace           seg017    00001356 0000005F R F . . B T .
LBX_DrawTextLine             seg017    000013B5 0000001C R F . . B T .
LBX_DrawStyledString         seg017    000013D1 0000020E R F . . B T .
LBX_DrawString               seg017    000015DF 00000188 R F . . B T .
UU_LBX_DrawRightAligned      seg017    00001788 00000037 R F . . B T .
UU_LBX_DrawCentered          seg017    000017BF 0000003D R F . . B T .
UU_LBX_DrawJustified         seg017    000017FC 00000024 R F . . B T .
UU_LBX_DrawFarString         seg017    00001820 0000002E R F . . B T .
UU_LBX_DrawRtAlgFar          seg017    0000184E 0000002E R F . . B T .
UU_LBX_DrawCenteredFar       seg017    0000187C 0000002E R F . . B T .
UU_LBX_DrawJustifiedFar      seg017    000018AA 00000031 R F . . B T .
Set_Font                     seg018    00000074 00000146 R F . . B T .
Print_Character              seg018    000001BA 00000074 R F . . B T .
VGA_DrawChar_NoAA            seg018    0000022E 00000074 R F . . B T .
LBX_DrawGlyph                seg018    000002A2 000000AF R F . . B T .
Print_Character_ASM_         seg018    00000351 0000007E R F . . . T .
VGA_DrawGlyph_NoAA           seg018    000003CF 00000087 R F . . . T .
Get_String_Width             seg018    00000456 00000055 R F . . B T .
VGA_DrawPartialChar          seg018    000004AB 00000054 R F . . B T .
VGA_DrawPartialGlyph         seg018    000004FF 000000BE R F . . . T .
VGA_DrawText                 seg019    0000000D 0000018B R F . . B T .
UU_VGA_WndDrawText           seg019    000001A1 00000100 R F . . B T .
UU_GUI_CreateHelpLinks       seg019    000002A1 0000030B R F . . B T .
VGA_TextSetup                seg019    000005B4 00000330 R F . . B T .
VGA_GetTextHeight            seg019    000008E4 0000005E R F . . B T .
VGA_GetTextWidth             seg019    00000942 00000125 R F . . B T .
UU_VGA_ResetFltBlocks        seg019    00000A70 0000000B R F . . B T .
UU_VGA_AddFloatingBlock      seg019    00000A7B 00000044 R F . . B T .
UU_VGA_AddFltIMGSpace        seg019    00000ABF 00000074 R F . . B T .
STR_RemoveLBSPs              seg019    00000B33 00000026 R F . . B T .
VGA_GetFontHeight            seg019    00000B59 00000021 R F . . B T .
UU_VGA_WidestGlyph           seg019    00000B7A 0000004B R F . . B T .
UU_LBX_DrawText              seg019    00000BC5 00000198 R F . . B T .
VGA_TextDraw_Init            seg019    00000D65 00000075 R F . . B T .
Load_Palette                 seg020    0000000A 000000E2 R F . . B T .
UU_VGA_ResetShade0           seg020    000000EC 0000001D R F . . B T .
VGA_SetShades_Grey0          seg020    00000109 0000007E R F . . B T .
UU_VGA_SetShades_Range       seg020    00000187 000000A0 R F . . B T .
Set_Palette_Changes          seg020    00000227 00000023 R F . . B T .
Clear_Palette_Changes        seg020    0000024A 00000022 R F . . B T .
UU_PAL_ClearPalette          seg020    0000026C 00000035 R F . . B T .
UU_PAL_SavePalette           seg020    000002A1 0000001F R F . . B T .
UU_PAL_RestorePalette        seg020    000002C0 0000002C R F . . B T .
UU_Set_Palette_Colors        seg020    000002EC 00000048 R F . . B T .
UU_Reset_Cycle_Palette_Color seg020    00000334 0000000B R F . . B T .
UU_Cycle_Palette_Color       seg020    0000033F 000001CC R F . . B T .
UU_Update_Cycle              seg020    0000050B 00000094 R F . . B T .
UU_Set_Palette_Gradient      seg020    0000059F 000000B8 R F . . B T .
Apply_Palette                seg021    00000034 00000060 R F . . . T .
VGA_DAC_DimWrite             seg021    00000094 000000E4 R F . . B T .
UU_VGA_ShadeScreen           seg021    00000178 00000136 R F . . B T .
UU_VGA_SlideColors           seg021    000002AE 000000D2 R F . . B T .
UU_VGA_ColorWave             seg021    00000380 00000095 R F . . B T .
VGA_ShadeColorMatch          seg021    00000415 00000230 R F . . B T .
FLIC_Load_Palette            seg021    00000645 000000B5 R F . . B T .
UU_VGA_Bleep                 seg022    0000000A 0000000C R F . . B T .
UU_KBD_GetKey                seg022    00000016 00000009 R F . . B T .
UU_Clock_Wait30              seg022    0000001F 00000014 R F . . B T .
UU_VGA_FlushFrames           seg022    00000033 00000041 R F . . B T .
UU_RNG_SetSeed               seg022    00000074 00000011 R F . . B T .
UU_RNG_GetSeed               seg022    00000085 00000024 R F . . B T .
Randomize                    seg022    000000A9 0000002B R F . . B T .
RNG_Random                   seg022    000000D4 00000090 R F . . B T .
RP_STR_ToUpper               seg022    00000164 00000036 R F . . B T .
RP_STR_ToLower               seg022    0000019A 00000036 R F . . B T .
RP_MEM_Copy                  seg022    000001D0 00000028 R F . . B T .
RP_MEM_CopyIfLess            seg022    000001F8 00000031 R F . . B T .
RP_MEM_Clear_Near            seg022    00000229 00000027 R F . . B T .
RP_MEM_Set_Near              seg022    00000250 00000028 R F . . B T .
RP_MEM_PullRecord            seg022    00000278 00000076 R F . . B T .
RP_MEM_PullDynRecord         seg022    000002EE 00000061 R F . . B T .
RP_MEM_CopyIfLess_Far        seg022    0000034F 0000004F R F . . B T .
RP_MEM_Clear_Far             seg022    0000039E 00000037 R F . . B T .
RP_MEM_Set_Far               seg022    000003D5 00000038 R F . . B T .
RP_MEM_SwapWord              seg022    0000040D 00000022 R F . . B T .
RP_MEM_SwapBytes             seg022    0000042F 00000033 R F . . B T .
ST_MoveData                  seg022    00000462 0000006E R F . . B T .
RP_MEM_TestBit_Far           seg022    000004D0 0000002D R F . . B T .
RP_MEM_SetBit_Far            seg022    000004FD 0000002D R F . . B T .
RP_MEM_ClearBit_Far          seg022    0000052A 00000033 R F . . B T .
RP_MEM_TestBit_Near          seg022    0000055D 00000037 R F . . B T .
UU_MEM_SetBit_Near           seg022    00000594 0000002A R F . . B T .
UU_MEM_ClearBit_Near         seg022    000005BE 0000002A R F . . B T .
UU_DBG_SetSelectSetting      seg022    000005E8 0000001D R F . . B T .
UU_DBG_SelectDialog          seg022    00000605 00000258 R F . . B T .
UU_VGA_GrowOutFlip           seg022    0000085D 00000188 R F . . B T .
UU_LBX_IMG_CpyDrawFrame      seg022    000009E5 00000049 R F . . B T .
UU_VGA_GetPointInDir         seg023    0000000E 000000FB R F . . B T .
TILE_GetDistance             seg023    00000109 0000004B R F . . B T .
TILE_SimpleDistance          seg023    00000154 00000061 R F . . B T .
UU_Tile_PlotLine             seg023    000001B5 00000181 R F . . B T .
TILE_PlotMapLine             seg023    00000336 0000023C R F . . B T .
DBG_Quit                     seg024    00000002 00000015 R F . . B T .
DBG_ScreenDump               seg024    00000017 00000403 R F . . B T .
VGA_GetDACBlock              seg024    0000041A 00000029 R F . . B T .
VGA_ReadScreenLine           seg024    00000443 0000005B R F . . B T .
Release_Version              seg024    0000049E 0000000B R F . . B T .
Check_Release_Version        seg024    000004A9 0000000A R F . . B T .
UU_VGA_SaveDrawSection       seg024    000004B3 0000005A R F . . B T .
UU_VGA_CopyToLBX             seg024    0000050D 00000086 R F . . B T .
VGA_FILEH_Draw               seg025    0000000A 000001C7 R F . . B T .
VGA_MapNextEMMPage           seg025    000001D1 0000002F R . . . . T .
VGA_FILEH_Draw_R             seg025    00000200 000001E7 R F . . B T .
Set_Page_On                  seg026    0000000A 00000012 R F . . . T .
Set_Page_Off                 seg026    0000001C 00000016 R F . . . T .
Check_Default_Video_Page     seg026    00000032 0000000F R F . . . T .
Page_Flip                    seg026    00000041 00000034 R F . . . T .
Copy_On_To_Off_Page          seg026    00000075 00000022 R F . . . T .
Copy_On_To_Off_YM            seg026    00000097 000000C6 R F . . B T .
Copy_On_To_Off_NM            seg026    0000015D 0000004F R F . . . T .
Copy_Off_To_Back             seg026    000001AC 00000043 R F . . . T .
Copy_Back_To_Off             seg026    000001EF 00000043 R F . . . T .
VGA_SaveDraw_Frame4          seg026    00000232 00000043 R F . . . T .
VGA_LoadDraw_Frame4          seg026    00000275 00000043 R F . . . T .
UU_VGA_CopyToDraw            seg026    000002B8 00000044 R F . . B T .
FLIC_Draw_Frame              seg027    0000000C 00000100 R F . . B T .
FLIC_Remap_Draw_Frame        seg027    0000010C 000001EC . F . . B T .
FLIC_Draw_EMM                seg027    000002FE 000001C3 . F . . B T .
FLIC_EMM_MapNextPages        seg027    000004C1 00000027 R . . . . T .
FLIC_Draw_Frame_EMM_Remap    seg027    000004E8 000001E9 R F . . B T .
UU_LBX_Image_Copy            seg028    00000001 00000047 R F . . B T .
FLIC_Prepare                 seg028    00000048 00000084 R F . . B T .
UU_LBX_IMG_VShiftRect        seg028    000000CC 00000203 R F . . B T .
UU_LBX_IMG_VertWarp          seg028    000002CF 00000151 R F . . B T .
UU_LBX_IMG_HorzWarp          seg028    00000420 00000168 R F . . B T .
UU_LBX_IMG_PrepFill          seg028    00000588 00000085 R F . . B T .
UU_LBX_IMG_ColorShift        seg028    0000060D 0000009E R F . . B T .
UU_LBX_IMG_CReplaceRect      seg028    000006AB 000000A4 R F . . B T .
UU_LBX_IMG_ClearGraphic      seg028    0000074F 0000005F R F . . B T .
UU_LBX_IMG_LoadPalette       seg028    000007AE 00000037 R F . . B T .
FLIC_Draw                    seg028    000007E5 000000EE R F . . B T .
FLIC_Draw_XY_Wnd             seg028    000008D3 000001BA R F . . B T .
Draw_Picture_To_Bitmap       seg028    00000A8D 00000171 R F . . B T .
FLIC_Set_CurrentFrame        seg028    00000BFE 00000066 R F . . B T .
FLIC_ResetFrame              seg028    00000C64 00000017 R F . . B T .
FLIC_Get_CurrentFrame        seg028    00000C7B 00000015 R F . . B T .
UU_LBX_IMG_GetFrameCnt       seg028    00000C90 00000015 R F . . B T .
LBX_IMG_GetWidth             seg028    00000CA5 00000014 R F . . B T .
LBX_IMG_GetHeight            seg028    00000CB9 00000015 R F . . B T .
UU_VGA_WndDrawRotateImg      seg028    00000CCE 00000081 R F . . B T .
UU_VGA_WndDrawImageRect      seg028    00000D4F 0000109B R F . . B T .
UU_VGA_WndDrawTransform      seg028    00001E0A 0000013B R F . . B T .
UU_VGA_RotateRect            seg028    00001F45 0000025F R F . . B T .
Draw_Picture                 seg028    000021A4 00000039 R F . . B T .
Draw_Picture_Windowed        seg028    000021DD 0000012D R F . . B T .
UU_LBX_IMG_CopyInto          seg028    0000230A 00000168 R F . . B T .
UU_LBX_IMG_Overlay           seg028    00002472 00000168 R F . . B T .
UU_LBX_IMG_StripColors       seg028    000025DA 00000066 R F . . B T .
UU_LBX_IMG_CropRect          seg028    00002640 00000128 R F . . B T .
UU_LBX_IMG_ClearRect         seg028    00002768 000000E6 R F . . B T .
UU_LBX_IMG_FullGScale        seg028    0000284E 00000020 R F . . B T .
UU_LBX_IMG_ExtGScaleEC       seg028    0000286E 00000033 R F . . B T .
UU_LBX_IMG_ExtGrayScale      seg028    000028A1 00000118 R F . . B T .
UU_LBX_IMG_Resize            seg028    000029B9 000000D9 R F . . B T .
VGA_FILEH_LoadFirst          seg028    00002A92 00000075 R F . . B T .
VGA_FILEH_DrawFrame          seg028    00002B07 0000016E R F . . B T .
VGA_FILEH_Loader             seg028    00002C75 00000173 R F . . B T .
UU_VGA_FILEH_GetFrame        seg028    00002DE8 00000016 R F . . B T .
UU_VGA_FILEH_SetFrame        seg028    00002DFE 00000018 R F . . B T .
LBX_IMG_IsFullFramed         seg028    00002E16 00000090 R F . . B T .
LBX_IMG_SetLoop1             seg028    00002EA6 00000018 R F . . B T .
UU_LBX_IMG_OutlineOvr        seg028    00002EBE 000000DA R F . . B T .
UU_LBX_IMG_OutlineOvr_R      seg028    00002F98 000000E2 R F . . B T .
UU_LBX_IMG_ColorScrmblr      seg028    0000307A 0000006F R F . . B T .
UU_LBX_IMG_DrawRect          seg028    000030E9 00000151 R F . . B T .
LBX_IMG_GetGFXSize           seg028    0000323A 000000E1 R F . . B T .
VGA_DrawPartialImage         seg029    00000014 00000235 R F . . B T .
VGA_DrawPartImage_R          seg029    00000249 000002EA R F . . B T .
VGA_DrawPartEmsImg           seg030    00000012 000002F8 R F . . B T .
EMM_MapNextIMGPages          seg030    0000030A 0000002C R . . . . T .
VGA_DrawPartEmsImg_R         seg030    00000336 000002F1 R F . . B T .
FLIC_DecPrepare              seg031    00000008 00000042 R F . . B T .
FLIC_LoadFrame               seg031    0000004C 000000A4 R F . . B T .
FLIC_LoadEMSFrame            seg031    000000F6 00000134 R F . . B T .
EMM_MapNextIMGPages2         seg031    0000022A 00000025 R . . . . T .
Draw_Picture_ASM             seg031    00000255 000000BC R F . . B T .
VGA_CopyLBXImage_R           seg031    00000311 000000BB R . . . . T .
UU_LBX_IMG_RawOverlay        seg031    000003CC 0000004B R F . . B T .
UU_LBX_IMG_HorzFlip          seg031    00000417 00000042 R F . . B T .
UU_LBX_IMG_ColorReplace      seg031    00000459 00000037 R F . . B T .
UU_LBX_IMG_GetIntensity      seg031    00000490 00000076 R F . . B T .
UU_LBX_IMG_PartialCopy       seg031    00000506 0000004B R F . . B T .
UU_MEM_Clear_Far2            seg031    00000551 0000001C R F . . B T .
UU_LBX_IMG_Shrink            seg031    00000573 000000AA R F . . B T .
UU_LBX_IMG_Stretch           seg031    0000061D 000000C8 R F . . B T .
UU_LBX_IMG_RandomDelete      seg031    000007E7 00000070 R F . . B T .
UU_LBX_IMG_Grayscale         seg031    00000857 00000082 R F . . B T .
UU_LBX_IMG_RevGrayscale      seg031    000008D9 00000086 R F . . B T .
UU_DUP_RevGrayscale          seg031    0000095F 00000086 R F . . B T .
UU_RNG_GFX_Random            seg031    000009E9 0000008E R F . . B T .
UU_MATH_Inverse_Tangent      seg032    000001F7 00000059 R F . . B T .
UU_MATH_Inverse_Tan_QI       seg032    00000250 000000AD R . . . . T .
UU_MATH_Int_Cos              seg032    000002FD 0000008E R F . . B T .
UU_MATH_Int_Sin              seg032    0000038B 0000008E R F . . B T .
UU_VGA_DrawTexture           seg032    00000435 0000017C R F . . B T .
UU_VGA_DrawTexture_R         seg032    000005B1 000001E5 R F . . B T .
Set_Mouse_List               seg033    0000006E 00000044 R F . . B T .
Check_Mouse_Shape            seg033    000000B2 0000005A R F . . B T .
Get_Pointer_Image_Number     seg033    0000010C 0000000B R F . . . T .
Get_Pointer_Offset           seg033    00000117 0000000B R F . . . T .
Mouse_Button                 seg033    00000122 00000026 R F . . . T .
MD_WaitRelease               seg033    00000148 00000020 R F . . . T .
MD_Init                      seg033    00000168 00000096 R F . . B T .
CRP_MD_EnableEmu             seg033    000001FE 0000000E R F . . . T .
MD_Reset                     seg033    0000020C 00000048 R F . . . T .
Mouse_Movement_Handler       seg033    00000254 00000035 R F . . . T .
Mouse_Button_Handler         seg033    00000289 00000035 R F . . . T .
MD_INT_Handler               seg033    000002BE 000000E0 R F . . B T .
Save_Mouse_State             seg033    0000039E 00000018 R F . . . T .
Restore_Mouse_State          seg033    000003B6 00000012 R F . . . T .
UU_MOUSE_GetHwOrEmu          seg033    000003C8 00000015 R F . . . T .
Keyboard_Status              seg033    000003DD 00000020 R F . . . T .
Read_Key                     seg033    000003FD 0000025F R F . . . T .
Pointer_X                    seg033    0000065C 0000000B R F . . . T .
Pointer_Y                    seg033    00000667 0000000B R F . . . T .
MD_MoveCursor                seg033    00000672 0000002B R F . . B T .
MD_MoveCursorINT             seg033    0000069D 00000016 R F . . B T .
MD_GetClickX                 seg033    000006B3 0000000B R F . . . T .
MD_GetClickY                 seg033    000006BE 0000000B R F . . . T .
MD_GetClickedBtns            seg033    000006C9 0000000B R F . . . T .
Mouse_Buffer                 seg033    000006D4 00000011 R F . . . T .
Mouse_Buffer2                seg033    000006E5 00000011 R F . . . T .
CRP_MD_SetClick_CilckRec2    seg033    000006F6 0000001E R F . . B T .
UU_MD_SetClick_All           seg033    00000714 0000002A R F . . B T .
MD_ISR_SaveClick             seg033    0000073E 0000003A R F . . B T .
Save_Mouse_On_Page           seg033    00000778 000000D3 R F . . B T .
Save_Mouse_Off_Page          seg033    0000084B 000000DE R F . . B T .
Save_Mouse_On_Page_          seg033    00000929 000000B7 R F . . B T .
Copy_Mouse_Off_To_Mouse_Back seg033    000009E0 0000001B R F . . . T .
Restore_Mouse_On_Page        seg033    000009FB 00000090 R F . . B T .
Restore_Mouse_Off_Page       seg033    00000A8B 00000094 R F . . B T .
UU_CRL_Restore_SP3           seg033    00000B1F 00000085 R F . . B T .
Draw_Mouse_On_Page           seg033    00000BA4 00000026 R F . . B T .
Draw_Mouse_Off_Page          seg033    00000BCA 00000026 R F . . B T .
Draw_Mouse_On_Page_          seg033    00000BF0 000000D1 R F . . B T .
Draw_Mouse_Off_Page_         seg033    00000CC1 000000D5 R F . . B T .
UU_GUI_ClearClickBuffer      seg033    00000D96 0000001E R F . . . T .
UU_GUI_ClickBuffReset        seg033    00000DB4 00000007 R F . . . T .
UU_GUI_SetClickBuffVar1      seg033    00000DBB 00000007 R F . . . T .
UU_GUI_ClearBufferClick      seg033    00000DC2 0000001E R F . . . T .
UU_GUI_FindEmptyClick        seg033    00000DE0 00000033 R F . . . T .
UU_GUI_GetClickBuffVar1      seg033    00000E13 00000004 R F . . . T .
UU_GUI_AddBufferClick        seg033    00000E17 0000004E R F . . B T .
UU_GUI_ProcBufferClick       seg033    00000E65 00000079 R F . . . T .
Interpret_Mouse_Input        seg034    0000000E 00000F5F R F . . B T .
UU_GUI_SetTimeOutP           seg034    00000F92 0000001C R F . . B T .
UU_GUI_SetTimeOutN           seg034    00000FAE 0000001C R F . . B T .
UU_GUI_EditStart             seg034    00000FCA 00000010 R F . . B T .
UU_GUI_SetClickBuffer        seg034    00000FDA 00000031 R F . . B T .
UU_GUI_LoadClickFile         seg034    0000100B 00000054 R F . . B T .
UU_GUI_SaveClickFile         seg034    0000105F 00000047 R F . . B T .
Disable_Cancel               seg034    000010A6 0000000B R F . . B T .
UU_Enable_Cancel             seg034    000010B1 0000000B R F . . B T .
UU_GUI_SetClickActivate      seg034    000010BC 0000000B R F . . B T .
UU_GUI_ReleaseActivate       seg034    000010C7 0000000B R F . . B T .
Set_Help_List                seg034    000010D2 00000017 R F . . B T .
Deactivate_Help_List         seg034    000010E9 00000017 R F . . B T .
Check_Help_List              seg034    00001100 000000C3 R F . . B T .
UU_GUI_GetControlWidth       seg034    000011C3 0000004D R F . . B T .
UU_GUI_GetControlHeight      seg034    00001210 0000004E R F . . B T .
UU_GUI_GetLastClickX         seg034    0000125E 0000000A R F . . B T .
UU_GUI_GetLastClickY         seg034    00001268 0000000A R F . . B T .
UU_GUI_GetLastClickBtns      seg034    00001272 0000000A R F . . B T .
Get_Mouse_Field              seg034    0000127C 0000000A R F . . B T .
CRP_GUI_KeyInputOnly         seg034    00001286 00000675 R F . . B T .
Interpret_Keyboard_Input     seg034    000018FB 000004AB R F . . B T .
GUI_GetNextControl           seg034    00001DC7 00000DAC R F . . B T .
GUI_MouseEmuMove             seg034    00002B73 00000118 R F . . B T .
Scan_Field                   seg034    00002C8B 000000D0 R F . . B T .
Scan_Input                   seg034    00002D5B 00000172 R F . . B T .
UU_GUI_KeyWaitLoop           seg034    00002ECD 00000017 R F . . B T .
G_GUI_ClearInput             seg034    00002EE4 0000003C R F . . B T .
G_GUI_PressAnyKey            seg034    00002F20 0000018B R F . . B T .
GUI_ProcessDirKey            seg034    000030AB 000002C8 R F . . B T .
GUI_EditBoxControl           seg034    000033A1 0000042B R F . . B T .
GUI_TextEditDialog           seg034    000037E3 000003C9 R F . . B T .
GUI_EditBoxRedraw            seg034    00003BC3 000003E7 R F . . B T .
GUI_CreateEditBox            seg034    00003FAA 000001CF R F . . B T .
UU_GUI_CreateEditSelect      seg034    00004179 00000163 R F . . B T .
UU_GUI_Ctrl_DecHeight        seg034    000042DC 0000002D R F . . B T .
UU_GUI_CreateDialogLine      seg034    00004309 00000210 R F . . B T .
UU_UI_AddAltKeyControl       seg034    00004519 00000150 R F . . B T .
RP_GUI_CreateImageLabel      seg034    00004669 00000147 R F . . B T .
RP_GUI_CreateWndImgLbl       seg034    000047B0 000001D0 R F . . B T .
UU_GUI_SetBtnClickDir        seg034    00004980 00000011 R F . . B T .
GUI_Create_IMG_Ctrl          seg034    00004991 0000018A R F . . B T .
GUI_CreateClickBtn           seg034    00004B1B 0000003F R F . . B T .
UU_GUI_CreateToggleBtn       seg034    00004B5A 00000054 R F . . B T .
UU_GUI_CreateLockblBtn       seg034    00004BAE 00000054 R F . . B T .
UU_GUI_CreateMStateBtn       seg034    00004C02 00000069 R F . . B T .
Add_Hidden_Field             seg034    00004C6B 000000FB R F . . B T .
UU_GUI_CreateWndLabel        seg034    00004D66 00000193 R F . . B T .
Add_Hot_Key                  seg034    00004EF9 000000F6 R F . . B T .
UU_GUI_CreateHelpLink        seg034    00004FEF 0000013A R F . . B T .
UU_GUI_CreateClickGrid       seg034    00005129 0000015E R F . . B T .
GUI_CreateNearSlider         seg034    00005287 00000031 R F . . B T .
GUI_CreateSlideBar           seg034    000052B8 000001DD R F . . B T .
GUI_SliderUpdate             seg034    00005495 0000027D R F . . B T .
GUI_SlideUp                  seg034    00005712 00000132 R F . . B T .
GUI_SlideDown                seg034    00005844 00000131 R F . . B T .
Clear_Fields                 seg034    00005975 00000023 R F . . B T .
UU_GUI_NoSelectColor         seg034    00005998 0000001D R F . . B T .
UU_GUI_ScrollableTxtDlg      seg034    000059B5 00000AA4 R F . . B T .
UU_GUI_CreateTextDialog      seg034    00006459 0000026C R F . . B T .
UU_GUI_NoMouseHighlight      seg034    000066C5 0000000B R F . . B T .
GUI_MoveDlgHighlight         seg034    000066D0 00000425 R F . . B T .
UU_GUI_SetControlCount       seg034    00006AF5 00000012 R F . . B T .
UU_GUI_GetControlCount       seg034    00006B07 0000000A R F . . B T .
Init_Mouse_Keyboard          seg034    00006B11 000000C9 R F . . B T .
Get_Input                    seg034    00006BDA 0000004A R F . . B T .
Set_Input_Delay              seg034    00006C24 00000015 R F . . B T .
UU_GUI_GetDelay              seg034    00006C39 0000000A R F . . B T .
UU_GUI_ClearSelectable       seg034    00006C43 0000002F R F . . B T .
UU_GUI_SetSelectable         seg034    00006C72 0000002F R F . . B T .
Draw_Fields                  seg034    00006CA1 000014EB R F . . B T .
Draw_Field                   seg034    000081A4 000010AC R F . . B T .
Push_Field_Down              seg034    0000926A 000001DB R F . . B T .
UU_GUI_SetDlgTitleHelp       seg034    00009445 0000000B R F . . B T .
UU_GUI_ClearTitleHelp        seg034    00009450 0000000B R F . . B T .
Assign_Auto_Function         seg034    0000945B 00000030 R F . . B T .
Save_Auto_Function           seg034    0000948B 00000013 R F . . B T .
Restore_Auto_Function        seg034    0000949E 00000019 R F . . B T .
Deactivate_Auto_Function     seg034    000094B7 0000000B R F . . B T .
UU_Activate_Auto_Function    seg034    000094C2 0000000B R F . . B T .
Invoke_Auto_Function         seg034    000094CD 00000010 R F . . B T .
Quick_Call_Auto_Function     seg034    000094DD 0000003A R F . . B T .
Call_Auto_Function           seg034    00009517 0000003A R F . . B T .
UU_CTRL_Clear_Skip1          seg034    00009551 00000012 R F . . B T .
Toggle_Pages                 seg034    00009563 0000004F R F . . B T .

Save_SAVE_GAM                ovr048    00000000 000003F8 R F . . B T .
Load_SAVE_GAM                ovr048    000003F8 000003E8 R F . . B T .

Settings_Screen              ovr049    00000000 000003B8 R F . . B T .
Settings_Screen_Draw         ovr049    000003B8 0000046F R F . . B T .
Load_MAGIC_SET               ovr049    00000827 00000127 R F . . B T .
Set_Default_Game_Settings_   ovr049    0000094E 000000E5 R F . . B T .
Add_Settings_Screen_Help     ovr049    00000A33 00000033 R F . . B T .

GAME_New_Create              ovr050    00000000 00000107 R F . . B T .
GAME_WizardsLaunch           ovr050    0000012F 00000047 R F . . B T .
Load_Screen                  ovr050    00000176 000003E6 R F . . B T .
Load_Screen_Draw             ovr050    0000055C 00000252 R F . . B T .
NewGame_Screen_1             ovr050    000007AE 00000430 R F . . B T .
NewGame_Screen_Draw_1        ovr050    00000BDE 000004C4 R F . . B T .
GAME_RandBookBinders         ovr050    000010A2 000000CE R F . . B T .
GAME_New_Screen_1            ovr050    00001170 00000231 R F . . B T .
Draw_NewGame_Screen1         ovr050    000013A1 000005F5 R F . . B T .
GAME_New_Screen_2            ovr050    00001996 000001EA R F . . B T .
GAME_New_Screen_3            ovr050    00001B80 0000010F R F . . B T .
GAME_Draw_NewScr3            ovr050    00001C8F 00000119 R F . . B T .
GAME_New_Screen_7            ovr050    00001DA8 00000229 R F . . B T .
GAME_Draw_NewScr7            ovr050    00001FDC 000001D4 R F . . B T .
GAME_New_Screen_6            ovr050    000021B0 00000457 R F . . B T .
GAME_Draw_NewScr6            ovr050    00002607 00000403 R F . . B T .
GAME_DrawRetortsStr          ovr050    00002A0A 0000014A R F . . B T .
NEWG_DrawDefShelf            ovr050    00002B54 000000F9 R F . . B T .
GAME_New_Screen_4            ovr050    00002C4D 00000836 R F . . B T .
GAME_Draw_NewScr4            ovr050    00003490 00000B95 R F . . B T .
GAME_New_Screen_5            ovr050    0000402F 0000077B R F . . B T .
GAME_SpellSel_GUI            ovr050    000047AA 000002E1 R F . . B T .
SCRN_Draw_NewScr5            ovr050    00004A8B 000002B4 R F . . B T .
SCRN_Draw_NewScr5_2          ovr050    00004D3F 00000935 R F . . B T .
WIZ_CopyDefault              ovr050    00005674 000002A5 R F . . B T .
Fade_Out                     ovr050    00005919 00000038 R F . . B T .
VGA_Fade_In                  ovr050    00005951 00000037 R F . . B T .
Load_Screen_Help             ovr050    00005988 00000033 R F . . B T .
HLP_Load_NewGOptions         ovr050    000059BB 00000033 R F . . B T .
HLP_Load_BannerSel           ovr050    000059EE 00000033 R F . . B T .
HLP_Load_PortraitSel         ovr050    00005A21 00000033 R F . . B T .
HLP_Load_WizardSel           ovr050    00005A54 0000003F R F . . B T .
HLP_Load_WizCreate           ovr050    00005A93 00000033 R F . . B T .
HLP_Load_RaceSel             ovr050    00005AC6 00000033 R F . . B T .
STR_ListSeparator            ovr050    00005AF9 0000003F R F . . B T .
CRP_Empty_Dialog_Fn2         ovr050    00005B38 00000005 R F . . B T .
CRP_Empty_Dialog_Fn1         ovr050    00005B3D 00000005 R F . . B T .
Do_Toggle_Pages              ovr050    00005B42 0000000A R F . . B T .

NEWG_CreateWorld             ovr051    00000000 000002B9 R F . . B T .
NEWG_EZ_MarkHadnLeft         ovr051    000002B9 0000008B R F . . B T .
NEWG_TileIsleExtend          ovr051    00000344 000003BE R F . . B T .
NEWG_CreateTowers            ovr051    00000715 000001BA R F . . B T .
NEWG_CreateCapitals          ovr051    000008CF 00000B22 R F . . B T .
NEWG_ClearLandmasses         ovr051    00001450 0000003E R F . . B T .
NEWG_EqualizeNodes           ovr051    0000148E 000002AD R F . . B T .
NEWG_SetSpecLands            ovr051    0000173B 0000043B R F . . B T .
NEWG_SetBaseLands            ovr051    00001B76 00000293 R F . . B T .
NEWG_CreateLands             ovr051    00001E09 000002BC R F . . B T .
NEWG_CreateNodes             ovr051    000020C5 000004FD R F . . B T .
NEWG_CreateNodeAura          ovr051    000025C2 0000019D R F . . B T .
NODE_IsAuraUnique            ovr051    0000275F 000000EA R F . . B T .
NEWG_SetNodeType             ovr051    00002849 00000260 R F . . B T .
TILE_SetLandMass             ovr051    00002AA9 000003DD R F . . B T .
NEWG_CreateEZs               ovr051    00002E86 000003A1 R F . . B T .
NEWG_CreateEncounter         ovr051    00003227 00000A5A R F . . B T .
UU_UNIT_RandomRacial         ovr051    00003CC7 00000080 R F . . B T .
NEWG_SetDeserts              ovr051    00003D47 00000378 R F . . B T .
NEWG_CreateShores            ovr051    000040BF 000011E2 R F . . B T .
NEWG_RandomizeTiles          ovr051    000052A1 00000432 R F . . B T .
NEWG_CreateRiver             ovr051    000056E3 000004AC R F . . B T .
NEWG_SetRiverTiles           ovr051    00005B8F 00001763 R F . . B T .
NEWG_CreateNeutrals          ovr051    000073C2 00000FB9 R F . . B T .
CTY_SetDefaultName           ovr051    000083F3 000000E0 R F . . B T .
NEWG_CreateRoads             ovr051    000084D3 000004D1 R F . . B T .
CRP_NEWG_CreatePathGrids     ovr051    000089A4 0000006F R F . . B T .
NEWG_CreateSpecials          ovr051    00008A13 000003DE R F . . B T .
NEWG_DesertSpecials          ovr051    00008DFF 0000004E R F . . B T .
NEWG_HillSpecials            ovr051    00008E61 00000063 R F . . B T .
NEWG_MntnSpecials            ovr051    00008EED 00000065 R F . . B T .
NEWG_SetScoutingMaps         ovr051    00008F7A 00000264 R F . . B T .
TILE_SetScouting             ovr051    000091DE 0000003E R F . . B T .
NEWG_SetMoveMaps             ovr051    0000921C 00000313 R F . . B T .
TILE_InvalidOutflow          ovr051    0000952F 00000044 R F . . B T .
NEWG_UpdateProgress          ovr051    00009573 00000062 R F . . B T .
UNIT_Create                  ovr051    000095D5 000002E0 R F . . B T .
NEWG_AnimateOceans           ovr051    000098B5 000000E7 R F . . B T .
TILE_IsOcean                 ovr051    0000999C 00000070 R F . . B T .
TILE_HasNode                 ovr051    00009A0C 0000005C R F . . B T .
TILE_HasTower                ovr051    00009A68 00000043 R F . . B T .
TILE_IsForest                ovr051    00009AAB 0000004B R F . . B T .
TILE_IsRiver                 ovr051    00009AF6 00000069 R F . . B T .
UU_TILE_IsRiverOutlet        ovr051    00009B5F 00000061 R F . . B T .
UU_TILE_IsShore              ovr051    00009BC0 0000006E R F . . B T .
TILE_IsMountains             ovr051    00009C2E 0000004B R F . . B T .
TILE_IsHills                 ovr051    00009C79 0000004B R F . . B T .
TILE_IsSwamp                 ovr051    00009CC4 0000004B R F . . B T .
TILE_IsDesert                ovr051    00009D0F 0000005D R F . . B T .
TILE_IsGrasslands            ovr051    00009D6C 00000056 R F . . B T .
TILE_GetFood                 ovr051    00009DC2 000000D8 R F . . B T .
TILE_GetMaxPop               ovr051    00009EC8 00000061 R F . . B T .
TILE_GetCatchment            ovr051    00009F29 0000008C R F . . B T .
UU_Empty_Tile_Fn             ovr051    00009FB5 00000005 R F . . B T .

UU_GAME_AutoSave             ovr052    00000000 00000067 R F . . B T .

UU_GUI_Confirm_Dialog        ovr053    00000000 000001F1 R F . . B T .
UU_GUI_DrawConfrmDialog      ovr053    000001F1 00000131 R F . . B T .
UU_GUI_WarningType1          ovr053    00000322 00000014 R F . . B T .
GUI_WarningType0             ovr053    00000336 00000014 R F . . B T .
Warning_Message              ovr053    0000034A 00000108 R F . . B T .
GUI_DrawRedMessage           ovr053    00000452 0000014B R F . . B T .
UU_GUI_ListSelect_Dlg        ovr053    0000059D 000004E4 R F . . B T .
UU_GUI_DrawListSelector      ovr053    00000A81 00000720 R F . . B T .
UU_GUI_SetListSelBounds      ovr053    000011A1 00000113 R F . . B T .
UU_GUI_LoadNotifyGfx         ovr053    000012B4 00000188 R F . . B T .
UU_GUI_Notify_Dialog         ovr053    00001444 00000218 R F . . B T .
UU_GUI_DrawNotifyDialog      ovr053    00001667 0000035D R F . . B T .
UU_GUI_Familiar_Notify       ovr053    000019C4 0000022C R F . . B T .
UU_GUI_DrawFamiliarNtfy      ovr053    00001BFB 0000026C R F . . B T .
UU_HLP_NoResizeExpand        ovr053    00001E67 00000036 R F . . B T .
UU_HLP_DrawExpanding         ovr053    00001E9D 00000366 R F . . B T .
UU_USW_DrawEHLines           ovr053    00002203 00000055 R F . . B T .
UU_HLP_DrawExpandLines       ovr053    00002258 000009FB R F . . B T .
UU_ITEM_GetViewDrawPos       ovr053    00002C53 00000148 R F . . B T .
UU_ITEM_ResetViewGrow        ovr053    00002D9B 0000000B R F . . B T .
EMM_Sandbox2VGAFILEH         ovr053    00002DA6 0000004A R F . . B T .
EMM_VGAFILEH2Sandbox         ovr053    00002DF0 0000004A R F . . B T .

CRP_SPATH_Arbitrary          ovr054    00000000 00000726 R F . . B T .
UU_SPATH_Segmented           ovr054    00000726 00000330 R F . . B T .
UU_SPATH_15by15              ovr054    00000A56 000002AE R F . . B T .
UU_SPATH_Segments            ovr054    00000D04 0000064B R F . . B T .
UU_SPATH_GetBorderTiles      ovr054    0000134F 0000019A R F . . B T .

CRP_NEWG_CreatePathGrid      ovr055    00000000 00000A00 R F . . B T .

WIZ_SetProfiles              ovr056    00000000 000000C3 R F . . B T .
NEWG_FinalizeTables          ovr056    000000C3 000000A7 R F . . B T .
AI_WIZ_SetRelations          ovr056    0000016A 00000224 R F . . B T .
NEWG_ClearItems              ovr056    0000038E 00000038 R F . . B T .
WIZ_Set_Sum_n_SoM            ovr056    000003C6 000000EC R F . . B T .
WIZ_SetPersonalities         ovr056    000004B2 000004A7 R F . . B T .
AI_WIZ_StrategyReset         ovr056    0000096D 0000009A R F . . B T .
WIZ_SetStartingStats         ovr056    00000A07 00000337 R F . . B T .
AI_CreateWizards             ovr056    00000D3E 000009C0 R F . . B T .
WIZ_SetSpells                ovr056    00001726 00000C24 R F . . B T .
NEWG_ClearEvents             ovr056    000023AB 000000C2 R F . . B T .
NEWG_CreateHeroes            ovr056    0000246D 00000660 R F . . B T .
CRP_MSG_Clear                ovr056    00002AE9 00000037 R F . . B T .
UU_ITEM_SetHeroSlots         ovr056    00002B20 00000071 R F . . B T .
WIZ_ConsolidateBooks         ovr056    00002B91 00000442 R F . . B T .
WIZ_GetNominalSkill          ovr056    00002FD3 00000062 R F . . B T .
RNG_WeightedPick16           ovr056    00003035 00000095 R F . . B T .

MoM_Init_Tables              ovr057    00000000 00000422 R F . . B T .

IntroAnim_Screen             ovr058    00000000 0000039F R F . . B T .
VGA_Fade_Out_2               ovr058    0000039F 00000038 R F . . B T .
VGA_Fade_Out_10              ovr058    000003D7 00000038 R F . . B T .
VGA_Fade_Out_20              ovr058    0000040F 00000038 R F . . B T .
VGA_Fade_In_2                ovr058    00000447 00000037 R F . . B T .
VGA_Fade_In_50               ovr058    0000047E 00000037 R F . . B T .

GUI_DisplayHelp              ovr059    00000000 0000057B R F . . B T .
GUI_Help_RedrawFn            ovr059    0000057B 00000005 R F . . B T .
_f590303_EMM_Map_VGAFILEH1_5 ovr059    00000580 00000019 R F . . B T .

Draw_Credits                 ovr060    00000000 000001D3 R F . . B T .
Init_Credits                 ovr060    000001D3 00000037 R F . . B T .

Hall_Of_Fame_Screen          ovr061    00000000 000001A2 R F . . B T .
Draw_Hall_Of_Fame_Screen     ovr061    000001A2 00000294 R F . . B T .
