





screen_window_x1
screen_window_x2
screen_window_y1
screen_window_y2

uiobj_minx
uiobj_maxx
uiobj_miny
uiobj_maxy




MoO1  STARMAP.EXE  seg024
MoM   WIZARDS.EXE  seg014
MoO2  Module: graphics


MoM   WIZARDS.EXE  seg016


Fill()
...
Line()


¿ Dot() vs. Clipped_Dot() ?

Module: fields
    Clipped_Dot()
        Address: 01:0011C358
        signed integer (2 bytes) x
        signed integer (2 bytes) y
        signed integer (2 bytes) color
Module: graphics
    Dot()
        Address: 01:0012972D
        signed integer (2 bytes) x
        signed integer (2 bytes) y
        signed integer (2 bytes) color

Clipped_Dot()
    |-> Dot()

VGA_PutPixel()
Dot()
¿ ?
Clipped_Dot()

ui_draw_pixel()  AKA vgabuf_back_putpixel()
no XREFs?



Line()
ui_draw_line1()




MoO1  draw_filled_rect()
MoM   Fill()
MoO2  Fill()



C:\STU\developp\1oom\src\ui\classic\uidraw.c
// Fill()
void ui_draw_filled_rect(int x0, int y0, int x1, int y1, uint8_t color, int scale)



MoO1  seg024
uiobj_set_limits_all()
Reset_Window()


uiobj_set_limits()
Set_Window()



VGA_WndDrawLine()          |-> VGA_WndDrawLineBase()
VGA_WndDrawPtrnLine()      |-> VGA_WndDrawLineBase()
ui_draw_line_limit()       |-> ui_draw_line_limit_do()
ui_draw_line_limit_ctbl()  |-> ui_draw_line_limit_do()
¿ ~ Clipped_Line()         |-> Line() ?
Draw_Multi_Colored_Box_
Draw_Directional_Multi_Colored_Line_

Module: line
    Line()
    Clip_Line()
        Draw_Horizontal_Run()
        Draw_Vertical_Run()
    Multi_Colored_Line()
        Draw_Horizontal_Run_Multi()
        Draw_Vertical_Run_Multi()
    Anti_Alias_Line()
        Draw_Horizontal_Run_Alias()
        Draw_Vertical_Run_Alias()









// WZD s14p06
// drake178: VGA_WndDrawFilldRect()
// MoO2  Module: graphics  Fill()  just uses a clip flag
void Clipped_Fill(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int8_t color)
MoO1  draw_filled_rect_limit()


MoO1  draw_filled_rect_limit()
    sub_1BEEA()
    sub_1BEEA()

MoM  Clipped_Fill()
    DBG_DrawTableCell()
    DBG_DrawTableCell()
    Redraw_Map_Unexplored_Area()

MoO2
    Building_Outposts_In_Main_Screen_()
    Change_MP_Game_Name_()
    Choose_Multi_Network_Game_Screen_()
    Choose_Network_Plyrs_Screen_()
    Colony_Combat_Screen_()
    Colony_Landing_Screen_()
    Colony_Screen_()
    Colony_Summary_Screen_()
    Diplomacy_Mouse_Init_()
    Diplomacy_Mouse_Init_()
    Disp_PSTR()
    Display_Input_Field()
    Do_Main_Game_Popup_()
    Do_Mainmenu_Load_Screen_()
    Draw_Anti_Missile_Rocket_Picture_()
    Draw_Bar_Indicator_()
    Draw_Bar_Indicator__COLREFIT()
    Draw_Bar_Indicator___COLXPORT()
    Draw_Bar_Indicator___COLXPORT()
    Draw_Board_Popup_()
    Draw_Build_Queue_Popup_()
    Draw_Colony_Bombing_Screen_()
    Draw_Colony_Combat_Popup_()
    Draw_Colony_Combat_Screen_()
    Draw_Colony_Landing_Screen_()
    Draw_Colony_Screen_()
    Draw_Colony_Summary_Screen_()
    Draw_Damage_Bars_()
    Draw_Damage_Bars_()
    Draw_Fleet_Screen_()
    Draw_Galaxy_Map_Box_()
    Draw_Generic_Net_Info_Screen_()
    Draw_Generic_Net_Info_Screen_()
    Draw_Hall_Of_Fame_Screen_()
    Draw_Hi_Score_Screen_()
    Draw_Large_Fleet_Box_Frame_()
    Draw_Legal_Boxes_()
    Draw_Logos_()
    Draw_Logos_()
    Draw_Logos_()
    Draw_Main_Menu_Screen_()
    Draw_Main_Screen_Filled_()
    Draw_Missile_Status_()
    Draw_Naming_Popup_()
    Draw_Net_Next_Turn_Screen_()
    Draw_Newgame_Screen_()
    Draw_Officer_Screen_To_Back_()
    Draw_Planet_Summary_()
    Draw_Planet_Summary_Screen_()
    Draw_Planet_Summary_Screen_()
    Draw_Scroll_Bar_Indicator_In_Base_Colony_Palette_()
    Draw_SendGet_Net_Info_Screen_()
    Draw_Ship_Status_()
    Draw_Ship_Status_()
    Draw_Small_Fleet_Box_Frame_()
    Draw_Special_Info_Popup_()
    Draw_Visible_Fields()
    Draw_Visible_Fields()
    Draw_Visible_Fields()
    Draw_Wait_For_New_Joiners_Screen_()
    Draw_Wait_For_Race_Info_Screen_()
    Event_Fade_In_()
    Fade_In_Function_()
    Fade_To_Black_()
    Fade_To_Palette_()
    Fast_Fade_Out_()
    Full_Draw_Combat_Screen_()
    Gradient_Fill()
    Info_Screen_()
    Init_Hall_Of_Fame_Screen_()
    Init_New_Game_()
    Load_Game_()
    Logo_Fade_Out_()
    Main_Menu_Screen_()
    Main_Menu_Screen_()
    Main_Screen_Text_Box_()
    Mini_Main_Screen_Text_Box_()
    Next_Turn_()
    Planet_Colonization_In_Main_Screen_()
    Play_Cinematic_()
    Play_Cinematic_()
    Play_Cinematic_()
    Print_Colony_Outpost_Marker_()
    Print_Colony_Outpost_Marker_()
    Print_Colony_Outpost_Marker_()
    Print_Colony_Outpost_Marker_()
    Race_Report_Screen_()
    Race_Screen_()
    Race_Selection_Screen_()
    Redraw_Main_Design_Screen_To_Background_()
    Redraw_Main_Screen_()
    Reload_Main_Antaran_Room_Screen_()
    Reload_Main_Council_Screen_()
    Restore_Game_Popup_()
    Science_Room_()
    Scrap_Freighters_()
    Scroll_Draw_Combat_Screen_()
    Setup_Back_Page_()
    Show_Command_Points_Screen_()
    Unloading_Transports_In_Main_Screen_()
    _Tech_List_()
    _Tech_List_()
    _Tech_Review_Subscreen_()
    _Tech_Select_()
    _Tech_Select_()
    _Tech_Select_()
    zzzDraw_Visible_Fields_Back()
    zzzDraw_Visible_Fields_Back()
    zzzDraw_Visible_Fields_Back()
    zzzFill_Edited_String_Field()






seg014 UU_Interlaced_Fill() calls out to seg016 Line()
not actually a fill
draws 4 lines, a box, with top-lft color1 and bottom-right color2
    doesn't look like MoO2 Box() though


Module: graphics

void Box(int16_t x,int16_t y,int16_t width,int16_t height,int16_t thickness,int16_t color)



Dot()

Line()


Gradient_Fill()



// WZD s14p15
// drake178: VGA_RectangleFX()
// MoO2  Module: graphics  Gradient_Fill()

MoO2  void Gradient_Fill(int x1, int y1, int x2, int y2, int fill_type, int * color_array, int color_count, int ripple_count, int seed)

MoO1  fill_type { 1, 3, 7, 13,     15     }
MoM   fill_type { 1, 3, 7,     14, 15, 16 }
MoO2  fill_type { 1, 3,            15, 16 }

1: 
3: "darken"
7: 
13: 
14: 
15: 
16: Gray_Scale_Fill()

Tint_Fill()
Gray_Scale_Fill()
Fill()



@@fill_type_01:
    Fill(x1, y1, x2, y2, (unt8_t)color_array[0])

@@fill_type_03:
    Tint_Fill(x1, y1, x2, y2, 0)

@@fill_type_15:
    Tint_Fill(x1, y1, x2, y2, (unt8_t)color_array[0])

@@fill_type_16:
    Gray_Scale_Fill(x1, y1, x2, y2, color_array, color_count, 0)



_Darken_Fill_
    Address: 01:00090006
Darken_Fill()
    Address: 01:001298DE

Darken_Fill(int16_t x1, int16_t y1, int16_t x2, int16_t y2)

#define Darken_Fill(_x1_,_y1_,_x2_,_y2_) { Tint_Fill((_x1_), (_y1_), (_x2_), (_y2_), 0); }

void 
        Num params: 4
        Return type: void (1 bytes) 
        signed integer (2 bytes) 
        signed integer (2 bytes) 
        signed integer (2 bytes) 
        signed integer (2 bytes) 
        Locals:
            signed integer (2 bytes) x1
            signed integer (2 bytes) y1
            signed integer (2 bytes) x2
            signed integer (2 bytes) y2




...
    |-> Dot()









// Gradient_Fill()
// uiobj_handle_ta_sub1()





Init_Drivers                                         seg014     00000001 0000009D R F . . B T .
UU_Legacy_Startup                                    seg014     0000009E 0000005A R F . . B T .
Load_Font_File                                       seg014     000000F8 000000C8 R F . . B T .

Init_Drivers                              seg024  00000001 0000009D R F . . B T .
UU_Legacy_Startup                         seg024  0000009E 0000005A R F . . B . .
Load_Font_File                            seg024  000000F8 000000C3 R F . . B T .


Set_Window                                           seg014     000001C0 00000057 R F . . B T .
Reset_Window                                         seg014     00000217 0000001D R F . . B T .

Set_Window                                seg024  000001BB 00000057 R F . . B T .
Reset_Window                              seg024  00000212 0000001D R F . . B . .


Fill                                                 seg016     00000020 00000104 R F . . B T .
Dot                                                  seg016     00000124 0000003A R F . . B T .
Line                                                 seg016     0000015E 00000109 R F . . B T .

draw_filled_rect                          seg026  00000016 00000104 R F . . B T .
vgabuf_back_putpixel                      seg026  0000011A 0000003A R F . . B T .
ui_draw_line1                             seg026  00000154 00000109 R F . . B T .


Clipped_Fill                                         seg014     00000234 0000007C R F . . B T .
Clipped_Dot                                          seg014     000002B0 0000003E R F . . B T .
VGA_WndDrawLine                                      seg014     000002EE 00000028 R F . . B T .
VGA_WndDrawPtrnLine                                  seg014     00000316 00000029 R F . . B T .
VGA_WndDrawLineBase                                  seg014     0000033F 000002C6 R F . . B T .
UU_Interlaced_Fill                                   seg014     00000605 00000061 R F . . B T .
UU_VGA_DrawRect                                      seg014     00000666 00000078 R F . . B T .
UU_VGA_WndDrawRect                                   seg014     000006DE 00000074 R F . . B T .
UU_VGA_DrawDblRect                                   seg014     00000752 000000BB R F . . B T .
Gradient_Fill                                        seg014     0000080D 000000B7 R F . . B T .
DOS_PrintString                                      seg014     000008E7 00000011 R F . . B T .

VGA_DrawPatternLine                                  seg016     00000267 00000122 R F . . B T .
UU_VGA_CreateColorWave                               seg016     00000389 00000037 R F . . . T .
UU_VGA_DiagColumns                                   seg016     000003C0 0000010D R F . . B T .
RNG_Direct_LFSR                                      seg016     000004CD 00000038 R F . . . T .
Tint_Fill                                            seg016     00000506 000001FD R F . . B T .
Gray_Scale_Fill                                      seg016     00000703 0000022D R F . . B T .
UU_VGA_Columns                                       seg016     00000930 000000EA R F . . B T .
UU_VGA_ScrambleRect                                  seg016     00000A1A 00000174 R F . . B T .

Clipped_Fill                              seg024  0000022F 0000007C R F . . B T .
sub_1A56B                                 seg024  000002AB 0000003E R F . . B . .
ui_draw_line_limit                        seg024  000002E9 00000028 R F . . B T .
ui_draw_line_limit_ctbl                   seg024  00000311 00000029 R F . . B T .
ui_draw_line_limit_do                     seg024  0000033A 000002C6 R F . . B T .
ui_draw_box1                              seg024  00000600 00000061 R F . . B T .
sub_1A921                                 seg024  00000661 00000078 R F . . B . .
sub_1A999                                 seg024  000006D9 00000074 R F . . B . .
ui_draw_box2                              seg024  0000074D 000000BB R F . . B T .
Gradient_Fill                             seg024  00000808 00000085 R F . . B T .

ui_draw_line_ctbl                         seg026  0000025D 00000122 R F . . B T .
sub_1AF2F                                 seg026  0000037F 00000037 R F . . . . .
ui_draw_box_fill                          seg026  000003B6 0000010D R F . . B T .
rnd_bitfiddle                             seg026  000004C3 00000038 R F . . . . .
lbxgfx_apply_colortable                   seg026  000004FC 000001FD R F . . B . .
ui_draw_box_grain                         seg026  000006F9 00000174 R F . . B . .














