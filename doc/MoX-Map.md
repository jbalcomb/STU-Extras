

MoX

Movement Map

Movement Map Scrolling



C:\STU\developp\1oom\src\ui\classic\uistarmap_common.c
void ui_starmap_handle_scrollkeys(struct starmap_data_s *d, int16_t oi)

C:\STU\developp\1oom\src\ui\classic\uistarmap_common.c
static int ui_starmap_scrollkey_accel(int zh)

¿ key press {u,j,h,k} ? up,down,left,right

ui_data.starmap.x
ui_data.starmap.y
ui_data.starmap.x2
ui_data.starmap.y2
ui_data.starmap.xhold
ui_data.starmap.yhold

C:\STU\developp\1oom\src\ui\classic\uigame.c
ui_turn_action_t ui_game_turn(struct game_s *g, int *load_game_i_ptr, int pi)
        ui_data.starmap.xhold = 0;
        ui_data.starmap.yhold = 0;

¿ ~== Screen_Control() ?


        SETRANGE(x, 0, g->galaxy_maxx - ((108 * ui_scale) / starmap_scale));
        SETRANGE(y, 0, g->galaxy_maxy - ((86 * ui_scale) / starmap_scale));

#define SETRANGE(a, b, c) do { if (((c) <= (b)) || ((b) > (a))) { (a) = (b); } else if ((c) < (a)) { (a) = (c); } } while (0)

SETRANGE(x, 0, g->galaxy_maxx - ((108 * ui_scale) / starmap_scale));

x = ¿ ?
```c
if(
    ((c) <= (b))
    ||
    ((b) > (a))
    )
{
    (a) = (b);
}
else if (
    (c) < (a)
    )
{
    (a) = (c);
}
```
```c
g->galaxy_maxx - ((108 * ui_scale) / starmap_scale)
g->galaxy_maxy - (( 86 * ui_scale) / starmap_scale)

if(
    ((c) <= 0)
    ||
    (0 > x)
    )
{
    x = 0;
}
else if (
    (c) < x
    )
{
    x = (c);
}
```
