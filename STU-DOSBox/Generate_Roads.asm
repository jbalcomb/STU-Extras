; =============== S U B R O U T I N E =======================================
; Attributes: bp-based frame
proc Generate_Roads far
Road_Ys= byte ptr -0A4h
Road_Xs= byte ptr -5Eh
dst_wy= word ptr -18h
dst_wx= word ptr -16h
src_wy= word ptr -14h
src_wx= word ptr -12h
city_idx= word ptr -10h
wy= word ptr -0Eh
wx= word ptr -0Ch
Line_Index= word ptr -0Ah
Road_Length= word ptr -8
Invalid_Road= word ptr -6
dst_city_idx= word ptr -4
src_city_idx= word ptr -2
_wp= word ptr  6
wp = di
push    bp
mov     bp, sp
sub     sp, 0A4h
push    si
push    wp
mov     wp, [bp+_wp]
mov     [bp+wy], 0
jmp     short loc_4BA97
; ---------------------------------------------------------------------------
loc_4BA66:
mov     [bp+wx], 0
jmp     short loc_4BA8E
; ---------------------------------------------------------------------------
loc_4BA6D:
mov     ax, wp
mov     dx, e_WORLD_SIZE
imul    dx
les     bx, [_map_square_flags]
add     bx, ax
mov     ax, [bp+wy]
mov     dx, e_WORLD_WIDTH
imul    dx
add     bx, ax
add     bx, [bp+wx]
mov     [byte ptr es:bx], 0
inc     [bp+wx]
loc_4BA8E:
cmp     [bp+wx], e_WORLD_WIDTH
jl      short loc_4BA6D
inc     [bp+wy]
loc_4BA97:
cmp     [bp+wy], e_WORLD_HEIGHT
jl      short loc_4BA66
mov     [bp+src_city_idx], 0
jmp     loc_4BE10
; ---------------------------------------------------------------------------
loc_4BAA5:
mov     ax, [bp+src_city_idx]
mov     dx, size s_CITY
imul    dx
les     bx, [_CITIES]
add     bx, ax
mov     al, [es:bx+s_CITY.wx]
cbw
mov     [bp+src_wx], ax
mov     ax, [bp+src_city_idx]
mov     dx, size s_CITY
imul    dx
les     bx, [_CITIES]
add     bx, ax
mov     al, [es:bx+s_CITY.wy]
cbw
mov     [bp+src_wy], ax
mov     ax, [bp+src_city_idx]
inc     ax
mov     [bp+dst_city_idx], ax
jmp     loc_4BE01
; ---------------------------------------------------------------------------
loc_4BADB:
mov     ax, [bp+dst_city_idx]
mov     dx, size s_CITY
imul    dx
les     bx, [_CITIES]
add     bx, ax
mov     al, [es:bx+s_CITY.wx]
cbw
mov     [bp+dst_wx], ax
mov     ax, [bp+dst_city_idx]
mov     dx, size s_CITY
imul    dx
les     bx, [_CITIES]
add     bx, ax
mov     al, [es:bx+s_CITY.wy]
cbw
mov     [bp+dst_wy], ax
mov     ax, [bp+src_city_idx]
mov     dx, size s_CITY
imul    dx
les     bx, [_CITIES]
add     bx, ax
mov     al, [es:bx+s_CITY.wp]
cbw
cmp     ax, wp
jz      short loc_4BB21
jmp     loc_4BDFE
; ---------------------------------------------------------------------------
loc_4BB21:
mov     ax, [bp+dst_city_idx]
mov     dx, size s_CITY
imul    dx
les     bx, [_CITIES]
add     bx, ax
mov     al, [es:bx+s_CITY.wp]
cbw
cmp     ax, wp
jz      short loc_4BB3B
jmp     loc_4BDFE
; ---------------------------------------------------------------------------
loc_4BB3B:
mov     ax, [bp+src_city_idx]
mov     dx, size s_CITY
imul    dx
les     bx, [_CITIES]
add     bx, ax
cmp     [es:bx+s_CITY.owner_idx], NEUTRAL_PLAYER_IDX
jz      short loc_4BB53
jmp     loc_4BDFE
; ---------------------------------------------------------------------------
loc_4BB53:
mov     ax, [bp+dst_city_idx]
mov     dx, size s_CITY
imul    dx
les     bx, [_CITIES]
add     bx, ax
cmp     [es:bx+s_CITY.owner_idx], NEUTRAL_PLAYER_IDX
jz      short loc_4BB6B
jmp     loc_4BDFE
; ---------------------------------------------------------------------------
loc_4BB6B:
mov     ax, wp
mov     dx, e_WORLD_SIZE
imul    dx
les     bx, [_landmasses]
add     bx, ax
mov     ax, [bp+src_wy]
mov     dx, e_WORLD_WIDTH
imul    dx
add     bx, ax
add     bx, [bp+src_wx]
mov     al, [es:bx]
push    ax
mov     ax, wp
mov     dx, e_WORLD_SIZE
imul    dx
les     bx, [_landmasses]
add     bx, ax
mov     ax, [bp+dst_wy]
mov     dx, e_WORLD_WIDTH
imul    dx
add     bx, ax
add     bx, [bp+dst_wx]
pop     ax
cmp     al, [es:bx]
jz      short loc_4BBAC
jmp     loc_4BDFE
; ---------------------------------------------------------------------------
loc_4BBAC:
push    [bp+dst_wy]
push    [bp+dst_wx]
push    [bp+src_wy]
push    [bp+src_wx]
call    Range
add     sp, 8
cmp     ax, 11
jl      short loc_4BBC8
jmp     loc_4BDFE
; ---------------------------------------------------------------------------
loc_4BBC8:
mov     ax, e_WORLD_WIDTH
push    ax
lea     ax, [bp+Road_Ys]
push    ax
lea     ax, [bp+Road_Xs]
push    ax
push    [bp+dst_wy]
push    [bp+dst_wx]
push    [bp+src_wy]
push    [bp+src_wx]
call    Path_Wrap
add     sp, 0Eh
mov     [bp+Road_Length], ax
mov     [bp+Invalid_Road], e_ST_FALSE
mov     [bp+Line_Index], 0
jmp     loc_4BD2E
; ---------------------------------------------------------------------------
loc_4BBF9:
lea     ax, [bp+Road_Xs]
mov     bx, [bp+Line_Index]
add     bx, ax
mov     al, [bx]
cbw
mov     [bp+wx], ax
lea     ax, [bp+Road_Ys]
mov     bx, [bp+Line_Index]
add     bx, ax
mov     al, [bx]
cbw
mov     [bp+wy], ax
mov     ax, wp
mov     dx, e_WORLD_SIZE_DW
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, [bp+wy]
mov     dx, e_WORLD_WIDTH_DW
imul    dx
add     bx, ax
mov     ax, [bp+wx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], tt_Shore1_Fst
jb      short loc_4BC64
mov     ax, wp
mov     dx, e_WORLD_SIZE_DW
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, [bp+wy]
mov     dx, e_WORLD_WIDTH_DW
imul    dx
add     bx, ax
mov     ax, [bp+wx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], tt_Shore1_Lst
ja      short loc_4BC64
mov     [bp+Invalid_Road], e_ST_TRUE
loc_4BC64:
mov     ax, wp
mov     dx, e_WORLD_SIZE_DW
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, [bp+wy]
mov     dx, e_WORLD_WIDTH_DW
imul    dx
add     bx, ax
mov     ax, [bp+wx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], tt_Ocean1
jnz     short loc_4BC8D
mov     [bp+Invalid_Road], e_ST_TRUE
loc_4BC8D:
mov     ax, wp
mov     dx, e_WORLD_SIZE_DW
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, [bp+wy]
mov     dx, e_WORLD_WIDTH_DW
imul    dx
add     bx, ax
mov     ax, [bp+wx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], _Shore00001R10
jb      short loc_4BCDC
mov     ax, wp
mov     dx, e_WORLD_SIZE_DW
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, [bp+wy]
mov     dx, e_WORLD_WIDTH_DW
imul    dx
add     bx, ax
mov     ax, [bp+wx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], TT_Shore2F_end
ja      short loc_4BCDC
mov     [bp+Invalid_Road], e_ST_TRUE
loc_4BCDC:
mov     ax, wp
mov     dx, e_WORLD_SIZE_DW
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, [bp+wy]
mov     dx, e_WORLD_WIDTH_DW
imul    dx
add     bx, ax
mov     ax, [bp+wx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], TT_Shore2_1st
jb      short loc_4BD2B
mov     ax, wp
mov     dx, e_WORLD_SIZE_DW
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, [bp+wy]
mov     dx, e_WORLD_WIDTH_DW
imul    dx
add     bx, ax
mov     ax, [bp+wx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], tt_Ocean2
ja      short loc_4BD2B
mov     [bp+Invalid_Road], e_ST_TRUE
loc_4BD2B:
inc     [bp+Line_Index]
loc_4BD2E:
mov     ax, [bp+Road_Length]
dec     ax
cmp     ax, [bp+Line_Index]
jle     short loc_4BD40
cmp     [bp+Invalid_Road], e_ST_FALSE
jnz     short loc_4BD40
jmp     loc_4BBF9
; ---------------------------------------------------------------------------
loc_4BD40:
cmp     [bp+Invalid_Road], e_ST_FALSE
jz      short loc_4BD49
jmp     loc_4BDFE
; ---------------------------------------------------------------------------
loc_4BD49:
mov     [bp+Line_Index], 0
jmp     loc_4BDF2
; ---------------------------------------------------------------------------
loc_4BD51:
lea     ax, [bp+Road_Xs]
mov     bx, [bp+Line_Index]
add     bx, ax
mov     al, [bx]
cbw
mov     [bp+wx], ax
lea     ax, [bp+Road_Ys]
mov     bx, [bp+Line_Index]
add     bx, ax
mov     al, [bx]
cbw
mov     [bp+wy], ax
mov     ax, wp
mov     dx, e_WORLD_SIZE
imul    dx
les     bx, [_map_square_flags]
add     bx, ax
mov     ax, [bp+wy]
mov     dx, e_WORLD_WIDTH
imul    dx
add     bx, ax
add     bx, [bp+wx]
mov     al, [es:bx]
or      al, TF_Road
push    ax
mov     ax, wp
mov     dx, e_WORLD_SIZE
imul    dx
les     bx, [_map_square_flags]
add     bx, ax
mov     ax, [bp+wy]
mov     dx, e_WORLD_WIDTH
imul    dx
add     bx, ax
add     bx, [bp+wx]
pop     ax
mov     [es:bx], al
cmp     wp, 1
jnz     short loc_4BDEF
mov     ax, wp
mov     dx, e_WORLD_SIZE
imul    dx
les     bx, [_map_square_flags]
add     bx, ax
mov     ax, [bp+wy]
mov     dx, e_WORLD_WIDTH
imul    dx
add     bx, ax
add     bx, [bp+wx]
mov     al, [es:bx]
or      al, TF_Enc_Road
push    ax
mov     ax, wp
mov     dx, e_WORLD_SIZE
imul    dx
les     bx, [_map_square_flags]
add     bx, ax
mov     ax, [bp+wy]
mov     dx, e_WORLD_WIDTH
imul    dx
add     bx, ax
add     bx, [bp+wx]
pop     ax
mov     [es:bx], al
loc_4BDEF:
inc     [bp+Line_Index]
loc_4BDF2:
mov     ax, [bp+Road_Length]
dec     ax
cmp     ax, [bp+Line_Index]
jle     short loc_4BDFE
jmp     loc_4BD51
; ---------------------------------------------------------------------------
loc_4BDFE:
inc     [bp+dst_city_idx]
loc_4BE01:
mov     ax, [bp+dst_city_idx]
cmp     ax, [_cities]
jge     short loc_4BE0D
jmp     loc_4BADB
; ---------------------------------------------------------------------------
loc_4BE0D:
inc     [bp+src_city_idx]
loc_4BE10:
mov     ax, [bp+src_city_idx]
cmp     ax, [_cities]
jge     short loc_4BE1C
jmp     loc_4BAA5
; ---------------------------------------------------------------------------
loc_4BE1C:
mov     [bp+city_idx], 0
jmp     loc_4BF12
; ---------------------------------------------------------------------------
loc_4BE24:
mov     ax, [bp+city_idx]
mov     dx, size s_CITY
imul    dx
les     bx, [_CITIES]
add     bx, ax
mov     al, [es:bx+s_CITY.wp]
cbw
mov     dx, e_WORLD_SIZE
imul    dx
les     bx, [_map_square_flags]
add     bx, ax
mov     ax, [bp+city_idx]
mov     dx, size s_CITY
imul    dx
push    es
les     si, [_CITIES]
add     si, ax
mov     al, [es:si+s_CITY.wy]
cbw
mov     dx, e_WORLD_WIDTH
imul    dx
add     bx, ax
mov     ax, [bp+city_idx]
mov     dx, size s_CITY
imul    dx
les     si, [_CITIES]
add     si, ax
mov     al, [es:si+s_CITY.wx]
cbw
add     bx, ax
pop     es
mov     al, [es:bx]
or      al, TF_Road
push    ax
mov     ax, [bp+city_idx]
mov     dx, size s_CITY
imul    dx
les     bx, [_CITIES]
add     bx, ax
mov     al, [es:bx+s_CITY.wp]
cbw
mov     dx, e_WORLD_SIZE
imul    dx
les     bx, [_map_square_flags]
add     bx, ax
mov     ax, [bp+city_idx]
mov     dx, size s_CITY
imul    dx
push    es
les     si, [_CITIES]
add     si, ax
mov     al, [es:si+s_CITY.wy]
cbw
mov     dx, e_WORLD_WIDTH
imul    dx
add     bx, ax
mov     ax, [bp+city_idx]
mov     dx, size s_CITY
imul    dx
les     si, [_CITIES]
add     si, ax
mov     al, [es:si+s_CITY.wx]
cbw
add     bx, ax
pop     es
pop     ax
mov     [es:bx], al
cmp     wp, 1
jnz     short loc_4BF0F
mov     ax, wp
mov     dx, e_WORLD_SIZE
imul    dx
les     bx, [_map_square_flags]
add     bx, ax
mov     ax, [bp+wy]
mov     dx, e_WORLD_WIDTH
imul    dx
add     bx, ax
add     bx, [bp+wx]
mov     al, [es:bx]
or      al, TF_Enc_Road
push    ax
mov     ax, wp
mov     dx, e_WORLD_SIZE
imul    dx
les     bx, [_map_square_flags]
add     bx, ax
mov     ax, [bp+wy]
mov     dx, e_WORLD_WIDTH
imul    dx
add     bx, ax
add     bx, [bp+wx]
pop     ax
mov     [es:bx], al
loc_4BF0F:
inc     [bp+city_idx]
loc_4BF12:
mov     ax, [bp+city_idx]
cmp     ax, [_cities]
jge     short @@Done
jmp     loc_4BE24
; ---------------------------------------------------------------------------
@@Done:
pop     wp
pop     si
mov     sp, bp
pop     bp
retf
endp Generate_Roads