Set_Font_Style_Shadow_Down                           seg017     0000000E 0000002D R F . . B T .
Set_Font_Style_Shadow_Up                             seg017     0000003B 0000002D R F . . B T .
Set_Font_Style_Shadow_Heavy                          seg017     00000068 0000002D R F . . B T .
Set_Font_Style_Outline                               seg017     00000095 0000002D R F . . B T .
Set_Font_Style_Outline_Heavy                         seg017     000000C2 0000002D R F . . B T .
Set_Outline_Color                                    seg017     000000EF 0000000B R F . . B T .
Set_Font_LF                                          seg017     000000FA 00000028 R F . . B T .
Set_Font_Spacing_Width                               seg017     00000122 00000018 R F . . B T .
Set_Font_Spacing                                     seg017     0000013A 00000018 R F . . B T .
Set_Font_Highlight_Colors                            seg017     00000152 0000002B R F . . B T .
Set_Font_Colors_15                                   seg017     0000017D 00000044 R F . . B T .
Set_Alias_Color                                      seg017     000001C1 00000077 R F . . B T .
Save_Alias_Colors                                    seg017     00000238 0000006E R F . . B T .
Restore_Alias_Colors                                 seg017     000002A6 0000007D R F . . B T .
Print_Far                                            seg017     00000323 0000002C R F . . B T .
Print_Centered_Far                                   seg017     0000034F 0000002C R F . . B T .
UU_VGA_DrawRtAlgFar                                  seg017     0000037B 0000002C R F . . B T .
Clipped_Print_Far                                    seg017     000003A7 0000002C R F . . B T .
UU_VGA_WndDrawCntrdFar                               seg017     000003D3 0000002C R F . . B T .
UU_VGA_WndDrawRtAlgFar                               seg017     000003FF 0000002C R F . . B T .
Print_Integer                                        seg017     0000042B 00000037 R F . . B T .
UU_Print_Long                                        seg017     00000462 0000003A R F . . B T .
Print_Right                                          seg017     0000049C 00000035 R F . . B T .
Print_Centered                                       seg017     000004D1 0000003B R F . . B . .
Print_Integer_Right                                  seg017     0000050C 00000036 R F . . B T .
Print_Integer_Centered                               seg017     00000542 00000036 R F . . B T .
UU_Print_Long_Right                                  seg017     00000578 00000039 R F . . B T .
VGA_WndDrawNumber                                    seg017     000005B1 00000037 R F . . B T .
UU_VGA_WndDrawLongN                                  seg017     000005E8 0000003A R F . . B T .
UU_VGA_WndDrawRtAligned                              seg017     00000622 00000035 R F . . B T .
Clipped_Print_Centered                               seg017     00000657 0000003B R F . . B T .
UU_VGA_WndDrawRtAlgNum                               seg017     00000692 00000036 R F . . B T .
UU_VGA_WndDrawRtAlgLong                              seg017     000006C8 00000039 R F . . B T .
Print_Full                                           seg017     00000701 00000022 R F . . B T .
Print                                                seg017     00000723 00000019 R F . . B T .
Print_Display                                        seg017     0000073C 000001F1 R F . . B T .
Print_String                                         seg017     0000092D 000001A3 R F . . B T .
UU_VGA_DisableAAPixels                               seg017     00000AF5 0000000B R F . . B T .
Set_Normal_Colors_On                                 seg017     00000B00 0000000E R F . . B T .
Set_Highlight_Colors_On                              seg017     00000B0E 00000027 R F . . B T .
Set_Special_Colors_On                                seg017     00000B35 00000027 R F . . B T .
Set_Color_Set                                        seg017     00000B5C 00000059 R F . . B T .
RP_DBG_TblDrawValue                                  seg017     00000BB5 0000009D R F . . B T .
UU_DBG_TblCellWrapper                                seg017     00000C52 00000024 R F . . B T .
UU_DBG_TblDrawString                                 seg017     00000C76 0000003A R F . . B T .
DBG_TblDrawS16                                       seg017     00000CB0 0000003D R F . . B T .
UU_DBG_TblDrawS32                                    seg017     00000CED 00000040 R F . . B T .
UU_DBG_TblDrawU32                                    seg017     00000D2D 0000004B R F . . B T .
DBG_DrawTableCell                                    seg017     00000D78 00000148 R F . . B T .
Clipped_Print                                        seg017     00000EC0 000001BA R F . . B T .
Clipped_Print_String                                 seg017     0000107A 000000F5 R F . . B T .
Clipped_Print_Character                              seg017     00001193 00000122 R F . . B T .
Get_Current_Font_Style                               seg017     000012B5 00000015 R F . . B T .
Get_Current_Normal_Color                             seg017     000012CA 00000015 R F . . B T .
Get_Current_Highlight_Color                          seg017     000012DF 00000015 R F . . B T .
Get_Current_Special_Color                            seg017     000012F4 00000015 R F . . B T .
UU_STR_CopyToNearLBX                                 seg017     00001309 0000001D R F . . B T .
VGA_GetVertSpacing                                   seg017     00001326 00000034 R F . . B T .
STR_TrimWhiteSpace                                   seg017     0000135A 0000005F R F . . B T .
Print_To_Bitmap                                      seg017     000013B9 0000001C R F . . B T .
Print_Display_To_Bitmap                              seg017     000013D5 0000020E R F . . B T .
Print_String_To_Bitmap                               seg017     000015E3 00000188 R F . . B . .
Print_Right_To_Bitmap                                seg017     0000178C 00000037 R F . . B T .
Print_Centered_To_Bitmap                             seg017     000017C3 0000003D R F . . B T .
Print_Full_To_Bitmap                                 seg017     00001800 00000024 R F . . B T .
Print_To_Bitmap_Far                                  seg017     00001824 0000002E R F . . B T .
UU_Print_Right_To_Bitmap_Far                         seg017     00001852 0000002E R F . . B T .
UU_Print_Centered_To_Bitmap_Far                      seg017     00001880 0000002E R F . . B T .
UU_Print_Full_To_Bitmap_Far                          seg017     000018AE 00000031 R F . . B T .
Set_Font                                             seg018     00000068 00000146 R F . . B T .
Print_Character                                      seg018     000001AE 00000074 R F . . B T .
Print_Character_No_Alias                             seg018     00000222 00000074 R F . . B . .
Print_Character_To_Bitmap                            seg018     00000296 000000AF R F . . B . .
Print_Character_ASM_                                 seg018     00000345 0000007E R F . . . T .
Print_Character_No_Alias_ASM_                        seg018     000003C3 00000087 R F . . . T .
Get_String_Width                                     seg018     0000044A 00000055 R F . . B T .
Print_Clipped_Character                              seg018     0000049F 00000054 R F . . B T .
Print_Clipped_Letter                                 seg018     000004F3 000000BE R F . . . T .
Print_Paragraph                                      seg019     00000001 0000018B R F . . B T .
UU_VGA_WndDrawText                                   seg019     00000195 00000100 R F . . B T .
UU_GUI_CreateHelpLinks                               seg019     00000295 0000030B R F . . B T .
Mark_Paragraph                                       seg019     000005A8 00000330 R F . . B T .
Get_Paragraph_Max_Height                             seg019     000008D8 0000005E R F . . B T .
Get_Paragraph_Max_Width                              seg019     00000936 00000125 R F . . B T .
Reset_Paragraph_Exclusions                           seg019     00000A64 0000000B R F . . B T .
Set_Paragraph_Exclusion_Area                         seg019     00000A6F 00000044 R F . . B T .
UU_Set_Paragraph_Exclusion_Picture                   seg019     00000AB3 00000074 R F . . B T .
Remove_Paragraph_Marks                               seg019     00000B27 00000026 R F . . B T .
Get_Font_Height                                      seg019     00000B4D 00000021 R F . . B T .
UU_VGA_WidestGlyph                                   seg019     00000B6E 0000004B R F . . B T .
LBX_DrawText                                         seg019     00000BB9 00000198 R F . . B T .
VGA_TextDraw_Init                                    seg019     00000D59 00000075 R F . . B T .
Load_Palette                                         seg020     0000000E 000000E2 R F . . B T .
Update_Remap_Gray_Palette                            seg020     000000F0 0000001D R F . . B T .
Calculate_Remap_Colors                               seg020     0000010D 0000007E R F . . B T .
Update_Remap_Color_Range                             seg020     0000018B 000000A0 R F . . B T .
Set_Palette_Changes                                  seg020     0000022B 00000023 R F . . B T .
Clear_Palette_Changes                                seg020     0000024E 00000022 R F . . B T .
UU_VGA_ClearDAC                                      seg020     00000270 00000035 R F . . B T .
UU_VGA_SaveDAC                                       seg020     000002A5 0000001F R F . . B T .
UU_VGA_RestoreDAC                                    seg020     000002C4 0000002C R F . . B T .
UU_VGA_LoadDAC                                       seg020     000002F0 00000048 R F . . B T .
Reset_Cycle_Palette_Color                            seg020     00000338 0000000B R F . . B T .
Cycle_Palette_Color                                  seg020     00000343 000001CC R F . . B T .
Update_Cycle                                         seg020     0000050F 00000094 R F . . B T .
UU_VGA_CreateHues                                    seg020     000005A3 000000B8 R F . . B T .
Apply_Palette                                        seg021     00000038 00000060 R F . . . T .
Cycle_Palette                                        seg021     00000098 000000E4 R F . . B T .
VGA_ShadeScreen                                      seg021     0000017C 00000136 R F . . B T .
VGA_SlideColors__STUB                                seg021     000002B2 000000D2 R F . . B T .
UU_VGA_ColorWave                                     seg021     00000384 00000095 R F . . B T .
Create_Remap_Palette                                 seg021     00000419 00000230 R F . . B T .
FLIC_Load_Palette                                    seg021     00000649 000000B5 R F . . B T .
UU_VGA_Bleep                                         seg022     0000000E 0000000C R F . . B T .
UU_KBD_GetKey                                        seg022     0000001A 00000009 R F . . B T .
UU_Clock_Wait30                                      seg022     00000023 00000014 R F . . B T .
VGA_FlushFrames                                      seg022     00000037 00000041 R F . . B T .
Set_Random_Seed                                      seg022     00000078 00000011 R F . . B T .
Get_Random_Seed                                      seg022     00000089 00000024 R F . . B T .
Randomize                                            seg022     000000AD 0000002B R F . . B T .
Random                                               seg022     000000D8 00000090 R F . . B T .
String_To_Upper                                      seg022     00000168 00000036 R F . . B T .
UU_STR_ToLower                                       seg022     0000019E 00000036 R F . . B T .
Copy_Memory_Near                                     seg022     000001D4 00000028 R F . . B T .
UU_MEM_CopyIfLess                                    seg022     000001FC 00000031 R F . . B T .
UU_MEM_Clear_Near                                    seg022     0000022D 00000027 R F . . B T .
UU_MEM_Set_Near                                      seg022     00000254 00000028 R F . . B T .
Clear_Structure                                      seg022     0000027C 00000076 R F . . B T .
Delete_Structure                                     seg022     000002F2 00000061 R F . . B T .
Copy_Memory_Less                                     seg022     00000353 0000004F R F . . B T .
MEM_Clear_Far                                        seg022     000003A2 00000037 R F . . B T .
Set_Memory_Far                                       seg022     000003D9 00000038 R F . . B T .
Swap_Short                                           seg022     00000411 00000022 R F . . B T .
MEM_SwapBytes                                        seg022     00000433 00000033 R F . . B T .
_fmemcpy                                             seg022     00000466 0000006E R F . . B T .
Test_Bit_Field                                       seg022     000004D4 0000002D R F . . B T .
Set_Bit_Field                                        seg022     00000501 0000002D R F . . B T .
UU_Clear_Bit_Field                                   seg022     0000052E 00000033 R F . . B T .
MEM_TestBit_Near                                     seg022     00000561 00000037 R F . . B T .
MEM_SetBit_Near                                      seg022     00000598 0000002A R F . . B T .
UU_MEM_ClearBit_Near                                 seg022     000005C2 0000002A R F . . B T .
UU_DBG_SetSelectSetting                              seg022     000005EC 0000001D R F . . B T .
UU_DBG_SelectDialog                                  seg022     00000609 00000258 R F . . B T .
RP_VGA_GrowOutFlip                                   seg022     00000861 00000188 R F . . B T .
RP_LBX_IMG_CpyDrawFrame                              seg022     000009E9 00000049 R F . . B T .
UU_VGA_GetPointInDir                                 seg023     00000002 000000FB R F . . B T .
TILE_GetDistance                                     seg023     000000FD 0000004B R F . . B T .
Delta_XY_With_Wrap                                   seg023     00000148 00000061 R F . . B T .
UU_Tile_PlotLine                                     seg023     000001A9 00000181 R F . . B T .
TILE_PlotMapLine                                     seg023     0000032A 0000023C R F . . B T .
DBG_Quit                                             seg024     00000006 00000015 R F . . B T .
DBG_ScreenDump                                       seg024     0000001B 00000403 R F . . B T .
VGA_GetDACBlock                                      seg024     0000041E 00000029 R F . . B T .
VGA_ReadScreenLine                                   seg024     00000447 0000005B R F . . B T .
Release_Version                                      seg024     000004A2 0000000B R F . . B T .
Check_Release_Version                                seg024     000004AD 0000000A R F . . B T .
Screen_Picture_Capture                               seg024     000004B7 0000005A R F . . B T .
Capture_Screen_Block                                 seg024     00000511 00000086 R F . . B T .

Draw_File_Animation_Frame                            seg025     0000000E 000001C7 R F . . B T .
VGA_MapNextEMMPage                                   seg025     000001D5 0000002F R . . . . T .
Remap_Draw_File_Animation_Frame__NOP                 seg025     00000204 000001E7 R F . . B T .

VGA_MosaicFlip__STUB                                 seg026     00000194 00000187 R F . . B T .
UU_VGA_SlowMosaicFlip                                seg026     0000031B 000001A8 R F . . B T .
RP_VGA_CutRight                                      seg026     000004C3 000000AF R F . . B T .
UU_VGA_InterleaveOut                                 seg026     00000572 00000093 R F . . B T .
UU_VGA_InterleaveIn                                  seg026     00000605 000000B2 R F . . B T .
UU_VGA_CrossSlideOut                                 seg026     000006B7 000000B3 R F . . B T .
UU_VGA_CrossSlideIn                                  seg026     0000076A 000000D6 R F . . B T .
UU_VGA_SplitCutOut                                   seg026     00000840 000000DA R F . . B T .
UU_VGA_CopyLine                                      seg026     0000091A 000001A7 R F . . B T .
GUI_PageFlipPrep                                     seg026     00000AC1 0000000D R F . . . T .
GUI_PageFlip                                         seg026     00000ACE 0000003F R F . . . T .
UU_VGA_ColWipeLeft                                   seg026     00000B0D 000000B0 R F . . B T .
UU_VGA_ColWipeRight                                  seg026     00000BBD 000000B9 R F . . B T .
VGA_ShiftCopyFromF3                                  seg026     00000C76 00000093 R F . . B T .

VGA_PartCopyFromF3                                   seg027     00000009 00000052 R F . . B T .
UU_VGA_SliceFlip                                     seg027     0000005B 00000053 R F . . B T .
UU_VGA_SquaresFlip                                   seg027     000000AE 00000058 R F . . B T .
UU_VGA_CopyRect                                      seg027     00000106 00000082 R F . . B T .
UU_VGA_InterleavedFlip                               seg027     00000188 00000035 R F . . B T .
UU_VGA_CrossSlideFlip                                seg027     000001BD 0000008B R F . . B T .
UU_VGA_WipeLeft                                      seg027     00000248 00000030 R F . . B T .
UU_VGA_WipeRight                                     seg027     00000278 00000030 R F . . B T .

Set_Page_On                                          seg028     00000008 00000012 R F . . . T .
Set_Page_Off                                         seg028     0000001A 00000016 R F . . . T .
Check_Default_Video_Page                             seg028     00000030 0000000F R F . . . T .
Page_Flip                                            seg028     0000003F 00000034 R F . . . T .
Copy_On_To_Off_Page                                  seg028     00000073 00000022 R F . . . T .
Copy_On_To_Off_YM                                    seg028     00000095 000000C6 R F . . B T .
Copy_On_To_Off_NM                                    seg028     0000015B 0000004F R F . . . T .
Copy_Off_To_Back                                     seg028     000001AA 00000043 R F . . . T .
Copy_Back_To_Off                                     seg028     000001ED 00000043 R F . . . T .
VGA_SaveDraw_Frame4                                  seg028     00000230 00000043 R F . . . T .
VGA_LoadDraw_Frame4                                  seg028     00000273 00000043 R F . . . T .
UU_VGA_CopyToDraw                                    seg028     000002B6 00000044 R F . . B T .
FLIC_Draw_Frame                                      seg029     0000000A 00000100 R F . . B T .
FLIC_Remap_Draw_Frame                                seg029     0000010A 000001E6 . F . . B T .
FLIC_Draw_Frame_EMM                                  seg029     000002FC 000001C3 . F . . B T .
VGA_MapNextEMMPages                                  seg029     000004BF 00000027 R . . . . T .
FLIC_Draw_Frame_EMM_Remap                            seg029     000004E6 000001E7 R F . . B T .
LBX_Image_Copy                                       seg030     0000000F 00000047 R F . . B T .
Create_Picture                                       seg030     00000056 00000084 R F . . B T .
LBX_IMG_VShiftRect                                   seg030     000000DA 00000203 R F . . B . .
LBX_IMG_VertWarp                                     seg030     000002DD 00000151 R F . . B T .
LBX_IMG_HorzWarp                                     seg030     0000042E 00000168 R F . . B T .
Create_Blank_Picture                                 seg030     00000596 00000085 R F . . B T .
UU_LBX_IMG_ColorShift                                seg030     0000061B 0000009E R F . . B T .
UU_LBX_IMG_CReplaceRect                              seg030     000006B9 000000A4 R F . . B T .
Replace_Color_All                                    seg030     0000075D 0000005F R F . . B T .
Load_Palette_From_Animation                          seg030     000007BC 00000037 R F . . B T .
FLIC_Draw                                            seg030     000007F3 000000EE R F . . B T .
Clipped_Draw                                         seg030     000008E1 000001BA R F . . B T .
Draw_Picture_To_Bitmap                               seg030     00000A9B 00000171 R F . . B T .
FLIC_Set_CurrentFrame                                seg030     00000C0C 00000066 R F . . B T .
FLIC_Reset_CurrentFrame                              seg030     00000C72 00000017 R F . . B T .
FLIC_Get_CurrentFrame                                seg030     00000C89 00000015 R F . . B T .
FLIC_Get_FrameCount                                  seg030     00000C9E 00000015 R F . . B T .
FLIC_Get_Width                                       seg030     00000CB3 00000014 R F . . B T .
FLIC_Get_Height                                      seg030     00000CC7 00000015 R F . . B T .
VGA_WndDrawRotateImg                                 seg030     00000CDC 00000081 R F . . B T .
VGA_WndDrawImageRect                                 seg030     00000D5D 0000109B R F . . B T .
UU_VGA_WndDrawTransform                              seg030     00001E18 0000013B R F . . B T .
VGA_RotateRect                                       seg030     00001F53 0000025F R F . . B T .
Draw_Picture                                         seg030     000021B2 00000039 R F . . B T .
Draw_Picture_Windowed                                seg030     000021EB 0000012D R F . . B T .
Clipped_Copy_Bitmap                                  seg030     00002318 00000168 R F . . B T .
LBX_IMG_Overlay                                      seg030     00002480 00000168 R F . . B T .
LBX_IMG_StripColors                                  seg030     000025E8 00000066 R F . . B T .
UU_LBX_IMG_CropRect                                  seg030     0000264E 00000128 R F . . B T .
Clear_Bitmap_Region                                  seg030     00002776 000000E6 R F . . B T .
UU_LBX_IMG_FullGScale                                seg030     0000285C 00000020 R F . . B T .
UU_LBX_IMG_ExtGScaleEC                               seg030     0000287C 00000033 R F . . B T .
UU_LBX_IMG_ExtGrayScale                              seg030     000028AF 00000118 R F . . B T .
Scale_Bitmap                                         seg030     000029C7 000000D9 R F . . B T .
Open_File_Animation                                  seg030     00002AA0 00000075 R F . . B T .
Draw_File_Animation                                  seg030     00002B15 0000016E R F . . B T .
Load_File_Animation_Frame                            seg030     00002C83 00000173 R F . . B T .
Get_File_Animation_Frame                             seg030     00002DF6 00000016 R F . . B T .
Set_File_Animation_Frame                             seg030     00002E0C 00000018 R F . . B T .
Get_Full_Store_Flag                                  seg030     00002E24 00000090 R F . . B T .
FLIC_Set_LoopFrame_1                                 seg030     00002EB4 00000018 R F . . B T .
Outline_Bitmap_Pixels                                seg030     00002ECC 000000DA R F . . B T .
Outline_Bitmap_Pixels_No_Glass                       seg030     00002FA6 000000E2 R F . . B T .
Bitmap_Aura_Pixels                                   seg030     00003088 0000006F R F . . B T .
UU_LBX_IMG_DrawRect                                  seg030     000030F7 00000151 R F . . B T .
Get_Bitmap_Actual_Size                               seg030     00003248 000000E1 R F . . B T .
Clipped_Draw_Frame                                   seg031     00000012 00000235 R F . . B T .

Clipped_Remap_Draw_Frame__NOP                        seg031     00000247 000002EA R F . . B T .

EMS_Clipped_Draw                                     seg032     00000010 000002F8 R F . . B T .
EMS_Clipped_Draw_Map4Pages                           seg032     00000308 0000002C R . . . . T .
EMS_Clipped_Remap_Draw                               seg032     00000334 000002F1 R F . . B T .

