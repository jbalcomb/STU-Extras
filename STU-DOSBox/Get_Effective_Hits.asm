; =============== S U B R O U T I N E =======================================
; Attributes: bp-based frame
proc Get_Effective_Hits far
hits= word ptr  6
defense= word ptr  8
_DI_defense = di
_SI_effective_hits = si
push    bp
mov     bp, sp
push    _SI_effective_hits
push    _DI_defense
mov     _DI_defense, [bp+defense]
mov     ax, [bp+hits]
mov     dx, 3
imul    dx
mov     _SI_effective_hits, ax
cmp     _DI_defense, 3
jle     short loc_89F05
mov     ax, _DI_defense
add     ax, -2
mov     dx, _DI_defense
add     dx, -2
imul    dx
cwd
sub     ax, dx
sar     ax, 1
push    ax
mov     ax, _SI_effective_hits
pop     dx
imul    dx
mov     _SI_effective_hits, ax
jmp     short loc_89F41
; ---------------------------------------------------------------------------
loc_89F05:
mov     bx, _DI_defense
cmp     bx, 3
ja      short loc_89F41
shl     bx, 1
jmp     [cs:off_89F4A+bx]
loc_89F13:
mov     ax, _SI_effective_hits
cwd
sub     ax, dx
sar     ax, 1
loc_89F1A:
mov     _SI_effective_hits, ax
jmp     short loc_89F41
; ---------------------------------------------------------------------------
loc_89F1E:
mov     ax, _SI_effective_hits
mov     dx, 3
imul    dx
jmp     short loc_89F39
; ---------------------------------------------------------------------------
loc_89F27:
mov     ax, _SI_effective_hits
mov     dx, 7
imul    dx
mov     bx, 10
jmp     short loc_89F3C
; ---------------------------------------------------------------------------
loc_89F33:
mov     ax, _SI_effective_hits
mov     cl, 2
shl     ax, cl
loc_89F39:
mov     bx, 5
loc_89F3C:
cwd
idiv    bx
jmp     short loc_89F1A
; ---------------------------------------------------------------------------
loc_89F41:
mov     ax, _SI_effective_hits
jmp     short $+2
pop     _DI_defense
pop     _SI_effective_hits
pop     bp
retf
endp Get_Effective_Hits
