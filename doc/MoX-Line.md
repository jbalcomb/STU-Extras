


2-D Computer Graphics
Top-Left Origin
Left-to-Right, Top-to-Bottom
coordinates are always non-negative



¿ assume X-major ?
¿ assume left-to-right ?
NO ¿ assume top-to-bottom ?
...
...treat X as 1 unit, calculate Y...

¿  ?


reversed line
    swap line ends

delta x, delta y

+/- line advance







### ¿ "reversed line" ?

¿ turn 8 cases into 4 cases ?
always draw left-to-right
X increasing

if((x2 - x1) <= -1)

x2 == x1  (23 - 23 ==  0)
x2 <  x1  (23 - 24 == -1)
x1 >  x2  (23 - 24 == -1)

AKA if Delta X is Negative




### Line Length

***...use delta_x or delta_y...***

```c
if(delta_x >= delta_y)
    length = delta_x + 1;
if(delta_x < delta_y)
    length = delta_y + 1;
```

```c
int16_t xslope = 0;
int16_t yslope = 0;
// ...
if(delta_x >= delta_y)

if(delta_x < delta_y)
    yslope = 256;
```

### ¿ horizontal vs. vertical ?

longer on the X axis than it is on the Y axis
means?
    to move in whole units of one axis, it must be the longer axis

lines moves more in the X direction than in the Y directions
¿ Special Case: line is one pixel thick ... ∵ x1 == x1 ∴ x_delta == 0 ?


[FICG]:
    ...works well enough for lines with absolute slope of 1 or less
    ...because the calculated points will be separated in y by more than unit

m = (y2 – y1)/(x2 – x1)
m = Δy/Δx

So, ...
    having a slope of 1 or less means delta x is larger than delta y
    i.e., X is *longer*, the line is (more) horizontal, the major axis is X
¿ entirely unrelated to the octants ?

∵   Because
∴   Therefore

...if the line's slope is infinite (x2 - x1 = 0), then an error condition exists



### ¿ x2 == x1  &&||  y2 == y1 ?
`SPECIAL CASE`
WHAT-IF
    1x1
    1xn
    nx1
...
as coded, if *X-Major* and *Delta X* is ZERO, xslope and yslope are not set and are assumed to be 0
          ...*Y-Major* and *Delta Y*...
    then, ...
        dec  [bp+length];  jnz  short loc_191E8
    so, ...
        can't/won't draw just 1 pixel, on either axis
        just protecting again division by zero



### increment X, in X-Major
xslope = 256
xerr = 128

xerr += xslope;
if((xerr & 0xff00) != 0)
    xerr &= 0xff;

128 + 256 = 384

 80h  128d  0000000010000000b
100h  256d  0000000100000000b
180h  384d  0000000110000000b


; x_delta >= y_delta
; x_delta != 0
mov     [bp+xslope], 256
mov     ah, dl                          ; y_delta
xor     al, al
mov     dl, dh
xor     dh, dh
div     cx                              ; ... / x_delta
mov     [bp+yslope], ax

; copy the low-byte of DX into the high-byte of AX
; clear the low-byte of AX
; copy the high-byte of DX into the low-byte of DX
; clear the high-byte of DX
; AX = AX / CX
; yslope = AX
; ... convert y_delta to units of x_delta
; ... how much Y for each X

mov     bx, dx                          ; y_delta
mov     ah, cl                          ; x_delta
xor     al, al
mov     dl, ch
xor     dh, dh
mov     cx, bx                          ; y_delta
div     cx                              ; ... / y_delta
mov     [bp+xslope], ax




https://www.scs.stanford.edu/05au-cs240c/lab/i386/DIV.htm

div  cx
F7 F1
Opcode    Instruction        Clocks   Description
F7 /6     DIV AX,r/m16       22/25    Unsigned divide DX:AX by r/m word (AX=Quo, DX=Rem)
Operation
temp := dividend / divisor;
IF temp does not fit in quotient
THEN Interrupt 0;
ELSE
   quotient := temp;
   remainder := dividend MOD (r/m);
