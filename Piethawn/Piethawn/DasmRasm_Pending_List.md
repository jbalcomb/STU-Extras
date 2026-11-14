


Still open: 19 triage rows
All 19 are engine (MoX) items. Almost all are waiting on code that isn't reconstructed yet:

Area	Rows	Blocked on
EMS / LBX cache	#335, #374, #459, #460, #461; #399 (Pic_EMS_Next_Pages)	EMS.c s13p04/10/11-13/18/30 and EMM_MapNextIMGPages2 are not reconstructed. ReMoM doesn't use EMS.
Timing	#333 Wait_Ticks; #383 Wait_Ticks_Or_Input	Release_Time is reconstructed. Wait_Ticks_Or_Input isn't.
DOS	#334 DOS_File_Open (a failed open isn't noticed when saving); #384 DOS_Drive_Ready	gfopen / gfwrite exist but the row isn't checked yet. DOS_Drive_Ready is only a name comment.
Sound	#346, #397 Sound_Fatal_Error	Audio_Error__STUB
Input	#338 UI_Poll_Input_Keyboard;

Picture drawing	#339 Pic_Blit_Clipped_Planar; #394 Pic_Keep_Rect; #396 Pic_Draw_Rect_Outline; #378 Pic_Draw_Warped_Core	#394 and #396 are comment-only functions. #378 is Draw_Bitmap_Animation_Block, which you set aside for a full 1:1 review.
Startup	#407 Engine_Init_Short	UU_Legacy_Startup, comment only
Two could be closed without reconstructing anything first:

#334: check gfopen / gfwrite against the asm.
#339: check Clipped_Draw_Frame against the asm.
Still open: 40 follow-ups
These are deviations and possible undefined behaviour found along the way, outside the numbered items (table).

Confirmed against the asm (9):
the area-dispel battle_units[99] overrun;
the _ITEMS[-1] read;
the uninitialised Melee_Unit_List[0];
the byte store that should be a word store;
Retreat_From_Combat's missing return value;
Draw_Bitmap_Animation_Block (#378);
Set_Font_Style's third colour set, which needs a DOSBox measurement;
two GUI checks, roads and mouse pages.
The other 31 aren't checked against the asm yet. They include the int8/uint16 sign questions (#126, mana_reserve), several suspected undefined reads, and a few stale notes.
The bq-base squash still waits on all of this.




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



// WZD s36p21
// RP_GUI_KeyInputOnly()
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\seg036\RP_GUI_KeyInputOnly.asm
[x] .\Rasm-Agy.cmd out\WIZARDS\seg036\RP_GUI_KeyInputOnly.asm --force

// UU_GUI_GetControlWidth                         seg036     000011C1 0000004D R F . . B T .
// UU_GUI_GetControlHeight                        seg036     0000120E 0000004E R F . . B T .
// UU_GUI_GetLastClickX                           seg036     0000125C 0000000A R F . . B T .
// UU_GUI_GetLastClickY                           seg036     00001266 0000000A R F . . B T .
// UU_GUI_GetLastClickBtns                        seg036     00001270 0000000A R F . . B T .
// Get_Mouse_Field                                seg036     0000127A 0000000A R F . . B T .
// RP_GUI_KeyInputOnly                            seg036     00001284 00000675 R F . . B T .
// Interpret_Keyboard_Input                       seg036     000018F9 000004AB R F . . B T .
// GUI_GetNextControl                             seg036     00001DC5 00000DAC R F . . B T .

// WZD s36p24
void UU_GUI_MouseEMUMoveTo(int16_t field_idx);

// WZD s36p30
int16_t Process_Direction_Key__STUB(int16_t dir_key);

// WZD s36p34
// GUI_CreateEditBox()


// WZD s26p02
// UU_VGA_SlowMosaicFlip()

// WZD s26p03
// RP_VGA_CutRight()
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\seg026\RP_VGA_CutRight.asm
[] .\Rasm-Agy.cmd out\WIZARDS\seg026\RP_VGA_CutRight.asm --force

// WZD s26p04
// UU_VGA_InterleaveOut()

// WZD s26p05
// UU_VGA_InterleaveIn()

// WZD s26p06
// UU_VGA_CrossSlideOut()

// WZD s26p07
// UU_VGA_CrossSlideIn()

// WZD s26p08
// UU_VGA_SplitCutOut()

// WZD s26p09
// UU_VGA_CopyLine()



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
