proc GFX_Swap_AppendUView far
_SI_itr = si
push    bp
mov     bp, sp
push    _SI_itr
xor     _SI_itr, _SI_itr
jmp     short loc_47652
loc_47638:
push    [g_graphics_cache_seg]
push    _SI_itr
mov     ax, offset special_lbx_file
push    ax
call    LBX_Reload_Next
add     sp, 6
mov     bx, _SI_itr
shl     bx, 1
mov     [special_seg+bx], ax
inc     _SI_itr
loc_47652:
cmp     _SI_itr, 111
jl      short loc_47638
xor     _SI_itr, _SI_itr
jmp     short loc_47675
loc_4765B:
push    [g_graphics_cache_seg]
push    _SI_itr
mov     ax, offset special2_lbx_file__2
push    ax
call    LBX_Reload_Next
add     sp, 6
mov     bx, _SI_itr
shl     bx, 1
mov     [(special_seg+0DEh)+bx], ax
inc     _SI_itr
loc_47675:
cmp     _SI_itr, 34
jl      short loc_4765B
push    [g_graphics_cache_seg]
xor     ax, ax
push    ax
mov     ax, offset unitview_lbx_file
push    ax
call    LBX_Reload_Next
add     sp, 6
mov     [unitview_small_background_seg], ax
push    [g_graphics_cache_seg]
mov     ax, 1
push    ax
mov     ax, offset unitview_lbx_file
push    ax
call    LBX_Reload_Next
add     sp, 6
mov     [unitview_large_background_seg], ax
push    [g_graphics_cache_seg]
mov     ax, 2
push    ax
mov     ax, offset unitview_lbx_file
push    ax
call    LBX_Reload_Next
add     sp, 6
mov     [unitview_button_background_seg], ax
push    [g_graphics_cache_seg]
mov     ax, 3
push    ax
mov     ax, offset unitview_lbx_file
push    ax
call    LBX_Reload_Next
add     sp, 6
mov     [unitview_up_arrow_seg], ax
push    [g_graphics_cache_seg]
mov     ax, 4
push    ax
mov     ax, offset unitview_lbx_file
push    ax
call    LBX_Reload_Next
add     sp, 6
mov     [unitview_down_arrow_seg], ax
xor     _SI_itr, _SI_itr
jmp     short loc_4770F
loc_476F0:
push    [g_graphics_cache_seg]
mov     ax, _SI_itr
add     ax, 5
push    ax
mov     ax, offset unitview_lbx_file
push    ax
call    LBX_Reload_Next
add     sp, 6
mov     bx, _SI_itr
shl     bx, 1
mov     [unitview_specials_borders_seg+bx], ax
inc     _SI_itr
loc_4770F:
cmp     _SI_itr, 8
jl      short loc_476F0
xor     _SI_itr, _SI_itr
jmp     short loc_47737
loc_47718:
push    [g_graphics_cache_seg]
mov     ax, _SI_itr
add     ax, 13
push    ax
mov     ax, offset unitview_lbx_file
push    ax
call    LBX_Reload_Next
add     sp, 6
mov     bx, _SI_itr
shl     bx, 1
mov     [unitview_stat_icons_seg+bx], ax
inc     _SI_itr
loc_47737:
cmp     _SI_itr, 15
jl      short loc_47718
xor     _SI_itr, _SI_itr
jmp     short loc_4775F
loc_47740:
push    [g_graphics_cache_seg]
mov     ax, _SI_itr
add     ax, 35
push    ax
mov     ax, offset unitview_lbx_file
push    ax
call    LBX_Reload_Next
add     sp, 6
mov     bx, _SI_itr
shl     bx, 1
mov     [unitview_stat_gold_icons_seg+bx], ax
inc     _SI_itr
loc_4775F:
cmp     _SI_itr, 15
jl      short loc_47740
push    [g_graphics_cache_seg]
mov     ax, 28
push    ax
mov     ax, offset unitview_lbx_file
push    ax
call    LBX_Reload_Next
add     sp, 6
mov     [_unitlist_background_seg], ax
push    [g_graphics_cache_seg]
mov     ax, 29
push    ax
mov     ax, offset unitview_lbx_file
push    ax
call    LBX_Reload_Next
add     sp, 6
mov     [_unitlist_bottom_seg], ax
push    [g_graphics_cache_seg]
mov     ax, 30
push    ax
mov     ax, offset unitview_lbx_file
push    ax
call    LBX_Reload_Next
add     sp, 6
mov     [IMG_OVL_BuildBtn_BG@], ax
push    [g_graphics_cache_seg]
mov     ax, 24
push    ax
mov     ax, offset backgrnd_lbx_file
push    ax
call    LBX_Reload_Next
add     sp, 6
mov     [red_button_seg], ax
push    [g_graphics_cache_seg]
mov     ax, 73
push    ax
mov     ax, offset backgrnd_lbx_file
push    ax
call    LBX_Reload_Next
add     sp, 6
mov     [black_gold1_icon_seg], ax
push    [g_graphics_cache_seg]
mov     ax, 42
push    ax
mov     ax, offset backgrnd_lbx_file
push    ax
call    LBX_Reload_Next
add     sp, 6
mov     [gold1_icon_seg], ax
push    [g_graphics_cache_seg]
mov     ax, 31
push    ax
mov     ax, offset unitview_lbx_file
push    ax
call    LBX_Reload_Next
add     sp, 6
mov     [IMG_CTY_LeftBldTab@], ax
push    [g_graphics_cache_seg]
mov     ax, 32
push    ax
mov     ax, offset unitview_lbx_file
push    ax
call    LBX_Reload_Next
add     sp, 6
mov     [IMG_CTY_RightBldTab@], ax
push    [g_graphics_cache_seg]
mov     ax, 33
push    ax
mov     ax, offset unitview_lbx_file
push    ax
call    LBX_Reload_Next
add     sp, 6
mov     [IMG_USW_Portrt_Brdr@], ax
loc_47833:
push    [g_graphics_cache_seg]
mov     ax, 34
push    ax
mov     ax, offset unitview_lbx_file
push    ax
call    LBX_Reload_Next
add     sp, 6
mov     [unit_water_diamond_seg], ax
push    [g_graphics_cache_seg]
mov     ax, 103
push    ax
mov     ax, offset cityscap_lbx_file
push    ax
call    LBX_Reload_Next
add     sp, 6
mov     [unit_grass_diamond_seg], ax
xor     _SI_itr, _SI_itr
jmp     short loc_47884
loc_47865:
push    [g_graphics_cache_seg]
mov     ax, _SI_itr
add     ax, 40
push    ax
mov     ax, offset backgrnd_lbx_file
push    ax
call    LBX_Reload_Next
add     sp, 6
mov     bx, _SI_itr
shl     bx, 1
mov     [city_lil_resource_icon_seg+bx], ax
inc     _SI_itr
loc_47884:
cmp     _SI_itr, 5
jl      short loc_47865
push    [g_graphics_cache_seg]
mov     ax, 94
push    ax
mov     ax, offset backgrnd_lbx_file
push    ax
call    LBX_Reload_Next
add     sp, 6
mov     [city_grey_lil_bread_icon_seg], ax
xor     _SI_itr, _SI_itr
jmp     short loc_478C3
loc_478A4:
push    [g_graphics_cache_seg]
mov     ax, _SI_itr
add     ax, 88
push    ax
mov     ax, offset backgrnd_lbx_file
push    ax
call    LBX_Reload_Next
add     sp, 6
mov     bx, _SI_itr
shl     bx, 1
mov     [city_big_resource_icon_seg+bx], ax
inc     _SI_itr
loc_478C3:
cmp     _SI_itr, 6
jl      short loc_478A4
push    [g_graphics_cache_seg]
mov     ax, 95
push    ax
mov     ax, offset backgrnd_lbx_file
push    ax
call    LBX_Reload_Next
add     sp, 6
mov     [city_grey_big_bread_icon_seg], ax
pop     _SI_itr
pop     bp
retf
endp GFX_Swap_AppendUView
