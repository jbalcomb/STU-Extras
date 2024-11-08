

MoX

Movement Map Scrolling





void IDK_CheckSet_MapDisplay_XY(void)
...
¿ Handle Movement Map Scrolling/Wrapping ... Wrap Clamp Limits ... ?


20240709:
Full Review
-Movement Map
-Movement Map Scroll

_prev_world_x
_prev_world_y

_map_x
_map_y

map_moved_flag

CRP_OVL_MapWindowX

#define 50
#define 20

...

Draw_Maps()

...

Main_Screen()
### Section 9.2
Right-Click Movement Map

if(entity_idx == ST_UNDEFINED)
    _prev_world_x += (_main_map_grid_x - (MAP_WIDTH  / 2));
    _prev_world_y += (_main_map_grid_y - (MAP_HEIGHT / 2));
    IDK_CheckSet_MapDisplay_XY();


// WZD o67p0
// AKA IDK_CheckSet_MapDisplay_XY
// ¿ 1oom starmap_handle_oi_ctrl() ?
/*
limit the map on the top and bottom
wrap the map on the left and right
jump map from left edge to right edge, when scrolling left
*/
void IDK_CheckSet_MapDisplay_XY(void)
{
    // bind Y on the bottom
    if(_prev_world_y + MAP_HEIGHT >= WORLD_HEIGHT)
    {
        _prev_world_y = WORLD_HEIGHT - MAP_HEIGHT;
    }

    // wrap X around to the right
    if(_prev_world_x >= WORLD_WIDTH)
    {
        _prev_world_x -= WORLD_WIDTH;
    }

    // wrap X around to the left
    if(_prev_world_x < 0)
    {
        _prev_world_x -= WORLD_WIDTH;
    }

    // bind Y on the top
    if(_prev_world_y < 0)
    {
        _prev_world_y = 0;
    }

    // ¿ reorient X if scroll will wrap ?
    if(_map_x == 0 && _prev_world_x > 49)
    {
        _map_x = WORLD_WIDTH;
    }
}











// WZD o67p0
// AKA IDK_CheckSet_MapDisplay_XY
// ¿ 1oom starmap_handle_oi_ctrl() ?
/*
limit the map on the top and bottom
wrap the map on the left and right
jump map from left edge to right edge, when scrolling left
*/
void IDK_CheckSet_MapDisplay_XY(void)




    code (0 bytes) Scroll_Draw_Combat_Screen_
    Address: 01:00033543

    code (0 bytes) Scroll_Combat_Screen_
    Address: 01:00049A41

    code (0 bytes) Scroll_Map_
    Address: 01:00089DF9

```c
    Scroll_Map_(int x, int y)
    {
        int scoll_map_x;
        int scoll_map_y;

        scoll_map_x = x;
        scoll_map_x -= Get_Up_Scaled_Value_(253);

        scoll_map_y = y;
        scoll_map_y -= Get_Up_Scaled_Value_(200);

        if(scoll_map_x < 0)
        {
            scoll_map_x = 0;
        }

        if(scoll_map_x > (_MAP_MAX_X - Get_Up_Scaled_Value_(506)))
        {
            scoll_map_x = _MAP_MAX_X - Get_Up_Scaled_Value_(506);
        }

        if(scoll_map_y < 0)
        {
            scoll_map_y = 0;
        }

        if(scoll_map_y > (_MAP_MAX_Y - Get_Up_Scaled_Value_(200)))
        {
            scoll_map_y = _MAP_MAX_Y - Get_Up_Scaled_Value_(200);
        }

        l_cur_map_x = scoll_map_x - _cur_map_x
        l_cur_map_y = scoll_map_y - _cur_map_y

Range(_cur_map_x, _cur_map_y, scoll_map_x, scoll_map_y)



cseg01:00089F67 ||
cseg01:00089F67 (
cseg01:00089F67 l_cur_map_x == 0
cseg01:00089F67 &&
cseg01:00089F67 l_cur_map_y == 0
cseg01:00089F67 )
cseg01:00089F67
cseg01:00089F67
cseg01:00089F67 loc_89F67:
cseg01:00089F67 mov     _animated_star, e_ST_UNDEFINED_DW
cseg01:00089F70 mov     _draw_main_screen_to_back, 1
cseg01:00089F70
cseg01:00089F70
cseg01:00089F77 mov     _cur_map_x, _SI_scoll_map_x
cseg01:00089F7E mov     _cur_map_y, _DI_scoll


    }



    int16_t Get_Up_Scaled_Value_(int16_t value)
    {
        int16_t up_scaled_value;
        up_scaled_value = (value * _cur_map_scale) / 10
        return up_scaled_value;
    }

    int16_t Get_Scaled_Value_(int16_t value)
    {
        int16_t scaled_value;
        scaled_value = (value * 10) / _cur_map_scale;
        return scaled_value;
    }

```