Create_Multi_Frame_Blank_Bitmap                      seg033     00000006 00000042 R F . . B T .
Add_Picture_To_Bitmap                                seg033     0000004A 000000A4 R F . . B T .
Add_Picture_To_Bitmap_EMM                            seg033     000000F4 00000134 R F . . B T .
EMM_MapNextIMGPages2                                 seg033     00000228 00000025 R . . . . T .
Draw_Picture_ASM                                     seg033     00000253 000000BC R F . . B T .
Remap_Draw_Picture_ASM                               seg033     0000030F 000000BB R . . . . T .
Color_Stream_Copy                                    seg033     000003CA 0000004B R F . . B T .
Flip_Bitmap                                          seg033     00000415 00000042 R F . . B T .
Replace_Color                                        seg033     00000457 00000037 R F . . B T .
UU_LBX_IMG_GetIntensity                              seg033     0000048E 00000076 R F . . B T .
LBX_IMG_PartialCopy                                  seg033     00000504 0000004B R F . . B T .
Clear_Memory_Far                                     seg033     0000054F 0000001C R F . . B . .
Reduce_Bitmap                                        seg033     00000571 000000AA R F . . B T .
Enlarge_Bitmap                                       seg033     0000061B 000000C8 R F . . B T .
Vanish_Bitmap                                        seg033     000007E5 00000070 R F . . B T .
Gray_Scale_Bitmap                                    seg033     00000855 00000082 R F . . B T .
LBX_IMG_RevGrayscale                                 seg033     000008D7 00000086 R F . . B T .
UU_DUP_RevGrayscale                                  seg033     0000095D 00000086 R F . . B T .
RNG_GFX_Random                                       seg033     000009E7 0000008E R F . . B T .
MATH_Inverse_Tangent                                 seg034     000001F5 00000059 R F . . B T .
MATH_Inverse_Tan_QI                                  seg034     0000024E 000000AD R . . . . T .
MATH_Int_Cos                                         seg034     000002FB 0000008E R F . . B T .
MATH_Int_Sin                                         seg034     00000389 0000008E R F . . B T .
VGA_DrawTexture                                      seg034     00000433 0000017C R F . . B T .
VGA_DrawTexture_R                                    seg034     000005AF 000001E5 R F . . B T .
Set_Mouse_List                                       seg035     0000006C 00000044 R F . . B . .
Check_Mouse_Shape                                    seg035     000000B0 0000005A R F . . B T .
Get_Pointer_Image_Number                             seg035     0000010A 0000000B R F . . . T .
Get_Pointer_Offset                                   seg035     00000115 0000000B R F . . . T .
Mouse_Button                                         seg035     00000120 00000026 R F . . . T .
Mouse_Wait                                           seg035     00000146 00000020 R F . . . T .
Init_Mouse_Driver                                    seg035     00000166 00000096 R F . . B T .
RP_MOUSE_SetUsable                                   seg035     000001FC 0000000E R F . . . T .
Reset_System_Mouse                                   seg035     0000020A 00000048 R F . . . T .
Mouse_Movement_Handler                               seg035     00000252 00000035 R F . . . T .
Mouse_Button_Handler                                 seg035     00000287 00000035 R F . . . T .
User_Mouse_Handler                                   seg035     000002BC 000000E0 R F . . B T .
Save_Mouse_State                                     seg035     0000039C 00000018 R F . . . T .
Restore_Mouse_State                                  seg035     000003B4 00000012 R F . . . T .
UU_MOUSE_Usable                                      seg035     000003C6 00000015 R F . . . T .
Keyboard_Status                                      seg035     000003DB 00000020 R F . . . T .
Read_Key                                             seg035     000003FB 0000025F R F . . . T .
Pointer_X                                            seg035     0000065A 0000000B R F . . . T .
Pointer_Y                                            seg035     00000665 0000000B R F . . . T .
Set_Pointer_Position                                 seg035     00000670 0000002B R F . . B T .
Set_Mouse_Position                                   seg035     0000069B 00000016 R F . . B T .
Mouse_Buffer_X                                       seg035     000006B1 0000000B R F . . . T .
Mouse_Buffer_Y                                       seg035     000006BC 0000000B R F . . . T .
Mouse_Buffer_Button                                  seg035     000006C7 0000000B R F . . . T .
Mouse_Buffer                                         seg035     000006D2 00000011 R F . . . T .
Mouse_Buffer2                                        seg035     000006E3 00000011 R F . . . T .
RP_MOUSE_SetSecClick                                 seg035     000006F4 0000001E R F . . B T .
UU_MOUSE_GenerateClick                               seg035     00000712 0000002A R F . . B T .
Check_Mouse_Buffer                                   seg035     0000073C 0000003A R F . . B T .
Save_Mouse_On_Page                                   seg035     00000776 000000D3 R F . . B T .
Save_Mouse_Off_Page                                  seg035     00000849 000000DE R F . . B T .
Save_Mouse_On_Page_                                  seg035     00000927 000000B7 R F . . B T .
Copy_Mouse_Off_To_Mouse_Back                         seg035     000009DE 0000001B R F . . . T .
Restore_Mouse_On_Page                                seg035     000009F9 00000090 R F . . B T .
Restore_Mouse_Off_Page                               seg035     00000A89 00000094 R F . . B T .
UU_VGA_RestoreToFrame3                               seg035     00000B1D 00000085 R F . . B T .
Draw_Mouse_On_Page                                   seg035     00000BA2 00000026 R F . . B T .
Draw_Mouse_Off_Page                                  seg035     00000BC8 00000026 R F . . B T .
Draw_Mouse_On_Page_                                  seg035     00000BEE 000000D1 R F . . B T .
Draw_Mouse_Off_Page_                                 seg035     00000CBF 000000D5 R F . . B T .
UU_GUI_ClearClickBuffer                              seg035     00000D94 0000001E R F . . . T .
UU_GUI_ClickBuffReset                                seg035     00000DB2 00000007 R F . . . T .
UU_GUI_SetClickBuffVar1                              seg035     00000DB9 00000007 R F . . . T .
UU_GUI_ClearBufferClick                              seg035     00000DC0 0000001E R F . . . T .
UU_GUI_FindEmptyClick                                seg035     00000DDE 00000033 R F . . . T .
UU_GUI_GetClickBuffVar1                              seg035     00000E11 00000004 R F . . . T .
UU_GUI_AddBufferClick                                seg035     00000E15 0000004E R F . . B T .
UU_GUI_ProcBufferClick                               seg035     00000E63 00000079 R F . . . T .

Interpret_Mouse_Input                                seg036     0000000C 00000F5F R F . . B T .
UU_GUI_SetTimeOutP                                   seg036     00000F90 0000001C R F . . B T .
UU_GUI_SetTimeOutN                                   seg036     00000FAC 0000001C R F . . B T .
UU_GUI_EditStart                                     seg036     00000FC8 00000010 R F . . B T .
UU_GUI_SetClickBuffer                                seg036     00000FD8 00000031 R F . . B T .
UU_GUI_LoadClickFile                                 seg036     00001009 00000054 R F . . B T .
UU_GUI_SaveClickFile                                 seg036     0000105D 00000047 R F . . B T .
Enable_Cancel                                        seg036     000010A4 0000000B R F . . B T .
UU_Disable_Cancel                                    seg036     000010AF 0000000B R F . . B T .
Disable_Mouse_Wait                                   seg036     000010BA 0000000B R F . . B T .
Enable_Mouse_Wait                                    seg036     000010C5 0000000B R F . . B T .
Set_Help_List                                        seg036     000010D0 00000017 R F . . B T .
Deactivate_Help_List                                 seg036     000010E7 00000017 R F . . B T .
Check_Help_List__STUB                                seg036     000010FE 000000C3 R F . . B T .
UU_GUI_GetControlWidth                               seg036     000011C1 0000004D R F . . B T .
UU_GUI_GetControlHeight                              seg036     0000120E 0000004E R F . . B T .
UU_GUI_GetLastClickX                                 seg036     0000125C 0000000A R F . . B T .
UU_GUI_GetLastClickY                                 seg036     00001266 0000000A R F . . B T .
UU_GUI_GetLastClickBtns                              seg036     00001270 0000000A R F . . B T .
Auto_Input                                           seg036     0000127A 0000000A R F . . B T .
RP_GUI_KeyInputOnly                                  seg036     00001284 00000675 R F . . B T .
Interpret_Keyboard_Input                             seg036     000018F9 000004AB R F . . B T .
GUI_GetNextControl                                   seg036     00001DC5 00000DAC R F . . B T .
UU_GUI_MouseEMUMoveTo                                seg036     00002B71 00000118 R F . . B T .
Scan_Field                                           seg036     00002C89 000000D0 R F . . B T .
Scan_Input                                           seg036     00002D59 00000172 R F . . B T .
UU_GUI_KeyWaitLoop                                   seg036     00002ECB 00000017 R F . . B T .
Reset_Wait_For_Input                                 seg036     00002EE2 0000003C R F . . B T .
Wait_For_Input                                       seg036     00002F1E 0000018B R F . . B T .
Process_Direction_Key__STUB                          seg036     000030A9 000002C8 R F . . B T .
GUI_EditBoxControl                                   seg036     0000339F 0000042B R F . . B T .
GUI_TextEditDialog                                   seg036     000037E1 000003C9 R F . . B T .
GUI_EditBoxRedraw                                    seg036     00003BC1 000003E7 R F . . B T .
Add_Input_Field                                      seg036     00003FA8 000001CF R F . . B T .
Add_Continuous_String_Input_Field                    seg036     00004177 00000163 R F . . B T .
UU_GUI_Ctrl_DecHeight                                seg036     000042DA 0000002D R F . . B T .
GUI_CreateDialogLine                                 seg036     00004307 00000210 R F . . B T .
Add_Multi_Hot_Key_Field                              seg036     00004517 00000150 R F . . B T .
Add_Picture_Field                                    seg036     00004667 00000147 R F . . B T .
UU_GUI_CreateWndImgLbl                               seg036     000047AE 000001D0 R F . . B T .
Set_Button_Down_Offsets                              seg036     0000497E 00000011 R F . . B T .
Add_Button_Info                                      seg036     0000498F 0000018A R F . . B T .
Add_Button_Field                                     seg036     00004B19 0000003F R F . . B T .
UU_GUI_CreateToggleBtn                               seg036     00004B58 00000054 R F . . B T .
UU_GUI_CreateLockblBtn                               seg036     00004BAC 00000054 R F . . B T .
UU_GUI_CreateMStateBtn                               seg036     00004C00 00000069 R F . . B T .
Add_Hidden_Field                                     seg036     00004C69 000000FB R F . . B T .
UU_Add_Clipped_Hidden_Field                          seg036     00004D64 00000193 R F . . B T .
Add_Hot_Key                                          seg036     00004EF7 000000F6 R F . . B T .
UU_GUI_CreateHelpLink                                seg036     00004FED 0000013A R F . . B T .
Add_Grid_Field                                       seg036     00005127 0000015E R F . . B T .
Add_Scroll_Field                                     seg036     00005285 00000031 R F . . B T .
Add_Scroll_Field_Far                                 seg036     000052B6 000001DD R F . . B T .
Find_Bar_Position                                    seg036     00005493 0000027D R F . . B T .
Increment_Scroll_Bar                                 seg036     00005710 00000132 R F . . B T .
Decrement_Scroll_Bar                                 seg036     00005842 00000131 R F . . B T .
Clear_Fields                                         seg036     00005973 00000023 R F . . B T .
UU_GUI_NoSelectColor                                 seg036     00005996 0000001D R F . . B T .
UU_GUI_ScrollableTxtDlg                              seg036     000059B3 00000AA4 R F . . B T .
GUI_CreateTextDialog                                 seg036     00006457 0000026C R F . . B T .
UU_GUI_NoMouseHighlight                              seg036     000066C3 0000000B R F . . B T .
GUI_MoveDlgHighlight__STUB                           seg036     000066CE 00000425 R F . . B T .
Clear_Fields_Above                                   seg036     00006AF3 00000012 R F . . B T .
Get_Field_Counter                                    seg036     00006B05 0000000A R F . . B T .
Init_Mouse_Keyboard                                  seg036     00006B0F 000000C9 R F . . B T .
Get_Input                                            seg036     00006BD8 0000004A R F . . B T .
Set_Input_Delay                                      seg036     00006C22 00000015 R F . . B T .
UU_Get_Input_Delay                                   seg036     00006C37 0000000A R F . . B T .
UU_GUI_ClearSelectable                               seg036     00006C41 0000002F R F . . B T .
UU_GUI_SetSelectable                                 seg036     00006C70 0000002F R F . . B T .
Draw_Fields                                          seg036     00006C9F 000014EB R F . . B T .
Draw_Field                                           seg036     000081A2 000010AC R F . . B T .
Push_Field_Down                                      seg036     00009268 000001DB R F . . B T .
UU_GUI_SetDlgTitleHelp                               seg036     00009443 0000000B R F . . B T .
UU_GUI_ClearTitleHelp                                seg036     0000944E 0000000B R F . . B T .
Assign_Auto_Function                                 seg036     00009459 00000030 R F . . B T .
Save_Auto_Function                                   seg036     00009489 00000013 R F . . B T .
Restore_Auto_Function                                seg036     0000949C 00000019 R F . . B T .
Deactivate_Auto_Function                             seg036     000094B5 0000000B R F . . B T .
UU_Activate_Auto_Function                            seg036     000094C0 0000000B R F . . B T .
Invoke_Auto_Function                                 seg036     000094CB 00000010 R F . . B T .
Quick_Call_Auto_Function                             seg036     000094DB 0000003A R F . . B T .
Call_Auto_Function                                   seg036     00009515 0000003A R F . . B T .
UU_GUI_ExtClear                                      seg036     0000954F 00000012 R F . . B T .
Toggle_Pages                                         seg036     00009561 0000004F R F . . B T .











