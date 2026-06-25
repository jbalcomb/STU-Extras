; Attributes: bp-based frame
proc _main far
found_file= byte ptr -5Ch
file_name= byte ptr -3Eh
SFX_HW= word ptr -16h
l_midi_driver= word ptr -14h
DIGI_DMA= word ptr -12h
DIGI_IRQ= word ptr -10h
DIGI_IO= word ptr -0Eh
DIGI_DRV= word ptr -0Ch
MIDI_IRQ= word ptr -0Ah
MIDI_IO= word ptr -8
itr_savegams= word ptr -6
file_handle= word ptr -4
main_menu_music_seg= word ptr -2
argc= word ptr  6
argv= word ptr  8
_SI_midi_driver = si
_DI_MIDI_DMA = di
push    bp
mov     bp, sp
loc_1320C:
sub     sp, 5Ch
push    _SI_midi_driver
push    _DI_MIDI_DMA
mov     [EMM_Pages_Reserved], 158
lea     ax, [bp+found_file]
push    ax
mov     ax, offset str_CONFIG_MOM
push    ax
call    DIR
pop     cx
pop     cx
or      ax, ax
jnz     short @@Found_ConfigMom
mov     ax, offset str_CONFIG_MOM_ERROR
push    ax
call    Exit_With_Message
pop     cx
@@Found_ConfigMom:
mov     ax, offset str_RB
push    ax
mov     ax, offset str_CONFIG_MOM
push    ax
call    fopen
pop     cx
pop     cx
mov     [bp+file_handle], ax
push    [bp+file_handle]
mov     ax, 1
push    ax
mov     ax, size s_CONFIG_MOM
push    ax
mov     ax, offset config_mom
push    ax
call    fread
add     sp, 8
push    [bp+file_handle]
call    fclose
pop     cx
call    j_Load_MAGIC_SET
mov     ax, [config_mom.MIDI_IO]
mov     [bp+MIDI_IO], ax
mov     ax, [config_mom.MIDI_IRQ]
mov     [bp+MIDI_IRQ], ax
mov     _DI_MIDI_DMA, e_ST_UNDEFINED
mov     ax, [config_mom.MIDI_HW]
mov     [bp+l_midi_driver], ax
mov     cx, 0Ch
mov     bx, offset SwitchCase_MIDI_DRV
@@Loop__MIDI_DRV:
mov     ax, [cs:bx]
cmp     ax, [bp+l_midi_driver]
jz      short loc_13295
add     bx, 2
loop    @@Loop__MIDI_DRV
@@Jmp_Check_SND_Roland:
jmp     short @@Check_SND_Roland
loc_13295:
jmp     [word ptr cs:bx+18h]
MIDI_DRV_00:
xor     _SI_midi_driver, _SI_midi_driver
@@Set_MIDI_IO_neg1:
mov     [bp+MIDI_IO], e_ST_UNDEFINED
@@Set_MIDI_IRQ_neg1:
mov     [bp+MIDI_IRQ], e_ST_UNDEFINED
@@Set_DI_neg1:
mov     _DI_MIDI_DMA, e_ST_UNDEFINED
jmp     short @@Check_SND_Roland
MIDI_DRV_01:
mov     _SI_midi_driver, SND_Speaker
jmp     short @@Set_MIDI_IO_neg1
MIDI_DRV_02:
mov     _SI_midi_driver, SND_AdLib
jmp     short @@Set_MIDI_IO_neg1
MIDI_DRV_03:
mov     _SI_midi_driver, SND_SB
jmp     short @@Set_MIDI_IRQ_neg1
MIDI_DRV_04:
mov     _SI_midi_driver, SND_SBPro
jmp     short @@Set_MIDI_IRQ_neg1
MIDI_DRV_06:
mov     _SI_midi_driver, SND_SBPro2
jmp     short @@Set_MIDI_IRQ_neg1
MIDI_DRV_05:
mov     _SI_midi_driver, SND_PAS
jmp     short @@Set_MIDI_IO_neg1
MIDI_DRV_07:
mov     _SI_midi_driver, SND_PAS16
jmp     short @@Set_MIDI_IO_neg1
MIDI_DRV_08:
mov     _SI_midi_driver, SND_ALG
jmp     short @@Set_MIDI_IO_neg1
MIDI_DRV_09:
jmp     short $+2
MIDI_DRV_10:
mov     _SI_midi_driver, SND_Roland
jmp     short @@Set_DI_neg1
MIDI_DRV_11:
mov     _SI_midi_driver, SND_GMIDI
jmp     short @@Set_DI_neg1
@@Check_SND_Roland:
cmp     _SI_midi_driver, SND_Roland
jl      short loc_132FC
@@WaitRolandDrvInit:
mov     ax, offset cnst_MIDI_Init
push    ax
call    DOS_PrintString__STUB
pop     cx
call    Mark_Time
mov     ax, 2
push    ax
call    Release_Time
pop     cx
loc_132FC:
mov     ax, [config_mom.SFX_IO]
mov     [bp+DIGI_IO], ax
mov     ax, [config_mom.SFX_IRQ]
mov     [bp+DIGI_IRQ], ax
mov     ax, [config_mom.SFX_DMA]
mov     [bp+DIGI_DMA], ax
mov     ax, [config_mom.SFX_HW]
mov     [bp+SFX_HW], ax
mov     cx, 0Ch
mov     bx, offset SwitchCase_SFX_HW
loc_1331A:
mov     ax, [cs:bx]
cmp     ax, [bp+SFX_HW]
jz      short loc_13329
add     bx, 2
loop    loc_1331A
jmp     short @@loc_13394
loc_13329:
jmp     [word ptr cs:bx+18h]
SFX_HW_00:
mov     [bp+DIGI_DRV], SND_None
loc_13332:
mov     [bp+DIGI_IO], -1
mov     [bp+DIGI_IRQ], -1
mov     [bp+DIGI_DMA], -1
jmp     short @@loc_13394
SFX_HW_01:
mov     [bp+DIGI_DRV], SND_Speaker
jmp     short loc_13332
SFX_HW_02:
jmp     short SFX_HW_00
SFX_HW_03:
mov     [bp+DIGI_DRV], SND_SB
jmp     short @@loc_13394
SFX_HW_04:
mov     [bp+DIGI_DRV], SND_SBPro
jmp     short @@loc_13394
SFX_HW_06:
mov     [bp+DIGI_DRV], SND_SBPro2
mov     ax, [bp+DIGI_IRQ]
mov     [bp+MIDI_IRQ], ax
mov     _DI_MIDI_DMA, [bp+DIGI_DMA]
jmp     short @@loc_13394
SFX_HW_05:
mov     [bp+DIGI_DRV], SND_PAS
jmp     short loc_13376
SFX_HW_07:
mov     [bp+DIGI_DRV], SND_PAS16
loc_13376:
mov     [bp+DIGI_IO], -1
jmp     short @@loc_13394
SFX_HW_08:
mov     [bp+DIGI_DRV], SND_ALG
jmp     short @@loc_13394
SFX_HW_09:
jmp     short $+2
SFX_HW_10:
mov     [bp+DIGI_DRV], SND_Roland
jmp     short loc_13332
SFX_HW_11:
mov     [bp+DIGI_DRV], SND_GMIDI
jmp     short loc_13332
@@loc_13394:
cmp     _SI_midi_driver, SND_AdLib
jnz     short @@MagicSet_SoundChannels
mov     [bp+DIGI_DRV], SND_AdLib
@@MagicSet_SoundChannels:
cmp     [magic_set.sound_channels], 2
jg      short @@loc_133AC
cmp     [magic_set.sound_channels], 0
jge     short @@loc_133B2
@@loc_133AC:
mov     [magic_set.sound_channels], 0
@@loc_133B2:
mov     [magic_set.input_type], 1
mov     [bp+itr_savegams], 1
jmp     @@loc_1347B
loc_133C0:
mov     bx, [bp+itr_savegams]
shl     bx, 1
cmp     magic_set.Have_Save[bx-2], e_ST_FALSE
jnz     short loc_133CF
jmp     loc_13478
loc_133CF:
mov     ax, 10
push    ax
lea     ax, [bp+found_file]
push    ax
push    [bp+itr_savegams]
call    itoa
add     sp, 6
mov     ax, offset SAVE_NAME
push    ax
lea     ax, [bp+file_name]
push    ax
call    strcpy
pop     cx
pop     cx
lea     ax, [bp+found_file]
push    ax
lea     ax, [bp+file_name]
push    ax
call    strcat
pop     cx
pop     cx
mov     ax, offset SAVE_EXT
push    ax
lea     ax, [bp+file_name]
push    ax
call    strcat
pop     cx
pop     cx
lea     ax, [bp+found_file]
push    ax
lea     ax, [bp+file_name]
push    ax
call    DIR
pop     cx
pop     cx
cmp     [bp+found_file], 0
jnz     short loc_13478
mov     bx, [bp+itr_savegams]
shl     bx, 1
mov     magic_set.Have_Save[bx-2], e_ST_FALSE
mov     ax, offset empty_string__MAIN
push    ax
mov     ax, [bp+itr_savegams]
mov     dx, 20
imul    dx
add     ax, offset magic_set.Save_Names-14h
push    ax
call    strcpy
pop     cx
pop     cx
mov     ax, offset str_WB
push    ax
mov     ax, offset str_MAGIC_SET
push    ax
call    fopen
pop     cx
pop     cx
mov     [bp+file_handle], ax
push    [bp+file_handle]
mov     ax, 1
push    ax
mov     ax, size s_MAGIC_SET
push    ax
mov     ax, offset magic_set
push    ax
call    fwrite
add     sp, 8
push    [bp+file_handle]
call    fclose
pop     cx
loc_13478:
inc     [bp+itr_savegams]
@@loc_1347B:
cmp     [bp+itr_savegams], 9
jge     short loc_13484
jmp     loc_133C0
loc_13484:
mov     ax, 2700
push    ax
call    EMM_Set_Minimum
pop     cx
mov     ax, 583
push    ax
call    RAM_Set_Minimum
pop     cx
mov     [magic_set.input_type], 1
mov     [magic_set.sound_channels], 2
push    [bp+DIGI_DMA]
push    [bp+DIGI_IRQ]
push    [bp+DIGI_IO]
push    [bp+DIGI_DRV]
push    _DI_MIDI_DMA
push    [bp+MIDI_IRQ]
push    [bp+MIDI_IO]
push    _SI_midi_driver
mov     ax, offset fonts_lbx_file__main
push    ax
push    [magic_set.sound_channels]
push    [magic_set.input_type]
call    Init_Drivers
add     sp, 16h
call    Release_Version
mov     ax, 6100
push    ax
call    j_Allocate_Data_Space
pop     cx
call    Disable_Cancel
mov     ax, e_ST_UNDEFINED
push    ax
xor     ax, ax
push    ax
call    Load_Palette
pop     cx
pop     cx
call    Apply_Palette
mov     bx, [bp+argv]
mov     bx, [bx+2]
cmp     [byte ptr bx], 'J'
jnz     short @@PlayIntro
mov     bx, [bp+argv]
mov     bx, [bx+2]
cmp     [byte ptr bx+1], 'E'
jnz     short @@PlayIntro
mov     bx, [bp+argv]
mov     bx, [bx+2]
cmp     [byte ptr bx+2], 'N'
jnz     short @@PlayIntro
mov     bx, [bp+argv]
mov     bx, [bx+2]
cmp     [byte ptr bx+3], 'N'
jnz     short @@PlayIntro
mov     bx, [bp+argv]
mov     bx, [bx+2]
cmp     [byte ptr bx+4], 'Y'
jnz     short @@PlayIntro
jmp     short @@PrepareMainMenu
@@PlayIntro:
call    j_Draw_Logos
@@PrepareMainMenu:
nop
push    cs
call    near ptr Load_MGC_Resources
nop
push    cs
call    near ptr Load_TERRSTAT
nop
push    cs
call    near ptr Load_SPELLDAT
call    j_Init_Credits
xor     ax, ax
push    ax
mov     ax, e_SCREEN_YMAX
push    ax
mov     ax, e_SCREEN_XMAX
push    ax
xor     ax, ax
push    ax
xor     ax, ax
push    ax
call    Fill
add     sp, 0Ah
call    Set_Page_On
xor     ax, ax
push    ax
mov     ax, e_SCREEN_YMAX
push    ax
mov     ax, e_SCREEN_XMAX
push    ax
xor     ax, ax
push    ax
xor     ax, ax
push    ax
call    Fill
add     sp, 0Ah
call    Set_Page_Off
call    Stop_Music__STUB
mov     ax, MUSIC_Main_Menu
push    ax
mov     ax, offset music_lbx__main
push    ax
call    LBX_Load
pop     cx
pop     cx
mov     [bp+main_menu_music_seg], ax
cmp     [magic_set.background_music], e_ST_TRUE
jnz     short @@MainMenu
push    [bp+main_menu_music_seg]
call    Play_Sound
pop     cx
@@MainMenu:
mov     ax, e_ST_UNDEFINED
push    ax
xor     ax, ax
push    ax
call    Load_Palette
pop     cx
pop     cx
call    Apply_Palette
nop
push    cs
call    near ptr Menu_Screen_Control
mov     ax, 8
push    ax
call    j_Save_SAVE_GAM
pop     cx
call    Exit_With_Size
pop     _DI_MIDI_DMA
pop     _SI_midi_driver
mov     sp, bp
pop     bp
retf
endp _main ; sp-analysis failed
