; =============== S U B R O U T I N E =======================================
; Attributes: bp-based frame
proc AI_Evaluation_Map far
nonhostiles= word ptr -12h
xy_ofst= word ptr -6
unit_owner_idx= word ptr -4
map_square_count= word ptr -2
player_idx= word ptr  6
itr = si
_DI_wp = di
push    bp
mov     bp, sp
sub     sp, 12h
push    itr
push    _DI_wp
call    j_EMM_Map_CONTXXX__WIP
xor     itr, itr
jmp     short loc_F8419
; ---------------------------------------------------------------------------
loc_F83AC:
mov     ax, [bp+player_idx]
mov     dx, size s_WIZARD
imul    dx
add     ax, itr
mov     bx, ax
cmp     [_players.Dipl.Dipl_Status+bx], DIPL_WizardPact
jz      short loc_F83FC
mov     ax, [bp+player_idx]
mov     dx, size s_WIZARD
imul    dx
add     ax, itr
mov     bx, ax
cmp     [_players.Dipl.Dipl_Status+bx], DIPL_Alliance
jz      short loc_F83FC
mov     ax, [bp+player_idx]
mov     dx, size s_WIZARD
imul    dx
add     ax, itr
mov     bx, ax
cmp     [_players.peace_duration+bx], 0
jg      short loc_F83FC
mov     ax, [bp+player_idx]
mov     dx, size s_WIZARD
imul    dx
mov     dx, itr
shl     dx, 1
add     ax, dx
mov     bx, ax
cmp     [_players.Hostility+bx], 2
jge     short loc_F840B
loc_F83FC:
mov     bx, itr
shl     bx, 1
lea     ax, [bp+nonhostiles]
add     bx, ax
mov     [word ptr bx], e_ST_TRUE
jmp     short loc_F8418
; ---------------------------------------------------------------------------
loc_F840B:
mov     bx, itr
shl     bx, 1
lea     ax, [bp+nonhostiles]
add     bx, ax
mov     [word ptr bx], e_ST_FALSE
loc_F8418:
inc     itr
loc_F8419:
cmp     itr, e_NUM_PLAYERS
jl      short loc_F83AC
mov     [bp+map_square_count], e_WORLD_SIZE
xor     _DI_wp, _DI_wp
jmp     short loc_F8447
; ---------------------------------------------------------------------------
loc_F8427:
xor     itr, itr
jmp     short loc_F8441
; ---------------------------------------------------------------------------
loc_F842B:
mov     bx, _DI_wp
mov     cl, 2
shl     bx, cl
les     bx, [g_ai_evaluation_map+bx]
mov     ax, itr
shl     ax, 1
add     bx, ax
mov     [word ptr es:bx], 0
inc     itr
loc_F8441:
cmp     itr, [bp+map_square_count]
jl      short loc_F842B
inc     _DI_wp
loc_F8447:
cmp     _DI_wp, e_NUM_PLANES
jl      short loc_F8427
xor     itr, itr
jmp     loc_F8525
; ---------------------------------------------------------------------------
loc_F8451:
mov     ax, itr
mov     cl, 5
shl     ax, cl
les     bx, [_UNITS]
add     bx, ax
mov     al, [es:bx+s_UNIT.owner_idx]
cbw
mov     [bp+unit_owner_idx], ax
mov     ax, [bp+unit_owner_idx]
cmp     ax, [bp+player_idx]
jnz     short loc_F8470
jmp     loc_F8524
; ---------------------------------------------------------------------------
loc_F8470:
mov     ax, itr
mov     cl, 5
shl     ax, cl
les     bx, [_UNITS]
add     bx, ax
mov     al, [es:bx+s_UNIT.owner_idx]
cbw
cmp     ax, [bp+player_idx]
jnz     short loc_F8489
jmp     loc_F8524
; ---------------------------------------------------------------------------
loc_F8489:
mov     ax, itr
mov     cl, 5
shl     ax, cl
les     bx, [_UNITS]
add     bx, ax
mov     al, [es:bx+s_UNIT.wp]
cbw
mov     _DI_wp, ax
mov     ax, itr
mov     cl, 5
shl     ax, cl
les     bx, [_UNITS]
add     bx, ax
mov     al, [es:bx+s_UNIT.wy]
cbw
mov     dx, e_WORLD_WIDTH
imul    dx
mov     dx, itr
mov     cl, 5
shl     dx, cl
les     bx, [_UNITS]
add     bx, dx
push    ax
mov     al, [es:bx+s_UNIT.wx]
cbw
pop     dx
add     dx, ax
mov     [bp+xy_ofst], dx
push    itr
call    j_Effective_Unit_Strength
pop     cx
mov     bx, 10
cwd
idiv    bx
mov     bx, _DI_wp
mov     cl, 2
shl     bx, cl
les     bx, [g_ai_evaluation_map+bx]
mov     dx, [bp+xy_ofst]
shl     dx, 1
add     bx, dx
add     [es:bx], ax
mov     bx, [bp+unit_owner_idx]
shl     bx, 1
lea     ax, [bp+nonhostiles]
add     bx, ax
cmp     [word ptr bx], e_ST_TRUE
jnz     short loc_F8524
mov     bx, _DI_wp
mov     cl, 2
shl     bx, cl
les     bx, [g_ai_evaluation_map+bx]
mov     ax, [bp+xy_ofst]
shl     ax, 1
add     bx, ax
mov     ax, [es:bx]
or      ax, 4000h
mov     bx, _DI_wp
mov     cl, 2
shl     bx, cl
les     bx, [g_ai_evaluation_map+bx]
mov     dx, [bp+xy_ofst]
shl     dx, 1
add     bx, dx
mov     [es:bx], ax
loc_F8524:
inc     itr
loc_F8525:
cmp     itr, [_units]
jge     short loc_F852E
jmp     loc_F8451
; ---------------------------------------------------------------------------
loc_F852E:
xor     itr, itr
jmp     loc_F8645
; ---------------------------------------------------------------------------
loc_F8533:
mov     ax, itr
mov     dx, size s_LAIR
imul    dx
les     bx, [_LAIRS]
add     bx, ax
cmp     [es:bx+s_LAIR.intact], e_ST_TRUE
jz      short loc_F854A
jmp     loc_F8644
; ---------------------------------------------------------------------------
loc_F854A:
mov     ax, itr
mov     dx, size s_LAIR
imul    dx
les     bx, [_LAIRS]
add     bx, ax
cmp     [es:bx+s_LAIR.guard1_count], 0
ja      short loc_F8561
jmp     loc_F8644
; ---------------------------------------------------------------------------
loc_F8561:
mov     ax, itr
mov     dx, size s_LAIR
imul    dx
les     bx, [_LAIRS]
add     bx, ax
mov     al, [es:bx+s_LAIR.wp]
cbw
mov     _DI_wp, ax
mov     ax, itr
mov     dx, size s_LAIR
imul    dx
les     bx, [_LAIRS]
add     bx, ax
mov     al, [es:bx+s_LAIR.wy]
cbw
mov     dx, e_WORLD_WIDTH
imul    dx
push    ax
mov     ax, itr
mov     dx, size s_LAIR
imul    dx
les     bx, [_LAIRS]
add     bx, ax
loc_F859A:
mov     al, [es:bx+s_LAIR.wx]
cbw
pop     dx
add     dx, ax
mov     [bp+xy_ofst], dx
mov     ax, itr
mov     dx, size s_LAIR
imul    dx
les     bx, [_LAIRS]
add     bx, ax
mov     al, [es:bx+s_LAIR.guard1_unit_type]
mov     ah, 0
push    ax
call    j_Effective_Unit_Type_Strength
pop     cx
mov     bx, 10
cwd
idiv    bx
push    ax
mov     ax, itr
mov     dx, size s_LAIR
imul    dx
les     bx, [_LAIRS]
add     bx, ax
mov     al, [es:bx+s_LAIR.guard1_count]
mov     ah, 0
and     ax, 0Fh
mov     dx, ax
pop     ax
imul    dx
mov     bx, _DI_wp
mov     cl, 2
shl     bx, cl
les     bx, [g_ai_evaluation_map+bx]
mov     dx, [bp+xy_ofst]
shl     dx, 1
add     bx, dx
add     [es:bx], ax
mov     ax, itr
mov     dx, size s_LAIR
imul    dx
les     bx, [_LAIRS]
add     bx, ax
mov     al, [es:bx+s_LAIR.guard2_unit_type]
mov     ah, 0
push    ax
call    j_Effective_Unit_Type_Strength
pop     cx
mov     bx, 10
cwd
idiv    bx
push    ax
mov     ax, itr
mov     dx, size s_LAIR
imul    dx
les     bx, [_LAIRS]
add     bx, ax
mov     al, [es:bx+s_LAIR.guard2_count]
mov     ah, 0
and     ax, 0Fh
mov     dx, ax
pop     ax
imul    dx
mov     bx, _DI_wp
mov     cl, 2
shl     bx, cl
les     bx, [g_ai_evaluation_map+bx]
mov     dx, [bp+xy_ofst]
shl     dx, 1
add     bx, dx
add     [es:bx], ax
loc_F8644:
inc     itr
loc_F8645:
cmp     itr, e_NUM_LAIRS
jge     short loc_F864D
jmp     loc_F8533
; ---------------------------------------------------------------------------
loc_F864D:
xor     itr, itr
jmp     loc_F86ED
; ---------------------------------------------------------------------------
loc_F8652:
mov     ax, itr
mov     dx, size s_CITY
imul    dx
les     bx, [_CITIES]
add     bx, ax
mov     al, [es:bx+s_CITY.wp]
cbw
mov     _DI_wp, ax
mov     ax, itr
mov     dx, size s_CITY
imul    dx
les     bx, [_CITIES]
add     bx, ax
mov     al, [es:bx+s_CITY.wy]
cbw
mov     dx, e_WORLD_WIDTH
imul    dx
push    ax
mov     ax, itr
mov     dx, size s_CITY
imul    dx
loc_F8685:
les     bx, [_CITIES]
add     bx, ax
mov     al, [es:bx+s_CITY.wx]
cbw
pop     dx
add     dx, ax
mov     [bp+xy_ofst], dx
mov     bx, _DI_wp
mov     cl, 2
shl     bx, cl
les     bx, [g_ai_evaluation_map+bx]
mov     ax, [bp+xy_ofst]
shl     ax, 1
add     bx, ax
mov     ax, [es:bx]
or      ax, 8000h
mov     bx, _DI_wp
mov     cl, 2
shl     bx, cl
les     bx, [g_ai_evaluation_map+bx]
mov     dx, [bp+xy_ofst]
shl     dx, 1
add     bx, dx
mov     [es:bx], ax
mov     ax, itr
mov     dx, size s_CITY
imul    dx
les     bx, [_CITIES]
add     bx, ax
mov     al, [es:bx+s_CITY.owner_idx]
cbw
cmp     ax, [bp+player_idx]
jz      short loc_F86EC
mov     bx, _DI_wp
mov     cl, 2
shl     bx, cl
les     bx, [g_ai_evaluation_map+bx]
mov     ax, [bp+xy_ofst]
shl     ax, 1
add     bx, ax
inc     [word ptr es:bx]
loc_F86EC:
inc     itr
loc_F86ED:
cmp     itr, [_cities]
jge     short loc_F86F6
jmp     loc_F8652
; ---------------------------------------------------------------------------
loc_F86F6:
xor     itr, itr
jmp     loc_F877E
; ---------------------------------------------------------------------------
loc_F86FB:
mov     ax, itr
mov     dx, size s_LAIR
imul    dx
les     bx, [_LAIRS]
add     bx, ax
cmp     [es:bx+s_LAIR.intact], e_ST_FALSE
jz      short loc_F877D
mov     ax, itr
mov     dx, size s_LAIR
imul    dx
les     bx, [_LAIRS]
add     bx, ax
mov     al, [es:bx+s_LAIR.wp]
cbw
mov     _DI_wp, ax
mov     ax, itr
mov     dx, size s_LAIR
imul    dx
les     bx, [_LAIRS]
add     bx, ax
mov     al, [es:bx+s_LAIR.wy]
cbw
mov     dx, e_WORLD_WIDTH
imul    dx
push    ax
mov     ax, itr
mov     dx, size s_LAIR
imul    dx
les     bx, [_LAIRS]
add     bx, ax
mov     al, [es:bx+s_LAIR.wx]
cbw
pop     dx
add     dx, ax
mov     [bp+xy_ofst], dx
mov     bx, _DI_wp
mov     cl, 2
shl     bx, cl
les     bx, [g_ai_evaluation_map+bx]
mov     ax, [bp+xy_ofst]
shl     ax, 1
add     bx, ax
mov     ax, [es:bx]
or      ax, 8000h
mov     bx, _DI_wp
mov     cl, 2
shl     bx, cl
les     bx, [g_ai_evaluation_map+bx]
mov     dx, [bp+xy_ofst]
shl     dx, 1
add     bx, dx
mov     [es:bx], ax
loc_F877D:
inc     itr
loc_F877E:
cmp     itr, e_NUM_LAIRS
jge     short loc_F8786
jmp     loc_F86FB
; ---------------------------------------------------------------------------
loc_F8786:
xor     itr, itr
jmp     short loc_F87F9
; ---------------------------------------------------------------------------
loc_F878A:
mov     ax, itr
mov     dx, size s_NODE
imul    dx
les     bx, [_NODES]
add     bx, ax
loc_F8797:
mov     al, [es:bx+s_NODE.wp]
cbw
mov     _DI_wp, ax
mov     ax, itr
mov     dx, size s_NODE
imul    dx
les     bx, [_NODES]
add     bx, ax
mov     al, [es:bx+s_NODE.wy]
cbw
mov     dx, e_WORLD_WIDTH
imul    dx
push    ax
mov     ax, itr
mov     dx, size s_NODE
imul    dx
les     bx, [_NODES]
add     bx, ax
mov     al, [es:bx+s_NODE.wx]
cbw
pop     dx
add     dx, ax
mov     [bp+xy_ofst], dx
mov     bx, _DI_wp
mov     cl, 2
shl     bx, cl
les     bx, [g_ai_evaluation_map+bx]
mov     ax, [bp+xy_ofst]
shl     ax, 1
add     bx, ax
mov     ax, [es:bx]
or      ax, 8000h
mov     bx, _DI_wp
mov     cl, 2
shl     bx, cl
les     bx, [g_ai_evaluation_map+bx]
mov     dx, [bp+xy_ofst]
shl     dx, 1
add     bx, dx
mov     [es:bx], ax
inc     itr
loc_F87F9:
cmp     itr, e_NUM_NODES
jl      short loc_F878A
pop     _DI_wp
pop     itr
mov     sp, bp
pop     bp
retf
endp AI_Evaluation_Map