start                                     seg000  00000000 00000163 R . . . . . .
sub_163                                   seg000  00000163 00000013 R F . . . . .
sub_176                                   seg000  00000176 00000028 R F . . . . .
exit_to_dos                               seg000  0000019E 00000009 R . . . B . .
store_int_handlers_hook_int0              seg000  000001B0 00000043 R . . . . . .
sub_1F3                                   seg000  000001F3 0000002D R F . . . . .
run_init_once_range                       seg000  00000220 00000044 R . . . . . .
sub_264                                   seg000  00000264 00000041 R . . . . . .
sub_2A5                                   seg000  000002A5 00000008 R . . . . . .
abs                                       seg000  000002C8 0000000E R F L . B . .
_atexit                                   seg000  000002D6 0000002D R F L . B T .
__DOSCMD                                  seg000  00000303 000000A9 R . L . B T .
__DOSENV                                  seg000  000003AC 000000FD R . L . B T .
__dos_getfileattr                         seg000  000004A9 0000001D R F L . B T .
__dos_setfileattr                         seg000  000004C6 0000001B R F L . B T .
__dos_findfirst                           seg000  000004E1 00000033 R F L . B T .
__dos_findnext                            seg000  00000514 0000002D R F L . B T .
__dos_getdrive                            seg000  00000541 00000013 R F L . B T .
__dos_setdrive                            seg000  00000554 00000017 R F L . B T .
nullsub_1                                 seg000  0000056B 00000001 R F L . . . .
sub_56C                                   seg000  0000056C 00000057 R . L . B . .
shutdown_and_exit_0_0                     seg000  000005C3 0000000F . F L . B . .
__exit                                    seg000  000005D2 00000012 . F L . B T .
__cexit                                   seg000  000005E4 0000000E R F L . . T .
__c_exit                                  seg000  000005F2 0000000C R F L . . T .
mul32_u                                   seg000  000005FE 00000017 R F . . . . .
memcpy_addr_addr_lencx                    seg000  00000615 0000001C R F . . B T .
N_LDIV@                                   seg000  00000631 00000003 R . L . . . .
div32_u                                   seg000  00000634 00000004 R F L . . . .
N_LUDIV@                                  seg000  00000638 00000008 R F L . . . .
N_LMOD@                                   seg000  00000640 00000003 R . L . . . .
mod32_u                                   seg000  00000643 0000009C R F L . B T .
near_shl_dx_ax_by_cl                      seg000  000006DF 00000003 R . . . . . .
shl_dx_ax_by_cl                           seg000  000006E2 0000001E R F . . . . .
N_LXURSH@                                 seg000  00000700 00000003 R . L . . . .
LXURSH@                                   seg000  00000703 0000001E R F L . . . .
N_PADD@                                   seg000  00000721 00000060 R F L . . . .
__IOERROR                                 seg000  00000781 00000039 R . L . B . .
__DOSERROR                                seg000  000007BA 00000012 R . L . B . .
sub_7CC                                   seg000  000007CC 00000012 R F . . B . .
sub_7DE                                   seg000  000007DE 00000013 R F . . B . .
sub_7F1                                   seg000  000007F1 00000012 R F . . B . .
sub_803                                   seg000  00000803 00000012 R F . . B . .
sub_815                                   seg000  00000815 00000012 R F . . B . .
sub_827                                   seg000  00000827 00000019 R F . . B . .
sub_840                                   seg000  00000840 00000012 R F . . B . .
sub_852                                   seg000  00000852 00000019 R F . . B . .
sub_86B                                   seg000  0000086B 00000012 R F . . B . .
sub_87D                                   seg000  0000087D 00000012 R F . . B . .
sub_88F                                   seg000  0000088F 00000012 R F . . B . .
sub_8A1                                   seg000  000008A1 00000012 R F . . B . .
sub_8B3                                   seg000  000008B3 00000040 R F . . B . .
sub_8F3                                   seg000  000008F3 00000028 R F . . B . .
sub_91B                                   seg000  0000091B 0000002A R F . . B . .
sub_945                                   seg000  00000945 00000010 R F . . B . .
sub_955                                   seg000  00000955 00000010 R F . . B . .
_isatty                                   seg000  00000965 00000011 R F L . B T .
__LONGTOA                                 seg000  00000976 0000007D R . L . B . .
__UTOA                                    seg000  000009F3 0000001D R . L . B . .
_lseek                                    seg000  00000A10 00000029 R F L . B T .
__MKNAME                                  seg000  00000A39 0000003F R . L . B T .
__TMPNAM                                  seg000  00000A78 00000043 R . L . B . .
N_LXMUL@                                  seg000  00000ABB 00000017 R . L . . . .
N_PCMP@                                   seg000  00000AD2 00000021 R . L . . . .
__read                                    seg000  00000AF3 0000002D R F L . B T .
sub_B20                                   seg000  00000B20 00000026 R . L . B . .
sub_B46                                   seg000  00000B46 00000019 R F L . B . .
sub_B5F                                   seg000  00000B5F 000000C1 R . L . B T .
sub_C20                                   seg000  00000C20 0000015E R . L . B T .
__SEARCHPATH                              seg000  00000D7E 00000014 R . L . B T .
__searchenv                               seg000  00000D92 00000033 R F L . B T .
__searchstr                               seg000  00000DC5 00000033 R F L . B T .
init_once_0                               seg000  00000DF8 000000A5 R . L . . . .
sub_E9D                                   seg000  00000E9D 0000002C R F . . B . .
_unlink                                   seg000  00000EC9 00000016 R F L . B T .
_stpcpy                                   seg000  00000EDF 00000029 R F L . B T .
___brk                                    seg000  00000F08 00000022 R . L . B . .
___sbrk                                   seg000  00000F2A 00000031 R . L . B . .
_brk                                      seg000  00000F5B 0000000C R F L . B T .
_sbrk                                     seg000  00000F67 00000010 R F L . B T .
_coreleft                                 seg000  00000F77 00000010 R F L . . T .
sub_10EE                                  seg000  000010EE 0000022F R . . . B . .
_execl                                    seg000  0000131D 0000001A R F L . B T .
sub_1343                                  seg000  00001343 00000063 R . L . . . .
sub_13A6                                  seg000  000013A6 00000071 R . L . . . .
sub_1417                                  seg000  00001417 00000029 R . L . . . .
sub_1440                                  seg000  00001440 00000037 R . L . . . .
_farfree                                  seg000  00001477 00000029 R F L . B T .
sub_14A0                                  seg000  000014A0 00000064 R . L . . . .
sub_1504                                  seg000  00001504 0000005A R . L . . . .
sub_155E                                  seg000  0000155E 00000023 R . L . . . .
_farmalloc                                seg000  00001581 0000007D R F L . B T .
sub_15FE                                  seg000  000015FE 0000007C R . L . . . .
sub_167A                                  seg000  0000167A 00000064 R . L . . T .
_farrealloc                               seg000  000016DE 0000007A R F L . B T .
sub_1758                                  seg000  00001758 00000076 R . L . B . .
__brk                                     seg000  000017CE 0000003F R . L . B . .
__sbrk                                    seg000  0000180D 0000008B R . L . B . .
_ioctl                                    seg000  00001898 00000027 R F L . B T .
__LoadProg                                seg000  000018BF 00000097 R . L . B . .
_free                                     seg000  00001956 0000001D R F L . . T .
sub_1973                                  seg000  00001973 0000003A R . L . . . .
sub_19AD                                  seg000  000019AD 00000039 R . L . . . .
sub_19E6                                  seg000  000019E6 0000001C R . L . . . .
sub_1A02                                  seg000  00001A02 00000023 R . L . . . .
_malloc                                   seg000  00001A25 00000060 R F L . . T .
sub_1A85                                  seg000  00001A85 00000040 R . L . . . .
sub_1AC5                                  seg000  00001AC5 00000029 R . L . . . .
sub_1AEE                                  seg000  00001AEE 00000019 R . L . . . .
sub_1B07                                  seg000  00001B07 00000035 R . L . B T .
sub_1B3C                                  seg000  00001B3C 00000042 R . L . . . .
_realloc                                  seg000  00001B7E 00000053 R F L . B T .
init_once_1                               seg000  00001BD3 00000093 R . L . . . .
sub_1C66                                  seg000  00001C66 00000028 R . L . . . .
_setblock                                 seg000  00001CEA 0000001C R F L . B T .
init_once_2                               seg000  00001D06 0000005D R . L . . . .
__chmod                                   seg000  00001D63 0000001B R F L . B T .
_close                                    seg000  00001D7E 00000028 R F L . B T .
__close                                   seg000  00001DA6 0000001E R F L . B T .
_fcloseall                                seg000  00001DC4 00000052 R F L . B T .
_eof                                      seg000  00001E16 00000077 R F L . B T .
fclose                                    seg000  00001E8D 00000081 R F L . B . .
_fflush                                   seg000  00001F0E 00000081 R F L . B T .
_flushall                                 seg000  00001F8F 0000003A R F L . B T .
sub_1FC9                                  seg000  00001FC9 000000BA R . L . B . .
__OPENFP                                  seg000  00002083 0000009C R . L . B T .
__GETFP                                   seg000  0000211F 0000002B R . L . . . .
fopen                                     seg000  0000214A 0000001F R F L . B . .
sub_2169                                  seg000  00002169 000000D7 R . L . B T .
fread                                     seg000  00002240 0000004B R F L . B T .
sub_228B                                  seg000  0000228B 0000005D R . L . B . .
_fseek                                    seg000  000022E8 00000068 R F L . B T .
_ftell                                    seg000  00002350 000000C7 R F L . B T .
fwrite                                    seg000  00002417 0000004C R F L . B T .
sub_2463                                  seg000  00002463 00000029 R . L . . . .
sub_248C                                  seg000  0000248C 00000059 R . L . B . .
__fgetc                                   seg000  000024E5 00000012 R F L . B T .
__Nfgetc                                  seg000  000024F7 00000003 R . L . . . .
_fgetc                                    seg000  000024FA 000000AF R F L . B T .
_fgetchar                                 seg000  000025A9 0000000A R F L . . T .
_getenv                                   seg000  000025B3 0000005F R F L . B T .
itoa                                      seg000  00002612 00000028 R F L . B . .
sub_263A                                  seg000  0000263A 0000001A R F L . B . .
num_32_2str                               seg000  00002654 00000025 R F L . B . .
_memcpy                                   seg000  00002679 0000001F R F L . B T .
sub_2698                                  seg000  00002698 00000019 R . L . B . .
sub_26B1                                  seg000  000026B1 00000012 R . L . B . .
_open                                     seg000  000026C3 00000158 R F L . B T .
__open                                    seg000  0000281B 0000004D R F L . B T .
__fputc                                   seg000  00002868 00000019 R F L . B T .
_fputc                                    seg000  00002881 00000136 R F L . B T .
_fputchar                                 seg000  000029B7 00000012 R F L . B T .
__FPUTN                                   seg000  000029C9 000001A8 R . L . B T .
___read                                   seg000  00002B71 000000CE R F L . B T .
_setvbuf                                  seg000  00002C3F 000000D8 R F L . B T .
sub_2D17                                  seg000  00002D17 0000003F R . L . B T .
sub_2D56                                  seg000  00002D56 00000047 R . L . B . .
__fnsplit                                 seg000  00002DAD 0000015C R F L . B T .
__splitpath                               seg000  00002F25 0000001B R F L . B T .
strcat                                    seg000  00002F40 00000039 R F L . B . .
strcpy                                    seg000  00002F79 00000022 R F L . B . .
strcasecmp                                seg000  00002F9B 00000040 R F L . B . .
strlen                                    seg000  00002FDB 0000001A R F L . B T .
_strncpy                                  seg000  00002FF5 0000002C R F L . B T .
_strupr                                   seg000  00003021 00000020 R F L . B T .
___write                                  seg000  00003041 0000010E R F L . B T .
__write                                   seg000  0000314F 0000003A R F L . B T .
__xfclose                                 seg000  00003189 00000028 R F L . . . .
__xfflush                                 seg000  000031B1 00000023 R F L . . . .
nullsub_2                                 seg000  00003208 00000001 R F L . . . .
main                                      seg001  00000009 00000706 R F . . B T .
ui_starmap_set_pos_focus                  seg001  0000079F 00000030 R F . . B . .
ui_starmap_set_pos                        seg001  000007CF 00000067 R F . . B T .
PageFlip_FX                               seg001  00000836 0000003D R F . . B . .
game_save                                 seg001  00000873 0000044A R F . . B T .
load_save_n_gam                           seg001  00000CBD 00000400 R F . . B T .
ui_palette_fadeout_a_f_1                  seg001  000010BD 00000038 R F . . B . .
Fade_In                                   seg001  000010F5 00000037 R F . . B . .
ui_palette_fadeout_4_3_1                  seg001  0000112C 00000038 R F . . B . .
game_get_random_shipnames                 seg001  00001164 00000128 R F . . B T .
game_get_random_unused_shipnames          seg001  0000128C 0000010B R F . . B T .
sound_play_24                             seg001  00001397 0000000F R F . . B . .
sound_play_06                             seg001  000013A6 0000000F R F . . B . .
init_stars_dist_matrix                    seg001  000013B5 000000B7 R F . . B . .
delay_1                                   seg001  0000146C 00000014 R F . . B . .
game_adjust_prod_by_special               seg002  00000000 00000059 R F . . B T .
game_update_production                    seg002  00000065 0000075B R F . . B . .
game_turn_reserve                         seg002  000007C8 000000EF R F . . B . .
game_update_maint_costs                   seg002  000008B7 0000028A R F . . B . .
game_get_pop_growth_max                   seg002  00000B41 000001B8 R F . . B T .
game_get_pop_growth_for_eco               seg002  00000CF9 000000A4 R F . . B T .
game_get_tech_prod                        seg002  00000D9D 00000064 R F . . B T .
game_adjust_slider_group_offs             seg002  00000E01 000000E6 R F . . B T .
game_adjust_slider_group_addr             seg002  00000EE7 0000010F R F . . B T .
game_update_tech_util                     seg002  00000FF6 00000935 R F . . B . .
game_tech_make_tbl_completed              seg002  0000192B 00000194 R F . . B T .
game_tech_is_completed                    seg002  00001ABF 00000019 R F . . B T .
find_best_tech_type                       seg002  00001AD8 00000040 R F . . B T .
game_misc_player_best_tech                seg002  00001B18 0000004A R F . . B T .
game_misc_player_has_tech                 seg002  00001B62 00000029 R F . . B T .
game_misc_player_has_tech_do              seg002  00001B8B 00000108 R F . . B T .
find_byte_in_tbl                          seg002  00001CA0 00000029 R F . . B T .
game_update_eco_on_waste                  seg002  00001CC9 000003F4 R F . . B T .
game_update_seen_by_orbit                 seg002  000020BD 000000E3 R F . . B . .
game_get_base_cost                        seg002  000021A0 00000104 R F . . B T .
game_turn_update_trade                    seg002  000022AC 0000022A R F . . B . .
game_turn_build_eco                       seg002  000024D6 00000AF3 R F . . B . .
game_turn_build_eco_sub1                  seg002  00002FC9 0000007A R F . . B T .
game_turn_build_ind                       seg002  00003043 0000045A R F . . B . .
game_add_planet_to_eco_finished           seg002  0000349D 00000047 R F . . B T .
game_update_total_research                seg002  000034E4 000000D7 R F . . B . .
game_tech_research                        seg002  000035BB 000002D1 R F . . B . .
game_planet_get_random2                   seg002  0000388C 0000005E R F . . B T .
game_turn_build_def                       seg002  000038EA 000003C6 R F . . B . .
game_turn_build_ship                      seg002  00003CB0 0000052F R F . . B . .
game_spy_build                            seg002  000041DF 00000108 R F . . B . .
game_spy_turn                             seg002  000042E7 00000473 R F . . B . .
game_spy_frame_random                     seg002  0000475A 00000065 R F . . B T .
game_planet_destroy                       seg002  000047BF 000003BA R F . . B T .
game_spy_esp_sub5                         seg002  00004B79 00000050 R F . . B . .
game_turn_process                         seg003  00000009 00000406 R F . . B . .
game_fleet_unrefuel                       seg003  0000040F 000001E7 R F . . B . .
ui_turn_msg                               seg003  000005F6 00000083 R F . . B . .
ui_turn_msg_draw_cb                       seg003  00000679 00000019 R F . . B . .
game_turn_update_seen                     seg003  00000692 000000F4 R F . . B . .
game_spy_report                           seg003  00000786 00000171 R F . . B . .
game_turn_init_hmm2                       seg003  000008F7 000000A4 R F . . B . .
game_turn_audiences                       seg003  0000099B 00000031 R F . . B . .
game_turn_claim                           seg003  000009CC 000000CA R F . . B . .
game_turn_move_ships                      seg003  00000A96 000005D4 R F . . B . .
ui_gmap_basic_draw_frame                  seg003  0000106A 000005C1 R F . . B . .
ui_gmap_set_countdown_neg1                seg003  0000162B 0000000B R F . . B . .
game_turn_show_newships                   seg003  00001636 00000129 R F . . B . .
ui_newships_draw_cb                       seg003  0000175F 0000023F R F . . B . .
game_turn_send_transport                  seg003  0000199E 0000026F R F . . B . .
game_autosave                             seg003  00001C0D 0000001C R F . . B . .
game_turn_check_end                       seg003  00001C29 00000260 R F . . B . .
game_turn_coup                            seg003  00001E89 00000233 R F . . B . .
game_turn_finished_slider                 seg003  000020BC 000006C7 R F . . B . .
game_turn_countdown_ceasefire             seg003  00002783 00000042 R F . . B . .
game_diplo_mood_relax                     seg003  000027C5 000000D4 R F . . B . .
game_spy_sab_human                        seg003  00002899 000000C7 R F . . B . .
game_spy_esp_human                        seg003  00002960 00000118 R F . . B . .
get_emperor_name_for_exeparam             seg003  00002A78 00000035 R F . . B T .
game_turn_unrest_hmm1                     seg003  00002AAD 00000101 R F . . B . .
game_turn_update_final_war                seg003  00002BAE 000000C3 R F . . B . .
game_turn_copy_within_frange              seg003  00002C71 0000001E R F . . B . .
game_turn_contact_broken                  seg003  00002C8F 00000163 R F . . B . .
copyprotection_lose                       seg003  00002DF2 00000112 R F . . B . .
game_ai_turn_p2                           seg003  00002F04 00000037 R F . . B . .
game_planet_get_w1                        seg003  00002F3B 000000F3 R F . . B T .
game_turn_limit_ships                     seg003  0000302E 000000C2 R F . . B . .
game_planet_update_home                   seg003  000030F0 00000079 R F . . B . .
game_turn_explore                         seg004  00000009 00000424 R F . . B . .
ui_explore                                seg004  0000042D 0000014F R F . . B . .
ui_explore_draw_cb                        seg004  0000057C 000001A5 R F . . B . .
ui_explore_load_data                      seg004  00000721 00000080 R F . . B . .
ui_explore_draw_planetinfo                seg004  000007A1 000002C8 R F . . B T .
game_turn_bomb                            seg005  00000003 0000032F R F . . B . .
ui_bomb_ask                               seg005  00000332 000000E0 R F . . B . .
ui_bomb_ask_draw_cb                       seg005  00000412 00000275 R F . . B . .
ui_bomb_show                              seg005  00000687 000000C7 R F . . B . .
ui_bomb_show_draw_cb                      seg005  0000074E 00000304 R F . . B . .
ui_bomb_load_data                         seg005  00000A6C 0000011E R F . . B . .
game_turn_bomb_damage                     seg005  00000B8A 00000596 R F . . B T .
game_turn_transport                       seg006  00000000 0000072F R F . . B . .
game_turn_ground                          seg006  0000072F 000003C0 R F . . B . .
game_event_new                            seg007  0000000F 00000824 R F . . B . .
game_event_run                            seg007  00000866 000022CA R F . . B . .
nop_ret_0                                 seg007  00002B30 00000009 R F . . B . .
game_battle_handle_all                    seg008  00000009 000004CF R F . . B . .
game_battle_prepare                       seg008  000004D8 000008C4 R F . . B T .
battle_ai_ai_resolve                      seg008  00000D9C 000000C3 R F . . B T .
game_battle_finish                        seg008  00000E5F 000001D1 R F . . B T .
game_battle_with_human                    seg008  00001030 00000376 R F . . B T .
ui_battle_draw_colony_destroyed_cb        seg008  000013A6 0000001E R F . . B . .
ui_prebattle_draw_cb                      seg008  000013C4 0000026B R F . . B . .
game_battle_post                          seg008  0000162F 000001D4 R F . . B T .
battle_ai_ai_resolve_do                   seg008  00001803 00000915 R F . . B T .
get_best_armor                            seg008  00002118 00000079 R F . . B T .
get_base_weapon_2                         seg008  000021AA 00000061 R F . . B T .
game_ai_turn_p3                           seg009  0000000B 00000AB5 R F . . B . .
game_ai_turn_p3_sub1                      seg009  00000AC0 0000017B R F . . B T .
ui_newtech                                seg010  0000000B 000004D6 R F . . B . .
ui_newtech_do                             seg010  000004EE 000003C6 R F . . B . .
game_tech_get_next_techs                  seg010  000008B4 00000143 R F . . B T .
ui_newtech_choose_next                    seg010  000009F7 0000027D R F . . B . .
ui_newtech_draw_cb1                       seg010  00000C74 00000308 R F . . B . .
ui_newtech_draw_frame                     seg010  00000F87 000001CC R F . . B . .
ui_newtech_choose_next_draw_cb            seg010  00001153 000001FE R F . . B . .
game_tech_setup_completed                 seg010  00001351 00000129 R F . . B T .
game_tech_ai_tech_next                    seg010  00001487 00000058 R F . . B T .
game_tech_get_field_percent               seg010  000014DF 0000007D R F . . B T .
game_tech_start_next                      seg010  0000155C 00000192 R F . . B T .
ui_newtech_adjust                         seg010  000016EE 000002F5 R F . . B . .
ui_newtech_draw_adjust_cb                 seg010  00001A01 00000344 R F . . B . .
game_planet_adjust_percent                seg010  00001D52 00000241 R F . . B T .
game_turn_update_hmm27c                   seg011  00000003 00000051 R F . . B . .
game_ai_turn_diplo_p2                     seg011  00000054 0000025C R F . . B . .
game_ai_classic_diplo_wage_war            seg011  000002B0 000003AC R F . . B T .
game_ai_diplo_wage_war_do                 seg011  0000065C 0000017D R F . . B T .
game_diplo_turn_p2_sub2                   seg011  000007D9 000000D7 R F . . B T .
Mark_Time                                 seg012  00000000 00000016 R F . . . . .
Time_Passed_                              seg012  00000016 00000010 R F . . . . .
Release_Time                              seg012  00000026 00000074 R F . . B T .
UU_CLK_ExtWait                            seg012  0000009A 00000057 R F . . B T .
Wait_For_Vsync_                           seg012  000000F1 00000010 R F . . . . .
sub_16CC5                                 seg013  00000005 0000005A R F . . B . .
sub_16D1F                                 seg013  0000005F 0000000F R F . . . . .
unlink                                    seg013  0000006E 00000016 R F . . B T .
get_file_size                             seg013  00000084 00000068 R F . . B . .
sub_16DAC                                 seg013  000000EC 0000004E R F . . B . .
sub_16DFA                                 seg013  0000013A 00000037 R F . . B . .
sub_16E31                                 seg013  00000171 0000002D R F . . B . .
find_file                                 seg014  0000000E 00000080 R F . . B . .
sub_16F00                                 seg015  00000000 00000137 R F . . B . .
Exit_With_Message                         seg015  00000137 0000002C R F . . B . .
run_orion_exe                             seg015  00000163 0000009B R F . . B T .
sub_170FE                                 seg015  000001FE 00000018 R F . . B . .
VGA_SetTextMode                           seg016  00000006 00000006 R F . . . . .
sub_1711C                                 seg016  0000000C 0000002D R F . . B . .
sub_17149                                 seg016  00000039 00000029 R F . . B . .
sub_17172                                 seg016  00000062 0000001E R F . . . . .
DBG_Open_ERROR_LOG                        seg016  00000080 00000030 R F . . . . .
DBG_Close_ERROR_LOG                       seg016  000000B0 00000032 R F . . . . .
sub_171F2                                 seg017  00000002 00000038 R F . . B . .
sub_1722A                                 seg017  0000003A 0000001F R F . . B . .
sub_17249                                 seg017  00000059 0000001D R F . . B . .
Check_Allocation                          seg017  00000076 00000027 R F . . B . .
farpeekb                                  seg017  0000009D 00000014 R F . . B . .
farpeekw                                  seg017  000000B1 00000012 R F . . B . .
farpeekdw                                 seg017  000000C3 00000019 R F . . B T .
farpokeb                                  seg017  000000DC 00000015 R F . . B T .
farpokew                                  seg017  000000F1 00000015 R F . . B T .
farpokedw__NIU                            seg017  00000106 00000019 R F . . B . .
Near_Allocate_Mark                        seg018  0000000F 0000000B R F . . B . .
sub_1731A                                 seg018  0000001A 0000000B R F . . B . .
sub_17325                                 seg018  00000025 00000037 R F . . B . .
sub_1735C                                 seg018  0000005C 0000003C R F . . B . .
Near_Allocation_Error                     seg018  00000098 0000004A R F . . B T .
heap_alloc_paragraphs                     seg018  000000E2 0000009F R F . . B T .
heap_alloc_basic_paragraphs               seg018  00000181 0000004A R F . . B . .
sub_174CB                                 seg018  000001CB 0000004A R F . . B . .
sub_17515                                 seg018  00000215 0000002E R F . . B . .
sub_17543                                 seg018  00000243 0000002E R F . . B . .
sub_17571                                 seg018  00000271 000000C9 R F . . B T .
sub_1763A                                 seg018  0000033A 000000D1 R F . . B T .
sub_1770B                                 seg018  0000040B 00000033 R F . . B . .
set_dx_arg_ax_0                           seg018  0000043E 00000023 R F . . B . .
sub_17761                                 seg018  00000461 0000002F R F . . B . .
sub_17790                                 seg018  00000490 00000021 R F . . B . .
Allocation_Error                          seg018  000004B1 000000F7 R F . . B . .
open_file                                 seg019  00000008 00000020 R F . . B . .
sub_178C8                                 seg019  00000028 00000014 R F . . B . .
fseek_from_start                          seg019  0000003C 00000029 R F . . B . .
sub_17905                                 seg019  00000065 00000029 R F . . B . .
read_to_seg_start                         seg019  0000008E 0000002D R F . . B . .
sub_1795B                                 seg019  000000BB 00000028 R F . . B . .
strcpy_optseg                             seg019  000000E3 00000039 R F . . B T .
lbx_load_item_opt000                      seg020  0000000C 0000001C R F . . B T .
lbx_load_item_optx10                      seg020  00000028 0000001D R F . . B T .
lbx_load_item_optx20                      seg020  00000045 0000001D R F . . B T .
lbxfile_load_table_opt00                  seg020  00000062 00000022 R F . . B T .
sub_17A34                                 seg020  00000084 00000023 R F . . B . .
sub_17A57                                 seg020  000000A7 00000023 R F . . B . .
sub_17A7A                                 seg020  000000CA 0000001D R F . . B . .
sub_17A97                                 seg020  000000E7 0000001E R F . . B . .
sub_17AB5                                 seg020  00000105 0000001E R F . . B . .
lbx_load_do                               seg020  00000123 000003CF R F . . B T .
sub_17EA2                                 seg020  000004F2 00000468 R F . . B T .
lbxtbl_get                                seg020  0000095A 000002E7 R F . . B T .
sub_185F1                                 seg020  00000C41 000001D2 R F . . B . .
error_with_file                           seg020  00000E13 00000170 R F . . B . .
fname_ucase_and_remove_ext                seg020  00000FA4 0000003D R F . . B . .
fopen_0                                   seg021  00000001 00000077 R F . . B . .
fread_sz_n                                seg021  00000078 0000002F R F . . B . .
fwrite_sz_n                               seg021  000000A7 0000002F R F . . B . .
sub_18A66                                 seg021  000000D6 00000016 R F . . B . .
fclose_0                                  seg021  000000EC 0000000D R F . . B . .
sub_18A8C                                 seg022  0000000C 00000058 R F . . . . .
sub_18AE4                                 seg022  00000064 0000001C R F . . . . .
emm_get_num_of_unalloc_pages              seg022  00000080 0000001C R F . . . . .
sub_18B1C                                 seg022  0000009C 00000024 R F . . B . .
sub_18B40                                 seg022  000000C0 0000003C R F . . B . .
sub_18B7C                                 seg022  000000FC 00000036 R F . . B . .
sub_18BB2                                 seg022  00000132 0000001A R F . . B . .
emm_copy_from_handle                      seg022  0000014C 0000007F R F . . B T .
emm_copy_to_handle                        seg022  000001CB 0000007F R F . . B T .
emm_get_stored_page_frame_segment         seg022  0000024A 0000000B R F . . . . .
emm_map_phys_pages                        seg022  00000255 0000003B R F . . B . .
EMM_Startup                               seg023  00000000 00000174 R F . . B . .
sub_18E84                                 seg023  00000174 00000012 R F . . B . .
emm_load_file_flag1                       seg023  00000186 00000013 R F . . B T .
emm_load_file_do                          seg023  00000199 00000214 R F . . B T .
sub_190BD                                 seg023  000003AD 00000019 R F . . B . .
sub_190D6                                 seg023  000003C6 0000001A R F . . B . .
sub_190F0                                 seg023  000003E0 000001E3 R F . . B . .
sub_192D3                                 seg023  000005C3 00000025 R F . . B . .
sub_192F8                                 seg023  000005E8 00000026 R F . . B . .
sub_1931E                                 seg023  0000060E 00000227 R F . . B . .
sub_19545                                 seg023  00000835 00000295 R F . . B T .
sub_197DA                                 seg023  00000ACA 00000201 R F . . B T .
sub_199DB                                 seg023  00000CCB 000002C2 R F . . B T .
sub_19C9D                                 seg023  00000F8D 000001A9 R F . . B . .
sub_19E46                                 seg023  00001136 00000055 R F . . B . .
sub_19E9B                                 seg023  0000118B 00000025 R F . . B . .
sub_19EC0                                 seg023  000011B0 00000035 R F . . B . .
sub_19EF5                                 seg023  000011E5 000000EB R F . . B . .
sub_19FE0                                 seg023  000012D0 0000000A R F . . B . .
emm_alloc_pages_for_name_get_handle       seg023  000012DA 0000023D R F . . B . .
vgafileh_copy_to_end_16k                  seg023  00001517 00000022 R F . . B T .
vgafileh_copy_from_end_16k                seg023  00001539 00000022 R F . . B T .
sub_1A26B                                 seg023  0000155B 0000002B R F . . B T .
sub_1A296                                 seg023  00001586 0000002B R F . . B T .



