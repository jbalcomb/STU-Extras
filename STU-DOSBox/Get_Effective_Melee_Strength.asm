; =============== S U B R O U T I N E =======================================
; Attributes: bp-based frame
proc Get_Effective_Melee_Strength far
melee= word ptr  6
thrown= word ptr  8
figures= word ptr  0Ah
attack_flags= word ptr  0Ch
ranged_type= word ptr  0Eh
_SI_attack_flags = si
_CX_melee_threat = cx
_DI_thrown_threat = di
push    bp
mov     bp, sp
push    _SI_attack_flags
push    _DI_thrown_threat
mov     _SI_attack_flags, [bp+attack_flags]
cmp     [bp+melee], 0
jnz     short loc_8A06D
@@JmpJmpDone_Return_ZERO:
xor     ax, ax
@@JmpDone:
jmp     @@Done
; ---------------------------------------------------------------------------
loc_8A06D:
cmp     [bp+melee], 3
jle     short loc_8A085
mov     ax, [bp+melee]
add     ax, -2
imul    [bp+figures]
mov     dx, 20
imul    dx
mov     _CX_melee_threat, ax
jmp     short loc_8A0BC
; ---------------------------------------------------------------------------
loc_8A085:
mov     ax, [bp+figures]
mov     dx, 40
imul    dx
mov     _CX_melee_threat, ax
mov     bx, [bp+melee]
cmp     bx, 3
ja      short loc_8A0BC
shl     bx, 1
jmp     [cs:off_8A1ED+bx]
loc_8A09E:
xor     _CX_melee_threat, _CX_melee_threat
jmp     short loc_8A0BC
; ---------------------------------------------------------------------------
loc_8A0A2:
mov     bx, 7
loc_8A0A5:
mov     ax, _CX_melee_threat
jmp     short loc_8A0B5
; ---------------------------------------------------------------------------
loc_8A0A9:
mov     bx, 3
jmp     short loc_8A0A5
; ---------------------------------------------------------------------------
loc_8A0AE:
mov     ax, _CX_melee_threat
shl     ax, 1
mov     bx, 3
loc_8A0B5:
cwd
idiv    bx
mov     _CX_melee_threat, ax
jmp     short $+2
loc_8A0BC:
xor     _DI_thrown_threat, _DI_thrown_threat
cmp     [bp+thrown], 0
jle     short loc_8A113
cmp     [bp+thrown], 3
jle     short loc_8A0DC
mov     ax, [bp+thrown]
add     ax, -2
imul    [bp+figures]
mov     dx, 30
imul    dx
mov     _DI_thrown_threat, ax
jmp     short loc_8A113
; ---------------------------------------------------------------------------
loc_8A0DC:
mov     ax, [bp+figures]
mov     dx, 60
imul    dx
mov     _DI_thrown_threat, ax
mov     bx, [bp+thrown]
cmp     bx, 3
ja      short loc_8A113
shl     bx, 1
jmp     [cs:off_8A1E5+bx]
loc_8A0F5:
xor     _DI_thrown_threat, _DI_thrown_threat
jmp     short loc_8A113
; ---------------------------------------------------------------------------
loc_8A0F9:
mov     bx, 7
loc_8A0FC:
mov     ax, _DI_thrown_threat
jmp     short loc_8A10C
; ---------------------------------------------------------------------------
loc_8A100:
mov     bx, 3
jmp     short loc_8A0FC
; ---------------------------------------------------------------------------
loc_8A105:
mov     ax, _DI_thrown_threat
shl     ax, 1
mov     bx, 3
loc_8A10C:
cwd
idiv    bx
mov     _DI_thrown_threat, ax
jmp     short $+2
loc_8A113:
add     _CX_melee_threat, _DI_thrown_threat
or      _SI_attack_flags, _SI_attack_flags
jg      short loc_8A11C
jmp     loc_8A1C8
; ---------------------------------------------------------------------------
loc_8A11C:
test    _SI_attack_flags, Att_DoomDmg
jz      short loc_8A12A
mov     ax, _CX_melee_threat
loc_8A124:
shl     ax, 1
mov     _CX_melee_threat, ax
jmp     short loc_8A13F
; ---------------------------------------------------------------------------
loc_8A12A:
test    _SI_attack_flags, Att_ArmorPrc
jz      short loc_8A13F
mov     ax, _CX_melee_threat
mov     dx, 5
imul    dx
mov     bx, 4
cwd
idiv    bx
mov     _CX_melee_threat, ax
loc_8A13F:
test    _SI_attack_flags, Att_Poison
jz      short loc_8A14F
mov     ax, [bp+figures]
mov     dx, 60
imul    dx
add     _CX_melee_threat, ax
loc_8A14F:
test    _SI_attack_flags, Att_LifeSteal
jz      short loc_8A15F
mov     ax, [bp+figures]
mov     dx, 60
imul    dx
add     _CX_melee_threat, ax
loc_8A15F:
test    _SI_attack_flags, Att_Destruct
jz      short loc_8A16F
mov     ax, [bp+figures]
mov     dx, 60
imul    dx
add     _CX_melee_threat, ax
loc_8A16F:
test    _SI_attack_flags, Att_StnTouch
jz      short loc_8A17F
mov     ax, [bp+figures]
mov     dx, 60
imul    dx
add     _CX_melee_threat, ax
loc_8A17F:
test    _SI_attack_flags, Att_DthTouch
jz      short loc_8A18F
mov     ax, [bp+figures]
mov     dx, 60
imul    dx
add     _CX_melee_threat, ax
loc_8A18F:
test    _SI_attack_flags, Att_Illusion
jz      short loc_8A19E
mov     ax, _CX_melee_threat
mov     dx, 5
imul    dx
mov     _CX_melee_threat, ax
loc_8A19E:
test    _SI_attack_flags, Att_1stStrike
jz      short loc_8A1B3
mov     ax, _CX_melee_threat
mov     dx, 5
imul    dx
mov     bx, 4
cwd
idiv    bx
mov     _CX_melee_threat, ax
loc_8A1B3:
test    _SI_attack_flags, Att_EldrWeap
jz      short loc_8A1C8
mov     ax, _CX_melee_threat
mov     dx, 5
imul    dx
mov     bx, 4
cwd
idiv    bx
mov     _CX_melee_threat, ax
loc_8A1C8:
cmp     [bp+ranged_type], srat_StoneGaze
jl      short @@JmpJmpDone_Return_CX
add     _CX_melee_threat, 300
cmp     [bp+ranged_type], srat_MultiGaze
jnz     short @@JmpJmpDone_Return_CX
add     _CX_melee_threat, 900
@@JmpJmpDone_Return_CX:
mov     ax, _CX_melee_threat
jmp     @@JmpDone
; ---------------------------------------------------------------------------
@@Done:
pop     _DI_thrown_threat
pop     _SI_attack_flags
pop     bp
retf
endp Get_Effective_Melee_Strength
