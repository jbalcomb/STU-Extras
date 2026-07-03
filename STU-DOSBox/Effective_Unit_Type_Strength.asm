; =============== S U B R O U T I N E =======================================
; Attributes: bp-based frame
proc Effective_Unit_Type_Strength far
temp= word ptr -2
unit_type= word ptr  6
_SI_unit_type = si
_DI_unit_type_strength = di
push    bp
mov     bp, sp
sub     sp, 2
push    _SI_unit_type
push    _DI_unit_type_strength
mov     _SI_unit_type, [bp+unit_type]
xor     _DI_unit_type_strength, _DI_unit_type_strength
mov     ax, _SI_unit_type
mov     dx, size s_UNIT_TYPE
imul    dx
mov     bx, ax
mov     al, [_unit_type_table.Hits+bx]
cbw
push    ax
mov     ax, _SI_unit_type
mov     dx, size s_UNIT_TYPE
imul    dx
mov     bx, ax
mov     al, [_unit_type_table.Figures+bx]
cbw
mov     dx, ax
pop     ax
imul    dx
mov     [bp+temp], ax
mov     ax, _SI_unit_type
mov     dx, size s_UNIT_TYPE
imul    dx
mov     bx, ax
mov     al, [_unit_type_table.Defense+bx]
cbw
push    ax
push    [bp+temp]
call    j_Get_Effective_Hits
pop     cx
pop     cx
add     _DI_unit_type_strength, ax
mov     ax, _SI_unit_type
mov     dx, size s_UNIT_TYPE
imul    dx
mov     bx, ax
mov     al, [_unit_type_table.Ranged_Type+bx]
cbw
mov     bx, 10
cwd
idiv    bx
cmp     ax, rag_Magic
jg      short loc_A1C2C
mov     ax, _SI_unit_type
mov     dx, size s_UNIT_TYPE
imul    dx
mov     bx, ax
push    [_unit_type_table.attack_attributes+bx]
mov     ax, _SI_unit_type
mov     dx, size s_UNIT_TYPE
imul    dx
mov     bx, ax
mov     al, [_unit_type_table.Figures+bx]
cbw
push    ax
mov     ax, _SI_unit_type
mov     dx, size s_UNIT_TYPE
imul    dx
mov     bx, ax
mov     al, [_unit_type_table.Ranged+bx]
cbw
push    ax
call    j_Get_Effective_Ranged_Strength
add     sp, 6
shl     ax, 1
add     _DI_unit_type_strength, ax
loc_A1C2C:
mov     ax, _SI_unit_type
mov     dx, size s_UNIT_TYPE
imul    dx
mov     bx, ax
cmp     [_unit_type_table.Ranged_Type+bx], srat_Thrown
jl      short loc_A1C5D
mov     ax, _SI_unit_type
mov     dx, size s_UNIT_TYPE
imul    dx
mov     bx, ax
mov     al, [_unit_type_table.Ranged+bx]
cbw
mov     [bp+temp], ax
mov     ax, _SI_unit_type
mov     dx, size s_UNIT_TYPE
imul    dx
mov     bx, ax
mov     al, [_unit_type_table.Ranged_Type+bx]
cbw
jmp     short loc_A1C65
; ---------------------------------------------------------------------------
loc_A1C5D:
mov     [bp+temp], 0
mov     ax, 0FFFFh
loc_A1C65:
push    ax
mov     ax, _SI_unit_type
mov     dx, size s_UNIT_TYPE
imul    dx
mov     bx, ax
push    [_unit_type_table.attack_attributes+bx]
mov     ax, _SI_unit_type
mov     dx, size s_UNIT_TYPE
imul    dx
mov     bx, ax
mov     al, [_unit_type_table.Figures+bx]
cbw
push    ax
push    [bp+temp]
mov     ax, _SI_unit_type
mov     dx, size s_UNIT_TYPE
imul    dx
mov     bx, ax
mov     al, [_unit_type_table.Melee+bx]
cbw
push    ax
call    j_Get_Effective_Melee_Strength
add     sp, 0Ah
add     _DI_unit_type_strength, ax
mov     ax, _DI_unit_type_strength
jmp     short $+2
pop     _DI_unit_type_strength
pop     _SI_unit_type
mov     sp, bp
pop     bp
retf
endp Effective_Unit_Type_Strength