lbxfont_select_set_12_1                   seg027  0000000D 0000002D R F . . B . .
sub_1B44A                                 seg027  0000003A 0000002D R F . . B . .
sub_1B477                                 seg027  00000067 0000002D R F . . B . .
lbxfont_select_set_12_4                   seg027  00000094 0000002D R F . . B . .
lbxfont_select_set_12_5                   seg027  000000C1 0000002D R F . . B . .
lbxfont_set_temp_color                    seg027  000000EE 0000000B R F . . B . .
lbxfont_set_gap_h                         seg027  000000F9 00000028 R F . . B . .
lbxfont_set_space_w                       seg027  00000121 00000018 R F . . B T .
sub_1B549                                 seg027  00000139 00000018 R F . . B . .
lbxfont_set_colors                        seg027  00000151 0000002D R F . . B T .
sub_1B58E                                 seg027  0000017E 00000046 R F . . B . .
lbxfont_print_str_normal_addr             seg027  000001C4 0000002C R F . . B T .
lbxfont_print_str_center_addr             seg027  000001F0 0000002C R F . . B T .
sub_1B62C                                 seg027  0000021C 0000002C R F . . B . .
sub_1B658                                 seg027  00000248 0000002C R F . . B . .
lbxfont_print_str_center_limit_unconst    seg027  00000274 0000002C R F . . B T .
sub_1B6B0                                 seg027  000002A0 0000002C R F . . B . .
lbxfont_print_num_normal                  seg027  000002CC 00000037 R F . . B . .
sub_1B713                                 seg027  00000303 0000003A R F . . B . .
lbxfont_print_str_right                   seg027  0000033D 00000035 R F . . B T .
lbxfont_print_str_center                  seg027  00000372 0000003B R F . . B T .
lbxfont_print_num_right                   seg027  000003AD 00000036 R F . . B T .
lbxfont_print_num32_right                 seg027  000003E3 00000039 R F . . B T .
sub_1B82C                                 seg027  0000041C 00000037 R F . . B . .
sub_1B863                                 seg027  00000453 0000003A R F . . B . .
sub_1B89D                                 seg027  0000048D 00000035 R F . . B T .
lbxfont_print_str_center_limit            seg027  000004C2 0000003B R F . . B T .
sub_1B90D                                 seg027  000004FD 00000036 R F . . B . .
sub_1B943                                 seg027  00000533 00000039 R F . . B . .
lbxfont_print_str_normal_do_w0            seg027  0000056C 00000022 R F . . B . .
lbxfont_print_str_normal                  seg027  0000058E 00000019 R F . . B . .
lbxfont_print_str_normal_do               seg027  000005A7 000001E5 R F . . B . .
lbxfont_print_str_do                      seg027  0000078C 0000016B R F . . B . .
lbxfont_select_subcolors_0                seg027  0000091B 0000000E R F . . B . .
lbxfont_select_subcolors_13not1           seg027  00000929 00000027 R F . . B . .
lbxfont_select_subcolors_13not2           seg027  00000950 00000027 R F . . B . .
lbxfont_select_subcolors                  seg027  00000977 00000059 R F . . B T .
sub_1BDE0                                 seg027  000009D0 00000056 R F . . B . .
ui_draw_text_overlay                      seg027  00000A26 00000016 R F . . B T .
sub_1BE4C                                 seg027  00000A3C 0000002F R F . . B T .
sub_1BE7B                                 seg027  00000A6B 00000032 R F . . B . .
sub_1BEAD                                 seg027  00000A9D 0000003D R F . . B . .
sub_1BEEA                                 seg027  00000ADA 00000148 R F . . B T .
lbxfont_print_str_normal_limit            seg027  00000C22 000001BA R F . . B T .
lbxfont_print_str_limit_do                seg027  00000DDC 000000F5 R F . . B T .
lbxfont_print_char_ret_x_limit            seg027  00000EF6 00000122 R F . . B T .
lbxfont_get_current_fontnum               seg027  00001018 00000015 R F . . B . .
lbxfont_get_current_fonta2                seg027  0000102D 00000015 R F . . B . .
lbxfont_get_current_fonta2b               seg027  00001042 00000015 R F . . B . .
lbxfont_get_current_fonta4                seg027  00001057 00000015 R F . . B . .
sub_1C47C                                 seg027  0000106C 0000001D R F . . B . .
lbxfont_get_gap_h                         seg027  00001089 00000034 R F . . B . .
trim_whitespace                           seg027  000010BD 0000005F R F . . B . .
Print_To_Bitmap                           seg027  0000111C 0000001C R F . . B T .
Print_Display_To_Bitmap                   seg027  00001138 0000020E R F . . B T .
Print_String_To_Bitmap                    seg027  00001346 0000016A R F . . B T .
sub_1C8E5                                 seg027  000014D5 00000037 R F . . B T .
sub_1C91C                                 seg027  0000150C 0000003D R F . . B T .
sub_1C959                                 seg027  00001549 00000024 R F . . B T .

lbxfont_select                            seg028  00000076 00000146 R F . . B . .
lbxfont_print_char_ret_x                  seg028  000001BC 00000074 R F . . B T .
Print_Character_To_Bitmap                 seg028  00000230 000000AF R F . . B T .
sub_1CC4F                                 seg028  000002DF 0000007E R F . . . . .
lbxfont_calc_str_width                    seg028  0000035D 00000055 R F . . B . .
lbxfont_plotchar_limit                    seg028  000003B2 00000054 R F . . B T .
lbxfont_plotchar_limit_do                 seg028  00000406 000000BE R F . . . . .

lbxfont_print_str_split                   seg029  00000004 0000018B R F . . B T .
sub_1CFC8                                 seg029  00000198 00000100 R F . . B . .
UU_GUI_CreateHelpLinks                    seg029  00000298 0000030B R F . . B . .
lbxfont_split_str                         seg029  000005AB 000003B1 R F . . B T .
lbxfont_calc_split_str_h                  seg029  0000095C 00000053 R F . . B T .
sub_1D7DF                                 seg029  000009AF 00000125 R F . . B . .
sub_1D90D                                 seg029  00000ADD 0000000B R F . . B . .
sub_1D918                                 seg029  00000AE8 00000044 R F . . B . .
sub_1D95C                                 seg029  00000B2C 00000074 R F . . B . .
lbxfont_fix_15_to_20                      seg029  00000BA0 00000026 R F . . B . .
lbxfont_get_height                        seg029  00000BC6 00000021 R F . . B . .
sub_1DA17                                 seg029  00000BE7 0000004B R F . . B . .
sub_1DA62                                 seg029  00000C32 000001D1 R F . . B . .

lbxpal_select                             seg030  0000000C 000000C6 R F . . B T .
lbxpal_build_colortables                  seg030  000000D2 0000007E R F . . B . .
Set_Palette_Changes                       seg030  00000150 00000023 R F . . B T .
sub_1DDA3                                 seg030  00000173 00000022 R F . . B . .
sub_1DDC5                                 seg030  00000195 00000035 R F . . B . .
UU_VGA_SaveDAC                            seg030  000001CA 0000001F R F . . B . .
UU_VGA_RestoreDAC                         seg030  000001E9 0000002C R F . . B . .
sub_1DE45                                 seg030  00000215 00000048 R F . . B . .

Apply_Palette                             seg031  0000003A 00000060 R F . . . . .
Cycle_Palette                             seg031  0000009A 000000E4 R F . . B . .
sub_1DFFE                                 seg031  0000017E 00000136 R F . . B . .
sub_1E134                                 seg031  000002B4 000000C3 R F . . B . .
sub_1E1F7                                 seg031  00000377 00000086 R F . . B . .
lbxpal_build_colortable                   seg031  000003FD 00000211 R F . . B T .
lbxgfx_apply_palette_do                   seg031  0000060E 000000D0 R F . . B . .

sub_1E55E                                 seg032  0000000E 0000000C R F . . B . .
sub_1E56A                                 seg032  0000001A 00000009 R F . . B . .
sub_1E573                                 seg032  00000023 00000014 R F . . B . .
sub_1E587                                 seg032  00000037 00000041 R F . . B . .
sub_1E5C8                                 seg032  00000078 00000011 R F . . B . .
sub_1E5D9                                 seg032  00000089 00000029 R F . . B . .
Randomize                                 seg032  000000B2 0000002B R F . . B . .
Random                                    seg032  000000DD 00000090 R F . . B T .
strtoupper                                seg032  0000016D 00000036 R F . . B . .
strtolower                                seg032  000001A3 00000036 R F . . B T .
memcpy                                    seg032  000001D9 00000028 R F . . B T .
sub_1E751                                 seg032  00000201 00000031 R F . . B . .
memclear                                  seg032  00000232 00000027 R F . . B T .
sub_1E7A9                                 seg032  00000259 00000028 R F . . B . .
util_table_remove_item_keep_order_zero    seg032  00000281 00000076 R F . . B T .
util_table_remove_item_keep_order         seg032  000002F7 00000061 R F . . B T .
sub_1E8A8                                 seg032  00000358 0000004F R F . . B . .
sub_1E8F7                                 seg032  000003A7 00000037 R F . . B . .
sub_1E92E                                 seg032  000003DE 00000038 R F . . B . .
swap_words_at_offsets                     seg032  00000416 00000022 R F . . B . .
sub_1E988                                 seg032  00000438 00000033 R F . . B . .
memcpy_optsegoffs                         seg032  0000046B 00000046 R F . . B T .
boolvec_get                               seg032  000004B1 00000037 R F . . B T .
boolvec_set1                              seg032  000004E8 0000002A R F . . B T .
sub_1EA62                                 seg032  00000512 0000002A R F . . B . .
sub_1EA8C                                 seg032  0000053C 0000001D R F . . B . .
sub_1EAA9                                 seg032  00000559 00000258 R F . . B . .
ui_battle_transition_to                   seg032  000007B1 00000188 R F . . B T .
hw_video_copy_buf_out                     seg032  00000939 00000049 R F . . B T .

