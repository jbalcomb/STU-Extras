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
; Attributes: bp-based frame
proc UU_VGA_SlowMosaicFlip far
Unused_1= word ptr -12h
Unused_2= word ptr -10h
Unused_3= word ptr -0Eh
Unused_4= word ptr -0Ch
push    bp
mov     bp, sp
sub     sp, 12h
push    es
push    di
push    si
push    ds
push    cs
call    near ptr Toggle_Pages_No_Draw_Fields_Pre
nop
mov     [bp+Unused_4], 0
mov     [bp+Unused_2], 13Fh
mov     [bp+Unused_3], 0
mov     [bp+Unused_1], 31h
mov     ax, [draw_page_num]
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
loc_1F141:
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
loc_1F15D:
mov     [cs:Mosaic_Rnd50_Index], 49
loc_1F164:
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
cmp     cx, 28h
js      short loc_1F199
sub     cx, 28h
cmp     cx, 28h
js      short loc_1F199
sub     cx, 28h
loc_1F199:
mov     bx, cx
mov     bl, [cs:bx+140h]
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
mov     ah, [cs:bx+0Eh]
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
jz      short loc_1F1D5
loc_1F1D1:
shl     ah, 1
loop    loc_1F1D1
loc_1F1D5:
mov     dx, 3C4h
mov     al, 2
out     dx, al
inc     dx
mov     al, ah
out     dx, al
movsb
add     si, 1F3Fh
add     di, 1F3Fh
movsb
sub     si, 0FA1h
sub     di, 0FA1h
movsb
add     si, 1F3Fh
add     di, 1F3Fh
movsb
sub     si, 2EE0h
sub     di, 2EE0h
mov     ah, [cs:bx+0Eh]
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
jz      short loc_1F221
loc_1F21D:
shl     ah, 1
loop    loc_1F21D
loc_1F221:
mov     dx, 3C4h
mov     al, 2
out     dx, al
inc     dx
mov     al, ah
out     dx, al
movsb
add     si, 1F3Fh
add     di, 1F3Fh
movsb
sub     si, 0FA1h
sub     di, 0FA1h
movsb
add     si, 1F3Fh
add     di, 1F3Fh
movsb
dec     [cs:Mosaic_Rnd50_Index]
js      short loc_1F251
jmp     loc_1F164
loc_1F251:
dec     [cs:Mosaic_Rnd40_Index]
js      short loc_1F25B
jmp     loc_1F15D
loc_1F25B:
dec     [cs:Mosaic_InitVar]
jz      short loc_1F286
push    ax
push    bx
push    cx
push    dx
push    ds
push    es
push    si
push    di
call    Mark_Time
mov     ax, 3
push    ax
call    Release_Time
add     sp, 2
pop     di
pop     si
pop     es
pop     ds
pop     dx
pop     cx
pop     bx
pop     ax
jmp     loc_1F141
loc_1F286:
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
endp UU_VGA_SlowMosaicFlip
; Attributes: bp-based frame
proc RP_VGA_CutRight far
push    bp
mov     bp, sp
sub     sp, 2
push    es
push    di
push    si
push    ds
push    cs
call    near ptr Toggle_Pages_No_Draw_Fields_Pre
nop
mov     ax, [draw_page_num]
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
mov     ax, [cs:VGA_Draw_Seg]
mov     ds, ax
mov     [cs:VGA_ScanLine_Offset], 0
loc_1F2D5:
mov     [cs:VGA_Write_Map], 1
mov     [cs:VGA_Read_Map], 0
loc_1F2E3:
mov     ax, [cs:VGA_ScanLine_Offset]
mov     si, ax
mov     di, ax
mov     dx, 3CEh
mov     al, 4
out     dx, al
inc     dx
mov     al, [byte ptr cs:VGA_Read_Map]
out     dx, al
mov     dx, 3C4h
mov     al, 2
out     dx, al
inc     dx
mov     al, [byte ptr cs:VGA_Write_Map]
out     dx, al
mov     cx, 200
loc_1F306:
movsb
add     si, 79
add     di, 79
loop    loc_1F306
mov     cx, 1000
loc_1F312:
nop
loop    loc_1F312
inc     [cs:VGA_Read_Map]
shl     [cs:VGA_Write_Map], 1
cmp     [cs:VGA_Write_Map], 16
jnz     short loc_1F2E3
inc     [cs:VGA_ScanLine_Offset]
mov     ax, [cs:VGA_ScanLine_Offset]
cmp     ax, 80
jnz     short loc_1F2D5
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
endp RP_VGA_CutRight
; Attributes: bp-based frame
proc UU_VGA_InterleaveOut far
push    bp
mov     bp, sp
sub     sp, 4
push    es
push    di
push    si
push    ds
mov     ax, [draw_page_num]
mov     ah, al
xor     al, al
shl     ax, 1
shl     ax, 1
add     ax, 0A000h
mov     ds, ax
mov     es, ax
mov     [cs:VGA_Write_Map], 1
mov     [cs:VGA_Read_Map], 0
loc_1F36C:
mov     dx, 3CEh
loc_1F36F:
mov     al, 4
out     dx, al
inc     dx
mov     al, [byte ptr cs:VGA_Read_Map]
out     dx, al
mov     dx, 3C4h
mov     al, 2
out     dx, al
inc     dx
mov     al, [byte ptr cs:VGA_Write_Map]
out     dx, al
mov     dx, 100
mov     si, 82
loc_1F38A:
mov     di, 80
loc_1F38D:
mov     cx, 39
rep movsw
add     si, 82
add     di, 82
dec     dx
jnz     short loc_1F38D
mov     dx, 100
mov     si, 76
mov     di, 78
loc_1F3A4:
mov     cx, 39
loc_1F3A7:
movsw
sub     si, 4
sub     di, 4
loop    loc_1F3A7
add     si, 238
add     di, 238
dec     dx
jnz     short loc_1F3A4
inc     [cs:VGA_Read_Map]
shl     [cs:VGA_Write_Map], 1
cmp     [cs:VGA_Write_Map], 10h
jnz     short loc_1F36C
pop     ds
pop     si
pop     di
pop     es
mov     sp, bp
pop     bp
retf
endp UU_VGA_InterleaveOut
; Attributes: bp-based frame
proc UU_VGA_InterleaveIn far
Column_In= word ptr  6
push    bp
mov     bp, sp
sub     sp, 4
push    es
push    di
push    si
push    ds
shl     [bp+Column_In], 1
mov     ax, [bp+Column_In]
mov     [cs:VGA_SlideInColumn], ax
mov     ax, [draw_page_num]
mov     ah, al
xor     al, al
shl     ax, 1
shl     ax, 1
add     ax, 0A000h
mov     [cs:VGA_Display_Seg], ax
loc_1F3FB:
mov     bx, 0A400h
sub     bx, ax
add     bx, 0A000h
mov     [cs:VGA_Draw_Seg], bx
mov     ax, [cs:VGA_Display_Seg]
mov     es, ax
mov     ax, [cs:VGA_Draw_Seg]
mov     ds, ax
mov     [cs:VGA_Write_Map], 1
mov     [cs:VGA_Read_Map], 0
loc_1F423:
mov     dx, 3CEh
mov     al, 4
out     dx, al
inc     dx
mov     al, [byte ptr cs:VGA_Read_Map]
out     dx, al
mov     dx, 3C4h
mov     al, 2
out     dx, al
inc     dx
mov     al, [byte ptr cs:VGA_Write_Map]
out     dx, al
loc_1F43B:
mov     di, 0
loc_1F43E:
mov     si, 78
loc_1F441:
sub     si, [cs:VGA_SlideInColumn]
mov     cx, 100
loc_1F449:
movsw
add     si, 158
add     di, 158
loop    loc_1F449
mov     di, 158
mov     si, 80
add     si, [cs:VGA_SlideInColumn]
mov     cx, 100
loc_1F462:
movsw
add     si, 158
add     di, 158
loop    loc_1F462
inc     [cs:VGA_Read_Map]
shl     [cs:VGA_Write_Map], 1
cmp     [cs:VGA_Write_Map], 10h
jnz     short loc_1F423
pop     ds
pop     si
pop     di
pop     es
mov     sp, bp
pop     bp
retf
endp UU_VGA_InterleaveIn
; Attributes: bp-based frame
proc UU_VGA_CrossSlideOut far
Start_Off= word ptr  6
Line_Count= word ptr  8
push    bp
mov     bp, sp
sub     sp, 8
push    es
push    di
push    si
push    ds
mov     ax, [bp+Start_Off]
mov     [cs:VGA_Start_Offset], ax
mov     ax, [bp+Line_Count]
mov     [cs:VGA_Line_Count], ax
mov     ax, [draw_page_num]
mov     ah, al
xor     al, al
shl     ax, 1
shl     ax, 1
add     ax, 0A000h
mov     ds, ax
mov     es, ax
mov     [cs:VGA_Write_Map], 1
mov     [cs:VGA_Read_Map], 0
loc_1F4BF:
mov     dx, 3CEh
mov     al, 4
out     dx, al
inc     dx
mov     al, [byte ptr cs:VGA_Read_Map]
out     dx, al
mov     dx, 3C4h
mov     al, 2
out     dx, al
inc     dx
mov     al, [byte ptr cs:VGA_Write_Map]
out     dx, al
mov     dx, [cs:VGA_Line_Count]
mov     si, [cs:VGA_Start_Offset]
mov     di, si
add     si, 82
add     di, 80
loc_1F4E9:
mov     cx, 39
rep movsw
add     si, 82
add     di, 82
dec     dx
jnz     short loc_1F4E9
mov     dx, [cs:VGA_Line_Count]
mov     si, [cs:VGA_Start_Offset]
mov     di, si
add     si, 76
add     di, 78
loc_1F509:
mov     cx, 39
loc_1F50C:
movsw
sub     si, 4
sub     di, 4
loop    loc_1F50C
add     si, 238
add     di, 238
dec     dx
jnz     short loc_1F509
inc     [cs:VGA_Read_Map]
shl     [cs:VGA_Write_Map], 1
cmp     [cs:VGA_Write_Map], 10h
jnz     short loc_1F4BF
pop     ds
pop     si
pop     di
pop     es
mov     sp, bp
pop     bp
retf
endp UU_VGA_CrossSlideOut
; Attributes: bp-based frame
proc UU_VGA_CrossSlideIn far
Columns_In= word ptr  6
Start_Off= word ptr  8
Line_Count= word ptr  0Ah
push    bp
mov     bp, sp
sub     sp, 4
push    es
push    di
push    si
push    ds
shl     [bp+Columns_In], 1
mov     ax, [bp+Start_Off]
mov     [cs:VGA_Start_Offset], ax
mov     ax, [bp+Line_Count]
mov     [cs:VGA_Line_Count], ax
mov     ax, [bp+Columns_In]
mov     [cs:VGA_SlideInColumn], ax
mov     ax, [draw_page_num]
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
mov     ax, [cs:VGA_Draw_Seg]
mov     ds, ax
mov     [cs:VGA_Write_Map], 1
mov     [cs:VGA_Read_Map], 0
loc_1F596:
mov     dx, 3CEh
mov     al, 4
out     dx, al
inc     dx
mov     al, [byte ptr cs:VGA_Read_Map]
out     dx, al
mov     dx, 3C4h
mov     al, 2
out     dx, al
inc     dx
mov     al, [byte ptr cs:VGA_Write_Map]
out     dx, al
mov     di, [cs:VGA_Start_Offset]
mov     si, di
add     si, 78
sub     si, [cs:VGA_SlideInColumn]
mov     cx, [cs:VGA_Line_Count]
loc_1F5C2:
movsw
add     si, 158
add     di, 158
add     si, 2
loop    loc_1F5C2
mov     di, [cs:VGA_Start_Offset]
mov     si, di
add     di, 158
add     si, 80
add     si, [cs:VGA_SlideInColumn]
mov     cx, [cs:VGA_Line_Count]
loc_1F5E8:
movsw
add     si, 158
add     di, 158
sub     si, 2
loop    loc_1F5E8
inc     [cs:VGA_Read_Map]
shl     [cs:VGA_Write_Map], 1
cmp     [cs:VGA_Write_Map], 10h
jnz     short loc_1F596
pop     ds
pop     si
pop     di
pop     es
mov     sp, bp
pop     bp
retf
endp UU_VGA_CrossSlideIn
; Attributes: bp-based frame
proc UU_VGA_SplitCutOut far
push    bp
mov     bp, sp
sub     sp, 2
push    es
push    di
push    si
push    ds
push    cs
call    near ptr Toggle_Pages_No_Draw_Fields_Pre
nop
mov     ax, [draw_page_num]
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
mov     ax, [cs:VGA_Draw_Seg]
mov     ds, ax
mov     [cs:VGA_ScanLine_Offset], 28h
loc_1F652:
mov     [cs:VGA_Write_Map], 1
mov     [cs:VGA_Read_Map], 0
loc_1F660:
mov     ax, [cs:VGA_ScanLine_Offset]
mov     si, ax
mov     di, ax
mov     dx, 3CEh
mov     al, 4
out     dx, al
inc     dx
mov     al, [byte ptr cs:VGA_Read_Map]
out     dx, al
loc_1F674:
mov     dx, 3C4h
mov     al, 2
out     dx, al
inc     dx
mov     al, [byte ptr cs:VGA_Write_Map]
out     dx, al
push    si
push    di
mov     cx, 0C8h
loc_1F685:
movsb
add     si, 4Fh
add     di, 4Fh
loop    loc_1F685
pop     si
pop     di
mov     bx, si
sub     bx, 28h
mov     ax, 27h
sub     ax, bx
mov     si, ax
mov     bx, di
sub     bx, 28h
mov     ax, 27h
sub     ax, bx
mov     di, ax
mov     cx, 0C8h
loc_1F6AB:
movsb
add     si, 4Fh
add     di, 4Fh
loop    loc_1F6AB
mov     cx, 7D0h
loc_1F6B7:
nop
loop    loc_1F6B7
inc     [cs:VGA_Read_Map]
shl     [cs:VGA_Write_Map], 1
cmp     [cs:VGA_Write_Map], 10h
jnz     short loc_1F660
inc     [cs:VGA_ScanLine_Offset]
mov     ax, [cs:VGA_ScanLine_Offset]
cmp     ax, 50h
jz      short loc_1F6DD
jmp     loc_1F652
loc_1F6DD:
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
endp UU_VGA_SplitCutOut
; Attributes: bp-based frame
proc UU_VGA_CopyLine far
X_1= word ptr  6
Y_1= word ptr  8
X_2= word ptr  0Ah
Y_2= word ptr  0Ch
UpScale= word ptr  0Eh
push    bp
mov     bp, sp
sub     sp, 2
push    es
push    ds
push    si
push    di
mov     ax, [draw_page_num]
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
mov     ax, [bp+X_2]
cmp     ax, [bp+X_1]
jns     short loc_1F733
mov     si, ax
mov     ax, [bp+X_1]
mov     [bp+X_2], ax
mov     [bp+X_1], si
mov     si, [bp+Y_2]
mov     ax, [bp+Y_1]
mov     [bp+Y_2], ax
loc_1F730:
mov     [bp+Y_1], si
loc_1F733:
mov     ax, [bp+X_1]
loc_1F736:
mov     bx, [bp+Y_1]
mov     cx, [bp+X_2]
mov     dx, [bp+Y_2]
sub     cx, ax
sub     dx, bx
mov     [cs:VGA_Y_Dir], 5
jns     short loc_1F755
neg     dx
mov     [cs:VGA_Y_Dir], -5
loc_1F755:
cmp     cx, dx
jns     short loc_1F77E
inc     dx
mov     [cs:VGA_Line_Length], dx
mov     [cs:VGA_Vert_Slope], 100h
dec     dx
jz      short loc_1F79C
mov     bx, dx
mov     ah, cl
xor     al, al
mov     dl, ch
xor     dh, dh
mov     cx, bx
div     cx
loc_1F777:
mov     [cs:VGA_Horz_Slope], ax
jmp     short loc_1F79C
align 2
loc_1F77E:
inc     cx
mov     [cs:VGA_Line_Length], cx
dec     cx
jz      short loc_1F79C
mov     [cs:VGA_Horz_Slope], 100h
mov     ah, dl
xor     al, al
mov     dl, dh
xor     dh, dh
div     cx
mov     [cs:VGA_Vert_Slope], ax
loc_1F79C:
cmp     [bp+UpScale], 0
jz      short loc_1F7C0
mov     ax, [cs:VGA_Line_Length]
shl     ax, 1
mov     [cs:VGA_Line_Length], ax
mov     ax, [cs:VGA_Horz_Slope]
shr     ax, 1
mov     [cs:VGA_Horz_Slope], ax
mov     ax, [cs:VGA_Vert_Slope]
shr     ax, 1
mov     [cs:VGA_Vert_Slope], ax
loc_1F7C0:
mov     bx, [bp+Y_1]
mov     ax, bx
shl     ax, 1
shl     ax, 1
add     ax, bx
mov     bx, ax
mov     cx, [bp+X_1]
add     ax, [cs:VGA_Display_Seg]
mov     es, ax
add     bx, [cs:VGA_Draw_Seg]
mov     ds, bx
mov     ax, cx
shr     ax, 1
shr     ax, 1
mov     di, ax
mov     ax, 1
and     cx, 3
mov     [cs:VGA_Read_Mask], cx
jz      short loc_1F7F5
shl     ax, cl
loc_1F7F5:
xor     ah, ah
mov     [cs:VGA_Write_Map], ax
mov     bx, 80h
mov     cx, 80h
mov     dx, 3C4h
mov     al, 2
out     dx, al
inc     dx
mov     al, [byte ptr cs:VGA_Write_Map]
out     dx, al
mov     dx, 3CEh
mov     al, 4
out     dx, al
inc     dx
mov     al, [byte ptr cs:VGA_Read_Mask]
out     dx, al
mov     si, di
jmp     short loc_1F87D
align 2
loc_1F81E:
add     bx, [cs:VGA_Horz_Slope]
cmp     bh, 0
jz      short loc_1F85F
xor     bh, bh
inc     [byte ptr cs:VGA_Read_Mask]
mov     al, [byte ptr cs:VGA_Write_Map]
shl     al, 1
cmp     al, 9
js      short loc_1F843
mov     [byte ptr cs:VGA_Read_Mask], 0
mov     al, 1
inc     di
inc     si
loc_1F843:
mov     [byte ptr cs:VGA_Write_Map], al
mov     dx, 3C4h
mov     al, 2
out     dx, al
inc     dx
mov     al, [byte ptr cs:VGA_Write_Map]
out     dx, al
mov     dx, 3CEh
mov     al, 4
out     dx, al
inc     dx
mov     al, [byte ptr cs:VGA_Read_Mask]
out     dx, al
loc_1F85F:
add     cx, [cs:VGA_Vert_Slope]
cmp     ch, 0
jz      short loc_1F87D
xor     ch, ch
mov     dx, es
add     dx, [cs:VGA_Y_Dir]
mov     es, dx
mov     dx, ds
add     dx, [cs:VGA_Y_Dir]
mov     ds, dx
loc_1F87D:
movsb
dec     si
dec     di
dec     [cs:VGA_Line_Length]
jz      short loc_1F889
jmp     short loc_1F81E
loc_1F889:
pop     di
pop     si
pop     ds
pop     es
mov     sp, bp
pop     bp
retf
endp UU_VGA_CopyLine
proc Toggle_Pages_No_Draw_Fields_Pre far
push    ds
mov     ax, ds
mov     [GUI_DS_Save], ax
call    Save_Mouse_State
pop     ds
retf
endp Toggle_Pages_No_Draw_Fields_Pre
proc Toggle_Pages_No_Draw_Fields far
push    ds
mov     ax, seg dseg
mov     ds, ax
mov     ax, [GUI_DS_Save]
mov     ds, ax
call    Pointer_Y
push    ax
call    Pointer_X
push    ax
call    Check_Mouse_Shape
call    Save_Mouse_Off_Page
call    Draw_Mouse_Off_Page
add     sp, 4
call    Page_Flip
call    Restore_Mouse_Off_Page
call    Copy_Mouse_Off_To_Mouse_Back
call    Restore_Mouse_State
pop     ds
retf
endp Toggle_Pages_No_Draw_Fields
; Attributes: bp-based frame
proc UU_VGA_ColWipeLeft far
Column_In= word ptr  6
push    bp
mov     bp, sp
sub     sp, 4
push    es
push    di
push    si
push    ds
shl     [bp+Column_In], 1
mov     ax, [bp+Column_In]
mov     [cs:VGA_SlideInColumn], ax
mov     ax, [draw_page_num]
mov     ah, al
xor     al, al
shl     ax, 1
shl     ax, 1
add     ax, 0A000h
mov     [cs:VGA_Display_Seg], ax
mov     es, ax
mov     bx, 0A400h
sub     bx, ax
add     bx, 0A000h
mov     [cs:VGA_Draw_Seg], bx
mov     [cs:VGA_Write_Map], 1
mov     [cs:VGA_Read_Map], 0
loc_1F921:
mov     dx, 3CEh
mov     al, 4
out     dx, al
inc     dx
mov     al, [byte ptr cs:VGA_Read_Map]
out     dx, al
mov     dx, 3C4h
mov     al, 2
out     dx, al
inc     dx
mov     al, [byte ptr cs:VGA_Write_Map]
out     dx, al
mov     ax, [cs:VGA_Display_Seg]
mov     ds, ax
mov     dx, 200
mov     si, 82
mov     di, 80
loc_1F948:
mov     cx, 39
rep movsw
add     si, 2
add     di, 2
dec     dx
jnz     short loc_1F948
mov     ax, [cs:VGA_Draw_Seg]
mov     ds, ax
mov     di, 158
mov     si, 80
add     si, [cs:VGA_SlideInColumn]
mov     cx, 200
loc_1F96A:
movsw
add     si, 78
add     di, 78
loop    loc_1F96A
inc     [cs:VGA_Read_Map]
shl     [cs:VGA_Write_Map], 1
cmp     [cs:VGA_Write_Map], 10h
jnz     short loc_1F921
pop     ds
pop     si
pop     di
pop     es
mov     sp, bp
pop     bp
retf
endp UU_VGA_ColWipeLeft
; Attributes: bp-based frame
proc UU_VGA_ColWipeRight far
Column_In= word ptr  6
push    bp
mov     bp, sp
sub     sp, 4
push    es
push    di
push    si
push    ds
shl     [bp+Column_In], 1
mov     ax, [bp+Column_In]
mov     [cs:VGA_SlideInColumn], ax
mov     ax, [draw_page_num]
mov     ah, al
xor     al, al
shl     ax, 1
shl     ax, 1
add     ax, 0A000h
loc_1F9AF:
mov     [cs:VGA_Display_Seg], ax
mov     es, ax
mov     bx, 0A400h
sub     bx, ax
add     bx, 0A000h
mov     [cs:VGA_Draw_Seg], bx
mov     [cs:VGA_Write_Map], 1
loc_1F9CA:
mov     [cs:VGA_Read_Map], 0
loc_1F9D1:
mov     dx, 3CEh
mov     al, 4
out     dx, al
inc     dx
mov     al, [byte ptr cs:VGA_Read_Map]
out     dx, al
mov     dx, 3C4h
mov     al, 2
out     dx, al
inc     dx
loc_1F9E4:
mov     al, [byte ptr cs:VGA_Write_Map]
out     dx, al
mov     ax, [cs:VGA_Display_Seg]
mov     ds, ax
mov     dx, 200
mov     si, 76
mov     di, 78
loc_1F9F8:
mov     cx, 39
loc_1F9FB:
movsw
sub     si, 4
loc_1F9FF:
sub     di, 4
loop    loc_1F9FB
add     si, 158
add     di, 158
dec     dx
jnz     short loc_1F9F8
mov     ax, [cs:VGA_Draw_Seg]
mov     ds, ax
mov     di, 0
loc_1FA18:
mov     si, 78
sub     si, [cs:VGA_SlideInColumn]
mov     cx, 0C8h
loc_1FA23:
movsw
add     si, 4Eh
add     di, 4Eh
loop    loc_1FA23
inc     [cs:VGA_Read_Map]
loc_1FA31:
shl     [cs:VGA_Write_Map], 1
cmp     [cs:VGA_Write_Map], 10h
jnz     short loc_1F9D1
pop     ds
pop     si
pop     di
pop     es
mov     sp, bp
pop     bp
retf
endp UU_VGA_ColWipeRight
; Attributes: bp-based frame
proc VGA_ShiftCopyFromF3 far
x= word ptr  6
y= word ptr  8
push    bp
mov     bp, sp
sub     sp, 4
push    ds
push    es
push    si
push    di
mov     bx, 1
sub     bx, [draw_page_num]
mov     bh, bl
xor     bl, bl
shl     bx, 1
shl     bx, 1
add     bx, 0A000h
mov     es, bx
mov     ax, [bp+y]
mov     cx, 80
mul     cx
mov     [cs:VGA_Line_Offset], ax
mov     di, ax
mov     ax, [bp+x]
shr     ax, 1
shr     ax, 1
add     di, ax
mov     [cs:VGA_Origin_Offset], di
mov     bx, [bp+x]
and     bx, 3
add     bx, offset VGA_WriteMapMasks3
loc_1FA8B:
mov     bh, [bx]
xor     bl, bl
mov     ax, 0A800h
mov     ds, ax
assume ds:nothing
loc_1FA94:
mov     al, 4
mov     dx, 3CEh
out     dx, al
inc     dx
mov     al, bl
out     dx, al
mov     dx, 3C4h
mov     al, 2
out     dx, al
inc     dx
mov     al, bh
out     dx, al
xor     si, si
mov     cx, 8000
loc_1FAAD:
mov     ax, [cs:VGA_Line_Offset]
shr     ax, 1
sub     cx, ax
rep movsw
inc     bl
shl     bh, 1
cmp     bh, 9
js      short loc_1FAC7
mov     bh, 1
loc_1FAC2:
inc     [cs:VGA_Origin_Offset]
loc_1FAC7:
mov     di, [cs:VGA_Origin_Offset]
loc_1FACC:
cmp     bl, 4
loc_1FACF:
jnz     short loc_1FA94
pop     di
pop     si
pop     es
pop     ds
assume ds:dseg
loc_1FAD5:
mov     sp, bp
pop     bp
locret_1FAD8:
retf
endp VGA_ShiftCopyFromF3
ends seg026
