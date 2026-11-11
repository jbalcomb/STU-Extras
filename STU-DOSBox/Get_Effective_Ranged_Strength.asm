; =============== S U B R O U T I N E =======================================
; Attributes: bp-based frame
proc Get_Effective_Ranged_Strength far
ranged= word ptr  6
figures= word ptr  8
attack_flags= word ptr  0Ah
_DI_figures = di
_SI_attack_flags = si
_CX_ranged_threat = cx
push    bp
mov     bp, sp
push    _SI_attack_flags
push    _DI_figures
mov     _DI_figures, [bp+figures]
mov     _SI_attack_flags, [bp+attack_flags]
xor     _CX_ranged_threat, _CX_ranged_threat
cmp     [bp+ranged], 0
jnz     short loc_89F6A
@@JmpJmpDone_Return_ZERO:
xor     ax, ax
@@JmpDone:
jmp     @@Done
; ---------------------------------------------------------------------------
loc_89F6A:
cmp     [bp+ranged], 3
jle     short loc_89F81
mov     ax, [bp+ranged]
add     ax, -2
imul    _DI_figures
mov     dx, 20
imul    dx
mov     _CX_ranged_threat, ax
jmp     short loc_89FB8
; ---------------------------------------------------------------------------
loc_89F81:
mov     ax, _DI_figures
mov     dx, 40
imul    dx
mov     _CX_ranged_threat, ax
mov     ax, [bp+ranged]
cmp     ax, 1
jz      short loc_89F9E
cmp     ax, 2
jz      short loc_89FAA
cmp     ax, 3
jz      short loc_89FAF
jmp     short loc_89FB8
; ---------------------------------------------------------------------------
loc_89F9E:
mov     bx, 7
loc_89FA1:
mov     ax, _CX_ranged_threat
loc_89FA3:
cwd
idiv    bx
mov     _CX_ranged_threat, ax
jmp     short loc_89FB8
; ---------------------------------------------------------------------------
loc_89FAA:
mov     bx, 3
jmp     short loc_89FA1
; ---------------------------------------------------------------------------
loc_89FAF:
mov     ax, _CX_ranged_threat
shl     ax, 1
mov     bx, 3
jmp     short loc_89FA3
; ---------------------------------------------------------------------------
loc_89FB8:
or      _SI_attack_flags, _SI_attack_flags
jg      short loc_89FBF
jmp     @@JmpJmpDone_Return_CX
; ---------------------------------------------------------------------------
loc_89FBF:
test    _SI_attack_flags, Att_DoomDmg
jz      short loc_89FCD
mov     ax, _CX_ranged_threat
shl     ax, 1
mov     _CX_ranged_threat, ax
jmp     short loc_89FE2
; ---------------------------------------------------------------------------
loc_89FCD:
test    _SI_attack_flags, Att_ArmorPrc
jz      short loc_89FE2
mov     ax, _CX_ranged_threat
mov     dx, 5
imul    dx
mov     bx, 4
cwd
idiv    bx
mov     _CX_ranged_threat, ax
loc_89FE2:
test    _SI_attack_flags, Att_Poison
jz      short loc_89FF1
mov     ax, _DI_figures
mov     dx, 60
imul    dx
add     _CX_ranged_threat, ax
loc_89FF1:
test    _SI_attack_flags, Att_LifeSteal
jz      short loc_8A000
mov     ax, _DI_figures
mov     dx, 60
imul    dx
add     _CX_ranged_threat, ax
loc_8A000:
test    _SI_attack_flags, Att_Destruct
jz      short loc_8A00F
mov     ax, _DI_figures
mov     dx, 60
imul    dx
add     _CX_ranged_threat, ax
loc_8A00F:
test    _SI_attack_flags, Att_StnTouch
jz      short loc_8A01E
mov     ax, _DI_figures
mov     dx, 60
imul    dx
add     _CX_ranged_threat, ax
loc_8A01E:
test    _SI_attack_flags, Att_DthTouch
jz      short loc_8A02D
mov     ax, _DI_figures
mov     dx, 60
imul    dx
add     _CX_ranged_threat, ax
loc_8A02D:
test    _SI_attack_flags, Att_Illusion
jz      short loc_8A03C
mov     ax, _CX_ranged_threat
mov     dx, 5
imul    dx
mov     _CX_ranged_threat, ax
loc_8A03C:
test    _SI_attack_flags, Att_EldrWeap
jz      short @@JmpJmpDone_Return_CX
mov     ax, _CX_ranged_threat
mov     dx, 5
imul    dx
mov     bx, 4
cwd
idiv    bx
mov     _CX_ranged_threat, ax
@@JmpJmpDone_Return_CX:
mov     ax, _CX_ranged_threat
jmp     @@JmpDone
; ---------------------------------------------------------------------------
@@Done:
pop     _DI_figures
pop     _SI_attack_flags
pop     bp
retf
endp Get_Effective_Ranged_Strength
