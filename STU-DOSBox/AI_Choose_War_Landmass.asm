;Attributes: bp-based frame
proc AI_Choose_War_Landmass far
Continent_Values_2= word ptr -10Ah
Continent_Values= word ptr -92h
city_owner_idx= word ptr -1Ah
First_Hostile_Player= word ptr -18h
Reevaluate= word ptr -16h
landmas_idx= word ptr -14h
Own_City_Count__distance= word ptr -12h
Transport_Tile_Y= word ptr -10h
Transport_Tile_X= word ptr -0Eh
Value_Sum= word ptr -0Ch
itr_landmasses= word ptr -0Ah
Closest_LoadTile_Dist= word ptr -8
wp= word ptr -6
Territory_Center_Y= word ptr -4
Territory_Center_X= word ptr -2
player_idx= word ptr  6
_DI_itr_players__cities__landmasses = di
push    bp
mov     bp, sp
sub     sp, 10Ah
push    si
push    _DI_itr_players__cities__landmasses
mov     [bp+First_Hostile_Player], e_ST_UNDEFINED
call    j_EMM_Map_CONTXXX__WIP
xor     _DI_itr_players__cities__landmasses, _DI_itr_players__cities__landmasses
jmp     short loc_F8842
loc_F881B:
mov     ax, [bp+player_idx]
mov     dx, size s_WIZARD
imul    dx
add     ax, _DI_itr_players__cities__landmasses
mov     bx, ax
cmp     [_players.Dipl.Dipl_Status+bx], 3
jge     short loc_F883E
mov     ax, _DI_itr_players__cities__landmasses
mov     dx, size s_WIZARD
imul    dx
mov     bx, ax
cmp     [_players.casting_spell_idx+bx], spl_Fire_Elemental
jnz     short loc_F8841
loc_F883E:
mov     [bp+First_Hostile_Player], _DI_itr_players__cities__landmasses
loc_F8841:
inc     _DI_itr_players__cities__landmasses
loc_F8842:
cmp     _DI_itr_players__cities__landmasses, [_num_players]
jge     short loc_F884E
cmp     [bp+First_Hostile_Player], e_ST_UNDEFINED
jz      short loc_F881B
loc_F884E:
mov     [bp+wp], 0
jmp     loc_F8F76
loc_F8856:
mov     [bp+Reevaluate], e_ST_FALSE
mov     bx, [bp+wp]
shl     bx, 1
mov     bx, [_ai_landmass_war_targets+bx]
mov     ax, [bp+player_idx]
shl     ax, 1
add     bx, ax
mov     ax, [bx]
mov     [bp+landmas_idx], ax
cmp     [bp+landmas_idx], 0
jnz     short loc_F887E
mov     [bp+Reevaluate], e_ST_TRUE
jmp     loc_F89DA
loc_F887E:
mov     ax, [bp+wp]
mov     dx, 900
imul    dx
push    ax
mov     ax, [bp+player_idx]
mov     dx, 180
imul    dx
pop     bx
add     bx, ax
add     bx, [bp+landmas_idx]
mov     al, [_ai_continents__0.Player_0.type_array+bx]
cbw
dec     ax
mov     bx, ax
cmp     bx, 5
jbe     short loc_F88A5
jmp     loc_F89DA
loc_F88A5:
shl     bx, 1
jmp     [cs:jt_sw_AiContinentAction+bx]
aica_01_02:
mov     [bp+Reevaluate], e_ST_FALSE
jmp     loc_F89DA
aica_05:
mov     [bp+Reevaluate], e_ST_TRUE
jmp     loc_F89DA
aica_00_03_04:
xor     _DI_itr_players__cities__landmasses, _DI_itr_players__cities__landmasses
jmp     short loc_F88CF
loc_F88C0:
mov     bx, _DI_itr_players__cities__landmasses
shl     bx, 1
lea     ax, [bp+Continent_Values]
add     bx, ax
mov     [word ptr bx], 0
inc     _DI_itr_players__cities__landmasses
loc_F88CF:
cmp     _DI_itr_players__cities__landmasses, e_NUM_PLAYERS
jl      short loc_F88C0
xor     _DI_itr_players__cities__landmasses, _DI_itr_players__cities__landmasses
jmp     loc_F895C
loc_F88D9:
mov     ax, _DI_itr_players__cities__landmasses
mov     dx, size s_CITY
imul    dx
les     bx, [_CITIES]
add     bx, ax
mov     al, [es:bx+s_CITY.wy]
cbw
mov     dx, e_WORLD_WIDTH
imul    dx
push    ax
mov     ax, [bp+wp]
mov     dx, e_WORLD_SIZE
imul    dx
les     bx, [_landmasses]
add     bx, ax
pop     ax
add     bx, ax
mov     ax, _DI_itr_players__cities__landmasses
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
cmp     ax, [bp+landmas_idx]
jnz     short loc_F895B
mov     ax, _DI_itr_players__cities__landmasses
mov     dx, size s_CITY
imul    dx
les     bx, [_CITIES]
add     bx, ax
mov     al, [es:bx+s_CITY.owner_idx]
cbw
cmp     ax, [bp+player_idx]
jz      short loc_F895B
mov     ax, _DI_itr_players__cities__landmasses
mov     dx, size s_CITY
imul    dx
les     bx, [_CITIES]
add     bx, ax
mov     al, [es:bx+s_CITY.owner_idx]
cbw
mov     [bp+city_owner_idx], ax
mov     bx, [bp+city_owner_idx]
shl     bx, 1
lea     ax, [bp+Continent_Values]
add     bx, ax
inc     [word ptr bx]
loc_F895B:
inc     _DI_itr_players__cities__landmasses
loc_F895C:
cmp     _DI_itr_players__cities__landmasses, [_cities]
jge     short loc_F8965
jmp     loc_F88D9
loc_F8965:
cmp     [bp+First_Hostile_Player], e_ST_UNDEFINED
jle     short loc_F8989
mov     bx, [bp+First_Hostile_Player]
shl     bx, 1
lea     ax, [bp+Continent_Values]
add     bx, ax
cmp     [word ptr bx], 0
jnz     short loc_F8982
mov     [bp+Reevaluate], e_ST_TRUE
jmp     short loc_F8987
loc_F8982:
mov     [bp+Reevaluate], e_ST_FALSE
loc_F8987:
jmp     short loc_F89D8
loc_F8989:
mov     [bp+Reevaluate], e_ST_TRUE
cmp     [bp+Continent_Values+0Ah], 0
jle     short loc_F899C
mov     [bp+Reevaluate], e_ST_FALSE
jmp     short loc_F89D8
loc_F899C:
xor     _DI_itr_players__cities__landmasses, _DI_itr_players__cities__landmasses
jmp     short loc_F89CC
loc_F89A0:
mov     ax, [bp+player_idx]
mov     dx, size s_WIZARD
imul    dx
mov     dx, _DI_itr_players__cities__landmasses
shl     dx, 1
add     ax, dx
mov     bx, ax
cmp     [_players.Hostility+bx], 2
jl      short loc_F89CB
mov     bx, _DI_itr_players__cities__landmasses
shl     bx, 1
lea     ax, [bp+Continent_Values]
add     bx, ax
cmp     [word ptr bx], 0
jle     short loc_F89CB
mov     [bp+Reevaluate], e_ST_FALSE
loc_F89CB:
inc     _DI_itr_players__cities__landmasses
loc_F89CC:
cmp     _DI_itr_players__cities__landmasses, [_num_players]
jge     short loc_F89D8
cmp     [bp+Reevaluate], e_ST_TRUE
jz      short loc_F89A0
loc_F89D8:
jmp     short $+2
loc_F89DA:
cmp     [bp+Reevaluate], e_ST_TRUE
jz      short loc_F89E3
jmp     loc_F8F73
loc_F89E3:
xor     _DI_itr_players__cities__landmasses, _DI_itr_players__cities__landmasses
jmp     short loc_F8A04
loc_F89E7:
mov     bx, _DI_itr_players__cities__landmasses
shl     bx, 1
lea     ax, [bp+Continent_Values_2]
add     bx, ax
mov     [word ptr bx], 0
mov     bx, _DI_itr_players__cities__landmasses
shl     bx, 1
lea     ax, [bp+Continent_Values]
add     bx, ax
mov     [word ptr bx], 0
inc     _DI_itr_players__cities__landmasses
loc_F8A04:
cmp     _DI_itr_players__cities__landmasses, e_NUM_LANDMASSES
jl      short loc_F89E7
mov     [bp+Territory_Center_X], 0
mov     [bp+Territory_Center_Y], 0
mov     [bp+Own_City_Count__distance], 0
xor     _DI_itr_players__cities__landmasses, _DI_itr_players__cities__landmasses
jmp     loc_F8B97
loc_F8A1D:
mov     ax, _DI_itr_players__cities__landmasses
mov     dx, size s_CITY
imul    dx
les     bx, [_CITIES]
add     bx, ax
mov     al, [es:bx+s_CITY.wp]
cbw
cmp     ax, [bp+wp]
jz      short loc_F8A37
jmp     loc_F8B96
loc_F8A37:
mov     ax, _DI_itr_players__cities__landmasses
mov     dx, size s_CITY
imul    dx
les     bx, [_CITIES]
add     bx, ax
mov     al, [es:bx+s_CITY.owner_idx]
cbw
mov     [bp+city_owner_idx], ax
mov     ax, [bp+city_owner_idx]
cmp     ax, [bp+player_idx]
jnz     short loc_F8A84
mov     ax, _DI_itr_players__cities__landmasses
mov     dx, size s_CITY
imul    dx
les     bx, [_CITIES]
add     bx, ax
mov     al, [es:bx+s_CITY.wx]
cbw
add     [bp+Territory_Center_X], ax
mov     ax, _DI_itr_players__cities__landmasses
mov     dx, size s_CITY
imul    dx
les     bx, [_CITIES]
add     bx, ax
mov     al, [es:bx+s_CITY.wy]
cbw
add     [bp+Territory_Center_Y], ax
inc     [bp+Own_City_Count__distance]
jmp     loc_F8B96
loc_F8A84:
cmp     [bp+First_Hostile_Player], e_ST_UNDEFINED
jg      short loc_F8A8D
jmp     loc_F8B40
loc_F8A8D:
mov     ax, [bp+city_owner_idx]
cmp     ax, [bp+First_Hostile_Player]
jnz     short loc_F8AE8
mov     ax, _DI_itr_players__cities__landmasses
mov     dx, size s_CITY
imul    dx
les     bx, [_CITIES]
add     bx, ax
mov     al, [es:bx+s_CITY.wy]
cbw
mov     dx, e_WORLD_WIDTH
imul    dx
push    ax
mov     ax, [bp+wp]
mov     dx, e_WORLD_SIZE
imul    dx
les     bx, [_landmasses]
add     bx, ax
pop     ax
add     bx, ax
mov     ax, _DI_itr_players__cities__landmasses
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
shl     ax, 1
lea     dx, [bp+Continent_Values]
add     ax, dx
mov     bx, ax
add     [word ptr bx], 10
jmp     short loc_F8B3E
loc_F8AE8:
cmp     [bp+city_owner_idx], e_NEUTRAL_PLAYER_IDX
jnz     short loc_F8B3E
mov     ax, _DI_itr_players__cities__landmasses
mov     dx, size s_CITY
imul    dx
les     bx, [_CITIES]
add     bx, ax
mov     al, [es:bx+s_CITY.wy]
cbw
mov     dx, e_WORLD_WIDTH
imul    dx
push    ax
mov     ax, [bp+wp]
mov     dx, e_WORLD_SIZE
imul    dx
les     bx, [_landmasses]
add     bx, ax
pop     ax
add     bx, ax
mov     ax, _DI_itr_players__cities__landmasses
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
shl     ax, 1
lea     dx, [bp+Continent_Values]
add     ax, dx
mov     bx, ax
inc     [word ptr bx]
loc_F8B3E:
jmp     short loc_F8B96
loc_F8B40:
cmp     [bp+city_owner_idx], e_NEUTRAL_PLAYER_IDX
jnz     short loc_F8B96
mov     ax, _DI_itr_players__cities__landmasses
mov     dx, size s_CITY
imul    dx
les     bx, [_CITIES]
add     bx, ax
mov     al, [es:bx+s_CITY.wy]
cbw
mov     dx, e_WORLD_WIDTH
imul    dx
push    ax
mov     ax, [bp+wp]
mov     dx, e_WORLD_SIZE
imul    dx
les     bx, [_landmasses]
add     bx, ax
pop     ax
add     bx, ax
mov     ax, _DI_itr_players__cities__landmasses
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
shl     ax, 1
lea     dx, [bp+Continent_Values]
add     ax, dx
mov     bx, ax
inc     [word ptr bx]
loc_F8B96:
inc     _DI_itr_players__cities__landmasses
loc_F8B97:
cmp     _DI_itr_players__cities__landmasses, [_cities]
jge     short loc_F8BA0
jmp     loc_F8A1D
loc_F8BA0:
cmp     [bp+Own_City_Count__distance], 0
jnz     short loc_F8BCE
mov     ax, [bp+player_idx]
mov     cl, 2
shl     ax, cl
les     bx, [_FORTRESSES]
add     bx, ax
mov     al, [es:bx+s_FORTRESS.wx]
cbw
mov     [bp+Territory_Center_X], ax
mov     ax, [bp+player_idx]
mov     cl, 2
shl     ax, cl
les     bx, [_FORTRESSES]
add     bx, ax
mov     al, [es:bx+s_FORTRESS.wy]
cbw
jmp     short loc_F8BDF
loc_F8BCE:
mov     ax, [bp+Territory_Center_X]
cwd
idiv    [bp+Own_City_Count__distance]
mov     [bp+Territory_Center_X], ax
mov     ax, [bp+Territory_Center_Y]
cwd
idiv    [bp+Own_City_Count__distance]
loc_F8BDF:
mov     [bp+Territory_Center_Y], ax
mov     [bp+itr_landmasses], 1
jmp     loc_F8DE8
loc_F8BEA:
mov     bx, [bp+wp]
mov     cl, 2
shl     bx, cl
les     bx, [_ai_landmass_dock_squares_heads+bx]
mov     ax, [bp+itr_landmasses]
shl     ax, 1
add     bx, ax
mov     ax, [es:bx]
mov     [bp+Value_Sum], ax
mov     [bp+Closest_LoadTile_Dist], 1000
jmp     loc_F8C95
loc_F8C0A:
mov     bx, [bp+wp]
mov     cl, 2
shl     bx, cl
les     bx, [g_world_embark_square_wx__load_init+bx]
add     bx, [bp+Value_Sum]
mov     al, [es:bx]
cbw
mov     [bp+Transport_Tile_X], ax
mov     bx, [bp+wp]
mov     cl, 2
shl     bx, cl
les     bx, [_ai_landmass_dock_squares_wy_array+bx]
add     bx, [bp+Value_Sum]
mov     al, [es:bx]
cbw
mov     [bp+Transport_Tile_Y], ax
mov     bx, [bp+wp]
mov     cl, 2
shl     bx, cl
les     bx, [g_ai_evaluation_map+bx]
mov     ax, [bp+Transport_Tile_Y]
mov     dx, e_WORLD_WIDTH
imul    dx
add     ax, [bp+Transport_Tile_X]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], 0
jnz     short loc_F8C6F
mov     ax, e_WORLD_WIDTH
push    ax
push    [bp+Territory_Center_Y]
push    [bp+Territory_Center_X]
push    [bp+Transport_Tile_Y]
push    [bp+Transport_Tile_X]
call    Delta_XY_With_Wrap
add     sp, 0Ah
mov     [bp+Own_City_Count__distance], ax
loc_F8C6F:
mov     ax, [bp+Own_City_Count__distance]
cmp     ax, [bp+Closest_LoadTile_Dist]
jge     short loc_F8C7D
mov     ax, [bp+Own_City_Count__distance]
mov     [bp+Closest_LoadTile_Dist], ax
loc_F8C7D:
mov     bx, [bp+wp]
mov     cl, 2
shl     bx, cl
les     bx, [CONTX_LoadTChain+bx]
mov     ax, [bp+Value_Sum]
shl     ax, 1
add     bx, ax
mov     ax, [es:bx]
mov     [bp+Value_Sum], ax
loc_F8C95:
cmp     [bp+Value_Sum], e_ST_UNDEFINED
jz      short loc_F8C9E
jmp     loc_F8C0A
loc_F8C9E:
cmp     [bp+Closest_LoadTile_Dist], 0
jnz     short loc_F8CA9
mov     [bp+Closest_LoadTile_Dist], 1
loc_F8CA9:
cmp     [bp+First_Hostile_Player], e_ST_UNDEFINED
jg      short loc_F8CB2
jmp     loc_F8D93
loc_F8CB2:
mov     bx, [bp+itr_landmasses]
shl     bx, 1
lea     ax, [bp+Continent_Values]
add     bx, ax
cmp     [word ptr bx], 0
jle     short loc_F8D0B
mov     bx, [bp+itr_landmasses]
shl     bx, 1
lea     ax, [bp+Continent_Values]
add     bx, ax
mov     ax, [bx]
mov     dx, 10
imul    dx
push    ax
mov     ax, 2000
pop     dx
sub     ax, dx
cwd
idiv    [bp+Closest_LoadTile_Dist]
mov     bx, [bp+itr_landmasses]
shl     bx, 1
lea     dx, [bp+Continent_Values_2]
add     bx, dx
mov     [bx], ax
mov     bx, [bp+itr_landmasses]
shl     bx, 1
lea     ax, [bp+Continent_Values_2]
add     bx, ax
cmp     [word ptr bx], 10
jge     short loc_F8D0B
mov     bx, [bp+itr_landmasses]
shl     bx, 1
lea     ax, [bp+Continent_Values_2]
add     bx, ax
mov     [word ptr bx], 10
loc_F8D0B:
mov     ax, [bp+player_idx]
mov     dx, size s_WIZARD
imul    dx
mov     dx, [bp+First_Hostile_Player]
shl     dx, 1
add     ax, dx
mov     bx, ax
cmp     [_players.Hostility+bx], 4
jnz     short loc_F8D91
mov     ax, [bp+First_Hostile_Player]
mov     cl, 2
shl     ax, cl
les     bx, [_FORTRESSES]
add     bx, ax
mov     al, [es:bx+s_FORTRESS.wp]
cbw
cmp     ax, [bp+wp]
jnz     short loc_F8D91
mov     ax, [bp+First_Hostile_Player]
mov     cl, 2
shl     ax, cl
les     bx, [_FORTRESSES]
add     bx, ax
mov     al, [es:bx+s_FORTRESS.wy]
cbw
mov     dx, e_WORLD_WIDTH
imul    dx
push    ax
mov     ax, [bp+wp]
mov     dx, e_WORLD_SIZE
imul    dx
les     bx, [_landmasses]
add     bx, ax
pop     ax
add     bx, ax
mov     ax, [bp+First_Hostile_Player]
mov     cl, 2
shl     ax, cl
push    es
les     si, [_FORTRESSES]
add     si, ax
mov     al, [es:si+s_FORTRESS.wx]
cbw
add     bx, ax
pop     es
mov     al, [es:bx]
mov     ah, 0
cmp     ax, [bp+itr_landmasses]
jnz     short loc_F8D91
mov     bx, [bp+itr_landmasses]
shl     bx, 1
lea     ax, [bp+Continent_Values_2]
add     bx, ax
add     [word ptr bx], 1000
loc_F8D91:
jmp     short loc_F8DE5
loc_F8D93:
mov     bx, [bp+itr_landmasses]
shl     bx, 1
lea     ax, [bp+Continent_Values]
add     bx, ax
mov     ax, [bx]
mov     dx, 1000
imul    dx
cwd
idiv    [bp+Closest_LoadTile_Dist]
mov     bx, [bp+itr_landmasses]
shl     bx, 1
lea     dx, [bp+Continent_Values_2]
add     bx, dx
mov     [bx], ax
mov     bx, [bp+itr_landmasses]
shl     bx, 1
lea     ax, [bp+Continent_Values]
add     bx, ax
cmp     [word ptr bx], 0
jle     short loc_F8DE5
mov     bx, [bp+itr_landmasses]
shl     bx, 1
lea     ax, [bp+Continent_Values_2]
add     bx, ax
cmp     [word ptr bx], 10
jge     short loc_F8DE5
mov     bx, [bp+itr_landmasses]
shl     bx, 1
lea     ax, [bp+Continent_Values_2]
add     bx, ax
mov     [word ptr bx], 10
loc_F8DE5:
inc     [bp+itr_landmasses]
loc_F8DE8:
cmp     [bp+itr_landmasses], e_NUM_LANDMASSES
jge     short loc_F8DF1
jmp     loc_F8BEA
loc_F8DF1:
mov     [bp+Value_Sum], 0
xor     _DI_itr_players__cities__landmasses, _DI_itr_players__cities__landmasses
jmp     short loc_F8E0A
loc_F8DFA:
mov     bx, _DI_itr_players__cities__landmasses
shl     bx, 1
lea     ax, [bp+Continent_Values_2]
add     bx, ax
mov     ax, [bx]
add     [bp+Value_Sum], ax
inc     _DI_itr_players__cities__landmasses
loc_F8E0A:
cmp     _DI_itr_players__cities__landmasses, e_NUM_LANDMASSES
jl      short loc_F8DFA
cmp     [bp+Value_Sum], 0
jz      short loc_F8E18
jmp     loc_F8F17
loc_F8E18:
xor     _DI_itr_players__cities__landmasses, _DI_itr_players__cities__landmasses
jmp     loc_F8F0F
loc_F8E1D:
mov     ax, [bp+wp]
mov     dx, 900
imul    dx
push    ax
mov     ax, [bp+player_idx]
mov     dx, 180
imul    dx
pop     bx
add     bx, ax
add     bx, _DI_itr_players__cities__landmasses
cmp     [_ai_continents__0.Player_0.type_array+bx], lmt_Own
jnz     short loc_F8E3D
jmp     loc_F8F0E
loc_F8E3D:
mov     ax, [bp+wp]
mov     dx, 900
imul    dx
push    ax
mov     ax, [bp+player_idx]
mov     dx, 180
imul    dx
pop     bx
add     bx, ax
add     bx, _DI_itr_players__cities__landmasses
cmp     [_ai_continents__0.Player_0.type_array+bx], lmt_NoTargets
jnz     short loc_F8E5D
jmp     loc_F8F0E
loc_F8E5D:
mov     ax, [bp+player_idx]
mov     cl, 2
shl     ax, cl
les     bx, [_FORTRESSES]
add     bx, ax
cmp     [es:bx+s_FORTRESS.active], e_ST_FALSE
jz      short loc_F8EB8
mov     ax, [bp+player_idx]
mov     cl, 2
shl     ax, cl
les     bx, [_FORTRESSES]
add     bx, ax
mov     al, [es:bx+s_FORTRESS.wy]
cbw
mov     dx, e_WORLD_WIDTH
imul    dx
push    ax
mov     ax, [bp+wp]
mov     dx, e_WORLD_SIZE
imul    dx
les     bx, [_landmasses]
add     bx, ax
pop     ax
add     bx, ax
mov     ax, [bp+player_idx]
mov     cl, 2
shl     ax, cl
push    es
les     si, [_FORTRESSES]
add     si, ax
mov     al, [es:si+s_FORTRESS.wx]
cbw
add     bx, ax
pop     es
mov     al, [es:bx]
mov     ah, 0
cmp     ax, _DI_itr_players__cities__landmasses
jz      short loc_F8F0E
loc_F8EB8:
mov     bx, [bp+wp]
mov     cl, 2
shl     bx, cl
les     bx, [CONTX_FirstTiles+bx]
mov     ax, _DI_itr_players__cities__landmasses
jmp     short loc_F8EE1
loc_F8EC7:
mov     bx, _DI_itr_players__cities__landmasses
shl     bx, 1
lea     ax, [bp+Continent_Values_2]
add     bx, ax
inc     [word ptr bx]
mov     bx, [bp+wp]
mov     cl, 2
shl     bx, cl
les     bx, [CONTX_TileChain+bx]
mov     ax, [bp+Value_Sum]
loc_F8EE1:
shl     ax, 1
add     bx, ax
mov     ax, [es:bx]
mov     [bp+Value_Sum], ax
cmp     [bp+Value_Sum], e_ST_UNDEFINED
jnz     short loc_F8EC7
mov     bx, _DI_itr_players__cities__landmasses
shl     bx, 1
lea     ax, [bp+Continent_Values_2]
add     bx, ax
cmp     [word ptr bx], 10
jge     short loc_F8F0E
mov     bx, _DI_itr_players__cities__landmasses
shl     bx, 1
lea     ax, [bp+Continent_Values_2]
add     bx, ax
mov     [word ptr bx], 0
loc_F8F0E:
inc     _DI_itr_players__cities__landmasses
loc_F8F0F:
cmp     _DI_itr_players__cities__landmasses, e_NUM_LANDMASSES
jge     short loc_F8F17
jmp     loc_F8E1D
loc_F8F17:
mov     [bp+Value_Sum], 0
xor     _DI_itr_players__cities__landmasses, _DI_itr_players__cities__landmasses
jmp     short loc_F8F30
loc_F8F20:
mov     bx, _DI_itr_players__cities__landmasses
shl     bx, 1
lea     ax, [bp+Continent_Values_2]
add     bx, ax
mov     ax, [bx]
add     [bp+Value_Sum], ax
inc     _DI_itr_players__cities__landmasses
loc_F8F30:
cmp     _DI_itr_players__cities__landmasses, e_NUM_LANDMASSES
jl      short loc_F8F20
cmp     [bp+Value_Sum], 0
jnz     short loc_F8F51
mov     bx, [bp+wp]
shl     bx, 1
mov     bx, [_ai_landmass_war_targets+bx]
mov     ax, [bp+player_idx]
shl     ax, 1
add     bx, ax
mov     [word ptr bx], 0
jmp     short loc_F8F73
loc_F8F51:
mov     ax, e_NUM_LANDMASSES
push    ax
lea     ax, [bp+Continent_Values_2]
push    ax
call    Get_Weighted_Choice
pop     cx
pop     cx
mov     bx, [bp+wp]
shl     bx, 1
mov     bx, [_ai_landmass_war_targets+bx]
mov     dx, [bp+player_idx]
shl     dx, 1
add     bx, dx
mov     [bx], ax
loc_F8F73:
inc     [bp+wp]
loc_F8F76:
cmp     [bp+wp], 2
jge     short @@Done
jmp     loc_F8856
@@Done:
pop     _DI_itr_players__cities__landmasses
pop     si
mov     sp, bp
pop     bp
retf
endp AI_Choose_War_Landmass

jt_sw_AiContinentAction dw offset aica_00_03_04
dw offset aica_01_02
dw offset aica_01_02
dw offset aica_00_03_04
dw offset aica_00_03_04
dw offset aica_05
