; =============== S U B R O U T I N E =======================================
; Attributes: bp-based frame
proc CONTX_CreateLChains__WIP far
Transport_Tile= word ptr -14h
wy_ofst= word ptr -12h
wx_ofst= word ptr -10h
Current_Tile= word ptr -0Eh
landmass_idx= word ptr -0Ch
count= word ptr -0Ah
itr= word ptr -6
wy= word ptr -4
wx= word ptr -2
_DI_wp = di
push    bp
mov     bp, sp
sub     sp, 14h
push    si
push    _DI_wp
push    cs
call    near ptr EMM_Map_CONTXXX
mov     [bp+itr], 0
jmp     short loc_D4F2B
; ---------------------------------------------------------------------------
loc_D4F08:
mov     ax, [bp+itr]
shl     ax, 1
les     bx, [CONTX_1stLoadTs]
add     bx, ax
mov     [word ptr es:bx], e_ST_UNDEFINED
mov     ax, [bp+itr]
shl     ax, 1
les     bx, [CONTX_1stLoadTs+4]
add     bx, ax
mov     [word ptr es:bx], e_ST_UNDEFINED
inc     [bp+itr]
loc_D4F2B:
cmp     [bp+itr], e_NUM_LANDMASSES
jl      short loc_D4F08
mov     [bp+count], 0
mov     [bp+count+2], 0
xor     _DI_wp, _DI_wp
jmp     loc_D50D9
; ---------------------------------------------------------------------------
loc_D4F40:
mov     [bp+wy], 0
jmp     loc_D50CF
; ---------------------------------------------------------------------------
loc_D4F48:
mov     [bp+wx], 0
jmp     loc_D50C3
; ---------------------------------------------------------------------------
loc_D4F50:
mov     ax, _DI_wp
mov     dx, e_WORLD_SIZE
imul    dx
les     bx, [_landmasses]
add     bx, ax
mov     ax, [bp+wy]
mov     dx, e_WORLD_WIDTH
imul    dx
add     bx, ax
add     bx, [bp+wx]
mov     al, [es:bx]
mov     ah, 0
mov     [bp+landmass_idx], ax
cmp     [bp+landmass_idx], 0
jnz     short loc_D4F7B
jmp     loc_D50C0
; ---------------------------------------------------------------------------
loc_D4F7B:
mov     [bp+Transport_Tile], e_ST_FALSE
mov     [bp+wx_ofst], -1
jmp     short loc_D4FC1
; ---------------------------------------------------------------------------
loc_D4F87:
mov     [bp+wy_ofst], -1
jmp     short loc_D4FB2
; ---------------------------------------------------------------------------
loc_D4F8E:
push    _DI_wp
mov     ax, [bp+wy]
add     ax, [bp+wy_ofst]
push    ax
mov     ax, [bp+wx]
add     ax, [bp+wx_ofst]
push    ax
call    j_Square_Is_Shoreline
add     sp, 6
cmp     ax, e_ST_TRUE
jnz     short loc_D4FAF
mov     [bp+Transport_Tile], e_ST_TRUE
loc_D4FAF:
inc     [bp+wy_ofst]
loc_D4FB2:
cmp     [bp+wy_ofst], 2
jge     short loc_D4FBE
cmp     [bp+Transport_Tile], e_ST_FALSE
jz      short loc_D4F8E
loc_D4FBE:
inc     [bp+wx_ofst]
loc_D4FC1:
cmp     [bp+wx_ofst], 2
jge     short loc_D4FCD
cmp     [bp+Transport_Tile], e_ST_FALSE
jz      short loc_D4F87
loc_D4FCD:
cmp     [bp+Transport_Tile], e_ST_TRUE
jz      short loc_D4FD6
jmp     loc_D50C0
; ---------------------------------------------------------------------------
loc_D4FD6:
mov     bx, _DI_wp
mov     cl, 2
shl     bx, cl
les     bx, [CONTX_1stLoadTs+bx]
mov     ax, [bp+landmass_idx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], e_ST_UNDEFINED
jnz     short loc_D4FFC
mov     bx, _DI_wp
mov     cl, 2
shl     bx, cl
les     bx, [CONTX_1stLoadTs+bx]
mov     ax, [bp+landmass_idx]
jmp     short loc_D504B
; ---------------------------------------------------------------------------
loc_D4FFC:
mov     bx, _DI_wp
mov     cl, 2
shl     bx, cl
les     bx, [CONTX_1stLoadTs+bx]
mov     ax, [bp+landmass_idx]
shl     ax, 1
add     bx, ax
mov     ax, [es:bx]
mov     [bp+itr], ax
mov     ax, [bp+itr]
mov     [bp+Current_Tile], ax
jmp     short loc_D5038
; ---------------------------------------------------------------------------
loc_D501B:
mov     ax, [bp+itr]
mov     [bp+Current_Tile], ax
mov     bx, _DI_wp
mov     cl, 2
shl     bx, cl
les     bx, [CONTX_LoadTChain+bx]
mov     ax, [bp+itr]
shl     ax, 1
add     bx, ax
mov     ax, [es:bx]
mov     [bp+itr], ax
loc_D5038:
cmp     [bp+itr], e_ST_UNDEFINED
jnz     short loc_D501B
mov     bx, _DI_wp
mov     cl, 2
shl     bx, cl
les     bx, [CONTX_LoadTChain+bx]
mov     ax, [bp+Current_Tile]
loc_D504B:
shl     ax, 1
add     bx, ax
mov     si, _DI_wp
shl     si, 1
lea     ax, [bp+count]
add     si, ax
mov     ax, [si]
mov     [es:bx], ax
mov     bx, _DI_wp
shl     bx, 1
lea     ax, [bp+count]
add     bx, ax
mov     si, _DI_wp
mov     cl, 2
shl     si, cl
les     ax, [g_world_embark_square_wx__load_init+si]
add     ax, [bx]
mov     dl, [byte ptr bp+wx]
mov     bx, ax
mov     [es:bx], dl
mov     bx, _DI_wp
shl     bx, 1
lea     ax, [bp+count]
add     bx, ax
mov     si, _DI_wp
mov     cl, 2
shl     si, cl
les     ax, [_ai_landmass_dock_squares_wy_array+si]
add     ax, [bx]
mov     dl, [byte ptr bp+wy]
mov     bx, ax
mov     [es:bx], dl
mov     bx, _DI_wp
shl     bx, 1
lea     ax, [bp+count]
add     bx, ax
mov     ax, [bx]
shl     ax, 1
mov     bx, _DI_wp
mov     cl, 2
shl     bx, cl
les     bx, [CONTX_LoadTChain+bx]
add     bx, ax
mov     [word ptr es:bx], e_ST_UNDEFINED
mov     bx, _DI_wp
shl     bx, 1
lea     ax, [bp+count]
add     bx, ax
inc     [word ptr bx]
loc_D50C0:
inc     [bp+wx]
loc_D50C3:
cmp     [bp+wx], e_WORLD_WIDTH
jge     short loc_D50CC
jmp     loc_D4F50
; ---------------------------------------------------------------------------
loc_D50CC:
inc     [bp+wy]
loc_D50CF:
cmp     [bp+wy], e_WORLD_HEIGHT
jge     short loc_D50D8
jmp     loc_D4F48
; ---------------------------------------------------------------------------
loc_D50D8:
inc     _DI_wp
loc_D50D9:
cmp     _DI_wp, e_NUM_PLANES
jge     short @@Done
jmp     loc_D4F40
; ---------------------------------------------------------------------------
@@Done:
call    EMMDATAH_Map
pop     _DI_wp
pop     si
mov     sp, bp
pop     bp
retf
endp CONTX_CreateLChains__WIP
