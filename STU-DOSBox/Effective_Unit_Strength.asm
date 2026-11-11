; =============== S U B R O U T I N E =======================================
; Attributes: bp-based frame
proc Effective_Unit_Strength far
unit_idx= word ptr  6
_SI_effective_strength = si
_DI_temp = di
push    bp
mov     bp, sp
push    _SI_effective_strength
push    _DI_temp
cmp     [bp+unit_idx], 0
jl      short @@JmpDone__Return_ZERO
mov     ax, [bp+unit_idx]
cmp     ax, [_units]
jle     short loc_A1CC1
@@JmpDone__Return_ZERO:
xor     ax, ax
@@JmpDone:
jmp     @@Done
; ---------------------------------------------------------------------------
loc_A1CC1:
xor     _SI_effective_strength, _SI_effective_strength
push    [word ptr global_battle_unit+2]
push    [word ptr global_battle_unit]
push    [bp+unit_idx]
call    j_Load_Battle_Unit
add     sp, 6
les     bx, [global_battle_unit]
mov     al, [es:bx+s_BATTLE_UNIT.Cur_Figures]
cbw
les     bx, [global_battle_unit]
push    ax
mov     al, [es:bx+s_BATTLE_UNIT.hits]
cbw
mov     dx, ax
pop     ax
imul    dx
les     bx, [global_battle_unit]
push    ax
mov     al, [es:bx+s_BATTLE_UNIT.front_figure_damage]
cbw
pop     dx
sub     dx, ax
mov     _DI_temp, dx
les     bx, [global_battle_unit]
mov     al, [es:bx+s_BATTLE_UNIT.defense]
cbw
push    ax
push    _DI_temp
call    j_Get_Effective_Hits
pop     cx
pop     cx
add     _SI_effective_strength, ax
les     bx, [global_battle_unit]
mov     al, [es:bx+s_BATTLE_UNIT.ranged_type]
cbw
mov     bx, 10
cwd
idiv    bx
cmp     ax, rag_Magic
jg      short loc_A1D61
les     bx, [global_battle_unit]
cmp     [es:bx+s_BATTLE_UNIT.ranged_type], rat_None
jz      short loc_A1D61
les     bx, [global_battle_unit]
mov     ax, [es:bx+s_BATTLE_UNIT.attack_attributes]
les     bx, [global_battle_unit]
or      ax, [es:bx+s_BATTLE_UNIT.ranged_attack_attributes]
push    ax
les     bx, [global_battle_unit]
mov     al, [es:bx+s_BATTLE_UNIT.Cur_Figures]
cbw
push    ax
les     bx, [global_battle_unit]
mov     al, [es:bx+s_BATTLE_UNIT.ranged]
cbw
push    ax
call    j_Get_Effective_Ranged_Strength
add     sp, 6
shl     ax, 1
add     _SI_effective_strength, ax
loc_A1D61:
les     bx, [global_battle_unit]
cmp     [es:bx+s_BATTLE_UNIT.ranged_type], srat_Thrown
jl      short loc_A1D82
les     bx, [global_battle_unit]
mov     al, [es:bx+s_BATTLE_UNIT.ranged]
cbw
mov     _DI_temp, ax
les     bx, [global_battle_unit]
mov     al, [es:bx+s_BATTLE_UNIT.ranged_type]
cbw
jmp     short loc_A1D87
; ---------------------------------------------------------------------------
loc_A1D82:
xor     _DI_temp, _DI_temp
mov     ax, e_ST_UNDEFINED
loc_A1D87:
push    ax
les     bx, [global_battle_unit]
mov     ax, [es:bx+s_BATTLE_UNIT.attack_attributes]
les     bx, [global_battle_unit]
or      ax, [es:bx+s_BATTLE_UNIT.melee_attack_attributes]
push    ax
les     bx, [global_battle_unit]
mov     al, [es:bx+s_BATTLE_UNIT.Cur_Figures]
cbw
push    ax
push    _DI_temp
les     bx, [global_battle_unit]
mov     al, [es:bx+s_BATTLE_UNIT.melee]
cbw
push    ax
call    j_Get_Effective_Melee_Strength
add     sp, 0Ah
add     _SI_effective_strength, ax
mov     ax, _SI_effective_strength
jmp     @@JmpDone
; ---------------------------------------------------------------------------
@@Done:
pop     _DI_temp
pop     _SI_effective_strength
pop     bp
retf
endp Effective_Unit_Strength