util_math_go_line_dist                    seg033  00000002 000000FB R F . . B T .
util_math_dist_fast                       seg033  000000FD 0000004B R F . . B T .
util_math_line_plot                       seg033  000001A9 00000181 R F . . B T .

cond_exit_with_memstats                   seg034  00000007 00000015 R F . . B . .
make_scrdmp                               seg034  0000001C 00000403 R F . . B . .
sub_1F7EF                                 seg034  0000041F 00000029 R F . . B . .
sub_1F818                                 seg034  00000448 0000005B R F . . B . .
Release_Version                           seg034  000004A3 0000000B R F . . B . .

ui_news_fade                              seg035  00000194 00000187 R F . . B . .
sub_1FC3A                                 seg035  000003CA 00000093 R F . . B . .
sub_1FCCD                                 seg035  0000045D 000000B2 R F . . B . .
sub_1FD7F                                 seg035  0000050F 000000B3 R F . . B . .
sub_1FE32                                 seg035  000005C2 000000D6 R F . . B . .
ui_draw_copy_line                         seg035  00000772 000001A7 R F . . B . .
sub_20189                                 seg035  00000919 0000000D R F . . . . .
sub_20196                                 seg035  00000926 0000003F R F . . . . .
sub_201D5                                 seg035  00000965 000000B0 R F . . B . .
sub_20285                                 seg035  00000A15 000000B9 R F . . B . .
sub_2033E                                 seg035  00000ACE 00000093 R F . . B . .

sub_203D1                                 seg036  00000001 00000063 R F . . B . .
sub_20434                                 seg036  00000064 00000053 R F . . B . .
ui_draw_finish_wipe_anim                  seg036  000000B7 00000058 R F . . B . .
ui_draw_finish_wipe_anim_do               seg036  0000010F 00000082 R F . . B T .

Set_Page_On                               seg037  00000002 00000012 R F . . . . .
Set_Page_Off                              seg037  00000014 00000016 R F . . . . .
Check_Default_Video_Page                  seg037  0000002A 0000000F R F . . . . .
Page_Flip                                 seg037  00000039 00000034 R F . . . . .
Copy_On_To_Off_Page                       seg037  0000006D 00000022 R F . . . . .
Copy_On_To_Off_YM                         seg037  0000008F 000000C6 R F . . B . .
Copy_On_To_Off_NM                         seg037  00000155 0000004F R F . . . . .
vgabuf_copy_back_to_page2                 seg037  000001A4 00000043 R F . . . . .
vgabuf_copy_page2_to_back                 seg037  000001E7 00000043 R F . . . . .
vgabuf_copy_back_to_page3                 seg037  0000022A 00000043 R F . . . . .
vgabuf_copy_page3_to_back                 seg037  0000026D 00000043 R F . . . . .

lbxgfx_draw_pixels_fmt0                   seg038  00000000 00000100 R F . . B T .
lbxgfx_draw_pixels_fmt1                   seg038  00000100 000001E6 R F . . B . .
sub_20C22                                 seg038  000002F2 000001C6 R F . . B . .
sub_20DE8                                 seg038  000004B8 0000002F R . . . . . .
sub_20E17                                 seg038  000004E7 000001E7 R F . . B . .

gfx_aux_copy                              seg039  00000000 00000047 R F . . B T .
gfx_aux_setup_wh                          seg039  00000047 00000084 R F . . B T .
gfx_aux_color_non0                        seg039  00000292 0000005F R F . . B T .
lbxgfx_apply_palette                      seg039  000002F1 00000037 R F . . B T .
lbxgfx_draw_frame                         seg039  00000328 000000EE R F . . B T .
lbxgfx_draw_frame_offs                    seg039  00000416 000001BA R F . . B T .
gfx_aux_draw_frame_to                     seg039  000005D0 00000171 R F . . B T .
lbxgfx_set_new_frame                      seg039  00000741 00000066 R F . . B T .
lbxgfx_set_frame_0                        seg039  000007A7 00000017 R F . . B T .
lbxgfx_get_frame                          seg039  000007BE 00000015 R F . . B . .
lbxgfx_get_w                              seg039  000007E8 00000014 R F . . B . .
lbxgfx_get_h                              seg039  000007FC 00000015 R F . . B . .
gfx_aux_draw_frame_from_rotate_do         seg039  00000892 0000109B R F . . B T .
gfx_aux_draw_frame_from_rotate            seg039  0000194D 0000013B R F . . B T .
sub_22A88                                 seg039  00001A88 0000025F R F . . B T .
gfx_aux_draw_frame_from                   seg039  00001CE7 00000039 R F . . B T .
gfx_aux_draw_frame_from_limit             seg039  00001D20 0000012D R F . . B T .
gfx_aux_overlay                           seg039  00001E4D 00000168 R F . . B T .
gfx_aux_overlay_clear_unused              seg039  00001FB5 00000168 R F . . B T .
gfx_aux_recolor_ctbl                      seg039  00002245 00000020 R F . . B T .
gfx_aux_recolor_do                        seg039  00002298 00000118 R F . . B T .
gfx_aux_scale                             seg039  000023B0 000000D9 R F . . B T .
sub_23489                                 seg039  00002489 00000075 R F . . B T .
sub_234FE                                 seg039  000024FE 0000016E R F . . B . .
sub_2366C                                 seg039  0000266C 00000173 R F . . B . .
lbxgfx_get_indep                          seg039  0000280D 0000008F R F . . B . .

lbxgfx_draw_pixels_hmm3_fmt0              seg040  0000000C 00000235 R F . . B . .
lbxgfx_draw_pixels_hmm3_fmt1              seg040  00000241 000002EA R F . . B . .

sub_23DEA                                 seg041  0000001A 000002F2 R F . . B . .
sub_240DC                                 seg041  0000030C 0000002F R . . . . . .
sub_2410B                                 seg041  0000033B 000002F1 R F . . B . .

lbxgfx_setup_auxbuf                       seg042  0000000C 00000042 R F . . B T .
sub_24440                                 seg042  00000050 000000A4 R F . . B . .
sub_244EA                                 seg042  000000FA 0000013C R F . . B . .
sub_24626                                 seg042  00000236 0000002F R . . . . . .
lbxgfx_draw_frame_from_aux_limit_do       seg042  0000026B 000000BC R F . . B T .
sub_24717                                 seg042  00000327 000000BB R . . . . . .
copy_overlay_to_buf                       seg042  000003E2 0000004B R F . . B T .
gfx_aux_flipx                             seg042  0000042D 00000042 R F . . B T .
gfx_aux_color_replace                     seg042  0000046F 00000037 R F . . B T .
gfx_aux_make_paltbl                       seg042  000004A6 00000076 R F . . B T .
clear_overlay_unused                      seg042  0000051C 0000004B R F . . B T .
sub_24957                                 seg042  00000567 0000001C R F . . B . .
lbxgfx_aux_scale_down                     seg042  00000589 000000AA R F . . B T .
lbxgfx_aux_scale_up                       seg042  00000633 000000C8 R F . . B T .
lbxgfx_aux_draw_cloak                     seg042  000007FD 00000070 R F . . B T .

game_math_calc_angle                      seg043  000001FD 00000059 R F . . B T .
calc_angle_do                             seg043  00000256 000000AD R . . . . . .
util_math_angle_dist_cos                  seg043  00000303 0000008E R F . . B T .
util_math_angle_dist_sin                  seg043  00000391 0000008E R F . . B T .
gfx_aux_draw_frame_from_rotate_do_sub1    seg043  0000043B 0000017C R F . . B T .
gfx_aux_draw_frame_from_rotate_do_sub2    seg043  000005B7 000001E5 R F . . B . .

ui_cursor_setup_area                      seg044  00000074 00000044 R F . . B T .
Check_Mouse_Shape                         seg044  000000B8 0000005A R F . . B T .
Get_Pointer_Image_Number                  seg044  00000112 0000000B R F . . . . .
Get_Pointer_Offset                        seg044  0000011D 0000000B R F . . . . .
Mouse_Button                              seg044  00000128 00000026 R F . . . . .
sub_2552E                                 seg044  0000014E 00000020 R F . . . . .
sub_2554E                                 seg044  0000016E 00000096 R F . . B . .
RP_MOUSE_SetUsable                        seg044  00000204 0000000E R F . . . . .
sub_255F2                                 seg044  00000212 00000048 R F . . . . .
Mouse_Movement_Handler                    seg044  0000025A 00000035 R F . . . . .
Mouse_Button_Handler                      seg044  0000028F 00000035 R F . . . . .
Save_Mouse_State                          seg044  000003A4 00000018 R F . . . . .
Restore_Mouse_State                       seg044  000003BC 00000012 R F . . . . .
Keyboard_Status                           seg044  000003E3 00000020 R F . . . . .
Read_Key                                  seg044  00000403 0000025F R F . . . . .
Pointer_X                                 seg044  00000662 0000000B R F . . . . .
Pointer_Y                                 seg044  0000066D 0000000B R F . . . . .
Set_Pointer_Position                      seg044  00000678 0000002B R F . . B . .
Set_Mouse_Position                        seg044  000006A3 00000016 R F . . B . .
sub_25A99                                 seg044  000006B9 0000000B R F . . . . .
sub_25AA4                                 seg044  000006C4 0000000B R F . . . . .
sub_25AAF                                 seg044  000006CF 0000000B R F . . . . .
Mouse_Buffer                              seg044  000006DA 00000011 R F . . . . .
Mouse_Buffer2                             seg044  000006EB 00000011 R F . . . . .
sub_25ADC                                 seg044  000006FC 0000001E R F . . B . .
sub_25AFA                                 seg044  0000071A 0000003A R F . . B . .
Save_Mouse_On_Page                        seg044  00000754 000000D3 R F . . B . .
Save_Mouse_Off_Page                       seg044  00000827 000000DE R F . . B . .
Save_Mouse_On_Page_                       seg044  00000905 000000B7 R F . . B . .
Copy_Mouse_Off_To_Mouse_Back              seg044  000009BC 0000001B R F . . . . .
Restore_Mouse_On_Page                     seg044  000009D7 00000090 R F . . B . .
Restore_Mouse_Off_Page                    seg044  00000A67 00000094 R F . . B . .
Draw_Mouse_On_Page                        seg044  00000B80 00000026 R F . . B . .
Draw_Mouse_Off_Page                       seg044  00000BA6 00000026 R F . . B . .
sub_25FAC                                 seg044  00000BCC 000000D1 R F . . B . .
sub_2607D                                 seg044  00000C9D 000000D5 R F . . B . .

Interpret_Mouse_Input                     seg045  00000002 000008B8 R F . . B . .
uiobj_get_clicked_oi                      seg045  000008F8 0000000A R F . . B . .
RP_GUI_KeyInputOnly                       seg045  00000902 000004A6 R F . . B . .
uiobj_handle_kbd                          seg045  00000DA8 00000427 R F . . B . .
uiobj_kbd_dir_key_dxdy                    seg045  000011F0 00000DAC R F . . B . .
uiobj_set_focus                           seg045  00001F9C 00000118 R F . . B T .
Scan_Field                                seg045  000020B4 000000D0 R F . . B . .
Scan_Input                                seg045  00002184 000000B8 R F . . B . .
sub_2838C                                 seg045  0000223C 00000017 R F . . B . .
Reset_Wait_For_Input                      seg045  00002253 00000032 R F . . B . .
Wait_For_Input                            seg045  00002285 0000009B R F . . B . .
uiobj_kbd_dir_key                         seg045  00002320 000002C8 R F . . B . .
sub_28767                                 seg045  00002617 000003F8 R F . . B . .
uiobj_read_str                            seg045  00002A25 00000390 R F . . B T .
sub_28F1B                                 seg045  00002DCB 000002C3 R F . . B . .
uiobj_add_textinput                       seg045  0000308E 000001CF R F . . B T .
uiobj_dec_y1                              seg045  0000325D 0000002D R F . . B . .
uiobj_add_ta                              seg045  0000328A 00000210 R F . . B T .
uiobj_add_alt_str                         seg045  0000349A 00000150 R F . . B T .
uiobj_set_xyoff                           seg045  00003901 00000011 R F . . B . .
Add_Button_Info                           seg045  00003912 0000018A R F . . B T .
Add_Button_Field                          seg045  00003A9C 0000003F R F . . B T .
uiobj_add_t1                              seg045  00003ADB 00000054 R F . . B T .
uiobj_add_t2                              seg045  00003B2F 00000054 R F . . B T .
uiobj_add_t3                              seg045  00003B83 00000069 R F . . B T .
uiobj_add_mousearea                       seg045  00003BEC 000000FB R F . . B T .
uiobj_add_mousearea_limited               seg045  00003CE7 00000193 R F . . B T .
uiobj_add_inputkey                        seg045  00003E7A 000000F6 R F . . B T .
UU_GUI_CreateHelpLink                     seg045  00003F70 0000013A R F . . B . .
uiobj_add_tb                              seg045  000040AA 0000015C R F . . B T .
uiobj_add_slider_noseg                    seg045  00004206 00000031 R F . . B T .
uiobj_add_slider                          seg045  00004237 000001DD R F . . B T .
sub_2A564                                 seg045  00004414 0000027D R F . . B . .
sub_2A7E1                                 seg045  00004691 00000132 R F . . B . .
sub_2A913                                 seg045  000047C3 00000131 R F . . B . .
Clear_Fields                              seg045  000048F4 00000017 R F . . B . .
ui_select_from_list2                      seg045  00004928 00000AA4 R F . . B T .
ui_select_from_list1                      seg045  000053CC 0000026C R F . . B T .
uiobj_select_list_no_update_at_cursor     seg045  00005638 0000000B R F . . B . .
uiobj_kbd_dir_key_dy_list                 seg045  00005643 00000425 R F . . B T .
uiobj_table_set_last                      seg045  00005A68 00000012 R F . . B . .
Get_Field_Counter                         seg045  00005A7A 0000000A R F . . B . .
Init_Mouse_Keyboard                       seg045  00005A84 000000C9 R F . . B . .
Get_Input                                 seg045  00005B4D 0000004A R F . . B . .
Set_Input_Delay                           seg045  00005B97 00000015 R F . . B . .
uiobj_ta_set_val_0                        seg045  00005BAC 0000002F R F . . B T .
uiobj_ta_set_val_1                        seg045  00005BDB 0000002F R F . . B T .
Draw_Fields                               seg045  00005C0A 00000DB5 R F . . B T .
Draw_Field                                seg045  000069D5 00000F20 R F . . B T .
Push_Field_Down                           seg045  0000790E 000001DB R F . . B T .
uiobj_set_help_id                         seg045  00007AE9 0000000B R F . . B . .
uiobj_set_callback_and_delay              seg045  00007AFF 00000030 R F . . B T .
uiobj_unset_callback                      seg045  00007B2F 0000000B R F . . B . .
Invoke_Auto_Function                      seg045  00007B45 00000010 R F . . B . .
sub_2DCA5                                 seg045  00007B55 0000003A R F . . B . .
Call_Auto_Function                        seg045  00007B8F 0000003A R F . . B . .
Toggle_Pages                              seg045  00007BDB 0000004F R F . . B . .

ui_help                                   seg046  0000000A 0000039E R F . . B T .
ui_help_draw                              seg046  000003A8 00000410 R F . . B T .
lbxpal_find_closest                       seg046  000007B8 000000B8 R F . . B T .

