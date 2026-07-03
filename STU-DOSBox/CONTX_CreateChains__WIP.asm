; =============== S U B R O U T I N E =======================================
; Attributes: bp-based frame
proc CONTX_CreateChains__WIP far
Current_Tile= word ptr -0Eh
landmass_idx= word ptr -0Ch
count= word ptr -0Ah
itr= word ptr -6
wy= word ptr -4
wx= word ptr -2
_DI_wp = di
push    bp
mov     bp, sp
sub     sp, 0Eh
push    si
push    _DI_wp
push    cs
call    near ptr EMM_Map_CONTXXX
mov     [bp+itr], 0
jmp     short loc_D4D5E
; ---------------------------------------------------------------------------
loc_D4D3B:
mov     ax, [bp+itr]
shl     ax, 1
les     bx, [CONTX_FirstTiles]
add     bx, ax
mov     [word ptr es:bx], e_ST_UNDEFINED
mov     ax, [bp+itr]
shl     ax, 1
les     bx, [CONTX_FirstTiles+4]
add     bx, ax
mov     [word ptr es:bx], e_ST_UNDEFINED
inc     [bp+itr]
loc_D4D5E:
cmp     [bp+itr], e_NUM_LANDMASSES
jl      short loc_D4D3B
mov     [bp+itr], 0
jmp     short loc_D4D8E
; ---------------------------------------------------------------------------
loc_D4D6B:
mov     ax, [bp+itr]
shl     ax, 1
les     bx, [CONTX_TileChain]
add     bx, ax
mov     [word ptr es:bx], e_ST_UNDEFINED
mov     ax, [bp+itr]
shl     ax, 1
les     bx, [CONTX_TileChain+4]
add     bx, ax
mov     [word ptr es:bx], e_ST_UNDEFINED
inc     [bp+itr]
loc_D4D8E:
cmp     [bp+itr], 1600
jl      short loc_D4D6B
mov     [bp+count], 0
mov     [bp+count+2], 0
xor     _DI_wp, _DI_wp
jmp     loc_D4EE2
; ---------------------------------------------------------------------------
loc_D4DA4:
mov     [bp+wy], 0
jmp     loc_D4ED8
; ---------------------------------------------------------------------------
loc_D4DAC:
mov     [bp+wx], 0
jmp     loc_D4ECC
; ---------------------------------------------------------------------------
loc_D4DB4:
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
jnz     short loc_D4DDF
jmp     loc_D4EC9
; ---------------------------------------------------------------------------
loc_D4DDF:
mov     bx, _DI_wp
mov     cl, 2
shl     bx, cl
les     bx, [CONTX_FirstTiles+bx]
mov     ax, [bp+landmass_idx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], e_ST_UNDEFINED
jnz     short loc_D4E05
mov     bx, _DI_wp
mov     cl, 2
shl     bx, cl
les     bx, [CONTX_FirstTiles+bx]
mov     ax, [bp+landmass_idx]
jmp     short loc_D4E54
; ---------------------------------------------------------------------------
loc_D4E05:
mov     bx, _DI_wp
mov     cl, 2
shl     bx, cl
les     bx, [CONTX_FirstTiles+bx]
mov     ax, [bp+landmass_idx]
shl     ax, 1
add     bx, ax
mov     ax, [es:bx]
mov     [bp+itr], ax
mov     ax, [bp+itr]
mov     [bp+Current_Tile], ax
jmp     short loc_D4E41
; ---------------------------------------------------------------------------
loc_D4E24:
mov     ax, [bp+itr]
mov     [bp+Current_Tile], ax
mov     bx, _DI_wp
mov     cl, 2
shl     bx, cl
les     bx, [CONTX_TileChain+bx]
mov     ax, [bp+itr]
shl     ax, 1
add     bx, ax
mov     ax, [es:bx]
mov     [bp+itr], ax
loc_D4E41:
cmp     [bp+itr], e_ST_UNDEFINED
jnz     short loc_D4E24
mov     bx, _DI_wp
mov     cl, 2
shl     bx, cl
les     bx, [CONTX_TileChain+bx]
mov     ax, [bp+Current_Tile]
loc_D4E54:
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
les     ax, [CONTX_TileXs+si]
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
les     ax, [CONTX_TileYs+si]
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
les     bx, [CONTX_TileChain+bx]
add     bx, ax
mov     [word ptr es:bx], e_ST_UNDEFINED
mov     bx, _DI_wp
shl     bx, 1
lea     ax, [bp+count]
add     bx, ax
inc     [word ptr bx]
loc_D4EC9:
inc     [bp+wx]
loc_D4ECC:
cmp     [bp+wx], e_WORLD_WIDTH
jge     short loc_D4ED5
jmp     loc_D4DB4
; ---------------------------------------------------------------------------
loc_D4ED5:
inc     [bp+wy]
loc_D4ED8:
cmp     [bp+wy], e_WORLD_HEIGHT
jge     short loc_D4EE1
jmp     loc_D4DAC
; ---------------------------------------------------------------------------
loc_D4EE1:
inc     _DI_wp
loc_D4EE2:
cmp     _DI_wp, 2
jge     short @@Done
jmp     loc_D4DA4
; ---------------------------------------------------------------------------
@@Done:
call    EMMDATAH_Map
pop     _DI_wp
pop     si
mov     sp, bp
pop     bp
retf
endp CONTX_CreateChains__WIP
