; Segment type: Pure code
segment seg026 byte public 'CODE' use16
assume cs:seg026
;org 0Eh
assume es:nothing, ss:nothing, ds:dseg, fs:nothing, gs:nothing
Mosaic_Rnd256_Bits db 2, 2, 2, 1, 1, 3, 0, 3, 3, 2, 0, 1, 3, 3, 1, 3, 3, 0, 3, 1, 2, 3, 0, 2, 1, 1, 2, 2, 1, 1, 3, 3, 2, 3, 3, 0, 2, 3, 0, 2, 0, 3, 2, 0, 2, 0, 0, 3, 1, 0, 1, 1, 1, 3, 3, 1, 1, 0, 0, 2, 1, 1, 2, 1, 0, 0, 0, 3, 3, 1, 1, 2, 2, 1, 2, 2, 0, 2, 3, 1, 2, 2, 2, 0, 0
db 0, 1, 1, 0, 2, 0, 3, 2, 2, 1, 0, 3, 1, 2, 3, 0, 0, 1, 0, 2, 2, 3, 2, 1, 1, 2, 2, 0, 3, 0, 0, 2, 0, 0, 3, 3, 1, 2, 1, 0, 0, 1, 3, 1, 1, 1, 0, 0, 2, 3, 1, 0, 3, 3, 0, 1, 0, 0, 0, 1, 3, 1, 3, 3, 2, 2, 3, 2, 0, 1, 1, 0, 3, 0, 2, 0, 2, 1, 1, 3, 1, 2, 1, 3, 1
db 0, 1, 3, 3, 1, 1, 3, 2, 3, 2, 3, 1, 1, 2, 0, 2, 3, 3, 2, 2, 0, 2, 3, 3, 3, 2, 2, 0, 2, 0, 1, 0, 1, 3, 2, 1, 2, 2, 0, 1, 0, 2, 1, 1, 3, 0, 3, 3, 3, 0, 3, 0, 2, 1, 1, 0, 0, 2, 1, 2, 3, 3, 1, 0, 1, 3, 0, 2, 3, 0, 2, 1, 2, 3, 0, 2, 2, 0, 2, 3, 1, 0, 3, 3, 3
db 0
Mosaic_Rnd50_Tbl db 0, 3, 1Eh, 12h, 1Ch, 16h, 1Dh, 0Ch, 22h, 2Fh, 1Fh, 20h, 7, 31h, 2Eh, 0Eh, 26h, 2Bh, 23h, 28h, 0Bh, 9, 24h, 4, 21h, 1Ah, 27h, 0Fh, 5, 15h, 2, 10h, 19h, 0Ah, 30h, 0Dh, 18h, 25h, 11h, 13h, 2Ch, 6, 8, 14h, 1, 17h, 29h, 1Bh, 2Dh, 2Ah
Mosaic_Rnd40_Tbl db 2, 9, 1Eh, 26h, 0Dh, 1Bh, 1Fh, 1, 13h, 8, 18h, 24h, 25h, 1Ah, 0, 0Bh, 17h, 20h, 16h, 0Ch, 10h, 14h, 1Dh, 12h, 1Ch, 7, 0Ah, 23h, 0Eh, 19h, 5, 21h, 0Fh, 15h, 4, 11h, 6, 3, 27h, 22h
db    0
db    0
db    0
db    0
Mosaic_InitVar dw 0
Mosaic_Rnd40_Index dw 0
Mosaic_Init64Times dw 0
Mosaic_Rnd50_Index dw 0
VGA_ScanLine_Offset dw 0
VGA_Write_Map dw 0
VGA_Read_Map dw 0
VGA_SlideInColumn dw 0
VGA_Line_Count dw 0
VGA_Start_Offset dw 0
db 2 dup(0)
VGA_Horz_Slope dw 0
VGA_Vert_Slope dw 0
VGA_Y_Dir dw 0
VGA_Line_Length dw 0
VGA_Display_Seg dw 0
VGA_Draw_Seg dw 0
VGA_Read_Mask dw 0
VGA_Line_Offset dw 0
VGA_Origin_Offset dw 0

