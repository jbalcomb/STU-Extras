; Page 133  (PDF Page 151)
; Advanced Programmer’s Guide to Super VGAs
; Programming Examples—256-Color Graphics


; Draw Solid Line
;   _Line is used to demonstrate techniques used in incremental algorithms. An initial
; page and offset are computed from the starting x,y coordinate of the line. The line is
; then classified according to its slope (the relative size of DX and DY), and whether x
; and y are increasing or decreasing. Each line will fall into one of eight different classes,
; with different sections of code applying to each class.
;   Although some code sections could be combined to reduce total code size, the code
; is left in eight distinct sections to make it easier to add patterns and last pixel don’t
; draw’ checks. Each of the eight sections is divided into two parts: incremental drawing
; and page updating. For example, lines with positive DX and DY and DX greater than
; DY use the incremental drawing code between the labels NL_xp_yp and NL__fix0.
;   This code is a standard adaptation of Bresenham’s line drawing algorithm, but with
; two added JC instructions for page boundary detection: one after x is updated (ADD
; DI,1) and one after y is updated (ADD DI,Pitch).


; Listing 7-3. File: 256COL\LINE.ASM


;************************************************************************
;* File: LINE.ASM - fl Bit Packed Solid Line                            *
;* Routine: _Line(xQ, yD, xl, yl, Color)                                *
;* Description: Draw line from (xD,yD) to (xl,yl) using color 'Color'   *
;************************************************************************

        INCLUDE VGA.INC

        EXTRN   Graf Seg:WORD
        EXTRN   Video_Pitch:WORD
        EXTRN   Select_Page:NEAR

        PUBLIC _Line

_TEXT   SEGMENT BYTE PUBLIC 'CODE'

Arg_xD          EQU     WORD PTR [BP+4] ;Formal parameters
Arq yD          EQU     WORD PTR [BP+6]
Arg_xl          EQU     WORD PTR [BP+8]
Arq yl          EQU     WORD PTR [BP+10]
Arg_Color       EQU     BYTE PTR [BP+12]

PageNo          EQU     BYTE PTR [BP-2] ;Local variables
D1              EQU     WORD PTR [BP-4] ;Local variables
D2              EQU     WORD PTR [BP-6]
Pitch           EQU     WORD PTR [BP-8]
Delta x         EQU     WORD PTR [BP-10]
First_Mask      EQU     BYTE PTR [BP-12]

_Line           PROC NEAR
        PUSH    BP                      ;Standard C entry point
        MOV     BP,SP
        SUB     SP,12                   ;Declare local variables

        PUSH    DI                      ;Preserve segment registers
        PUSH    SI
        PUSH    DS
        PUSH    ES

;----------------------------------------------------------------
; Convert (x,y) starting point to Seg:Off and select page
;----------------------------------------------------------------
        MOV AX,Arg_y0                   ;Fetch y coordinate
        MUL CS:Video_Pitch              ;   multiply by width in bytes
        ADD AX,Arg_x0                   ;   add x coordinate to compute offset



; ...
; ...
; ...
; ...
; ...



NL_linedone:

;----------------------------------------------------------------
; Clean up and return to caller
;----------------------------------------------------------------

End_Line:
        POP     ES                      ;Restore segment registers
        POP     DS
        POP     SI
        POP     DI

        MOV     SP, BP ;Standard C exit point
        POP     BP
        RET

_Line           ENDP

_TEXT   ENDS
        END
