; =============== S U B R O U T I N E =======================================
; Attributes: bp-based frame
proc Square_Is_Shoreline far
wx= word ptr  6
wy= word ptr  8
wp= word ptr  0Ah
_CX_terrain_type = cx
push    bp
mov     bp, sp
mov     ax, [bp+wp]
mov     dx, e_WORLD_SIZE_DB
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, [bp+wy]
mov     dx, e_WORLD_WIDTH_DB
imul    dx
add     bx, ax
mov     ax, [bp+wx]
shl     ax, 1
add     bx, ax
mov     ax, [es:bx]
mov     bx, NUM_TERRAIN_TYPES
xor     dx, dx
div     bx
mov     _CX_terrain_type, dx
cmp     _CX_terrain_type, _Tundra00001000
jl      short loc_F3F66
@@JmpDone__Return_FALSE:
xor     ax, ax
@@JmpDone:
jmp     short @@Done
; ---------------------------------------------------------------------------
loc_F3F66:
cmp     _CX_terrain_type, _Shore1100000R
jl      short loc_F3F6E
jmp     short @@JmpDone__Return_TRUE
; ---------------------------------------------------------------------------
loc_F3F6E:
cmp     _CX_terrain_type, _River1111_1
jl      short loc_F3F76
jmp     short @@JmpDone__Return_FALSE
; ---------------------------------------------------------------------------
loc_F3F76:
cmp     _CX_terrain_type, _Shore00011R11
jl      short loc_F3F7E
jmp     short @@JmpDone__Return_TRUE
; ---------------------------------------------------------------------------
loc_F3F7E:
cmp     _CX_terrain_type, _River1100_3
jl      short loc_F3F86
jmp     short @@JmpDone__Return_FALSE
; ---------------------------------------------------------------------------
loc_F3F86:
cmp     _CX_terrain_type, _1LakeRiv_W
jl      short loc_F3F8E
jmp     short @@JmpDone__Return_TRUE
; ---------------------------------------------------------------------------
loc_F3F8E:
cmp     _CX_terrain_type, _Grasslands1
jl      short loc_F3F96
jmp     short @@JmpDone__Return_FALSE
; ---------------------------------------------------------------------------
loc_F3F96:
cmp     _CX_terrain_type, TT_BugGrass
jnz     short loc_F3F9D
jmp     short @@JmpDone__Return_FALSE
; ---------------------------------------------------------------------------
loc_F3F9D:
cmp     _CX_terrain_type, _1Lake
jnz     short @@JmpDone__Return_TRUE
jmp     short @@JmpDone__Return_FALSE
; ---------------------------------------------------------------------------
@@JmpDone__Return_TRUE:
mov     ax, e_ST_TRUE
jmp     short @@JmpDone
; ---------------------------------------------------------------------------
@@Done:
pop     bp
retf
endp Square_Is_Shoreline
