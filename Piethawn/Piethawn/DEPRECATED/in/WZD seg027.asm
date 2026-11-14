; Segment type: Pure code
segment seg027 byte public 'CODE' use16
assume cs:seg027
;org 9
assume es:nothing, ss:nothing, ds:dseg, fs:nothing, gs:nothing
; Attributes: bp-based frame
proc VGA_PartCopyFromF3 far
x= word ptr  6
y= word ptr  8
_SI_x = si
_DI_y = di
push    bp
loc_1FADA:
mov     bp, sp
push    _SI_x
push    _DI_y
loc_1FADE:
mov     _SI_x, [bp+x]
mov     _DI_y, [bp+y]
push    _DI_y
push    _SI_x
loc_1FAE6:
call    VGA_ShiftCopyFromF3
pop     cx
loc_1FAEC:
pop     cx
or      _SI_x, _SI_x
jz      short loc_1FB0A
xor     ax, ax
push    ax
mov     ax, e_SCREEN_YMAX
push    ax
mov     ax, _SI_x
dec     ax
push    ax
xor     ax, ax
push    ax
xor     ax, ax
push    ax
call    Fill
add     sp, 0Ah
loc_1FB0A:
or      _DI_y, _DI_y
jz      short @@Done
xor     ax, ax
push    ax
mov     ax, _DI_y
dec     ax
push    ax
mov     ax, e_SCREEN_XMAX
push    ax
xor     ax, ax
push    ax
xor     ax, ax
push    ax
call    Fill
add     sp, 0Ah
@@Done:
pop     _DI_y
pop     _SI_x
pop     bp
retf
endp VGA_PartCopyFromF3
; Attributes: bp-based frame
proc UU_VGA_SliceFlip far
_SI_itr = si
push    bp
mov     bp, sp
push    _SI_itr
push    di
mov     _SI_itr, 19
jmp     short loc_1FB63
loc_1FB35:
xor     di, di
jmp     short loc_1FB5C
loc_1FB39:
xor     ax, ax
push    ax
mov     ax, di
add     ax, _SI_itr
push    ax
mov     ax, 319
push    ax
mov     ax, di
add     ax, _SI_itr
push    ax
xor     ax, ax
push    ax
call    UU_VGA_CopyLine
add     sp, 0Ah
mov     ax, di
add     ax, 20
mov     di, ax
loc_1FB5C:
cmp     di, 200
jl      short loc_1FB39
dec     _SI_itr
loc_1FB63:
or      _SI_itr, _SI_itr
jge     short loc_1FB35
loc_1FB67:
call    Pointer_Y
push    ax
call    Pointer_X
push    ax
call    Save_Mouse_On_Page
pop     cx
pop     cx
pop     di
pop     _SI_itr
pop     bp
retf
endp UU_VGA_SliceFlip
; Attributes: bp-based frame
proc UU_VGA_SquaresFlip far
Loop_Var= word ptr -2
push    bp
mov     bp, sp
sub     sp, 2
push    si
push    di
mov     [bp+Loop_Var], 0
jmp     short loc_1FBB7
loc_1FB8D:
xor     si, si
jmp     short loc_1FBAE
loc_1FB91:
xor     di, di
jmp     short loc_1FBA5
loc_1FB95:
push    [bp+Loop_Var]
loc_1FB98:
push    di
push    si
nop
push    cs
call    near ptr UU_VGA_CopyRect
add     sp, 6
add     di, 20
loc_1FBA5:
cmp     di, 200
jl      short loc_1FB95
add     si, 20
loc_1FBAE:
cmp     si, 320
jl      short loc_1FB91
loc_1FBB4:
inc     [bp+Loop_Var]
loc_1FBB7:
cmp     [bp+Loop_Var], 10
jl      short loc_1FB8D
loc_1FBBD:
call    Pointer_Y
push    ax
call    Pointer_X
push    ax
call    Save_Mouse_On_Page
pop     cx
pop     cx
pop     di
pop     si
mov     sp, bp
pop     bp
retf
endp UU_VGA_SquaresFlip
; Attributes: bp-based frame
proc UU_VGA_CopyRect far
Bottom= word ptr -4
Right= word ptr -2
Left= word ptr  6
Top= word ptr  8
Offset= word ptr  0Ah
push    bp
mov     bp, sp
sub     sp, 4
push    si
push    di
mov     si, [bp+Left]
mov     di, [bp+Top]
mov     ax, si
add     ax, 19
mov     [bp+Right], ax
mov     ax, di
add     ax, 19
mov     [bp+Bottom], ax
add     si, [bp+Offset]
add     di, [bp+Offset]
mov     ax, [bp+Offset]
sub     [bp+Right], ax
mov     ax, [bp+Offset]
sub     [bp+Bottom], ax
xor     ax, ax
push    ax
push    di
push    [bp+Right]
push    di
push    si
call    UU_VGA_CopyLine
add     sp, 0Ah
xor     ax, ax
push    ax
push    [bp+Bottom]
push    si
push    di
push    si
call    UU_VGA_CopyLine
loc_1FC25:
add     sp, 0Ah
xor     ax, ax
push    ax
push    [bp+Bottom]
push    [bp+Right]
push    [bp+Bottom]
push    si
call    UU_VGA_CopyLine
add     sp, 0Ah
xor     ax, ax
push    ax
push    [bp+Bottom]
loc_1FC43:
push    [bp+Right]
push    di
push    [bp+Right]
call    UU_VGA_CopyLine
add     sp, 0Ah
loc_1FC52:
pop     di
pop     si
loc_1FC54:
mov     sp, bp
pop     bp
retf
endp UU_VGA_CopyRect
; Attributes: bp-based frame
proc UU_VGA_InterleavedFlip far
_SI_itr = si
push    bp
mov     bp, sp
push    _SI_itr
xor     _SI_itr, _SI_itr
jmp     short loc_1FC6D
loc_1FC60:
call    UU_VGA_InterleaveOut
push    _SI_itr
call    UU_VGA_InterleaveIn
pop     cx
loc_1FC6C:
inc     _SI_itr
loc_1FC6D:
cmp     _SI_itr, 40
jl      short loc_1FC60
call    Page_Flip
loc_1FC77:
call    Pointer_Y
push    ax
loc_1FC7D:
call    Pointer_X
push    ax
call    Save_Mouse_On_Page
pop     cx
pop     cx
loc_1FC8A:
pop     _SI_itr
pop     bp
retf
endp UU_VGA_InterleavedFlip
; Attributes: bp-based frame
proc UU_VGA_CrossSlideFlip far
Columns_In= word ptr -4
Line_Count= word ptr -2
push    bp
mov     bp, sp
sub     sp, 4
push    si
push    di
xor     si, si
jmp     short loc_1FCF4
loc_1FC99:
cmp     si, 40
jge     short loc_1FCAB
xor     di, di
mov     ax, si
inc     ax
mov     [bp+Line_Count], ax
mov     [bp+Columns_In], si
jmp     short loc_1FCCB
loc_1FCAB:
mov     [bp+Columns_In], 39
mov     ax, si
add     ax, -39
mov     di, ax
cmp     di, 60
jge     short loc_1FCC3
mov     [bp+Line_Count], 40
jmp     short loc_1FCCB
loc_1FCC3:
mov     ax, 100
sub     ax, di
mov     [bp+Line_Count], ax
loc_1FCCB:
push    [bp+Line_Count]
mov     ax, di
mov     dx, 160
imul    dx
push    ax
call    UU_VGA_CrossSlideOut
pop     cx
pop     cx
push    [bp+Line_Count]
mov     ax, di
mov     dx, 160
imul    dx
push    ax
push    [bp+Columns_In]
call    UU_VGA_CrossSlideIn
add     sp, 6
inc     si
loc_1FCF4:
cmp     si, 139
jl      short loc_1FC99
loc_1FCFA:
call    Page_Flip
call    Pointer_Y
push    ax
call    Pointer_X
push    ax
call    Save_Mouse_On_Page
pop     cx
pop     cx
pop     di
loc_1FD13:
pop     si
mov     sp, bp
pop     bp
retf
endp UU_VGA_CrossSlideFlip
; Attributes: bp-based frame
proc UU_VGA_WipeLeft far
_SI_itr = si
push    bp
mov     bp, sp
push    _SI_itr
loc_1FD1C:
xor     _SI_itr, _SI_itr
jmp     short loc_1FD28
loc_1FD20:
push    _SI_itr
call    UU_VGA_ColWipeLeft
pop     cx
inc     _SI_itr
loc_1FD28:
cmp     _SI_itr, 40
loc_1FD2B:
jl      short loc_1FD20
call    Page_Flip
call    Pointer_Y
push    ax
call    Pointer_X
push    ax
call    Save_Mouse_On_Page
pop     cx
pop     cx
pop     _SI_itr
pop     bp
retf
endp UU_VGA_WipeLeft
; Attributes: bp-based frame
proc UU_VGA_WipeRight far
push    bp
mov     bp, sp
push    si
xor     si, si
jmp     short loc_1FD58
loc_1FD50:
push    si
loc_1FD51:
call    UU_VGA_ColWipeRight
loc_1FD56:
pop     cx
inc     si
loc_1FD58:
cmp     si, 40
loc_1FD5B:
jl      short loc_1FD50
loc_1FD5D:
call    Page_Flip
loc_1FD62:
call    Pointer_Y
push    ax
loc_1FD68:
call    Pointer_X
push    ax
loc_1FD6E:
call    Save_Mouse_On_Page
pop     cx
pop     cx
pop     si
loc_1FD76:
pop     bp
retf
endp UU_VGA_WipeRight
ends seg027