sub_2E99A                                 seg047  000003BA 00000038 R F . . . . .
sub_2E9D2                                 seg047  000003F2 00000018 R F . . . . .
sub_2EABB                                 seg047  000004DB 00000034 R . . . . . .
sub_2EAFE                                 seg047  0000051E 00000044 R . . . . . .
sub_2EB4A                                 seg047  0000056A 00000028 R . . . . . .
sub_2EB7A                                 seg047  0000059A 00000020 R . . . B . .
sub_2EBA3                                 seg047  000005C3 0000002A R F . . B . .
sub_2EBCD                                 seg047  000005ED 0000004B R F . . B . .
sub_2EC18                                 seg047  00000638 0000008D R . . . . . .
sub_2ECAD                                 seg047  000006CD 00000043 R . . . . . .
sub_2ECF8                                 seg047  00000718 0000005C R . . . B . .
sub_2ED5D                                 seg047  0000077D 000000C2 R . . . B . .
sub_2EE28                                 seg047  00000848 00000040 R . . . B . .
sub_2EE71                                 seg047  00000891 00000016 R . . . . . .
sub_2EE8F                                 seg047  000008AF 0000001F R . . . B . .
sub_2EED5                                 seg047  000008F5 00000028 R F . . B . .
sub_2EF1B                                 seg047  0000093B 0000004C R . . . B . .
sub_2EF70                                 seg047  00000990 00000038 R . . . B . .
sub_2EFED                                 seg047  00000A0D 0000000C R . . . . . .
sub_2F001                                 seg047  00000A21 00000099 R . . . B . .
sub_2F12B                                 seg047  00000B4B 00000026 R F . . B . .
sub_2F151                                 seg047  00000B71 00000006 R F . . . . .
sub_2F157                                 seg047  00000B77 000000A9 R . . . B . .
sub_2F209                                 seg047  00000C29 00000033 R F . . . . .
sub_2F254                                 seg047  00000C74 00000006 R F . . . . .
sub_2F25A                                 seg047  00000C7A 00000006 R F . . . . .
sub_2F266                                 seg047  00000C86 00000006 R F . . . . .
sub_2F26C                                 seg047  00000C8C 00000006 R F . . . . .
sub_2F296                                 seg047  00000CB6 00000006 R F . . . . .
sub_2F29C                                 seg047  00000CBC 00000006 R F . . . . .
sub_2F2A2                                 seg047  00000CC2 00000006 R F . . . . .
sub_2F2A8                                 seg047  00000CC8 00000006 R F . . . . .
sub_2F2AE                                 seg047  00000CCE 00000006 R F . . . . .
sub_2F2B4                                 seg047  00000CD4 00000006 R F . . . . .
sub_2F2BA                                 seg047  00000CDA 00000006 R F . . . . .
sub_2F2D2                                 seg047  00000CF2 00000006 R F . . . . .
sub_2F2D8                                 seg047  00000CF8 00000006 R F . . . . .
sub_2F2E4                                 seg047  00000D04 00000006 R F . . . . .
sub_2F2EA                                 seg047  00000D0A 00000006 R F . . . . .
sub_2F2F6                                 seg047  00000D16 00000006 R F . . . . .
sub_2F45A                                 seg047  00000E7A 00000037 R F . . . . .
sub_2F491                                 seg047  00000EB1 00000037 R F . . . . .
sub_2F50F                                 seg047  00000F2F 0000000F R F . . . . .
sub_2F51E                                 seg047  00000F3E 00000017 R F . . . . .
sub_2F535                                 seg047  00000F55 0000003B R F . . B . .
sub_2F570                                 seg047  00000F90 000000B3 R F . . B . .
sub_2F623                                 seg047  00001043 0000001A R F . . B . .
sub_2F690                                 seg047  000010B0 00000055 R F . . . . .
sub_2F6E5                                 seg047  00001105 00000027 R F . . . . .
sub_2F70C                                 seg047  0000112C 00000157 R F . . . . .
sub_2F863                                 seg047  00001283 00000053 R F . . B . .
sub_2F8B6                                 seg047  000012D6 0000002B R F . . B . .
sub_2F8E1                                 seg047  00001301 000000D7 R F . . B . .
sub_2F9B8                                 seg047  000013D8 00000080 R F . . B . .
sub_2FE91                                 seg047  000018B1 00000044 R F . . B . .



