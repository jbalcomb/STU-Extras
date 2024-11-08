

¿ MoX Migration Plan ?
*.MoX.* ==> ./ReMoM/MoX
...MOX1,MOX2...main1(),main2()...



¿ SDL Migration Plan ?
MoM & MoX & PFL ... Win & SDL ... cmake / premake









// WZD o62p05
// Select_Unit_Stack(... *map_x, *map_y, map_plane ...) |-> OVL_BringIntoView(*map_x, *map_y, unit_x, unit_y, map_plane);
// MoO2  ¿ Star_On_Screen_(); Ship_Completely_On_Screen_(); Ship_On_Screen_(); ?
void OVL_BringIntoView(int16_t *map_x, int16_t *map_y, int16_t unit_x, int16_t unit_y, int16_t map_plane)

MoO2
Star_On_Screen_()
Ship_Completely_On_Screen_()


all combat stuff, but seems nearest leftover from MoXv2 and MoXv1

_cur_ship
_combat_data
_global_combat_data
~ s_CMBT_DATA


Ship_Done_Check_
Move_Ship_
Next_Ship_
Set_Cur_Ship_To_


Star_On_Screen_()
    On_Combat_Screen_()

Ship_Completely_On_Screen_()
    On_Combat_Screen_()

pretty sure Ship_Completely_On_Screen_() does a little extra work to accomodate the ship *size* ~ if v +1,+2

On_Combat_Screen_() returns F/T
Star_On_Screen_() returns On_Combat_Screen_()
Ship_Completely_On_Screen_() returns On_Combat_Screen_()

Ship_Completely_On_Screen_() most like OVL_BringIntoView()

Next_Ship_()
    Ship_Completely_On_Screen_()
    if F
    Snap_Center_Combat_Screen_()

_combat_near_x
_combat_near_y
_combat_mid_x
_combat_far_x
_combat_mid_y
_combat_far_y

_cur_combat_x
_cur_combat_y

Refresh_Combat_Screen_Full_()
    Draw_Main_Combat_Screen_()
    _TOGGLE_PAGES_()




Screen_Control()
    scr_Continue
    |-> Loaded_Game_Update()
        |-> GAME_Overland_Init()
            |-> G_WLD_StaticAssetRfrsh()
            |-> Reset_Draw_Active_Stack()
                |-> Reset_Map_Draw()
                    map_draw_prev_x = ST_UNDEFINED;
                    map_draw_prev_y = ST_UNDEFINED;
            _prev_world_x = 0;
            _prev_world_y = 0;
            _map_x = 0;
            _map_y = 0;
            |-> Center_Map(&_map_x, &_map_y, _FORTRESSES[0].world_x, _FORTRESSES[0].world_y, _map_plane);
            |-> Set_Entities_On_Map_Window(_map_x, _map_y, _map_plane);
            |-> WIZ_NextIdleStack(_human_player_idx, &_map_x, &_map_y, &_map_plane);
                |-> Reset_Map_Draw()
                    map_draw_prev_x = ST_UNDEFINED;
                    map_draw_prev_y = ST_UNDEFINED;
                |-> WIZ_NextUnit()
                    _active_world_x = s_UNIT.world_x
                    _active_world_y = s_UNIT.world_y
                |-> Select_Unit_Stack()
                    |-> OVL_BringIntoView()
                        |-> Set_Entities_On_Map_Window()
        _unit_stack_count = 0
        |-> GAME_NextHumanStack()
            |-> WIZ_NextIdleStack(_human_player_idx, &_map_x, &_map_y, &_map_plane);
                |-> Reset_Map_Draw()
                    map_draw_prev_x = ST_UNDEFINED;
                    map_draw_prev_y = ST_UNDEFINED;
                |-> WIZ_NextUnit()
                    _active_world_x = s_UNIT.world_x
                    _active_world_y = s_UNIT.world_y
                |-> Select_Unit_Stack()
                    |-> OVL_BringIntoView()
                        |-> Set_Entities_On_Map_Window()


















GUI_String_1

Module: MOX
    data (0 bytes) _temp_string
    Address: 02:00187F78





what uses what is loaded by GFX_Swap_Cities()

    Load_Unit_StatFigs()
    GFX_Swap_AppendItems()
    GFX_Swap_AppndCtScap()
    City_Screen_Load_Pictures()
    Spellbook_Load_Small_Pictures()

City_Screen_Load_Pictures()
    City Screen, Outpost Screen, Enemy City Screen


GFX_Swap_Overland()
    GFX_Swap_Reset()
    GFX_Swap_AppendUView()
    GFX_Swap_AppendItems()
    GFX_Swap_AppndCtScap()





screen_seg


Allocate_Data_Space_For_Game_Popup_+27 mov     eax, _screen_seg                                                                         

Allocate_First_Screen_Seg_+16          mov     eax, _screen_seg                                                                         

Allocate_Help_Buffer_+6                mov     eax, _screen_seg                                                                         

Allocate_Space_For_Summary_Screen_+2A  mov     eax, _screen_seg                                                                         

Draw_Help_Entry_+1EA                   mov     eax, _screen_seg                                                                         

Init_Leaders_+1C                       mov     ebx, _screen_seg                                                                         