FI;
Note
Divisions are unsigned. The divisor is given by the r/m operand.
Description
DIV performs an unsigned division. The dividend is implicit; only the divisor is given as an operand. The remainder is always less than the divisor. The type of the divisor determines which registers to use as follows:
    Size    Dividend     Divisor   Quotient   Remainder
    byte    AX           r/m8       AL          AH
    word    DX:AX        r/m16      AX          DX






sub     cx, ax                          ; x_delta = x2 - x1
sub     dx, bx                          ; y_delta = y2 - y1

mov     [bp+Y_Dir], 5
jns     short loc_1916B                 ; if(y_delta < 0)
neg     dx
mov     [bp+Y_Dir], -5
...
...
...
mov     dx, es
add     dx, [bp+Y_Dir]  ; add 5 to the segment address, i.e., increment by 80 bytes
mov     es, dx


x_delta = x2 - x1
y_delta = y2 - y1
Y_Dir = 5
if(y_delta < 0)
    y_delta = -(y_delta)
    Y_Dir = -5

¿ as in 'Mode Y' VGA VRAM planar memory ?
as seen in FLIC_Draw_Frame()

mov     ax, [bp+y_start]
mov     _BX_itr, ax
shl     ax, 1
shl     ax, 1
add     ax, _BX_itr                     ; * 5 as a segment address = * 80 total bytes which,
                                        ; since each byte is 4 pixels, equals the draw row






```c
Line(int x1, int y1, int x2, int y2, char color)
{
    signed short int Y_Dir;
    signed short int Horz_Slope;
    signed short int Line_Length;
    signed short int Vert_Slope;

    if(x2 > x1)
    {
        SWAP(x1,x2);
        SWAP(y1,y2);
    }


add     bx, [bp+Horz_Slope]             ; xerr += xslope;

yerr += yslope;



}
```





https://forum.vcfed.org/index.php?threads/dda-lines-with-1-byte-error-accumulators.1070213/

Imagine we have a line that goes from 0,0 to 249,99.

The X Distance (horizontal axis difference) is 250 and the Y Distance is 100.

This means that our fraction is 250/100





#### BEGIN:  old MoX Graphics.C line code

Where did I get this?
...old DOS code archive?
...still on J drive (EHDD_8TB)?