; Attributes: bp-based frame
proc VGA_MosaicFlip__STUB far
UU_y2= word ptr -12h
UU_x2= word ptr -10h
UU_y1= word ptr -0Eh
UU_x1= word ptr -0Ch
push    bp
mov     bp, sp
sub     sp, 12h
push    es
push    di
loc_1EF6C:
push    si
push    ds
push    cs
call    near ptr Toggle_Pages_No_Draw_Fields_Pre
nop
mov     [bp+UU_x1], 0
loc_1EF78:
mov     [bp+UU_x2], 319
loc_1EF7D:
mov     [bp+UU_y1], 0
mov     [bp+UU_y2], 49
mov     ax, [draw_page_num]
loc_1EF8A:
mov     ah, al
xor     al, al
shl     ax, 1
shl     ax, 1
add     ax, 0A000h
mov     [cs:VGA_Display_Seg], ax
mov     bx, 0A400h
sub     bx, ax
add     bx, 0A000h
mov     [cs:VGA_Draw_Seg], bx
mov     ax, [cs:VGA_Display_Seg]
mov     es, ax
mov     [cs:Mosaic_InitVar], 4
mov     ax, [cs:VGA_Draw_Seg]
mov     ds, ax
loc_1EFBA:
mov     ax, [cs:Mosaic_InitVar]
dec     ax
shl     ax, 1
shl     ax, 1
shl     ax, 1
shl     ax, 1
shl     ax, 1
shl     ax, 1
mov     [cs:Mosaic_Init64Times], ax
mov     [cs:Mosaic_Rnd40_Index], 39
loc_1EFD6:
mov     [cs:Mosaic_Rnd50_Index], 49
loc_1EFDD:
mov     bx, [cs:Mosaic_Rnd50_Index]
mov     bl, [cs:Mosaic_Rnd50_Tbl+bx]
xor     bh, bh
mov     cx, bx
mov     dx, bx
shl     bx, 1
shl     bx, 1
add     bx, cx
shl     bx, 1
shl     bx, 1
shl     bx, 1
shl     bx, 1
mov     ax, bx
add     cx, [cs:Mosaic_Rnd40_Index]
cmp     cx, 40
js      short loc_1F012
sub     cx, 40
cmp     cx, 40
js      short loc_1F012
sub     cx, 40
loc_1F012:
mov     bx, cx
mov     bl, [cs:Mosaic_Rnd40_Tbl+bx]
xor     bh, bh
mov     cx, bx
shl     bx, 1
add     ax, bx
mov     si, ax
mov     di, ax
add     cx, dx
add     cx, [cs:Mosaic_Init64Times]
xor     ch, ch
mov     bx, cx
mov     ah, [cs:Mosaic_Rnd256_Bits+bx]
inc     bl
mov     dx, 3CEh
mov     al, 4
out     dx, al
inc     dx
mov     al, ah
out     dx, al
mov     cl, ah
mov     ah, 1
cmp     cl, 0
jz      short loc_1F04E
loc_1F04A:
shl     ah, 1
loop    loc_1F04A
loc_1F04E:
mov     dx, 3C4h
mov     al, 2
out     dx, al
inc     dx
mov     al, ah
out     dx, al
movsb
add     si, 7999
add     di, 7999
movsb
sub     si, 4001
sub     di, 4001
movsb
add     si, 7999
add     di, 7999
movsb
sub     si, 12000
sub     di, 12000
mov     ah, [cs:Mosaic_Rnd256_Bits+bx]
inc     bl
mov     dx, 3CEh
mov     al, 4
out     dx, al
inc     dx
mov     al, ah
out     dx, al
mov     cl, ah
mov     ah, 1
cmp     cl, 0
jz      short loc_1F09A
loc_1F096:
shl     ah, 1
loop    loc_1F096
loc_1F09A:
mov     dx, 3C4h
mov     al, 2
out     dx, al
inc     dx
mov     al, ah
out     dx, al
movsb
add     si, 7999
add     di, 7999
movsb
sub     si, 4001
sub     di, 4001
movsb
add     si, 7999
add     di, 7999
movsb
dec     [cs:Mosaic_Rnd50_Index]
js      short loc_1F0CA
jmp     loc_1EFDD
loc_1F0CA:
dec     [cs:Mosaic_Rnd40_Index]
js      short loc_1F0D4
jmp     loc_1EFD6
loc_1F0D4:
dec     [cs:Mosaic_InitVar]
jz      short @@Done
jmp     loc_1EFBA
@@Done:
push    cs
call    near ptr Toggle_Pages_No_Draw_Fields
nop
pop     ds
pop     si
pop     di
pop     es
mov     sp, bp
pop     bp
retf
endp VGA_MosaicFlip__STUB