Init_Players_+261                      mov     ebx, _screen_seg                                                                         

Load_FP_Data_From_LBX_+A               mov     eax, _screen_seg                                                                         

Load_Game_+7C3                         mov     eax, _screen_seg                                                                         

Load_Game_Popup_Pictures_+11C          mov     ebx, _screen_seg                                                                         

Load_Pictures_+3C                      mov     _screen_seg, eax                ; _screen_seg = Allocate_Next_Block(_global_screen_seg, )

Main_Menu_Screen_+22                   mov     ebx, _screen_seg                ; base_seg                                               

Main_Screen_+48                        mov     eax, _screen_seg                                                                         

Mainmenu_Load_Game_Popup_+18           mov     ebx, _screen_seg                                                                         






GFX buffer?

world data?

_global_cache_seg looks like it might be World_Data
lotsa combat stuff
or _global_data_seg?

_global_screen_seg
vs.
_screen_seg

might be what I have as _screen_seg is _global_screen_seg
and GFX_Swap is actually _screen_seg

Clue?  Release_All_Blocks_()



GFX_Swap_Cities()  ~==  Load_Colony_Screen_Seg_()

GFX_Swap_Cities()
    |-> GFX_Swap_Reset()
    |-> Load_Unit_StatFigs()             |-> LBX_Reload_Next()
    |-> GFX_Swap_AppendItems()           |-> LBX_Reload_Next()
    |-> GFX_Swap_AppndCtScap()           |-> LBX_Reload_Next()
    |-> City_Screen_Load_Pictures()      |-> LBX_Reload_Next()
    |-> Spellbook_Load_Small_Pictures()  |-> LBX_Reload_Next()

GFX_Swap_Seg is just all EMM LBX FLIC/pict headers?
so the size is just 16 * some number of anim/pict headers?

¿ along with Allocate_Help_Buffer_() ?

Colony_Screen_() calls the loaders directly
    the first loader does a Far_Reload(), rather than Far_Reload_Next(), which ~clears the buffer/seg/sub-allocation

MoO2 Load_Main_Screen_Pictures_() reloads into _global_screen_seg

MoM Main_Screen_Load_Pictures() does all LBX_Load()'s ~ allocation type 0 ~ calls malloc()

main__0()  |-> Load_Pictures_()  |-> Load_Main_Screen_Pictures_()

_main()  |-> Load_WZD_Resources()  |-> Main_Screen_Load_Pictures()



MoO2 Main_Menu_Screen_()
    starts with
        _mainmenu_first_seg = Far_Reload(mainmenu_lbx, 1, _screen_seg)
        _mainmenu = Allocate_First_Block(_global_data_seg, 210)
    then does
        Reload_Main_Menu_Screen_() does all Far_Reload_Next() into _screen_seg




Cache_Load_Bldg_() looks like the same kind of logic as the load figures, with the calculating the LBX filename





Allocate_First_Screen_Seg_()



Reload_Officer_Screen_Pictures_()
    |-> Allocate_First_Screen_Seg_()
    |-> Allocate_Fltscrn_Next_Screen_Seg_()








Allocate_Data_Space_()

mov     eax, 1790000                    ; size
call    Allocate_Space
mov     _global_screen_seg, eax

call    Linear_Free
sub     eax, 50
imul    eax, 1000
mov     _global_cache_size, eax
call    Allocate_Buffer_Space
mov     _global_cache_seg, eax
OR

cmp     edx, 1791000   
sub     edx, 266000
mov     _nebula_screen_seg, eax
mov     eax, edx
call    Allocate_Buffer_Space
mov     _global_cache_seg, eax
mov     _global_cache_size, edx

mov     eax, 10240
call    Allocate_Dos_Space
mov     _global_data_seg, eax





_global_cache_seg

Buffer_Reload()

buffer_space_ptr
buffer_space_seg
buffer_space_size

file_buffer
file_buffer_count
global_load_count



MoO2

Module: MOX

    data (0 bytes) _temp_string_buf
    Address: 02:00175640




    data (0 bytes) _global_cache_size
    Address: 02:0018B170

    data (0 bytes) _global_cache_seg
    Address: 02:0018B174

    data (0 bytes) _global_music_handle
    Address: 02:0018B17C

    data (0 bytes) _global_bitmap_seg
    Address: 02:0018B2D0


    data (0 bytes) _sound_buffer
    Address: 02:0019116C


    data (0 bytes) _g_ship_move_info
    Address: 02:001912A8


    data (0 bytes) _screen_data
    Address: 02:00191830

    data (0 bytes) _popup_base_y
    Address: 02:0019183A

    data (0 bytes) _popup_base_x
    Address: 02:0019183C



    data (0 bytes) _ship_data_id
    Address: 02:001918D2

    data (0 bytes) _cur_ptr
    Address: 02:001918D8

    data (0 bytes) _cur_ship
    Address: 02:001918F8

    data (0 bytes) _ship_icon_player_id
    Address: 02:00191966



    data (0 bytes) _twinkle_star_ctr
    Address: 02:00191978



    data (0 bytes) _first_field_in_group
    Address: 02:00174F18



    data (0 bytes) _personality_labels
    Address: 02:0018FFA0

    data (0 bytes) _treaty_labels
    Address: 02:00191150