game_diplo_turn_p2_sub1                   ovr060  00000000 000007E5 R F . . B T .
game_ai_turn_diplo_p2_sub3                ovr060  000007F0 00000291 R F . . B T .
game_turn_update_mood_blunder             ovr060  00000A81 000003BC R F . . B . .
game_fleet_any_dest_player                ovr060  00000E3D 00000051 R F . . B T .
game_ai_diplo_wage_war_fleet_w            ovr060  00000E8E 00000212 R F . . B T .
game_ai_diplo_wage_war_prod_w             ovr060  000010A0 00000107 R F . . B T .
ui_gameopts                               ovr061  00000000 000002E4 R F . . B . .
sub_429E4                                 ovr061  000002E4 00000035 R F . . B . .
ui_load_game                              ovr061  00000319 0000027D R F . . B . .
load_game_draw_cb                         ovr061  00000596 000000DF R F . . B . .
ui_save_game                              ovr061  00000675 000002BB R F . . B . .
sub_43030                                 ovr061  00000930 0000006A R F . . B . .
ui_design                                 ovr062  00000000 0000077F R F . . B T .
copy_active_shipdesign_to                 ovr062  00000788 00000018 R F . . B T .
active_shipdesign_get_wpnt                ovr062  000007A0 00000021 R F . . B T .
shipdesign_setup                          ovr062  000007C1 00000048 R F . . B T .
game_design_clear_set_name_hull           ovr062  00000809 00000047 R F . . B T .
game_design_clear                         ovr062  00000850 00000073 R F . . B . .
init_lbx_design                           ovr062  000008C3 000000F4 R F . . B . .
design_init_ois                           ovr062  000009B7 000003FD R F . . B . .
design_init_maxtech_haveflags             ovr062  00000DB4 000000C8 R F . . B . .
design_update_haveflags                   ovr062  00000E7C 00000507 R F . . B . .
ui_design_draw_cb                         ovr062  00001383 00000C21 R F . . B . .
sub_45055                                 ovr062  00001FB5 0000001C R F . . B . .
game_design_set_hp                        ovr062  00001FD1 0000003D R F . . B . .
sub_450AE                                 ovr062  0000200E 00000010 R F . . B . .
sub_450BE                                 ovr062  0000201E 0000000B R F . . B . .
sub_450C9                                 ovr062  00002029 00000075 R F . . B . .
get_active_shipdesign_special_i           ovr062  0000209E 0000001B R F . . B . .
game_design_get_hull_space                ovr062  000020B9 0000008C R F . . B . .
lbxfont_print_num_right_0                 ovr062  00002145 0000002F R F . . B T .
sub_45214                                 ovr062  00002174 0000007A R F . . B T .
lbxfont_print_range_right                 ovr062  000021EE 0000006A R F . . B T .
game_design_update_engines                ovr062  00002258 0000016E R F . . B . .
game_design_calc_cost                     ovr062  000023C6 000005BC R F . . B T .
game_design_calc_space                    ovr062  0000298F 00000607 R F . . B T .
game_design_to_parsed                     ovr063  00000000 00000644 R F . . B T .
sub_46694                                 ovr063  00000644 00000019 R F . . B . .
max3                                      ovr063  0000065D 0000001E R F . . B . .
get_best_comp                             ovr063  0000067B 0000004D R F . . B T .
game_get_best_jammer                      ovr063  000006C8 0000004C R F . . B T .
get_base_cost_mod_jammer                  ovr063  00000714 000000B9 R F . . B T .
get_base_cost_mod_comp                    ovr063  000007CD 0000007C R F . . B T .
get_base_weapon                           ovr063  00000849 000000B3 R F . . B T .
get_base_mod_cost_weap                    ovr063  000008FC 00000063 R F . . B T .
get_base_shield                           ovr063  0000095F 00000050 R F . . B T .
get_base_mod_cost_shield                  ovr063  000009AF 0000007C R F . . B T .
get_base_mod_cost_armor                   ovr063  00000A2B 00000095 R F . . B T .
get_shield_absorb                         ovr063  00000AC0 00000020 R F . . B T .
ui_specs_print_weap                       ovr063  00000AE0 00000077 R F . . B T .
ui_design_sub                             ovr063  00000B57 000000C2 R F . . B . .
design_draw_sub_cb                        ovr063  00000C38 0000000F R F . . B . .
game_design_build_tbl_fit_armor           ovr063  00000C47 000000A5 R F . . B . .
game_design_build_tbl_fit_comp            ovr063  00000CEC 000000A8 R F . . B T .
game_design_build_tbl_fit_shield          ovr063  00000D94 000000A9 R F . . B T .
game_design_build_tbl_fit_engine          ovr063  00000E3D 000000A4 R F . . B . .
game_design_build_tbl_fit_man             ovr063  00000EE1 00000070 R F . . B . .
game_design_build_tbl_fit_jammer          ovr063  00000F51 000000A8 R F . . B . .
game_design_build_tbl_fit_weapon          ovr063  00000FF9 00000172 R F . . B T .
game_design_build_tbl_fit_special         ovr063  0000116B 00000143 R F . . B T .
ui_design_sel_armor                       ovr063  000012AE 0000047D R F . . B . .
ui_design_sel_shield                      ovr063  0000172B 000005CB R F . . B . .
ui_design_sel_comp                        ovr063  00001CF6 000005CB R F . . B . .
ui_design_sel_jammer                      ovr063  000022C1 000005C7 R F . . B . .
ui_design_sel_engine                      ovr063  00002888 000006E5 R F . . B . .
ui_design_sel_man                         ovr063  00002F6D 0000062F R F . . B . .
ui_design_sel_weapon                      ovr063  0000359C 0000099C R F . . B T .
ui_design_sel_special                     ovr063  00003F38 00000742 R F . . B T .
game_design_calc_cost_item                ovr063  0000467A 000003AE R F . . B . .
game_design_calc_space_item               ovr063  00004A4F 0000036B R F . . B T .
active_design_set_wpnt                    ovr063  00004DE1 00000011 R F . . B . .
active_design_set_wpnn                    ovr063  00004DF2 00000011 R F . . B . .
active_design_set_special                 ovr063  00004E03 00000023 R F . . B T .
active_design_set_comp                    ovr063  00004E26 0000000B R F . . B . .
active_design_set_jammer                  ovr063  00004E31 0000000B R F . . B . .
active_design_set_shield                  ovr063  00004E3C 0000000B R F . . B . .
active_design_set_engine                  ovr063  00004E47 0000000B R F . . B . .
active_design_set_armor                   ovr063  00004E52 0000000B R F . . B . .
active_design_set_man                     ovr063  00004E5D 0000000B R F . . B . .
active_design_update_engines_space        ovr063  00004E68 0000001C R F . . B . .
ui_battle_draw_cb0                        ovr064  00000000 00000017 R F . . B . .
init_lbx_space                            ovr064  00000017 000003F5 R F . . B . .
ui_gfx_get_battle_bg                      ovr064  0000040C 00000016 R F . . B T .
battle_alloc_tables                       ovr064  00000422 00000086 R F . . B . .
game_battle_prepare_sub1                  ovr064  000004A8 00000301 R F . . B . .
battle_item_from_parsed                   ovr064  000007A9 000002AA R F . . B T .
battle_item_add                           ovr064  00000A53 00000176 R F . . B T .
sub_4BAA9                                 ovr064  00000BC9 000000D1 R F . . B T .
game_battle_place_items                   ovr064  00000C9A 00000345 R F . . B . .
ui_battle_draw_stream2                    ovr064  00000FDF 000002FC R F . . B T .
ui_battle_draw_technull                   ovr064  000012DB 0000025D R F . . B T .
ui_battle_draw_stasis_sub1                ovr064  00001538 00000300 R F . . B T .
ui_battle_draw_stasis                     ovr064  00001838 00000168 R F . . B T .
game_battle_pulsar_get_dmg                ovr064  000019A0 0000013C R F . . B T .
game_battle_repulse                       ovr064  00001ADC 000001AA R F . . B T .
ui_battle_draw_pulsar                     ovr064  00001C86 00000347 R F . . B T .
game_battle_stasis_target                 ovr064  00001FCD 00000177 R F . . B . .
ui_battle_do_sub1                         ovr064  00002144 0000030F R F . . B T .
ui_battle_draw_misshield                  ovr064  00002453 0000056C R F . . B T .
ui_battle_draw_retreat                    ovr064  000029BF 00000490 R F . . B . .
ui_battle_draw_cloaking                   ovr064  00002E4F 00000180 R F . . B T .
ui_battle_draw_repulse                    ovr064  00002FCF 0000069F R F . . B T .
game_battle_with_human_do                 ovr065  00000000 00000301 R F . . B T .
ui_battle_draw_bottom                     ovr065  0000031A 00000031 R F . . B . .
game_battle_missile_turn_done             ovr065  0000034B 00000097 R F . . B . .
game_battle_missile_remove_unused         ovr065  000003E2 0000004E R F . . B . .
game_battle_item_done                     ovr065  00000430 00000043 R F . . B . .
game_battle_with_human_do_sub3            ovr065  00000473 00000834 R F . . B . .
game_battle_reset_specials                ovr065  00000CFE 000003F7 R F . . B . .
ui_battle_draw_focusinfo                  ovr065  000010F5 000001F3 R F . . B . .
ui_battle_draw_arena                      ovr065  000012E8 000001E7 R F . . B T .
ui_battle_draw_item                       ovr065  000014CF 000004BB R F . . B T .
game_battle_area_setup                    ovr065  0000198A 00000A55 R F . . B . .
game_battle_item_move                     ovr065  000023DF 00000435 R F . . B T .
game_battle_get_weap_maxrange             ovr065  00002814 000002ED R F . . B . .
get_possible_distance_increase            ovr065  00002B01 000000DC R F . . B T .
util_math_dist_maxabs                     ovr065  00002BDD 0000002D R F . . B T .
game_battle_build_priority                ovr065  00002C0A 000002BB R F . . B . .
ui_battle_draw_bottom_add_ois             ovr065  00002EC5 00000509 R F . . B . .
ui_battle_draw_bottom_no_ois              ovr065  000033CE 00000382 R F . . B . .
ui_battle_draw_missile1                   ovr065  00003750 0000043D R F . . B T .
get_firing_target_x                       ovr065  00003B96 00000019 R F . . B . .
get_firing_target_y                       ovr065  00003BAF 00000019 R F . . B . .
get_firing_param_x                        ovr065  00003BC8 000000AC R F . . B T .
get_firing_param_y                        ovr065  00003C74 000000AD R F . . B . .
game_battle_get_absorbdiv                 ovr066  00000000 00000033 R F . . B T .
game_battle_item_finish                   ovr066  00000033 00000134 R F . . B . .
ui_battle_draw_scan_cb                    ovr066  00000167 000005D5 R F . . B . .
ui_battle_draw_scan                       ovr066  0000073C 00000080 R F . . B T .
game_battle_area_check_line_ok            ovr066  000007BC 00000096 R F . . B T .
game_battle_set_route_from_tbl            ovr066  00000852 00000066 R F . . B T .
game_battle_extend_route_from_tbl         ovr066  000008B8 00000062 R F . . B T .
game_math_get_route_len                   ovr066  0000091A 000000A7 R F . . B T .
game_math_route_step_len                  ovr066  000009C1 0000001B R F . . B T .
game_battle_item_move_find_route          ovr066  000009DC 00000408 R F . . B T .
battle_missile_rock_collide               ovr066  00000DE4 00000073 R F . . B T .
game_battle_missile_move                  ovr066  00000E57 000003EA R F . . B T .
game_battle_move_retaliate                ovr066  00001241 00000166 R F . . B T .
reverse_ctbl                              ovr066  000013A7 00000022 R F . . B T .
ui_battle_draw_beam_line                  ovr066  000013C9 0000011A R F . . B T .
ui_battle_draw_beam_attack_do1            ovr066  000014E3 00000476 R F . . B T .
ui_battle_draw_beam_attack                ovr066  00001959 000003EE R F . . B T .
ui_battle_draw_beam_attack_do2            ovr066  00001D51 000013F3 R F . . B T .
ui_battle_draw_damage                     ovr066  00003144 00000383 R F . . B T .
game_battle_get_xy_notsame                ovr066  000034C7 00000091 R F . . B T .
ui_battle_draw_bomb_attack                ovr066  00003558 00000163 R F . . B T .
game_battle_special                       ovr066  000036BB 00000F61 R F . . B T .
game_battle_damage_planet                 ovr066  0000462C 000000C3 R F . . B T .
game_battle_missile_hit                   ovr066  000046EF 0000058C R F . . B T .
game_battle_missile_spawn                 ovr067  00000000 000001E8 R F . . B T .
game_battle_attack                        ovr067  000001E8 00000B9B R F . . B T .
game_battle_item_destroy                  ovr067  00000D83 0000025C R F . . B T .
game_ai_battle_dmgmax                     ovr067  00000FDF 00000320 R F . . B T .
game_ai_battle_dmggive                    ovr067  000012FF 00000998 R F . . B T .
game_ai_battle_missile_dmg                ovr067  00001C97 00000282 R F . . B T .
game_ai_battle_incoming_missiles_dmg      ovr067  00001F19 00000069 R F . . B T .
game_ai_battle_rival                      ovr067  00001F82 0000052E R F . . B T .
game_battle_with_human_do_sub2            ovr067  000024B0 0000062B R F . . B . .
game_battle_ai_missile_evade              ovr067  00002ADB 00000255 R F . . B . .
game_battle_ai_target1_sub4               ovr067  00002D30 00000363 R F . . B . .
game_battle_ai_target1_sub5               ovr067  00003093 000003A8 R F . . B . .
game_battle_ai_target1                    ovr067  0000343B 00000174 R F . . B T .
game_missile_none_fired_by                ovr067  000035AF 00000031 R F . . B T .
game_battle_with_human_do_turn_ai         ovr067  000035E0 000004FE R F . . B . .
game_battle_ai_best_range                 ovr067  00003ADE 00000559 R F . . B T .
eval_pos_for_pulsar_use                   ovr067  00004037 000000FA R F . . B T .
game_battle_ai_target1_sub3               ovr067  00004131 0000010B R F . . B T .
ui_battle_draw_planetinfo                 ovr067  0000423C 00000412 R F . . B T .
ui_sound_play_sfx                         ovr067  0000464E 00000044 R F . . B T .
ui_gmap                                   ovr068  00000000 0000029E R F . . B . .
ui_gmap_draw_cb                           ovr068  0000029E 00000CC3 R F . . B . .
game_xy_is_in_nebula                      ovr068  00000F61 00000083 R F . . B T .
ui_starview                               ovr068  00000FE4 000001D5 R F . . B . .
ui_starview_draw_cb                       ovr068  000011B9 00000B79 R F . . B . .
ui_planets                                ovr069  00000000 00000477 R F . . B . .
ui_planets_draw_cb                        ovr069  00000477 0000095E R F . . B . .
ui_planets_transfer                       ovr069  00000DD5 0000037A R F . . B . .
ui_planets_transfer_draw_cb               ovr069  0000114F 0000015C R F . . B . .
game_print_prod_of_total                  ovr069  000012AB 000000E1 R F . . B T .
ui_tech                                   ovr069  0000138C 00000A82 R F . . B . .
ui_tech_build_completed                   ovr069  00001E32 00000163 R F . . B . .
ui_tech_draw_cb                           ovr069  00001FA2 00000ADF R F . . B . .
game_tech_get_name                        ovr069  00002AA6 0000011A R F . . B T .
game_tech_get_description                 ovr069  00002BC0 000000F0 R F . . B T .
sub_5FFB0                                 ovr069  00002CB0 00000040 R F . . B . .
get_research_d0_b0                        ovr069  00002CF0 0000002E R F . . B T .
get_research_d0_b1                        ovr069  00002D1E 00000041 R F . . B T .
game_tech_get_current_research_percent2   ovr069  00002D5F 000001DF R F . . B T .
game_tech_get_current_research_percent1   ovr069  00002F3E 000001AD R F . . B T .
sub_603EB                                 ovr069  000030EB 000000A0 R F . . B . .
ui_audience                               ovr070  00000000 000004B3 R F . . B . .
ui_audience_draw_cb1                      ovr070  000004C1 000000A1 R F . . B . .
game_audience_start_human                 ovr070  00000562 00000090 R F . . B . .
game_audience_get_str1                    ovr070  000005F2 000004A0 R F . . B T .
game_audience_print_tech                  ovr070  00000AC3 0000005F R F . . B T .
sub_60FB2                                 ovr070  00000B22 00000050 R F . . B T .
strlen_argbuf                             ovr070  00000B72 00000017 R F . . B . .
game_diplo_act                            ovr070  00000B89 000005B2 R F . . B T .
game_diplo_start_war                      ovr070  0000113B 000001EC R F . . B T .
game_diplo_break_treaty                   ovr070  00001327 000002E7 R F . . B T .
game_diplo_break_trade                    ovr070  0000160E 000001C4 R F . . B T .
game_turn_diplo_adjust                    ovr070  000017D2 00000475 R F . . B . .
game_audience_sub3                        ovr070  00001C47 00000456 R F . . B . .
ui_audience_draw_cb2                      ovr070  000020B9 000000A0 R F . . B . .
game_audience_str_append_offer            ovr070  00002159 00000092 R F . . B T .
sub_6267B                                 ovr070  000021EB 000000C5 R F . . B . .
ui_audience_draw_cb4                      ovr070  000022B0 000000A8 R F . . B . .
game_diplo_set_trade                      ovr070  00002358 00000152 R F . . B T .
game_diplo_set_treaty                     ovr070  000024AA 00000038 R F . . B T .
game_diplo_stop_war                       ovr070  000024E2 000000D9 R F . . B T .
game_audience_sub2                        ovr070  000025BB 000000DC R F . . B . .
game_diplo_annoy                          ovr070  00002697 0000005D R F . . B T .
game_spy_esp_get_value                    ovr070  000026F4 0000043F R F . . B T .
game_spy_esp_sub2                         ovr070  00002B4C 0000025E R F . . B T .
game_spy_sub3                             ovr070  00002DAA 00000124 R F . . B T .
game_spy_sub3_sub1                        ovr070  00002ECE 00000055 R F . . B T .
sub_633B3                                 ovr070  00002F23 00000075 R F . . B T .
game_spy_esp_sub1                         ovr070  00002F98 00000101 R F . . B T .
game_tech_get_new                         ovr070  00003099 0000046A R F . . B T .
game_diplo_battle_finish                  ovr070  0000351B 00000202 R F . . B T .
game_ai_turn_diplo_p1                     ovr070  0000371D 000003E4 R F . . B . .
game_ai_turn_diplo_p1_sub1                ovr070  00003B01 0000008D R F . . B . .
audience_menu_main                        ovr070  00003B8E 00000362 R F . . B . .
audience_menu_treaty                      ovr070  00003EFE 0000043D R F . . B . .
audience_menu_race                        ovr070  00004348 0000013B R F . . B T .
game_ai_aud_check_mood                    ovr070  00004483 00000160 R F . . B . .
game_audience_set_dtype                   ovr070  000045EB 000001B2 R F . . B T .
game_audience_sweeten                     ovr070  000047A5 00000259 R F . . B . .
ui_audience_draw_cb3                      ovr070  000049FE 0000007B R F . . B . .
game_diplo_get_mood                       ovr070  00004A79 00000063 R F . . B . .
audience_menu_threat                      ovr070  00004ADC 000003A9 R F . . B . .
audience_menu_trade                       ovr070  00004E8D 0000015C R F . . B . .
audience_menu_tech                        ovr070  00004FE9 000005EE R F . . B . .
audience_menu_tribute                     ovr070  000055D7 00000627 R F . . B . .
ui_audience_play_music                    ovr070  00005BFE 0000003C R F . . B . .
ui_audience_draw_race                     ovr070  00005C3A 00000236 R F . . B . .
ui_audience_load_data                     ovr070  00005E88 00000130 R F . . B . .
game_diplo_limit_mood_treaty              ovr070  00005FB8 00000078 R F . . B . .
ui_starmap_orbit_own                      ovr071  00000000 00000F6D R F . . B . .
ui_starmap_orbit_own_draw_cb              ovr071  00000F6D 000006B9 R F . . B . .
game_send_fleet_from_orbit                ovr071  00001626 000002EB R F . . B T .
game_update_visibility                    ovr071  00001911 000002DA R F . . B . .
game_check_coord_is_visible               ovr071  00001BF9 000001BE R F . . B T .
ui_stars_set_xoffs                        ovr071  00001DC7 00000039 R F . . B T .
ui_draw_stars                             ovr071  00001E00 00000228 R F . . B T .
game_update_within_range                  ovr071  00002028 00000491 R F . . B . .
game_get_min_dist                         ovr071  000024C8 00000084 R F . . B T .
sub_68A0C                                 ovr071  0000254C 00000069 R F . . B T .
game_calc_eta                             ovr071  000025B5 00000094 R F . . B T .
ui_starmap_enroute                        ovr071  00002649 000007EE R F . . B . .
ui_starmap_draw_enroute_cb                ovr071  00002E37 00000714 R F . . B . .
starmap_orbit_update_reserve_fuel         ovr071  0000354B 00000045 R F . . B . .
ui_starmap_transport                      ovr072  00000000 000005E2 R F . . B . .
ui_starmap_transport_draw_cb              ovr072  000005E2 000006F4 R F . . B . .
ui_fleet                                  ovr072  00000CD6 000004A9 R F . . B . .
ui_fleet_sub                              ovr072  0000117F 000000DC R F . . B . .
ui_fleet_draw_cb1                         ovr072  0000125B 00000543 R F . . B . .
ui_specs                                  ovr072  0000179E 00000189 R F . . B . .
ui_specs_draw_cb1                         ovr072  00001927 000003F4 R F . . B . .
ui_specs_mustscrap                        ovr072  00001D1B 00000118 R F . . B . .
ui_specs_draw_mustscrap_cb                ovr072  00001E33 000000ED R F . . B . .
game_design_scrap                         ovr072  00001F20 000002E4 R F . . B T .
game_remove_empty_fleets                  ovr072  00002204 000000A8 R F . . B . .
ui_starmap_orbit_en                       ovr072  000022AC 00000596 R F . . B . .
ui_starmap_orbit_en_draw_cb               ovr072  00002842 000002D3 R F . . B . .
ui_specs_before                           ovr072  00002B15 000000A1 R F . . B . .
ui_specs_before_draw_cb                   ovr072  00002BB6 00000061 R F . . B . .
sub_6C984                                 ovr072  00002F34 000000DF R F . . B . .
ui_spy_sabotage_ask                       ovr073  00000000 00000648 R F . . B T .
ui_spy_sabotage_ask_draw_cb               ovr073  00000648 0000036F R F . . B . .
ui_gmap_draw_planet_border                ovr073  000009B7 00000087 R F . . B T .
ui_spy_sabotage_done                      ovr073  00000A3E 00000244 R F . . B . .
ui_spy_sabotage_done_draw_cb              ovr073  00000C82 00000651 R F . . B . .
ui_spy_sabotage_load_data                 ovr073  000012D3 00000163 R F . . B . .
ui_spy_steal                              ovr073  00001436 000001D5 R F . . B . .
ui_spy_steal_draw_cb                      ovr073  0000160B 00000184 R F . . B . .
ui_spy_steal_load_data                    ovr073  0000178F 00000054 R F . . B . .
ui_spy_stolen                             ovr073  000017E3 0000007B R F . . B . .
ui_spy_stolen_draw_cb                     ovr073  0000185E 0000011D R F . . B . .
ui_starmap_draw_planetinfo_2              ovr073  0000197B 0000033A R F . . B T .
ui_starmap_do                             ovr074  00000000 000018C5 R F . . B . .
ui_starmap_draw_cb1                       ovr074  000018D9 000006C3 R F . . B . .
ui_starmap_draw_planetinfo                ovr074  00001F9C 00000440 R F . . B . .
starmap_draw_sliders_and_prod             ovr074  000023DC 000010E1 R F . . B . .
ui_starmap_fill_oi_tbl_stars              ovr074  000034BD 0000009A R F . . B . .
ui_starmap_fill_oi_tbls                   ovr074  00003557 00000314 R F . . B . .
starmap_draw_range_parsec                 ovr074  0000386B 000000BB R F . . B T .
ui_starmap_reloc                          ovr075  00000000 000004F1 R F . . B . .
ui_starmap_reloc_draw_cb                  ovr075  000004F1 000002C5 R F . . B . .
ui_starmap_trans                          ovr075  000007B6 0000068C R F . . B . .
ui_starmap_trans_draw_cb                  ovr075  00000E42 000008DB R F . . B . .
ui_starmap_do_help                        ovr075  0000171D 000003ED R F . . B . .
starmap_remove_build_finished             ovr075  00001B0A 0000005D R F . . B . .
starmap_draw_textbox_finished             ovr075  00001B67 000002F0 R F . . B . .
starmap_draw_textbox_spending_ratios      ovr075  00001E61 00000255 R F . . B T .
ui_starmap_draw_textbox_2str              ovr075  000020B6 00000243 R F . . B T .
ui_draw_textbox_2str                      ovr075  000022F9 00000243 R F . . B T .
ui_starmap_draw_hmm5                      ovr075  0000253C 000000AD R F . . B . .
starmap_add_oi_bottom_buttons             ovr076  00000000 0000011D R F . . B . .
starmap_draw_button_text                  ovr076  0000011D 000001AC R F . . B . .
starmap_draw_starmap                      ovr076  000002C9 00000CEA R F . . B . .
ui_draw_line_3h                           ovr076  00000FB3 00000052 R F . . B T .
tenths_2str                               ovr076  00001005 0000006C R F . . B T .
tech_slider_get_color                     ovr076  00001071 0000001D R F . . B T .
planet_slider_get_color                   ovr076  0000108E 0000002D R F . . B T .
ui_starmap_fill_oi_slider                 ovr076  000010BB 0000042F R F . . B . .
ui_starmap_fill_oi_ctrl                   ovr076  000014EA 0000006D R F . . B . .
ui_starmap_clear_oi_ctrl                  ovr076  00001557 00000035 R F . . B . .
starmap_handle_oi_ctrl                    ovr076  0000158C 000000F9 R F . . B T .
filebuf::terminate(void)                  ovr076  00001685 00000005 R F L . B . .
ui_races                                  ovr077  00000000 00000889 R F . . B . .
ui_races_draw_cb                          ovr077  00000889 00000AF7 R F . . B . .
sub_77060                                 ovr077  00001380 00000049 R F . . B . .
game_update_empire_contact                ovr077  000013C9 000001CE R F . . B . .
ui_empirestatus                           ovr077  00001597 000000ED R F . . B . .
ui_empirestatus_draw_cb                   ovr077  00001684 000003AC R F . . B . .
game_stat_fleet                           ovr077  00001A3A 0000015D R F . . B T .
game_stat_tech                            ovr077  00001B97 0000006D R F . . B T .
game_stat_prod                            ovr077  00001C04 000000AF R F . . B T .
game_stat_pop                             ovr077  00001CB3 0000005A R F . . B T .
game_stat_planets                         ovr077  00001D0D 00000056 R F . . B T .
ui_empirereport                           ovr077  00001D63 000000BA R F . . B . .
ui_empirereport_draw_cb                   ovr077  00001E1D 00000955 R F . . B . .
ui_basescrap                              ovr077  0000278B 0000027B R F . . B . .
ui_basescrap_draw_cb                      ovr077  00002A06 0000011D R F . . B . .
ui_caught                                 ovr077  00002B23 000000C2 R F . . B . .
ui_caught_draw_cb                         ovr077  00002BE5 00000156 R F . . B . .
game_election                             ovr078  00000000 000013AB R F . . B . .
ui_election_draw_cb                       ovr078  000013AB 0000036C R F . . B . .
game_election_prepare                     ovr078  00001717 000001D0 R F . . B . .
ui_election_prepare                       ovr078  000018E7 000001D9 R F . . B . .
ui_election_find_race_gfx                 ovr078  00001AC0 0000002D R F . . B T .
game_tech_final_war_share                 ovr078  00001AED 000003F6 R F . . B . .
game_ground_init                          ovr079  00000000 000005A1 R F . . B . .
ui_landing                                ovr079  000005A1 000001A2 R F . . B . .
ui_landing_draw_cb1                       ovr079  00000743 000001F3 R F . . B . .
ui_landing_prepare                        ovr079  00000936 00000123 R F . . B . .
ui_ground                                 ovr079  00000A59 0000029B R F . . B . .
game_ground_finish                        ovr079  00000CF4 0000028E R F . . B . .
game_ground_kill                          ovr079  00000F82 0000003D R F . . B . .
ui_ground_draw_cb1                        ovr079  00000FBF 000006CF R F . . B . .
ui_ground_draw_item                       ovr079  0000168E 000000E6 R F . . B T .
ui_ground_prepare                         ovr079  00001774 0000011E R F . . B . .
game_turn_transport_shoot                 ovr079  00001892 000004DD R F . . B T .
game_ai_turn_p2_do                        ovr080  00000000 00000749 R F . . B T .
game_ai_design_scrap                      ovr080  00000749 000000D4 R F . . B T .
game_ai_design_ship                       ovr080  0000081D 0000033B R F . . B T .
game_ai_design_ship_get_look              ovr080  00000B58 000000C4 R F . . B T .
game_ai_design_ship_sub2                  ovr080  00000C1C 0000008B R F . . B T .
game_ai_design_ship_base                  ovr080  00000CA7 0000042E R F . . B T .
game_ai_design_ship_get_item              ovr080  000010D5 00000037 R F . . B T .
find_havebuf_item                         ovr080  0000110C 00000025 R F . . B T .
count_havebuf_items                       ovr080  00001131 00000024 R F . . B T .
game_ai_design_ship_weapons               ovr080  00001155 000000AC R F . . B T .
game_ai_design_ship_weapon                ovr080  00001201 00000192 R F . . B T .
monster_get_next_dest                     ovr081  00000000 000000D3 R F . . B T .
game_ai_focus_tech                        ovr081  000000D3 000000B8 R F . . B T .
game_ai_focus_ship                        ovr081  0000018B 000000B8 R F . . B T .
game_ai_add_reserve                       ovr081  00000243 00000083 R F . . B T .
ui_news                                   ovr081  000002C6 00000332 R F . . B . .
ui_news_draw_cb1                          ovr081  000005F8 00000208 R F . . B . .
game_news_get_msg                         ovr081  00000800 00000422 R F . . B . .
get_monster_start                         ovr081  00000C48 00000103 R F . . B T .
sub_7E773                                 ovr081  00000D53 0000003B R F . . B . .
game_turn_new_event_get_trader            ovr081  00000D8E 00000050 R F . . B T .
game_tech_get_orion_loot                  ovr081  00000DDE 00000193 R F . . B T .
game_tech_get_artifact_loot               ovr081  00000F71 0000017C R F . . B T .
game_turn_new_event_get_victim            ovr081  000010ED 00000161 R F . . B . .
game_ai_turn_p1                           ovr082  00000000 000004BD R F . . B . .
game_ai_turn_p1_have_colony_ship          ovr082  000004BD 0000016B R F . . B T .
game_ai_turn_send_scout                   ovr082  00000628 000005B8 R F . . B T .
game_ai_turn_send_colony_ships            ovr082  00000BE0 0000044E R F . . B T .
game_ai_turn_p1_front                     ovr082  0000102E 0000049A R F . . B T .
game_turn_p1_front_find_planet            ovr082  000014C8 0000014E R F . . B T .
game_turn_fleet_send                      ovr082  00001616 000002BC R F . . B T .
game_planet_get_random                    ovr082  000018D2 00000066 R F . . B T .
game_ai_turn_p1_planet_w                  ovr082  00001938 000002B9 R F . . B T .
game_ai_turn_p1_send_attack               ovr082  00001BF1 000006A7 R F . . B T .
game_ai_turn_p1_send_defend               ovr082  00002298 0000044B R F . . B T .
game_ai_turn_p1_send_idle                 ovr082  000026E3 00000222 R F . . B T .
game_ai_turn_trans_en                     ovr082  00002905 00000305 R F . . B T .
game_ai_turn_trans_own                    ovr082  00002C0A 00000234 R F . . B T .
game_ai_turn_p1_build_defending_ships     ovr082  00002E3E 00000180 R F . . B T .
game_ai_turn_p1_fund_developing           ovr082  00002FBE 0000009F R F . . B . .
game_ai_turn_p1_tax                       ovr082  0000305D 00000042 R F . . B T .
game_update_have_reserve_fuel             ovr082  0000309F 000000C2 R F . . B . .
game_ai_best_speed                        ovr082  00003161 00000024 R F . . B T .
game_spy_espionage                        ovr083  00000000 000002EC R F . . B T .
game_spy_sabotage                         ovr083  000002EC 0000048F R F . . B T .
init_alloc_large_tables                   ovr084  00000000 0000015A R F . . B . .
init_load_lbxs                            ovr084  0000015A 000008C7 R F . . B . .
game_new_generate_other_emperor_name      ovr084  00000A21 000000DB R F . . B T .
game_math_dist_steps                      ovr084  00000AFC 00000073 R F . . B T .