```c
/*
https://www.phatcode.net/res/224/files/html/ch37/37-01.html
https://github.com/jagregory/abrash-black-book/blob/master/src/chapter-37.md

; Local variables.
AdjUp          equ -2          ;error term adjust up on each advance
AdjDown        equ -4          ;error term adjust down when error term turns over
WholeStep      equ -6          ;minimum run length
XAdvance       equ -8          ;1 or -1, for direction in which X advances

MoO2
Module: line
    function (0 bytes) Line
    Address: 01:00134D2D
        Num params: 4
        Return type: void (1 bytes) 
        signed integer (2 bytes) 
        signed integer (2 bytes) 
        signed integer (2 bytes) 
        signed integer (2 bytes) 
        signed integer (2 bytes) 
        Locals:
            signed integer (2 bytes) x1
            signed integer (2 bytes) y1
            signed integer (2 bytes) x2
            signed integer (2 bytes) y2
            signed integer (2 bytes) color
            signed integer (2 bytes) temp
            signed integer (2 bytes) adj_up
            signed integer (2 bytes) adj_down
            signed integer (2 bytes) error_term
            signed integer (2 bytes) x_advance
            signed integer (2 bytes) x_delta
            signed integer (2 bytes) y_delta
            signed integer (2 bytes) whole_step
            signed integer (2 bytes) initial_pixel_count
            signed integer (2 bytes) final_pixel_count
            signed integer (2 bytes) i
            signed integer (2 bytes) j
            signed integer (2 bytes) run_length
            pointer (4 bytes) long_current_video_page

*/
/*
Per drake178:
    Bresenham's line-drawing algorithm (limited to the increasing X directions)
*/
void Line(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int8_t color)
{
    line_fast(x1, y1, x2, y2, color);
}


// ¿ Fastest Possible Intel-CPU Absolute Value *Function* ?
#define ABS(x)  ( x = (x >= 0) ? x : -x );

#define sgn(x)  ( (x < 0) ? -1 : ((x > 0) ? 1 : 0) )    /* macro to return the sign of a number */

// byte * VGA = (byte *)0xA0000000L;        /* this points to video memory. */


/**************************************************************************
 *  plot_pixel                                                            *
 *    Plot a pixel by directly writing to video memory, with no           *
 *    multiplication.                                                     *
 **************************************************************************/

void plot_pixel(int x, int y, byte color)
{
  /*  y*320 = y*256 + y*64 = y*2^8 + y*2^6   */
  // VGA[ (y << 8) + (y << 6) + x] = color;
  current_video_page[ (y << 8) + (y << 6) + x] = color;

}


/**************************************************************************
 *  line_slow                                                             *
 *    draws a line using multiplication and division.                     *
 **************************************************************************/

void line_slow(int x1, int y1, int x2, int y2, byte color)
{
    int dx,dy,sdx,sdy,px,py,dxabs,dyabs,i;
    float slope;

    dx = x2 - x1;      /* the horizontal distance of the line */
    dy = y2 - y1;      /* the vertical distance of the line */
    dxabs = abs(dx);
    dyabs = abs(dy);
    sdx = sgn(dx);
    sdy = sgn(dy);

    if (dxabs >= dyabs)  /* the line is more horizontal than vertical */
    {
        slope = (float)dy / (float)dx;
        for(i = 0; i != dx; i += sdx)
        {
            px = i + x1;
            py = slope * i + y1;
            plot_pixel(px, py, color);
        }
    }
    else /* the line is more vertical than horizontal */
    {
        slope = (float)dx / (float)dy;
        for(i = 0; i != dy; i += sdy)
        {
            px = slope * i + x1;
            py = i + y1;
            plot_pixel(px, py, color);
        }
    }
}


/**************************************************************************
 *  line_fast                                                             *
 *    draws a line using Bresenham's line-drawing algorithm, which uses   *
 *    no multiplication or division.                                      *
 **************************************************************************/

void line_fast(int x1, int y1, int x2, int y2, byte color)
{
    int i,dx,dy,sdx,sdy,dxabs,dyabs,x,y,px,py;
    uint8_t * screen_page;
    uint16_t screen_page_offset;

    dx = x2 - x1;      /* the horizontal distance of the line */
    dy = y2 - y1;      /* the vertical distance of the line */
    dxabs = abs(dx);
    dyabs = abs(dy);
    sdx = sgn(dx);
    sdy = sgn(dy);
    x = dyabs >> 1;
    y = dxabs >> 1;
    px = x1;
    py = y1;

    screen_page = video_page_buffer[1 - draw_page_num];

    *(screen_page + (py * SCREEN_WIDTH) + px) = color;

    if (dxabs >= dyabs) /* the line is more horizontal than vertical */
    {
        for(i = 0; i < dxabs; i++)
        {
            y += dyabs;
            if(y >= dxabs)
            {
                y -= dxabs;
                py += sdy;
            }
            px += sdx;
            // plot_pixel(px, py, color);
            *(screen_page + (py * SCREEN_WIDTH) + px) = color;
        }
    }
    else /* the line is more vertical than horizontal */
    {
        for(i = 0; i < dyabs; i++)
        {
            x += dxabs;
            if(x >= dyabs)
            {
                x -= dyabs;
                px += sdx;
            }
            py += sdy;
            // plot_pixel(px, py, color);
            *(screen_page + (py * SCREEN_WIDTH) + px) = color;
        }
    }
}
```

#### END:  old MoX Graphics.C line code





∵   Because
∴   Therefore
Δ   Delta
