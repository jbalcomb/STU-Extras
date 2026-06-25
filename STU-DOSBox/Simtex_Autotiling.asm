; =============== S U B R O U T I N E =======================================
; Attributes: bp-based frame
proc Simtex_Autotiling far
terrtype= word ptr -0Ah
adj_wx= word ptr -8
mask= word ptr -6
wp= word ptr -4
wx= word ptr -2
wy = di
push    bp
mov     bp, sp
sub     sp, 0Ah
push    si
push    wy
mov     ax, 2560
push    ax
call    Near_Allocate_First
pop     cx
mov     [bp+terrtype], ax
mov     ax, 512
push    ax
mov     ax, 5
push    ax
xor     ax, ax
push    ax
push    [bp+terrtype]
xor     ax, ax
push    ax
mov     ax, offset terrtype_lbx_file__MGC_ovr051
push    ax
call    LBX_Load_Data_Static
add     sp, 0Ch
mov     [bp+wp], 0
jmp     loc_479FD
; ---------------------------------------------------------------------------
loc_47679:
xor     wy, wy
jmp     loc_479F2
; ---------------------------------------------------------------------------
loc_4767E:
mov     [bp+wx], 0
jmp     loc_479E8
; ---------------------------------------------------------------------------
loc_47686:
mov     ax, [bp+wp]
mov     dx, e_WORLD_SIZE_DW
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
mov     dx, e_WORLD_WIDTH_DW
imul    dx
add     bx, ax
mov     ax, [bp+wx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], tte_Ocean
jz      short loc_476AD
jmp     loc_479E5
; ---------------------------------------------------------------------------
loc_476AD:
mov     [bp+mask], 0
mov     ax, [bp+wx]
dec     ax
mov     [bp+adj_wx], ax
cmp     [bp+adj_wx], 0
jge     short loc_476C3
add     [bp+adj_wx], e_WORLD_WIDTH
loc_476C3:
mov     ax, [bp+wp]
mov     dx, e_WORLD_SIZE_DW
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
dec     ax
mov     dx, e_WORLD_WIDTH_DW
imul    dx
add     bx, ax
mov     ax, [bp+adj_wx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], tte_Grasslands1
jb      short loc_47717
mov     ax, [bp+wp]
mov     dx, e_WORLD_SIZE_DW
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
dec     ax
mov     dx, e_WORLD_WIDTH_DW
imul    dx
add     bx, ax
mov     ax, [bp+adj_wx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], tte_Hills
ja      short loc_47717
mov     ax, wy
dec     ax
jl      short loc_47717
inc     [bp+mask]
loc_47717:
mov     ax, [bp+wp]
mov     dx, e_WORLD_SIZE_DW
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
dec     ax
mov     dx, e_WORLD_WIDTH_DW
imul    dx
add     bx, ax
mov     ax, [bp+wx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], tte_Grasslands1
jb      short loc_4776C
mov     ax, [bp+wp]
mov     dx, e_WORLD_SIZE_DW
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
dec     ax
mov     dx, e_WORLD_WIDTH_DW
imul    dx
add     bx, ax
mov     ax, [bp+wx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], tte_Hills
ja      short loc_4776C
mov     ax, wy
dec     ax
jl      short loc_4776C
add     [bp+mask], 2
loc_4776C:
mov     ax, [bp+wx]
inc     ax
mov     [bp+adj_wx], ax
cmp     [bp+adj_wx], e_WORLD_WIDTH
jl      short loc_4777D
sub     [bp+adj_wx], e_WORLD_WIDTH
loc_4777D:
mov     ax, [bp+wp]
mov     dx, e_WORLD_SIZE_DW
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
dec     ax
mov     dx, e_WORLD_WIDTH_DW
imul    dx
add     bx, ax
mov     ax, [bp+adj_wx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], tte_Grasslands1
jb      short loc_477D2
mov     ax, [bp+wp]
mov     dx, e_WORLD_SIZE_DW
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
dec     ax
mov     dx, e_WORLD_WIDTH_DW
imul    dx
add     bx, ax
mov     ax, [bp+adj_wx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], tte_Hills
ja      short loc_477D2
mov     ax, wy
dec     ax
jl      short loc_477D2
add     [bp+mask], 4
loc_477D2:
mov     ax, [bp+wx]
inc     ax
mov     [bp+adj_wx], ax
cmp     [bp+adj_wx], e_WORLD_WIDTH
jl      short loc_477E3
sub     [bp+adj_wx], e_WORLD_WIDTH
loc_477E3:
mov     ax, [bp+wp]
mov     dx, e_WORLD_SIZE_DW
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
mov     dx, e_WORLD_WIDTH_DW
imul    dx
add     bx, ax
mov     ax, [bp+adj_wx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], tte_Grasslands1
jb      short loc_47831
mov     ax, [bp+wp]
mov     dx, e_WORLD_SIZE_DW
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
mov     dx, e_WORLD_WIDTH_DW
imul    dx
add     bx, ax
mov     ax, [bp+adj_wx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], tte_Hills
ja      short loc_47831
add     [bp+mask], 8
loc_47831:
mov     ax, [bp+wx]
inc     ax
mov     [bp+adj_wx], ax
cmp     [bp+adj_wx], e_WORLD_WIDTH
jl      short loc_47842
sub     [bp+adj_wx], e_WORLD_WIDTH
loc_47842:
mov     ax, [bp+wp]
mov     dx, e_WORLD_SIZE_DW
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
inc     ax
mov     dx, e_WORLD_WIDTH_DW
imul    dx
add     bx, ax
mov     ax, [bp+adj_wx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], tte_Grasslands1
jb      short loc_47897
mov     ax, [bp+wp]
mov     dx, e_WORLD_SIZE_DW
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
inc     ax
mov     dx, e_WORLD_WIDTH_DW
imul    dx
add     bx, ax
mov     ax, [bp+adj_wx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], tte_Hills
ja      short loc_47897
cmp     wy, e_WORLD_HEIGHT
jge     short loc_47897
add     [bp+mask], 16
loc_47897:
mov     ax, [bp+wp]
mov     dx, e_WORLD_SIZE_DW
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
inc     ax
mov     dx, e_WORLD_WIDTH_DW
imul    dx
add     bx, ax
mov     ax, [bp+wx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], tte_Grasslands1
jb      short loc_478EC
mov     ax, [bp+wp]
mov     dx, e_WORLD_SIZE_DW
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
inc     ax
mov     dx, e_WORLD_WIDTH_DW
imul    dx
add     bx, ax
mov     ax, [bp+wx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], tte_Hills
ja      short loc_478EC
cmp     wy, e_WORLD_HEIGHT
jge     short loc_478EC
add     [bp+mask], 32
loc_478EC:
mov     ax, [bp+wx]
dec     ax
mov     [bp+adj_wx], ax
cmp     [bp+adj_wx], 0
jge     short loc_478FD
add     [bp+adj_wx], e_WORLD_WIDTH
loc_478FD:
mov     ax, [bp+wp]
mov     dx, e_WORLD_SIZE_DW
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
inc     ax
mov     dx, e_WORLD_WIDTH_DW
imul    dx
add     bx, ax
mov     ax, [bp+adj_wx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], tte_Grasslands1
jb      short loc_47952
mov     ax, [bp+wp]
mov     dx, e_WORLD_SIZE_DW
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
inc     ax
mov     dx, e_WORLD_WIDTH_DW
imul    dx
add     bx, ax
mov     ax, [bp+adj_wx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], tte_Hills
ja      short loc_47952
cmp     wy, e_WORLD_HEIGHT
jge     short loc_47952
add     [bp+mask], 64
loc_47952:
mov     ax, [bp+wx]
dec     ax
mov     [bp+adj_wx], ax
cmp     [bp+adj_wx], 0
jge     short loc_47963
add     [bp+adj_wx], e_WORLD_WIDTH
loc_47963:
mov     ax, [bp+wp]
mov     dx, e_WORLD_SIZE_DW
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
mov     dx, e_WORLD_WIDTH_DW
imul    dx
add     bx, ax
mov     ax, [bp+adj_wx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], tte_Grasslands1
jb      short loc_479B2
mov     ax, [bp+wp]
mov     dx, e_WORLD_SIZE_DW
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
mov     dx, e_WORLD_WIDTH_DW
imul    dx
add     bx, ax
mov     ax, [bp+adj_wx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], tte_Hills
ja      short loc_479B2
add     [bp+mask], 128
loc_479B2:
cmp     [bp+mask], 0
jle     short loc_479E5
mov     ax, [bp+wp]
mov     dx, e_WORLD_SIZE_DW
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
mov     dx, e_WORLD_WIDTH_DW
imul    dx
add     bx, ax
mov     ax, [bp+wx]
shl     ax, 1
add     bx, ax
mov     ax, [bp+mask]
shl     ax, 1
mov     si, [bp+terrtype]
add     si, ax
mov     ax, [si]
mov     [es:bx], ax
loc_479E5:
inc     [bp+wx]
loc_479E8:
cmp     [bp+wx], e_WORLD_WIDTH
jge     short loc_479F1
jmp     loc_47686
; ---------------------------------------------------------------------------
loc_479F1:
inc     wy
loc_479F2:
cmp     wy, e_WORLD_HEIGHT
jge     short loc_479FA
jmp     loc_4767E
; ---------------------------------------------------------------------------
loc_479FA:
inc     [bp+wp]
loc_479FD:
cmp     [bp+wp], 2
jge     short loc_47A06
jmp     loc_47679
; ---------------------------------------------------------------------------
loc_47A06:
mov     [bp+wp], 0
jmp     loc_47E9C
; ---------------------------------------------------------------------------
loc_47A0E:
xor     wy, wy
jmp     loc_47E91
; ---------------------------------------------------------------------------
loc_47A13:
mov     [bp+wx], 0
jmp     loc_47E87
; ---------------------------------------------------------------------------
loc_47A1B:
mov     ax, [bp+wp]
mov     dx, e_WORLD_SIZE_DW
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
mov     dx, e_WORLD_WIDTH_DW
imul    dx
add     bx, ax
mov     ax, [bp+wx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], tte_Mountain
jz      short loc_47A43
jmp     loc_47E84
; ---------------------------------------------------------------------------
loc_47A43:
mov     [bp+mask], 0
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
dec     ax
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+wx]
dec     ax
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], tte_Mountain
jz      short loc_47ABD
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
dec     ax
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+wx]
dec     ax
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], _Mount0010
jb      short loc_47ACB
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
dec     ax
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+wx]
dec     ax
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], _Mount1001
ja      short loc_47ACB
loc_47ABD:
mov     ax, [bp+wx]
dec     ax
jl      short loc_47ACB
mov     ax, wy
dec     ax
jl      short loc_47ACB
inc     [bp+mask]
loc_47ACB:
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
dec     ax
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+wx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], tte_Mountain
jz      short loc_47B3D
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
dec     ax
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+wx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], _Mount0010
jb      short loc_47B46
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
dec     ax
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+wx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], _Mount1001
ja      short loc_47B46
loc_47B3D:
mov     ax, wy
dec     ax
jl      short loc_47B46
add     [bp+mask], 2
loc_47B46:
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
dec     ax
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+wx]
inc     ax
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], tte_Mountain
jz      short loc_47BBB
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
dec     ax
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+wx]
inc     ax
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], _Mount0010
jb      short loc_47BCD
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
dec     ax
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+wx]
inc     ax
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], _Mount1001
ja      short loc_47BCD
loc_47BBB:
mov     ax, [bp+wx]
inc     ax
cmp     ax, e_WORLD_WIDTH
jge     short loc_47BCD
mov     ax, wy
dec     ax
jl      short loc_47BCD
add     [bp+mask], 4
loc_47BCD:
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+wx]
inc     ax
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], tte_Mountain
jz      short loc_47C3F
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+wx]
inc     ax
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], _Mount0010
jb      short loc_47C4C
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+wx]
inc     ax
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], _Mount1001
ja      short loc_47C4C
loc_47C3F:
mov     ax, [bp+wx]
inc     ax
cmp     ax, e_WORLD_WIDTH
jge     short loc_47C4C
add     [bp+mask], 8
loc_47C4C:
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
inc     ax
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+wx]
inc     ax
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], tte_Mountain
jz      short loc_47CC1
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
inc     ax
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+wx]
inc     ax
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], _Mount0010
jb      short loc_47CD3
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
inc     ax
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+wx]
inc     ax
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], _Mount1001
ja      short loc_47CD3
loc_47CC1:
mov     ax, [bp+wx]
inc     ax
cmp     ax, e_WORLD_WIDTH
jge     short loc_47CD3
cmp     wy, e_WORLD_HEIGHT
jge     short loc_47CD3
add     [bp+mask], 16
loc_47CD3:
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
inc     ax
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+wx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], tte_Mountain
jz      short loc_47D45
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
inc     ax
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+wx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], _Mount0010
jb      short loc_47D4E
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
inc     ax
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+wx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], _Mount1001
ja      short loc_47D4E
loc_47D45:
cmp     wy, e_WORLD_HEIGHT
jge     short loc_47D4E
add     [bp+mask], 32
loc_47D4E:
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
inc     ax
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+wx]
dec     ax
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], tte_Mountain
jz      short loc_47DC3
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
inc     ax
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+wx]
dec     ax
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], _Mount0010
jb      short loc_47DD2
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
inc     ax
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+wx]
dec     ax
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], _Mount1001
ja      short loc_47DD2
loc_47DC3:
mov     ax, [bp+wx]
dec     ax
jl      short loc_47DD2
cmp     wy, e_WORLD_HEIGHT
jge     short loc_47DD2
add     [bp+mask], 64
loc_47DD2:
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+wx]
dec     ax
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], tte_Mountain
jz      short loc_47E44
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+wx]
dec     ax
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], _Mount0010
jb      short loc_47E4F
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+wx]
dec     ax
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], _Mount1001
ja      short loc_47E4F
loc_47E44:
mov     ax, [bp+wx]
dec     ax
jl      short loc_47E4F
add     [bp+mask], 128
loc_47E4F:
cmp     [bp+mask], 0
jle     short loc_47E84
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+wx]
shl     ax, 1
add     bx, ax
mov     ax, [bp+mask]
shl     ax, 1
mov     si, [bp+terrtype]
add     si, ax
mov     ax, [si+200h]
mov     [es:bx], ax
loc_47E84:
inc     [bp+wx]
loc_47E87:
cmp     [bp+wx], e_WORLD_WIDTH
jge     short loc_47E90
jmp     loc_47A1B
; ---------------------------------------------------------------------------
loc_47E90:
inc     wy
loc_47E91:
cmp     wy, e_WORLD_HEIGHT
jge     short loc_47E99
jmp     loc_47A13
; ---------------------------------------------------------------------------
loc_47E99:
inc     [bp+wp]
loc_47E9C:
cmp     [bp+wp], 2
jge     short loc_47EA5
jmp     loc_47A0E
; ---------------------------------------------------------------------------
loc_47EA5:
mov     [bp+wp], 0
jmp     loc_4836E
; ---------------------------------------------------------------------------
loc_47EAD:
xor     wy, wy
jmp     loc_48363
; ---------------------------------------------------------------------------
loc_47EB2:
mov     [bp+wx], 0
jmp     loc_48359
; ---------------------------------------------------------------------------
loc_47EBA:
mov     ax, [bp+wp]
mov     dx, e_WORLD_SIZE_DW
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
mov     dx, e_WORLD_WIDTH_DW
imul    dx
add     bx, ax
mov     ax, [bp+wx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], tte_Tundra1
jz      short loc_47EE2
jmp     loc_48356
; ---------------------------------------------------------------------------
loc_47EE2:
mov     [bp+mask], 0
mov     ax, [bp+wx]
dec     ax
mov     [bp+adj_wx], ax
cmp     [bp+adj_wx], 0
jge     short loc_47EF8
add     [bp+adj_wx], e_WORLD_WIDTH
loc_47EF8:
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
dec     ax
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+adj_wx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], tte_Tundra1
jz      short loc_47F72
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
dec     ax
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+adj_wx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], _Tundra00001000
jb      short loc_47F6A
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
dec     ax
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+adj_wx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], _TerType_Count
jbe     short loc_47F72
loc_47F6A:
mov     ax, wy
dec     ax
jl      short loc_47F72
inc     [bp+mask]
loc_47F72:
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
dec     ax
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+wx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], tte_Tundra1
jz      short loc_47FED
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
dec     ax
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+wx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], _Tundra00001000
jb      short loc_47FE4
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
dec     ax
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+wx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], _TerType_Count
jbe     short loc_47FED
loc_47FE4:
mov     ax, wy
dec     ax
jl      short loc_47FED
add     [bp+mask], 2
loc_47FED:
mov     ax, [bp+wx]
inc     ax
mov     [bp+adj_wx], ax
cmp     [bp+adj_wx], 3Ch
jl      short loc_47FFE
sub     [bp+adj_wx], 3Ch
loc_47FFE:
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
dec     ax
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+adj_wx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], tte_Tundra1
jz      short loc_48079
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
dec     ax
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+adj_wx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], _Tundra00001000
jb      short loc_48070
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
dec     ax
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+adj_wx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], _TerType_Count
jbe     short loc_48079
loc_48070:
mov     ax, wy
dec     ax
jl      short loc_48079
add     [bp+mask], 4
loc_48079:
mov     ax, [bp+wx]
inc     ax
mov     [bp+adj_wx], ax
cmp     [bp+adj_wx], 3Ch
jl      short loc_4808A
sub     [bp+adj_wx], 3Ch
loc_4808A:
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+adj_wx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], tte_Tundra1
jz      short loc_48106
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+adj_wx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], _Tundra00001000
jb      short loc_480F9
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+adj_wx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], _TerType_Count
jbe     short loc_48106
loc_480F9:
mov     ax, [bp+wx]
inc     ax
cmp     ax, 3Ch
jge     short loc_48106
add     [bp+mask], 8
loc_48106:
mov     ax, [bp+wx]
inc     ax
mov     [bp+adj_wx], ax
cmp     [bp+adj_wx], e_WORLD_WIDTH
jl      short loc_48117
sub     [bp+adj_wx], e_WORLD_WIDTH
loc_48117:
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
inc     ax
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+adj_wx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], tte_Tundra1
jz      short loc_48192
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
inc     ax
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+adj_wx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], _Tundra00001000
jb      short loc_48189
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
inc     ax
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+adj_wx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], _TerType_Count
jbe     short loc_48192
loc_48189:
cmp     wy, 28h
jge     short loc_48192
add     [bp+mask], 16
loc_48192:
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
inc     ax
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+wx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], tte_Tundra1
jz      short loc_4820D
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
inc     ax
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+wx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], _Tundra00001000
jb      short loc_48204
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
inc     ax
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+wx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], _TerType_Count
jbe     short loc_4820D
loc_48204:
cmp     wy, e_WORLD_HEIGHT
jge     short loc_4820D
add     [bp+mask], 32
loc_4820D:
mov     ax, [bp+wx]
dec     ax
mov     [bp+adj_wx], ax
cmp     [bp+adj_wx], 0
jge     short loc_4821E
add     [bp+adj_wx], e_WORLD_WIDTH
loc_4821E:
mov     ax, [bp+wp]
mov     dx, e_WORLD_SIZE_DW
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
inc     ax
mov     dx, e_WORLD_WIDTH_DW
imul    dx
add     bx, ax
mov     ax, [bp+adj_wx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], tte_Tundra1
jz      short loc_48299
mov     ax, [bp+wp]
mov     dx, e_WORLD_SIZE_DW
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
inc     ax
mov     dx, e_WORLD_WIDTH_DW
imul    dx
add     bx, ax
mov     ax, [bp+adj_wx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], _Tundra00001000
jb      short loc_48290
mov     ax, [bp+wp]
mov     dx, e_WORLD_SIZE_DW
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
inc     ax
mov     dx, e_WORLD_WIDTH_DW
imul    dx
add     bx, ax
mov     ax, [bp+adj_wx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], _TerType_Count
jbe     short loc_48299
loc_48290:
cmp     wy, e_WORLD_HEIGHT
jge     short loc_48299
add     [bp+mask], 64
loc_48299:
mov     ax, [bp+wx]
dec     ax
mov     [bp+adj_wx], ax
cmp     [bp+adj_wx], 0
jge     short loc_482AA
add     [bp+adj_wx], e_WORLD_WIDTH
loc_482AA:
mov     ax, [bp+wp]
mov     dx, e_WORLD_SIZE_DW
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
mov     dx, e_WORLD_WIDTH_DW
imul    dx
add     bx, ax
mov     ax, [bp+adj_wx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], tte_Tundra1
jz      short loc_4831E
mov     ax, [bp+wp]
mov     dx, e_WORLD_SIZE_DW
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
mov     dx, e_WORLD_WIDTH_DW
imul    dx
add     bx, ax
mov     ax, [bp+adj_wx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], _Tundra00001000
jb      short loc_48319
mov     ax, [bp+wp]
mov     dx, e_WORLD_SIZE_DW
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
mov     dx, e_WORLD_WIDTH_DW
imul    dx
add     bx, ax
mov     ax, [bp+adj_wx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], _TerType_Count
jbe     short loc_4831E
loc_48319:
add     [bp+mask], 128
loc_4831E:
cmp     [bp+mask], 0
jle     short loc_48356
mov     ax, [bp+mask]
shl     ax, 1
mov     bx, [bp+terrtype]
add     bx, ax
mov     ax, [bx]
add     ax, 600
push    ax
mov     ax, [bp+wp]
mov     dx, e_WORLD_SIZE_DW
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
mov     dx, e_WORLD_WIDTH_DW
imul    dx
add     bx, ax
mov     ax, [bp+wx]
shl     ax, 1
add     bx, ax
pop     ax
mov     [es:bx], ax
loc_48356:
inc     [bp+wx]
loc_48359:
cmp     [bp+wx], e_WORLD_WIDTH
jge     short loc_48362
jmp     loc_47EBA
; ---------------------------------------------------------------------------
loc_48362:
inc     wy
loc_48363:
cmp     wy, e_WORLD_HEIGHT
jge     short loc_4836B
jmp     loc_47EB2
; ---------------------------------------------------------------------------
loc_4836B:
inc     [bp+wp]
loc_4836E:
cmp     [bp+wp], 2
jge     short loc_48377
jmp     loc_47EAD
; ---------------------------------------------------------------------------
loc_48377:
mov     [bp+wp], 0
jmp     loc_48812
; ---------------------------------------------------------------------------
loc_4837F:
xor     wy, wy
jmp     loc_48807
; ---------------------------------------------------------------------------
loc_48384:
mov     [bp+wx], 0
jmp     loc_487FD
; ---------------------------------------------------------------------------
loc_4838C:
mov     ax, [bp+wp]
mov     dx, e_WORLD_SIZE_DW
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
mov     dx, e_WORLD_WIDTH_DW
imul    dx
add     bx, ax
mov     ax, [bp+wx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], tte_Hills
jz      short loc_483B4
jmp     loc_487FA
; ---------------------------------------------------------------------------
loc_483B4:
mov     [bp+mask], 0
mov     ax, [bp+wp]
mov     dx, e_WORLD_SIZE_DW
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
dec     ax
mov     dx, e_WORLD_WIDTH_DW
imul    dx
add     bx, ax
mov     ax, [bp+wx]
dec     ax
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], tte_Hills
jz      short loc_4842E
mov     ax, [bp+wp]
mov     dx, e_WORLD_SIZE_DW
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
dec     ax
mov     dx, e_WORLD_WIDTH_DW
imul    dx
add     bx, ax
mov     ax, [bp+wx]
dec     ax
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], _Hills_0010
jb      short loc_4843C
mov     ax, [bp+wp]
mov     dx, e_WORLD_SIZE_DW
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
dec     ax
mov     dx, e_WORLD_WIDTH_DW
imul    dx
add     bx, ax
mov     ax, [bp+wx]
dec     ax
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], _1Hills2
ja      short loc_4843C
loc_4842E:
mov     ax, [bp+wx]
dec     ax
jl      short loc_4843C
mov     ax, wy
dec     ax
jl      short loc_4843C
inc     [bp+mask]
loc_4843C:
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
dec     ax
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+wx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], tte_Hills
jz      short loc_484AE
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
dec     ax
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+wx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], _Hills_0010
jb      short loc_484B7
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
dec     ax
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+wx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], _1Hills2
ja      short loc_484B7
loc_484AE:
mov     ax, wy
dec     ax
jl      short loc_484B7
add     [bp+mask], 2
loc_484B7:
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
dec     ax
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+wx]
inc     ax
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], tte_Hills
jz      short loc_4852C
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
dec     ax
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+wx]
inc     ax
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], _Hills_0010
jb      short loc_4853E
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
dec     ax
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+wx]
inc     ax
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], _1Hills2
ja      short loc_4853E
loc_4852C:
mov     ax, [bp+wx]
inc     ax
cmp     ax, e_WORLD_WIDTH
jge     short loc_4853E
mov     ax, wy
dec     ax
jl      short loc_4853E
add     [bp+mask], 4
loc_4853E:
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+wx]
inc     ax
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], tte_Hills
jz      short loc_485B0
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+wx]
inc     ax
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], _Hills_0010
jb      short loc_485BD
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+wx]
inc     ax
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], _1Hills2
ja      short loc_485BD
loc_485B0:
mov     ax, [bp+wx]
inc     ax
cmp     ax, 3Ch
jge     short loc_485BD
add     [bp+mask], 8
loc_485BD:
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
inc     ax
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+wx]
inc     ax
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], tte_Hills
jz      short loc_48632
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
inc     ax
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+wx]
inc     ax
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], _Hills_0010
jb      short loc_48644
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
inc     ax
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+wx]
inc     ax
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], _1Hills2
ja      short loc_48644
loc_48632:
mov     ax, [bp+wx]
inc     ax
cmp     ax, 3Ch
jge     short loc_48644
cmp     wy, 28h
jge     short loc_48644
add     [bp+mask], 16
loc_48644:
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
inc     ax
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+wx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], tte_Hills
jz      short loc_486B6
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
inc     ax
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+wx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], _Hills_0010
jb      short loc_486BF
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
inc     ax
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+wx]
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], _1Hills2
ja      short loc_486BF
loc_486B6:
cmp     wy, 28h
jge     short loc_486BF
add     [bp+mask], 32
loc_486BF:
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
inc     ax
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+wx]
dec     ax
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], tte_Hills
jz      short loc_48734
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
inc     ax
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+wx]
dec     ax
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], _Hills_0010
jb      short loc_48743
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
inc     ax
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+wx]
dec     ax
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], _1Hills2
ja      short loc_48743
loc_48734:
mov     ax, [bp+wx]
dec     ax
jl      short loc_48743
cmp     wy, 28h
jge     short loc_48743
add     [bp+mask], 64
loc_48743:
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+wx]
dec     ax
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], tte_Hills
jz      short loc_487B5
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+wx]
dec     ax
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], _Hills_0010
jb      short loc_487C0
mov     ax, [bp+wp]
mov     dx, 12C0h
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
mov     dx, 78h
imul    dx
add     bx, ax
mov     ax, [bp+wx]
dec     ax
shl     ax, 1
add     bx, ax
cmp     [word ptr es:bx], _1Hills2
ja      short loc_487C0
loc_487B5:
mov     ax, [bp+wx]
dec     ax
jl      short loc_487C0
add     [bp+mask], 128
loc_487C0:
cmp     [bp+mask], 0
jle     short loc_487FA
mov     ax, [bp+mask]
shl     ax, 1
mov     bx, [bp+terrtype]
add     bx, ax
mov     ax, [bx+200h]
add     ax, 16
push    ax
mov     ax, [bp+wp]
mov     dx, e_WORLD_SIZE_DW
imul    dx
les     bx, [_world_maps]
add     bx, ax
mov     ax, wy
mov     dx, e_WORLD_WIDTH_DW
imul    dx
add     bx, ax
mov     ax, [bp+wx]
shl     ax, 1
add     bx, ax
pop     ax
mov     [es:bx], ax
loc_487FA:
inc     [bp+wx]
loc_487FD:
cmp     [bp+wx], e_WORLD_WIDTH
jge     short loc_48806
jmp     loc_4838C
; ---------------------------------------------------------------------------
loc_48806:
inc     wy
loc_48807:
cmp     wy, e_WORLD_HEIGHT
jge     short loc_4880F
jmp     loc_48384
; ---------------------------------------------------------------------------
loc_4880F:
inc     [bp+wp]
loc_48812:
cmp     [bp+wp], 2
jge     short @@Done
jmp     loc_4837F
; ---------------------------------------------------------------------------
@@Done:
pop     wy
pop     si
mov     sp, bp
pop     bp
retf
endp Simtex_Autotiling
