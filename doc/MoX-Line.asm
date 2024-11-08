
; 
; DS:SI     
; ES:DI     
;
;   SI      'temp' for swap of x1,x2 and y1,y2
;
;   AX        
;       AL      color
;       AH      VGA Map Mask   3C5
;
;   BX      
;
;   CX      Delta X  (x2 - x1)
;
;   DX      Delta Y  (y2 - y1)
;
;



; drake178: VGA_DrawLine()
; draws a line into the current draw segment using
; Bresenham's line-drawing algorithm (limited to the
; increasing X directions)
; Attributes: bp-based frame

; int __cdecl __far Line(int x1, int y1, int x2, int y2, char color)
proc Line far

Y_Dir= word ptr -8
Horz_Slope= word ptr -6
Line_Length= word ptr -4
Vert_Slope= word ptr -2
x1= word ptr  6
y1= word ptr  8
x2= word ptr  0Ah
y2= word ptr  0Ch
color= byte ptr  0Eh


push    bp
mov     bp, sp
sub     sp, 0Ah
push    es
push    ds
push    si
push    di


mov     dx, e_SC_INDEX
mov     al, e_SC_MAPMASK
out     dx, al

mov     ax, [bp+x2]
cmp     ax, [bp+x1]
jns     short @@LineIsTopToBottom       ; MoO2 does if y1 > y2

; x2 < x1

@@Swap_X2X1_Y2Y1:
mov     _SI_color, ax
mov     ax, [bp+x1]
mov     [bp+x2], ax
mov     [bp+x1], _SI_color

mov     _SI_color, [bp+y2]
mov     ax, [bp+y1]
mov     [bp+y2], ax
mov     [bp+y1], _SI_color


@@LineIsTopToBottom:
mov     ax, [bp+x1]
mov     bx, [bp+y1]
mov     cx, [bp+x2]
mov     dx, [bp+y2]


sub     cx, ax                          ; x_delta = x2 - x1
sub     dx, bx                          ; y_delta = y2 - y1


mov     [bp+Y_Dir], 5                   ; 5 as segment adresses, 5 * 16 = 80


jns     short loc_1916B                 ; if(y_delta < 0)

; (y2 < y1) AKA (delta y < 0)
neg     dx
mov     [bp+Y_Dir], -5

if (dxabs >= dyabs)
/* the line is more horizontal than vertical */

loc_1916B:
cmp     cx, dx
jns     short loc_1918F

delta_x < delta_y
inc     dx
mov     [bp+Line_Length], dx
mov     [bp+Vert_Slope], 256
dec     dx
jz      short loc_191A8

mov     bx, dx
mov     ah, cl
xor     al, al
mov     dl, ch
xor     dh, dh
mov     cx, bx
div     cx
mov     [bp+Horz_Slope], ax
jmp     short loc_191A8

; delta y < delta x

loc_1918F:
inc     cx
mov     [bp+Line_Length], cx
dec     cx
jz      short loc_191A8

mov     [bp+Horz_Slope], 256
mov     ah, dl
xor     al, al
mov     dl, dh
xor     dh, dh
div     cx
mov     [bp+Vert_Slope], ax


loc_191A8:
mov     bx, [bp+y1]
mov     ax, bx
shl     ax, 1
shl     ax, 1
add     ax, bx                          ; * 5 as a segment address = * 80 total bytes which,
                                        ; since each byte is 4 pixels, equals the draw row
add     ax, [current_video_page]
mov     es, ax
mov     cx, [bp+x1]
mov     ax, cx
shr     ax, 1
shr     ax, 1
mov     di, ax                          ; X / 4 = pixel offset in video memory
                                        ; X mod 4 = plane index of the pixel
and     cx, 3
mov     ax, 1
shl     ax, cl
mov     ah, al                          ; ah = write map mask


mov     al, [bp+color]


mov     bx, 128
mov     cx, 128


mov     _SI_color, ax                   ; _SI = _AX = color

mov     dx, e_SC_INDEX
mov     al, e_SC_MAPMASK
out     dx, al                          ; EGA: sequencer address reg
                                        ; map mask: data bits 0-3 enable writes to bit planes 0-3
inc     dx                              ; DX = 3C5h
mov     al, ah
out     dx, al                          ; EGA port: sequencer data register

mov     ax, _SI_color
jmp     short loc_19217


loc_191E8:                              ; CODE XREF: Line+FFj
add     bx, [bp+Horz_Slope]
cmp     bh, 0
jz      short loc_19206
xor     bh, bh
shl     ah, 1                           ; next write plane
cmp     ah, 1001b                       ; wrapped around?
js      short loc_191FC
mov     ah, 1                           ; reset, and
inc     di                              ; increase the write address

loc_191FC:                              ; CODE XREF: Line+D9j
mov     _SI_color, ax
mov     dx, 3C5h
mov     al, ah
out     dx, al                          ; EGA port: sequencer data register
mov     ax, _SI_color

loc_19206:                              ; CODE XREF: Line+D0j
add     cx, [bp+Vert_Slope]
cmp     ch, 0
jz      short loc_19217
xor     ch, ch
mov     dx, es
add     dx, [bp+Y_Dir]                  ; +/- 1 line .. add 5 to the segment address, i.e., increment by 80 bytes
mov     es, dx

loc_19217:                              ; CODE XREF: Line+C7j ...
mov     [es:di], al
dec     [bp+Line_Length]
jnz     short loc_191E8

pop     di
pop     si
pop     ds
pop     es
mov     sp, bp
pop     bp
retf
