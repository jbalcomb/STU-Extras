; =============== S U B R O U T I N E =======================================
; Attributes: bp-based frame
proc AI_Continent_Reeval__WIP far
Sum_City_Y_NonOwn_UnitCost= word ptr -204h; not own unit strength (value total)
Sum_City_X_Own_UnitCost= word ptr -114h; own unit strength (value total)
Tile_Linear= word ptr -24h
found_target= word ptr -22h
Center_Y= word ptr -20h
Target_Tile_Landmass= word ptr -1Eh
Tile_Weight= word ptr -1Ch
Target_Tile_Y= word ptr -1Ah
Target_Tile_X= word ptr -18h
Center_X= word ptr -16h
Best_Tile_Weight= word ptr -14h
unit_type= word ptr -12h
Y_2= word ptr -10h
X_2= word ptr -0Eh
landmass_node_index= word ptr -0Ch
city_count= word ptr -0Ah
Tile_Y= word ptr -8
Tile_X= word ptr -6
itr= word ptr -4
landmass_idx= word ptr -2
player_idx= word ptr  6
wp = di
push    bp
mov     bp, sp
sub     sp, 204h
push    si
push    wp
call    j_EMM_Map_CONTXXX__WIP
mov     [bp+found_target], e_ST_FALSE
xor     wp, wp
jmp     loc_F74B9
; ---------------------------------------------------------------------------
loc_F742A:
mov     [bp+landmass_idx], 0
jmp     short loc_F74A9
; ---------------------------------------------------------------------------
loc_F7431:
mov     ax, wp
mov     dx, size s_CONTINENTS_TABLE
imul    dx
push    ax
mov     ax, [bp+player_idx]
mov     dx, size s_LANDMASSES
imul    dx
pop     bx
add     bx, ax
add     bx, [bp+landmass_idx]
cmp     [_ai_continents__0.Player_0.type_array+bx], lmt_NoPresence
jnz     short loc_F74A6
mov     ax, wp
mov     dx, size s_CONTINENTS_TABLE
imul    dx
push    ax
mov     ax, [bp+player_idx]
mov     dx, size s_LANDMASSES
imul    dx
pop     bx
add     bx, ax
add     bx, [bp+landmass_idx]
mov     al, [_ai_continents__0.Player_0.wy_array+bx]
cbw
mov     dx, e_WORLD_WIDTH
imul    dx
push    ax
mov     ax, wp
mov     dx, size s_CONTINENTS_TABLE
imul    dx
push    ax
mov     ax, [bp+player_idx]
mov     dx, size s_LANDMASSES
imul    dx
pop     bx
add     bx, ax
add     bx, [bp+landmass_idx]
mov     al, [byte ptr _ai_continents__0.Player_0.wx_array+bx]
cbw
pop     dx
add     dx, ax
shl     dx, 1
mov     bx, wp
mov     cl, 2
shl     bx, cl
les     bx, [g_ai_evaluation_map+bx]
add     bx, dx
cmp     [word ptr es:bx], 0
jz      short loc_F74A6
mov     [bp+found_target], e_ST_TRUE
loc_F74A6:
inc     [bp+landmass_idx]
loc_F74A9:
cmp     [bp+landmass_idx], e_NUM_LANDMASSES
jge     short loc_F74B8
cmp     [bp+found_target], e_ST_FALSE
jnz     short loc_F74B8
jmp     loc_F7431
; ---------------------------------------------------------------------------
loc_F74B8:
inc     wp
loc_F74B9:
cmp     wp, e_NUM_PLANES
jge     short loc_F74C1
jmp     loc_F742A
; ---------------------------------------------------------------------------
loc_F74C1:
cmp     [bp+found_target], e_ST_TRUE
jnz     short loc_F74D6
mov     ax, [bp+player_idx]
shl     ax, 1
mov     bx, [_ai_reevaluate_continents_countdown]
add     bx, ax
mov     [word ptr bx], 0
loc_F74D6:
mov     ax, [bp+player_idx]
shl     ax, 1
mov     bx, [_ai_reevaluate_continents_countdown]
add     bx, ax
dec     [word ptr bx]
mov     ax, [bp+player_idx]
shl     ax, 1
mov     bx, [_ai_reevaluate_continents_countdown]
add     bx, ax
cmp     [word ptr bx], 0
jle     short loc_F74F8
xor     ax, ax
jmp     loc_F8395
; ---------------------------------------------------------------------------
loc_F74F8:
mov     ax, 10
push    ax
call    Random
pop     cx
push    ax
mov     ax, 10
push    ax
call    Random
pop     cx
pop     dx
add     dx, ax
add     dx, 25
mov     ax, [bp+player_idx]
shl     ax, 1
mov     bx, [_ai_reevaluate_continents_countdown]
add     bx, ax
mov     [bx], dx
mov     [bp+itr], 0
jmp     loc_F75B6
; ---------------------------------------------------------------------------
loc_F7528:
mov     ax, [bp+itr]
mov     cl, 5
shl     ax, cl
les     bx, [_UNITS]
add     bx, ax
mov     al, [es:bx+s_UNIT.owner_idx]
cbw
cmp     ax, [bp+player_idx]
jnz     short loc_F75B3
mov     ax, [bp+itr]
mov     cl, 5
shl     ax, cl
les     bx, [_UNITS]
add     bx, ax
mov     al, [es:bx+s_UNIT.type]
mov     ah, 0
mov     dx, size s_UNIT_TYPE
imul    dx
mov     bx, ax
cmp     [_unit_type_table.Construction+bx], 0
jnz     short loc_F75B3
mov     ax, [bp+itr]
mov     cl, 5
shl     ax, cl
les     bx, [_UNITS]
add     bx, ax
mov     al, [es:bx+s_UNIT.dst_wy]
cbw
mov     dx, e_WORLD_WIDTH
imul    dx
mov     dx, [bp+itr]
mov     cl, 5
shl     dx, cl
les     bx, [_UNITS]
add     bx, dx
push    ax
mov     al, [es:bx+s_UNIT.dst_wx]
cbw
pop     dx
add     dx, ax
shl     dx, 1
mov     bx, wp
mov     cl, 2
shl     bx, cl
les     bx, [g_ai_evaluation_map+bx]
add     bx, dx
cmp     [word ptr es:bx], 0
jnz     short loc_F75B3
mov     ax, [bp+itr]
mov     cl, 5
shl     ax, cl
les     bx, [_UNITS]
add     bx, ax
mov     [es:bx+s_UNIT.Status], us_Ready
loc_F75B3:
inc     [bp+itr]
loc_F75B6:
mov     ax, [bp+itr]
cmp     ax, [_units]
jge     short loc_F75C2
jmp     loc_F7528
; ---------------------------------------------------------------------------
loc_F75C2:
xor     wp, wp
jmp     short loc_F7626
; ---------------------------------------------------------------------------
loc_F75C6:
mov     [bp+landmass_idx], 0
jmp     short loc_F761F
; ---------------------------------------------------------------------------
loc_F75CD:
mov     ax, wp
mov     dx, size s_CONTINENTS_TABLE
imul    dx
push    ax
mov     ax, [bp+player_idx]
mov     dx, size s_LANDMASSES
imul    dx
pop     bx
add     bx, ax
add     bx, [bp+landmass_idx]
mov     [_ai_continents__0.Player_0.type_array+bx], lmt_Invalid
mov     ax, wp
mov     dx, 120
imul    dx
mov     dx, [bp+landmass_idx]
shl     dx, 1
add     ax, dx
lea     dx, [bp+Sum_City_X_Own_UnitCost]
add     ax, dx
mov     bx, ax
mov     [word ptr bx], 0
mov     ax, wp
mov     dx, 120
imul    dx
mov     dx, [bp+landmass_idx]
shl     dx, 1
add     ax, dx
lea     dx, [bp+Sum_City_Y_NonOwn_UnitCost]
add     ax, dx
mov     bx, ax
mov     [word ptr bx], 0
inc     [bp+landmass_idx]
loc_F761F:
cmp     [bp+landmass_idx], e_NUM_LANDMASSES
jl      short loc_F75CD
inc     wp
loc_F7626:
cmp     wp, e_NUM_PLANES
jl      short loc_F75C6
mov     [bp+itr], 0
jmp     loc_F7749
; ---------------------------------------------------------------------------
loc_F7633:
mov     ax, [bp+itr]
mov     dx, size s_CITY
imul    dx
les     bx, [_CITIES]
add     bx, ax
mov     al, [es:bx+s_CITY.owner_idx]
cbw
cmp     ax, [bp+player_idx]
jz      short loc_F764E
jmp     loc_F7746
; ---------------------------------------------------------------------------
loc_F764E:
mov     ax, [bp+itr]
mov     dx, size s_CITY
imul    dx
les     bx, [_CITIES]
add     bx, ax
mov     al, [es:bx+s_CITY.wp]
cbw
mov     wp, ax
mov     ax, [bp+itr]
mov     dx, size s_CITY
imul    dx
les     bx, [_CITIES]
add     bx, ax
mov     al, [es:bx+s_CITY.wy]
cbw
mov     dx, e_WORLD_WIDTH
imul    dx
push    ax
mov     ax, wp
mov     dx, e_WORLD_SIZE
imul    dx
les     bx, [_landmasses]
add     bx, ax
pop     ax
add     bx, ax
mov     ax, [bp+itr]
mov     dx, size s_CITY
imul    dx
push    es
les     si, [_CITIES]
add     si, ax
mov     al, [es:si+s_CITY.wx]
cbw
add     bx, ax
pop     es
mov     al, [es:bx]
mov     ah, 0
mov     [bp+landmass_idx], ax
cmp     [bp+landmass_idx], e_NUM_LANDMASSES
jl      short loc_F76B4
jmp     loc_F7746
; ---------------------------------------------------------------------------
loc_F76B4:
mov     ax, [bp+itr]
mov     dx, size s_CITY
imul    dx
les     bx, [_CITIES]
add     bx, ax
mov     al, [es:bx+s_CITY.wx]
cbw
push    ax
mov     ax, wp
mov     dx, 120
imul    dx
mov     dx, [bp+landmass_idx]
shl     dx, 1
add     ax, dx
lea     dx, [bp+Sum_City_X_Own_UnitCost]
add     ax, dx
mov     bx, ax
pop     ax
add     [bx], ax
mov     ax, [bp+itr]
mov     dx, size s_CITY
imul    dx
les     bx, [_CITIES]
add     bx, ax
mov     al, [es:bx+s_CITY.wy]
cbw
push    ax
mov     ax, wp
mov     dx, 120
imul    dx
mov     dx, [bp+landmass_idx]
shl     dx, 1
add     ax, dx
lea     dx, [bp+Sum_City_Y_NonOwn_UnitCost]
add     ax, dx
mov     bx, ax
pop     ax
add     [bx], ax
mov     ax, wp
mov     dx, size s_CONTINENTS_TABLE
imul    dx
push    ax
mov     ax, [bp+player_idx]
mov     dx, size s_LANDMASSES
imul    dx
pop     bx
add     bx, ax
add     bx, [bp+landmass_idx]
mov     al, [_ai_continents__0.Player_0.type_array+bx]
inc     al
push    ax
mov     ax, wp
mov     dx, size s_CONTINENTS_TABLE
imul    dx
push    ax
mov     ax, [bp+player_idx]
mov     dx, size s_LANDMASSES
imul    dx
pop     bx
add     bx, ax
add     bx, [bp+landmass_idx]
pop     ax
mov     [_ai_continents__0.Player_0.type_array+bx], al
loc_F7746:
inc     [bp+itr]
loc_F7749:
mov     ax, [bp+itr]
cmp     ax, [_cities]
jge     short loc_F7755
jmp     loc_F7633
; ---------------------------------------------------------------------------
loc_F7755:
xor     wp, wp
jmp     loc_F7805
; ---------------------------------------------------------------------------
loc_F775A:
mov     [bp+landmass_idx], 0
jmp     loc_F77FB
; ---------------------------------------------------------------------------
loc_F7762:
mov     ax, wp
mov     dx, size s_CONTINENTS_TABLE
imul    dx
push    ax
mov     ax, [bp+player_idx]
mov     dx, size s_LANDMASSES
imul    dx
pop     bx
add     bx, ax
add     bx, [bp+landmass_idx]
mov     al, [_ai_continents__0.Player_0.type_array+bx]
cbw
mov     [bp+city_count], ax
cmp     [bp+city_count], 0
jle     short loc_F77F8
mov     ax, wp
mov     dx, 120
imul    dx
mov     dx, [bp+landmass_idx]
shl     dx, 1
add     ax, dx
lea     dx, [bp+Sum_City_X_Own_UnitCost]
add     ax, dx
mov     bx, ax
mov     ax, [bx]
xor     dx, dx
div     [bp+city_count]
push    ax
mov     ax, wp
mov     dx, size s_CONTINENTS_TABLE
imul    dx
push    ax
mov     ax, [bp+player_idx]
mov     dx, size s_LANDMASSES
imul    dx
pop     bx
add     bx, ax
add     bx, [bp+landmass_idx]
pop     ax
mov     [byte ptr _ai_continents__0.Player_0.wx_array+bx], al
mov     ax, wp
mov     dx, 120
imul    dx
mov     dx, [bp+landmass_idx]
shl     dx, 1
add     ax, dx
lea     dx, [bp+Sum_City_Y_NonOwn_UnitCost]
add     ax, dx
mov     bx, ax
mov     ax, [bx]
xor     dx, dx
div     [bp+city_count]
push    ax
mov     ax, wp
mov     dx, size s_CONTINENTS_TABLE
imul    dx
push    ax
mov     ax, [bp+player_idx]
mov     dx, size s_LANDMASSES
imul    dx
pop     bx
add     bx, ax
add     bx, [bp+landmass_idx]
pop     ax
mov     [_ai_continents__0.Player_0.wy_array+bx], al
loc_F77F8:
inc     [bp+landmass_idx]
loc_F77FB:
cmp     [bp+landmass_idx], e_NUM_LANDMASSES
jge     short loc_F7804
jmp     loc_F7762
; ---------------------------------------------------------------------------
loc_F7804:
inc     wp
loc_F7805:
cmp     wp, e_NUM_PLANES
jge     short loc_F780D
jmp     loc_F775A
; ---------------------------------------------------------------------------
loc_F780D:
mov     [bp+itr], 0
jmp     short loc_F7853
; ---------------------------------------------------------------------------
loc_F7814:
mov     bx, [bp+itr]
shl     bx, 1
lea     ax, [bp+Sum_City_X_Own_UnitCost]
add     bx, ax
mov     [word ptr bx], 0
mov     bx, [bp+itr]
shl     bx, 1
lea     ax, [bp+Sum_City_X_Own_UnitCost+78h]
add     bx, ax
mov     [word ptr bx], 0
mov     bx, [bp+itr]
shl     bx, 1
lea     ax, [bp+Sum_City_Y_NonOwn_UnitCost]
add     bx, ax
mov     [word ptr bx], 0
mov     bx, [bp+itr]
shl     bx, 1
lea     ax, [bp+Sum_City_Y_NonOwn_UnitCost+78h]
add     bx, ax
mov     [word ptr bx], 0
inc     [bp+itr]
loc_F7853:
cmp     [bp+itr], e_NUM_LANDMASSES
jl      short loc_F7814
mov     [bp+itr], 0
jmp     loc_F793F
; ---------------------------------------------------------------------------
loc_F7861:
mov     ax, [bp+itr]
mov     cl, 5
shl     ax, cl
les     bx, [_UNITS]
add     bx, ax
mov     al, [es:bx+s_UNIT.type]
mov     ah, 0
mov     [bp+unit_type], ax
mov     ax, [bp+unit_type]
mov     dx, size s_UNIT_TYPE
imul    dx
mov     bx, ax
test    [_unit_type_table.Abilities+bx], UA_CREATEOUTPOST
jz      short loc_F788C
jmp     loc_F793C
; ---------------------------------------------------------------------------
loc_F788C:
mov     ax, [bp+itr]
mov     cl, 5
shl     ax, cl
les     bx, [_UNITS]
add     bx, ax
mov     al, [es:bx+s_UNIT.wp]
cbw
mov     wp, ax
mov     ax, [bp+itr]
mov     cl, 5
shl     ax, cl
les     bx, [_UNITS]
add     bx, ax
mov     al, [es:bx+s_UNIT.wy]
cbw
mov     dx, e_WORLD_WIDTH
imul    dx
push    ax
mov     ax, wp
mov     dx, e_WORLD_SIZE
imul    dx
les     bx, [_landmasses]
add     bx, ax
pop     ax
add     bx, ax
mov     ax, [bp+itr]
mov     cl, 5
shl     ax, cl
push    es
les     si, [_UNITS]
add     si, ax
mov     al, [es:si+s_UNIT.wx]
cbw
add     bx, ax
pop     es
mov     al, [es:bx]
mov     ah, 0
mov     [bp+landmass_idx], ax
cmp     [bp+landmass_idx], e_NUM_LANDMASSES
jge     short loc_F793C
mov     ax, [bp+itr]
mov     cl, 5
shl     ax, cl
les     bx, [_UNITS]
add     bx, ax
mov     al, [es:bx+s_UNIT.owner_idx]
cbw
cmp     ax, [bp+player_idx]
jnz     short loc_F7916
mov     ax, wp
mov     dx, 120
imul    dx
mov     dx, [bp+landmass_idx]
shl     dx, 1
add     ax, dx
lea     dx, [bp+Sum_City_X_Own_UnitCost]
jmp     short loc_F7928
; ---------------------------------------------------------------------------
loc_F7916:
mov     ax, wp
mov     dx, 120
imul    dx
mov     dx, [bp+landmass_idx]
shl     dx, 1
add     ax, dx
lea     dx, [bp+Sum_City_Y_NonOwn_UnitCost]
loc_F7928:
add     ax, dx
push    ax
mov     ax, [bp+unit_type]
mov     dx, size s_UNIT_TYPE
imul    dx
mov     bx, ax
mov     ax, [_unit_type_table.cost+bx]
pop     bx
add     [bx], ax
loc_F793C:
inc     [bp+itr]
loc_F793F:
mov     ax, [bp+itr]
cmp     ax, [_units]
jge     short loc_F794B
jmp     loc_F7861
; ---------------------------------------------------------------------------
loc_F794B:
xor     wp, wp
jmp     loc_F7A49
; ---------------------------------------------------------------------------
loc_F7950:
mov     [bp+itr], 0
jmp     loc_F7A3F
; ---------------------------------------------------------------------------
loc_F7958:
mov     ax, wp
mov     dx, size s_CONTINENTS_TABLE
imul    dx
push    ax
mov     ax, [bp+player_idx]
mov     dx, size s_LANDMASSES
imul    dx
pop     bx
add     bx, ax
add     bx, [bp+itr]
cmp     [_ai_continents__0.Player_0.type_array+bx], lmt_NoTargets
jnz     short loc_F7993
mov     ax, wp
mov     dx, 120
imul    dx
mov     dx, [bp+itr]
shl     dx, 1
add     ax, dx
lea     dx, [bp+Sum_City_Y_NonOwn_UnitCost]
add     ax, dx
mov     bx, ax
cmp     [word ptr bx], 0
jnz     short loc_F7993
jmp     loc_F7A3C
; ---------------------------------------------------------------------------
loc_F7993:
mov     ax, wp
mov     dx, size s_CONTINENTS_TABLE
imul    dx
push    ax
mov     ax, [bp+player_idx]
mov     dx, size s_LANDMASSES
imul    dx
pop     bx
add     bx, ax
add     bx, [bp+itr]
cmp     [_ai_continents__0.Player_0.type_array+bx], lmt_Invalid
jle     short loc_F7A21
mov     ax, wp
mov     dx, 120
imul    dx
mov     dx, [bp+itr]
shl     dx, 1
add     ax, dx
lea     dx, [bp+Sum_City_X_Own_UnitCost]
add     ax, dx
push    ax
mov     ax, wp
mov     dx, 120
imul    dx
mov     dx, [bp+itr]
shl     dx, 1
add     ax, dx
lea     dx, [bp+Sum_City_Y_NonOwn_UnitCost]
add     ax, dx
mov     bx, ax
mov     ax, [bx]
mov     dx, 10
imul    dx
pop     bx
cmp     [bx], ax
jbe     short loc_F7A04
mov     ax, wp
mov     dx, size s_CONTINENTS_TABLE
imul    dx
push    ax
mov     ax, [bp+player_idx]
mov     dx, size s_LANDMASSES
imul    dx
pop     bx
add     bx, ax
add     bx, [bp+itr]
mov     [_ai_continents__0.Player_0.type_array+bx], lmt_Own
jmp     short loc_F7A1F
; ---------------------------------------------------------------------------
loc_F7A04:
mov     ax, wp
mov     dx, size s_CONTINENTS_TABLE
imul    dx
push    ax
mov     ax, [bp+player_idx]
mov     dx, size s_LANDMASSES
imul    dx
pop     bx
add     bx, ax
add     bx, [bp+itr]
mov     [_ai_continents__0.Player_0.type_array+bx], lmt_Contested
loc_F7A1F:
jmp     short loc_F7A3C
; ---------------------------------------------------------------------------
loc_F7A21:
mov     ax, wp
mov     dx, size s_CONTINENTS_TABLE
imul    dx
push    ax
mov     ax, [bp+player_idx]
mov     dx, size s_LANDMASSES
imul    dx
pop     bx
add     bx, ax
add     bx, [bp+itr]
mov     [_ai_continents__0.Player_0.type_array+bx], lmt_NoPresence
loc_F7A3C:
inc     [bp+itr]
loc_F7A3F:
cmp     [bp+itr], e_NUM_LANDMASSES
jge     short loc_F7A48
jmp     loc_F7958
; ---------------------------------------------------------------------------
loc_F7A48:
inc     wp
loc_F7A49:
cmp     wp, e_NUM_PLANES
jge     short loc_F7A51
jmp     loc_F7950
; ---------------------------------------------------------------------------
loc_F7A51:
mov     [bp+itr], 0
jmp     loc_F7B4F
; ---------------------------------------------------------------------------
loc_F7A59:
mov     ax, [bp+itr]
mov     dx, size s_CITY
imul    dx
les     bx, [_CITIES]
add     bx, ax
mov     al, [es:bx+s_CITY.owner_idx]
cbw
cmp     ax, [bp+player_idx]
jnz     short loc_F7A74
jmp     loc_F7B4C
; ---------------------------------------------------------------------------
loc_F7A74:
mov     ax, [bp+itr]
mov     dx, size s_CITY
imul    dx
les     bx, [_CITIES]
add     bx, ax
cmp     [es:bx+s_CITY.owner_idx], e_NEUTRAL_PLAYER_IDX
jnz     short loc_F7A8C
jmp     loc_F7B4C
; ---------------------------------------------------------------------------
loc_F7A8C:
mov     ax, [bp+itr]
mov     dx, size s_CITY
imul    dx
les     bx, [_CITIES]
add     bx, ax
mov     al, [es:bx+s_CITY.owner_idx]
cbw
push    ax
mov     ax, [bp+player_idx]
mov     dx, size s_WIZARD
imul    dx
pop     dx
add     ax, dx
mov     bx, ax
cmp     [_players.Dipl.Dipl_Status+bx], DIPL_Alliance
jz      short loc_F7AB7
jmp     loc_F7B4C
; ---------------------------------------------------------------------------
loc_F7AB7:
mov     ax, [bp+itr]
mov     dx, size s_CITY
imul    dx
les     bx, [_CITIES]
add     bx, ax
mov     al, [es:bx+s_CITY.wp]
cbw
mov     wp, ax
mov     ax, [bp+itr]
mov     dx, size s_CITY
imul    dx
les     bx, [_CITIES]
add     bx, ax
mov     al, [es:bx+s_CITY.wy]
cbw
mov     dx, e_WORLD_WIDTH
imul    dx
push    ax
mov     ax, wp
mov     dx, e_WORLD_SIZE
imul    dx
les     bx, [_landmasses]
add     bx, ax
pop     ax
add     bx, ax
mov     ax, [bp+itr]
mov     dx, size s_CITY
imul    dx
push    es
les     si, [_CITIES]
add     si, ax
mov     al, [es:si+s_CITY.wx]
cbw
add     bx, ax
pop     es
mov     al, [es:bx]
mov     ah, 0
mov     [bp+landmass_idx], ax
mov     ax, wp
mov     dx, size s_CONTINENTS_TABLE
imul    dx
push    ax
mov     ax, [bp+player_idx]
mov     dx, size s_LANDMASSES
imul    dx
pop     bx
add     bx, ax
add     bx, [bp+landmass_idx]
cmp     [_ai_continents__0.Player_0.type_array+bx], lmt_NoPresence
jnz     short loc_F7B4C
mov     ax, wp
mov     dx, size s_CONTINENTS_TABLE
imul    dx
push    ax
mov     ax, [bp+player_idx]
mov     dx, size s_LANDMASSES
imul    dx
pop     bx
add     bx, ax
add     bx, [bp+landmass_idx]
mov     [_ai_continents__0.Player_0.type_array+bx], lmt_NoLanding
loc_F7B4C:
inc     [bp+itr]
loc_F7B4F:
mov     ax, [bp+itr]
cmp     ax, [_cities]
jge     short loc_F7B5B
jmp     loc_F7A59
; ---------------------------------------------------------------------------
loc_F7B5B:
mov     ax, [bp+player_idx]
mov     dx, size s_LANDMASSES
imul    dx
mov     bx, ax
mov     [_ai_continents__0.Player_0.type_array+bx], lmt_NoLanding
mov     ax, [bp+player_idx]
mov     dx, size s_LANDMASSES
imul    dx
mov     bx, ax
mov     [_ai_continents__1.Player_0.type_array+bx], lmt_NoLanding
xor     wp, wp
jmp     loc_F7D16
; ---------------------------------------------------------------------------
loc_F7B7E:
mov     [bp+Best_Tile_Weight], 1000
mov     [bp+landmass_idx], 1
jmp     loc_F7CB4
; ---------------------------------------------------------------------------
loc_F7B8B:
mov     ax, wp
mov     dx, size s_CONTINENTS_TABLE
imul    dx
push    ax
mov     ax, [bp+player_idx]
mov     dx, size s_LANDMASSES
imul    dx
pop     bx
add     bx, ax
add     bx, [bp+landmass_idx]
cmp     [_ai_continents__0.Player_0.type_array+bx], lmt_NoPresence
jz      short loc_F7BAB
jmp     loc_F7CB1
; ---------------------------------------------------------------------------
loc_F7BAB:
mov     ax, wp
mov     dx, 120
imul    dx
mov     dx, [bp+landmass_idx]
shl     dx, 1
add     ax, dx
lea     dx, [bp+Sum_City_Y_NonOwn_UnitCost]
add     ax, dx
mov     bx, ax
cmp     [word ptr bx], 0
jz      short loc_F7BC9
jmp     loc_F7CB1
; ---------------------------------------------------------------------------
loc_F7BC9:
mov     bx, wp
mov     cl, 2
shl     bx, cl
les     bx, [CONTX_FirstTiles+bx]
mov     ax, [bp+landmass_idx]
jmp     loc_F7C98
; ---------------------------------------------------------------------------
loc_F7BD9:
mov     bx, wp
mov     cl, 2
shl     bx, cl
les     bx, [CONTX_TileXs+bx]
add     bx, [bp+landmass_node_index]
mov     al, [es:bx]
cbw
mov     [bp+Tile_X], ax
mov     bx, wp
mov     cl, 2
shl     bx, cl
les     bx, [CONTX_TileYs+bx]
add     bx, [bp+landmass_node_index]
mov     al, [es:bx]
cbw
mov     [bp+Tile_Y], ax
mov     ax, [bp+Tile_Y]
mov     dx, e_WORLD_WIDTH
imul    dx
add     ax, [bp+Tile_X]
mov     [bp+Tile_Linear], ax
mov     bx, wp
mov     cl, 2
shl     bx, cl
les     bx, [g_ai_evaluation_map+bx]
mov     ax, [bp+Tile_Linear]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], 0
jnz     short loc_F7C8B
mov     ax, e_WORLD_WIDTH
push    ax
push    [bp+Y_2]
push    [bp+X_2]
push    [bp+Tile_Y]
push    [bp+Tile_X]
call    Delta_XY_With_Wrap
add     sp, 0Ah
push    ax
mov     ax, 20
push    ax
call    Random
pop     cx
pop     dx
add     dx, ax
mov     [bp+Tile_Weight], dx
mov     ax, [bp+Tile_Weight]
cmp     ax, [bp+Best_Tile_Weight]
jge     short loc_F7C8B
mov     ax, [bp+Tile_Weight]
mov     [bp+Best_Tile_Weight], ax
mov     bx, wp
mov     cl, 2
shl     bx, cl
les     bx, [CONTX_TileXs+bx]
add     bx, [bp+landmass_node_index]
mov     al, [es:bx]
cbw
mov     [bp+Target_Tile_X], ax
mov     bx, wp
mov     cl, 2
shl     bx, cl
les     bx, [CONTX_TileYs+bx]
add     bx, [bp+landmass_node_index]
mov     al, [es:bx]
cbw
mov     [bp+Target_Tile_Y], ax
mov     ax, [bp+landmass_idx]
mov     [bp+Target_Tile_Landmass], ax
loc_F7C8B:
mov     bx, wp
mov     cl, 2
shl     bx, cl
les     bx, [CONTX_TileChain+bx]
mov     ax, [bp+landmass_node_index]
loc_F7C98:
shl     ax, 1
add     bx, ax
mov     ax, [es:bx]
mov     [bp+landmass_node_index], ax
cmp     [bp+landmass_node_index], e_ST_UNDEFINED
jz      short loc_F7CB1
cmp     [bp+found_target], e_ST_FALSE
jnz     short loc_F7CB1
jmp     loc_F7BD9
; ---------------------------------------------------------------------------
loc_F7CB1:
inc     [bp+landmass_idx]
loc_F7CB4:
cmp     [bp+landmass_idx], e_NUM_LANDMASSES
jge     short loc_F7CBD
jmp     loc_F7B8B
; ---------------------------------------------------------------------------
loc_F7CBD:
cmp     [bp+Best_Tile_Weight], 1000
jge     short loc_F7D02
mov     bx, wp
shl     bx, 1
mov     bx, [AI_NewColConts+bx]
mov     ax, [bp+player_idx]
shl     ax, 1
add     bx, ax
mov     ax, [bp+Target_Tile_Landmass]
mov     [bx], ax
mov     bx, wp
shl     bx, 1
mov     bx, [AI_NewColTgtXs+bx]
mov     ax, [bp+player_idx]
shl     ax, 1
add     bx, ax
mov     ax, [bp+Target_Tile_X]
mov     [bx], ax
mov     bx, wp
shl     bx, 1
mov     bx, [AI_NewColTgtYs+bx]
mov     ax, [bp+player_idx]
shl     ax, 1
add     bx, ax
mov     ax, [bp+Target_Tile_Y]
mov     [bx], ax
jmp     short loc_F7D15
; ---------------------------------------------------------------------------
loc_F7D02:
mov     bx, wp
shl     bx, 1
mov     bx, [AI_NewColConts+bx]
mov     ax, [bp+player_idx]
shl     ax, 1
add     bx, ax
mov     [word ptr bx], 0
loc_F7D15:
inc     wp
loc_F7D16:
cmp     wp, 2
jge     short loc_F7D1E
jmp     loc_F7B7E
; ---------------------------------------------------------------------------
loc_F7D1E:
xor     wp, wp
jmp     loc_F7FA2
; ---------------------------------------------------------------------------
loc_F7D23:
mov     [bp+landmass_idx], 1
jmp     loc_F7F98
; ---------------------------------------------------------------------------
loc_F7D2B:
mov     ax, wp
mov     dx, size s_CONTINENTS_TABLE
imul    dx
push    ax
mov     ax, [bp+player_idx]
mov     dx, size s_LANDMASSES
imul    dx
pop     bx
add     bx, ax
add     bx, [bp+landmass_idx]
cmp     [_ai_continents__0.Player_0.type_array+bx], lmt_Contested
jz      short loc_F7D85
mov     ax, wp
mov     dx, size s_CONTINENTS_TABLE
imul    dx
push    ax
mov     ax, [bp+player_idx]
mov     dx, size s_LANDMASSES
imul    dx
pop     bx
add     bx, ax
add     bx, [bp+landmass_idx]
loc_F7D5E:
cmp     [_ai_continents__0.Player_0.type_array+bx], lmt_Own
jz      short loc_F7D85
mov     ax, wp
mov     dx, size s_CONTINENTS_TABLE
imul    dx
push    ax
mov     ax, [bp+player_idx]
mov     dx, size s_LANDMASSES
imul    dx
pop     bx
add     bx, ax
add     bx, [bp+landmass_idx]
cmp     [_ai_continents__0.Player_0.type_array+bx], lmt_NoLanding
jz      short loc_F7D85
jmp     loc_F7F95
; ---------------------------------------------------------------------------
loc_F7D85:
mov     ax, wp
mov     dx, size s_CONTINENTS_TABLE
imul    dx
push    ax
mov     ax, [bp+player_idx]
mov     dx, size s_LANDMASSES
imul    dx
pop     bx
add     bx, ax
add     bx, [bp+landmass_idx]
mov     al, [byte ptr _ai_continents__0.Player_0.wx_array+bx]
cbw
mov     [bp+Center_X], ax
mov     ax, wp
mov     dx, size s_CONTINENTS_TABLE
imul    dx
push    ax
mov     ax, [bp+player_idx]
mov     dx, size s_LANDMASSES
imul    dx
pop     bx
add     bx, ax
add     bx, [bp+landmass_idx]
mov     al, [_ai_continents__0.Player_0.wy_array+bx]
cbw
mov     [bp+Center_Y], ax
mov     bx, wp
mov     cl, 2
shl     bx, cl
les     bx, [g_ai_evaluation_map+bx]
mov     ax, [bp+Center_Y]
mov     dx, e_WORLD_WIDTH
imul    dx
add     ax, [bp+Center_X]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], 0
jnz     short loc_F7E07
mov     ax, wp
mov     dx, e_WORLD_SIZE
imul    dx
les     bx, [_landmasses]
add     bx, ax
mov     ax, [bp+Center_Y]
mov     dx, e_WORLD_WIDTH
imul    dx
add     bx, ax
add     bx, [bp+Center_X]
mov     al, [es:bx]
mov     ah, 0
cmp     ax, [bp+landmass_idx]
jnz     short loc_F7E07
jmp     loc_F7F95
; ---------------------------------------------------------------------------
loc_F7E07:
mov     [bp+Best_Tile_Weight], 1000
mov     bx, wp
mov     cl, 2
shl     bx, cl
les     bx, [CONTX_FirstTiles+bx]
mov     ax, [bp+landmass_idx]
jmp     loc_F7EEE
; ---------------------------------------------------------------------------
loc_F7E1C:
mov     ax, e_WORLD_WIDTH
push    ax
push    [bp+Center_Y]
push    [bp+Center_X]
mov     bx, wp
mov     cl, 2
shl     bx, cl
les     bx, [CONTX_TileYs+bx]
add     bx, [bp+landmass_node_index]
mov     al, [es:bx]
cbw
push    ax
mov     bx, wp
mov     cl, 2
shl     bx, cl
les     bx, [CONTX_TileXs+bx]
add     bx, [bp+landmass_node_index]
mov     al, [es:bx]
cbw
push    ax
call    Delta_XY_With_Wrap
add     sp, 0Ah
mov     [bp+Tile_Weight], ax
mov     ax, [bp+Tile_Weight]
cmp     ax, [bp+Best_Tile_Weight]
jl      short loc_F7E60
jmp     loc_F7EE1
; ---------------------------------------------------------------------------
loc_F7E60:
mov     bx, wp
mov     cl, 2
shl     bx, cl
les     bx, [CONTX_TileXs+bx]
add     bx, [bp+landmass_node_index]
mov     al, [es:bx]
cbw
mov     [bp+Tile_X], ax
mov     bx, wp
mov     cl, 2
shl     bx, cl
les     bx, [CONTX_TileYs+bx]
add     bx, [bp+landmass_node_index]
mov     al, [es:bx]
cbw
mov     [bp+Tile_Y], ax
mov     bx, wp
mov     cl, 2
shl     bx, cl
les     bx, [g_ai_evaluation_map+bx]
mov     ax, [bp+Tile_Y]
mov     dx, e_WORLD_WIDTH
imul    dx
add     ax, [bp+Tile_X]
shl     ax, 1
add     bx, ax
mov     ax, [es:bx]
mov     [bp+found_target], ax
cmp     [bp+found_target], e_ST_FALSE
jnz     short loc_F7EE1
mov     ax, [bp+Tile_Weight]
mov     [bp+Best_Tile_Weight], ax
mov     bx, wp
mov     cl, 2
shl     bx, cl
les     bx, [CONTX_TileXs+bx]
add     bx, [bp+landmass_node_index]
mov     al, [es:bx]
cbw
mov     [bp+Target_Tile_X], ax
mov     bx, wp
mov     cl, 2
shl     bx, cl
les     bx, [CONTX_TileYs+bx]
add     bx, [bp+landmass_node_index]
mov     al, [es:bx]
cbw
mov     [bp+Target_Tile_Y], ax
mov     ax, [bp+landmass_idx]
mov     [bp+Target_Tile_Landmass], ax
loc_F7EE1:
mov     bx, wp
mov     cl, 2
shl     bx, cl
les     bx, [CONTX_TileChain+bx]
mov     ax, [bp+landmass_node_index]
loc_F7EEE:
shl     ax, 1
add     bx, ax
mov     ax, [es:bx]
mov     [bp+landmass_node_index], ax
cmp     [bp+landmass_node_index], e_ST_UNDEFINED
jz      short loc_F7F01
jmp     loc_F7E1C
; ---------------------------------------------------------------------------
loc_F7F01:
cmp     [bp+Best_Tile_Weight], 1000
jge     short loc_F7F44
mov     ax, wp
mov     dx, size s_CONTINENTS_TABLE
imul    dx
push    ax
mov     ax, [bp+player_idx]
mov     dx, size s_LANDMASSES
imul    dx
pop     bx
add     bx, ax
add     bx, [bp+landmass_idx]
mov     al, [byte ptr bp+Target_Tile_X]
mov     [byte ptr _ai_continents__0.Player_0.wx_array+bx], al
mov     ax, wp
mov     dx, size s_CONTINENTS_TABLE
imul    dx
push    ax
mov     ax, [bp+player_idx]
mov     dx, size s_LANDMASSES
imul    dx
pop     bx
add     bx, ax
add     bx, [bp+landmass_idx]
mov     al, [byte ptr bp+Target_Tile_Y]
mov     [_ai_continents__0.Player_0.wy_array+bx], al
jmp     short loc_F7F95
; ---------------------------------------------------------------------------
loc_F7F44:
mov     ax, wp
mov     dx, size s_CONTINENTS_TABLE
imul    dx
push    ax
mov     ax, [bp+player_idx]
mov     dx, size s_LANDMASSES
imul    dx
pop     bx
add     bx, ax
add     bx, [bp+landmass_idx]
mov     [byte ptr _ai_continents__0.Player_0.wx_array+bx], 0
mov     ax, wp
mov     dx, size s_CONTINENTS_TABLE
imul    dx
push    ax
mov     ax, [bp+player_idx]
mov     dx, size s_LANDMASSES
imul    dx
pop     bx
add     bx, ax
add     bx, [bp+landmass_idx]
mov     [_ai_continents__0.Player_0.wy_array+bx], 0
mov     ax, wp
mov     dx, size s_CONTINENTS_TABLE
imul    dx
push    ax
mov     ax, [bp+player_idx]
mov     dx, size s_LANDMASSES
imul    dx
pop     bx
add     bx, ax
add     bx, [bp+landmass_idx]
mov     [_ai_continents__0.Player_0.type_array+bx], lmt_NoLanding
loc_F7F95:
inc     [bp+landmass_idx]
loc_F7F98:
cmp     [bp+landmass_idx], e_WORLD_WIDTH
jge     short loc_F7FA1
jmp     loc_F7D2B
; ---------------------------------------------------------------------------
loc_F7FA1:
inc     wp
loc_F7FA2:
cmp     wp, e_NUM_PLANES
jge     short loc_F7FAA
jmp     loc_F7D23
; ---------------------------------------------------------------------------
loc_F7FAA:
xor     wp, wp
loc_F7FAC:
jmp     loc_F8195
; ---------------------------------------------------------------------------
loc_F7FAF:
mov     [bp+landmass_idx], 1
jmp     loc_F818B
; ---------------------------------------------------------------------------
loc_F7FB7:
mov     ax, wp
mov     dx, size s_CONTINENTS_TABLE
imul    dx
push    ax
mov     ax, [bp+player_idx]
mov     dx, size s_LANDMASSES
imul    dx
pop     bx
add     bx, ax
add     bx, [bp+landmass_idx]
cmp     [_ai_continents__0.Player_0.type_array+bx], lmt_NoPresence
jz      short loc_F7FD7
jmp     loc_F8188
; ---------------------------------------------------------------------------
loc_F7FD7:
mov     ax, wp
mov     dx, size s_CONTINENTS_TABLE
imul    dx
push    ax
mov     ax, [bp+player_idx]
mov     dx, size s_LANDMASSES
imul    dx
pop     bx
add     bx, ax
add     bx, [bp+landmass_idx]
mov     al, [byte ptr _ai_continents__0.Player_0.wx_array+bx]
cbw
mov     [bp+Center_X], ax
mov     ax, wp
mov     dx, size s_CONTINENTS_TABLE
imul    dx
push    ax
mov     ax, [bp+player_idx]
mov     dx, size s_LANDMASSES
imul    dx
pop     bx
add     bx, ax
add     bx, [bp+landmass_idx]
mov     al, [_ai_continents__0.Player_0.wy_array+bx]
cbw
mov     [bp+Center_Y], ax
mov     [bp+Best_Tile_Weight], 1000
mov     bx, wp
mov     cl, 2
shl     bx, cl
les     bx, [CONTX_1stLoadTs+bx]
mov     ax, [bp+landmass_idx]
jmp     loc_F80E1
; ---------------------------------------------------------------------------
loc_F8028:
mov     bx, wp
mov     cl, 2
shl     bx, cl
les     bx, [g_world_embark_square_wx__load_init+bx]
add     bx, [bp+landmass_node_index]
mov     al, [es:bx]
cbw
mov     [bp+Tile_X], ax
mov     bx, wp
mov     cl, 2
shl     bx, cl
les     bx, [_ai_landmass_dock_squares_wy_array+bx]
add     bx, [bp+landmass_node_index]
mov     al, [es:bx]
cbw
mov     [bp+Tile_Y], ax
mov     bx, wp
mov     cl, 2
shl     bx, cl
les     bx, [g_ai_evaluation_map+bx]
mov     ax, [bp+Tile_Y]
mov     dx, e_WORLD_WIDTH
imul    dx
add     ax, [bp+Tile_X]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], 0
jnz     short loc_F80D4
mov     ax, e_WORLD_WIDTH
push    ax
push    [bp+Center_Y]
push    [bp+Center_X]
push    [bp+Tile_Y]
push    [bp+Tile_X]
call    Delta_XY_With_Wrap
add     sp, 0Ah
push    ax
mov     ax, 20
push    ax
call    Random
pop     cx
pop     dx
add     dx, ax
mov     [bp+Tile_Weight], dx
mov     ax, [bp+Tile_Weight]
cmp     ax, [bp+Best_Tile_Weight]
jge     short loc_F80D4
mov     ax, [bp+Tile_Weight]
mov     [bp+Best_Tile_Weight], ax
mov     bx, wp
mov     cl, 2
shl     bx, cl
les     bx, [g_world_embark_square_wx__load_init+bx]
add     bx, [bp+landmass_node_index]
mov     al, [es:bx]
cbw
mov     [bp+Target_Tile_X], ax
mov     bx, wp
mov     cl, 2
shl     bx, cl
loc_F80C0:
les     bx, [_ai_landmass_dock_squares_wy_array+bx]
add     bx, [bp+landmass_node_index]
mov     al, [es:bx]
cbw
mov     [bp+Target_Tile_Y], ax
mov     ax, [bp+landmass_idx]
mov     [bp+Target_Tile_Landmass], ax
loc_F80D4:
mov     bx, wp
mov     cl, 2
shl     bx, cl
les     bx, [CONTX_LoadTChain+bx]
mov     ax, [bp+landmass_node_index]
loc_F80E1:
shl     ax, 1
add     bx, ax
mov     ax, [es:bx]
mov     [bp+landmass_node_index], ax
cmp     [bp+landmass_node_index], e_ST_UNDEFINED
jz      short loc_F80F4
jmp     loc_F8028
; ---------------------------------------------------------------------------
loc_F80F4:
cmp     [bp+Best_Tile_Weight], 1000
jge     short loc_F8137
mov     ax, wp
mov     dx, size s_CONTINENTS_TABLE
imul    dx
push    ax
mov     ax, [bp+player_idx]
mov     dx, size s_LANDMASSES
imul    dx
pop     bx
add     bx, ax
add     bx, [bp+landmass_idx]
mov     al, [byte ptr bp+Target_Tile_X]
mov     [byte ptr _ai_continents__0.Player_0.wx_array+bx], al
mov     ax, wp
mov     dx, size s_CONTINENTS_TABLE
imul    dx
push    ax
mov     ax, [bp+player_idx]
loc_F8123:
mov     dx, size s_LANDMASSES
imul    dx
pop     bx
add     bx, ax
add     bx, [bp+landmass_idx]
loc_F812E:
mov     al, [byte ptr bp+Target_Tile_Y]
mov     [_ai_continents__0.Player_0.wy_array+bx], al
jmp     short loc_F8188
; ---------------------------------------------------------------------------
loc_F8137:
mov     ax, wp
loc_F8139:
mov     dx, size s_CONTINENTS_TABLE
imul    dx
push    ax
mov     ax, [bp+player_idx]
loc_F8142:
mov     dx, size s_LANDMASSES
imul    dx
pop     bx
add     bx, ax
add     bx, [bp+landmass_idx]
loc_F814D:
mov     [byte ptr _ai_continents__0.Player_0.wx_array+bx], 0
mov     ax, wp
mov     dx, size s_CONTINENTS_TABLE
loc_F8157:
imul    dx
push    ax
mov     ax, [bp+player_idx]
mov     dx, size s_LANDMASSES
imul    dx
pop     bx
add     bx, ax
add     bx, [bp+landmass_idx]
mov     [_ai_continents__0.Player_0.wy_array+bx], 0
mov     ax, wp
mov     dx, size s_CONTINENTS_TABLE
imul    dx
push    ax
mov     ax, [bp+player_idx]
mov     dx, size s_LANDMASSES
imul    dx
pop     bx
add     bx, ax
add     bx, [bp+landmass_idx]
mov     [_ai_continents__0.Player_0.type_array+bx], lmt_NoLanding
loc_F8188:
inc     [bp+landmass_idx]
loc_F818B:
cmp     [bp+landmass_idx], e_NUM_LANDMASSES
loc_F818F:
jge     short loc_F8194
loc_F8191:
jmp     loc_F7FB7
; ---------------------------------------------------------------------------
loc_F8194:
inc     wp
loc_F8195:
cmp     wp, e_NUM_PLANES
jge     short loc_F819D
jmp     loc_F7FAF
; ---------------------------------------------------------------------------
loc_F819D:
xor     wp, wp
jmp     loc_F8388
; ---------------------------------------------------------------------------
loc_F81A2:
mov     bx, wp
shl     bx, 1
mov     bx, [_ai_landmass_war_targets+bx]
mov     ax, [bp+player_idx]
shl     ax, 1
add     bx, ax
cmp     [word ptr bx], 0
jnz     short loc_F81B9
jmp     loc_F8387
; ---------------------------------------------------------------------------
loc_F81B9:
mov     [bp+landmass_idx], 1
jmp     loc_F837E
; ---------------------------------------------------------------------------
loc_F81C1:
mov     ax, wp
mov     dx, size s_CONTINENTS_TABLE
imul    dx
push    ax
mov     ax, [bp+player_idx]
mov     dx, size s_LANDMASSES
imul    dx
pop     bx
add     bx, ax
add     bx, [bp+landmass_idx]
cmp     [_ai_continents__0.Player_0.type_array+bx], lmt_Own
jz      short loc_F81E1
jmp     loc_F837B
; ---------------------------------------------------------------------------
loc_F81E1:
mov     [bp+X_2], 0
mov     [bp+Y_2], 0
mov     [bp+Tile_Linear], 0
mov     bx, wp
shl     bx, 1
mov     bx, [_ai_landmass_war_targets+bx]
mov     ax, [bp+player_idx]
shl     ax, 1
add     bx, ax
mov     ax, [bx]
shl     ax, 1
mov     bx, wp
mov     cl, 2
shl     bx, cl
les     bx, [CONTX_1stLoadTs+bx]
jmp     short loc_F8249
; ---------------------------------------------------------------------------
loc_F820F:
mov     bx, wp
mov     cl, 2
shl     bx, cl
les     bx, [g_world_embark_square_wx__load_init+bx]
add     bx, [bp+landmass_node_index]
mov     al, [es:bx]
cbw
add     [bp+X_2], ax
mov     bx, wp
mov     cl, 2
shl     bx, cl
les     bx, [_ai_landmass_dock_squares_wy_array+bx]
add     bx, [bp+landmass_node_index]
mov     al, [es:bx]
cbw
add     [bp+Y_2], ax
inc     [bp+Tile_Linear]
mov     bx, wp
mov     cl, 2
shl     bx, cl
les     bx, [CONTX_LoadTChain+bx]
mov     ax, [bp+landmass_node_index]
shl     ax, 1
loc_F8249:
add     bx, ax
mov     ax, [es:bx]
mov     [bp+landmass_node_index], ax
cmp     [bp+landmass_node_index], e_ST_UNDEFINED
jnz     short loc_F820F
mov     ax, [bp+X_2]
cwd
idiv    [bp+Tile_Linear]
mov     [bp+X_2], ax
mov     ax, [bp+Y_2]
cwd
idiv    [bp+Tile_Linear]
mov     [bp+Y_2], ax
mov     [bp+Best_Tile_Weight], 1000
mov     bx, wp
mov     cl, 2
shl     bx, cl
les     bx, [CONTX_1stLoadTs+bx]
mov     ax, [bp+landmass_idx]
jmp     loc_F8327
; ---------------------------------------------------------------------------
loc_F8280:
mov     ax, e_WORLD_WIDTH
push    ax
push    [bp+Y_2]
push    [bp+X_2]
mov     bx, wp
mov     cl, 2
shl     bx, cl
les     bx, [_ai_landmass_dock_squares_wy_array+bx]
add     bx, [bp+landmass_node_index]
mov     al, [es:bx]
cbw
push    ax
mov     bx, wp
mov     cl, 2
shl     bx, cl
les     bx, [g_world_embark_square_wx__load_init+bx]
add     bx, [bp+landmass_node_index]
mov     al, [es:bx]
cbw
push    ax
call    Delta_XY_With_Wrap
add     sp, 0Ah
mov     [bp+Tile_Weight], ax
mov     ax, [bp+Tile_Weight]
cmp     ax, [bp+Best_Tile_Weight]
jge     short loc_F831A
mov     bx, wp
mov     cl, 2
shl     bx, cl
les     bx, [g_world_embark_square_wx__load_init+bx]
add     bx, [bp+landmass_node_index]
mov     al, [es:bx]
cbw
mov     [bp+Tile_X], ax
mov     bx, wp
mov     cl, 2
shl     bx, cl
les     bx, [_ai_landmass_dock_squares_wy_array+bx]
add     bx, [bp+landmass_node_index]
mov     al, [es:bx]
cbw
mov     [bp+Tile_Y], ax
mov     bx, wp
mov     cl, 2
shl     bx, cl
les     bx, [g_ai_evaluation_map+bx]
mov     ax, [bp+Tile_Y]
mov     dx, e_WORLD_WIDTH
imul    dx
add     ax, [bp+Tile_X]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], 0
jnz     short loc_F831A
mov     ax, [bp+Tile_Weight]
mov     [bp+Best_Tile_Weight], ax
mov     ax, [bp+Tile_X]
mov     [bp+Target_Tile_X], ax
mov     ax, [bp+Tile_Y]
mov     [bp+Target_Tile_Y], ax
loc_F831A:
mov     bx, wp
mov     cl, 2
shl     bx, cl
les     bx, [CONTX_LoadTChain+bx]
mov     ax, [bp+landmass_node_index]
loc_F8327:
shl     ax, 1
add     bx, ax
mov     ax, [es:bx]
mov     [bp+landmass_node_index], ax
cmp     [bp+landmass_node_index], e_ST_UNDEFINED
jz      short loc_F833A
jmp     loc_F8280
; ---------------------------------------------------------------------------
loc_F833A:
cmp     [bp+Best_Tile_Weight], 1000
jge     short loc_F837B
mov     ax, wp
mov     dx, size s_CONTINENTS_TABLE
imul    dx
push    ax
mov     ax, [bp+player_idx]
mov     dx, size s_LANDMASSES
imul    dx
pop     bx
add     bx, ax
add     bx, [bp+landmass_idx]
mov     al, [byte ptr bp+Target_Tile_X]
mov     [byte ptr _ai_continents__0.Player_0.wx_array+bx], al
mov     ax, wp
mov     dx, size s_CONTINENTS_TABLE
imul    dx
push    ax
mov     ax, [bp+player_idx]
mov     dx, size s_LANDMASSES
imul    dx
pop     bx
add     bx, ax
add     bx, [bp+landmass_idx]
mov     al, [byte ptr bp+Target_Tile_Y]
mov     [_ai_continents__0.Player_0.wy_array+bx], al
loc_F837B:
inc     [bp+landmass_idx]
loc_F837E:
cmp     [bp+landmass_idx], e_NUM_LANDMASSES
jge     short loc_F8387
jmp     loc_F81C1
; ---------------------------------------------------------------------------
loc_F8387:
inc     wp
loc_F8388:
cmp     wp, e_NUM_PLANES
jge     short loc_F8390
jmp     loc_F81A2
; ---------------------------------------------------------------------------
loc_F8390:
call    EMMDATAH_Map
loc_F8395:
pop     wp
pop     si
mov     sp, bp
pop     bp
retf
endp AI_Continent_Reeval__WIP
