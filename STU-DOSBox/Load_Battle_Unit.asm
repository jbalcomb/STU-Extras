; =============== S U B R O U T I N E =======================================
; Attributes: bp-based frame
proc Load_Battle_Unit far
Item_Charges= word ptr -2
unit_idx= word ptr  6
battle_unit= dword ptr  8
_SI_unit_idx = si
_DI_itr = di
push    bp
mov     bp, sp
sub     sp, 2
push    _SI_unit_idx
push    _DI_itr
mov     _SI_unit_idx, [bp+unit_idx]
mov     ax, size s_UNIT_TYPE
push    ax
push    ds
mov     ax, _SI_unit_idx
mov     cl, 5
shl     ax, cl
les     bx, [_UNITS]
add     bx, ax
mov     al, [es:bx+s_UNIT.type]
mov     ah, 0
mov     dx, size s_UNIT_TYPE
imul    dx
add     ax, offset _unit_type_table.Melee
push    ax
push    [word ptr bp+battle_unit+2]
push    [word ptr bp+battle_unit]
call    _fmemcpy
add     sp, 0Ah
les     bx, [bp+battle_unit]
mov     [es:bx+s_BATTLE_UNIT.Combat_Effects], 0
les     bx, [bp+battle_unit]
mov     [es:bx+s_BATTLE_UNIT.melee_tohit], 0
les     bx, [bp+battle_unit]
mov     [es:bx+s_BATTLE_UNIT.ranged_tohit], 0
les     bx, [bp+battle_unit]
mov     [es:bx+s_BATTLE_UNIT.tohit], 0
les     bx, [bp+battle_unit]
mov     [es:bx+s_BATTLE_UNIT.toblock], 0
les     bx, [bp+battle_unit]
mov     [es:bx+s_BATTLE_UNIT.Weapon_Plus1], 0
les     bx, [bp+battle_unit]
mov     [es:bx+s_BATTLE_UNIT.melee_attack_attributes], 0
les     bx, [bp+battle_unit]
mov     [es:bx+s_BATTLE_UNIT.ranged_attack_attributes], 0
les     bx, [bp+battle_unit]
mov     [word ptr es:bx+(s_BATTLE_UNIT.item_enchantments+2)], 0
mov     [word ptr es:bx+s_BATTLE_UNIT.item_enchantments], 0
les     bx, [bp+battle_unit]
mov     [es:bx+s_BATTLE_UNIT.Extra_Hits], 0
les     bx, [bp+battle_unit]
mov     [es:bx+s_BATTLE_UNIT.unit_idx], _SI_unit_idx
les     bx, [bp+battle_unit]
mov     [es:bx+s_BATTLE_UNIT.Web_HP], 0
les     bx, [bp+battle_unit]
mov     [es:bx+s_BATTLE_UNIT.Gold_Melee], 0
les     bx, [bp+battle_unit]
mov     [es:bx+s_BATTLE_UNIT.Gold_Ranged], 0
les     bx, [bp+battle_unit]
mov     [es:bx+s_BATTLE_UNIT.Gold_Defense], 0
les     bx, [bp+battle_unit]
mov     [es:bx+s_BATTLE_UNIT.Gold_Resist], 0
les     bx, [bp+battle_unit]
mov     [es:bx+s_BATTLE_UNIT.Gold_Hits], 0
les     bx, [bp+battle_unit]
mov     [es:bx+s_BATTLE_UNIT.Grey_Melee], 0
les     bx, [bp+battle_unit]
mov     [es:bx+s_BATTLE_UNIT.Grey_Ranged], 0
les     bx, [bp+battle_unit]
mov     [es:bx+s_BATTLE_UNIT.Grey_Defense], 0
les     bx, [bp+battle_unit]
mov     [es:bx+s_BATTLE_UNIT.Grey_Resist], 0
les     bx, [bp+battle_unit]
mov     [es:bx+s_BATTLE_UNIT.bufpi], e_ST_UNDEFINED
les     bx, [bp+battle_unit]
mov     [es:bx+s_BATTLE_UNIT.status], 0
mov     ax, _SI_unit_idx
mov     cl, 5
shl     ax, cl
les     bx, [_UNITS]
add     bx, ax
mov     al, [es:bx+s_UNIT.owner_idx]
les     bx, [bp+battle_unit]
mov     [es:bx+s_BATTLE_UNIT.controller_idx], al
xor     _DI_itr, _DI_itr
jmp     short loc_9848A
; ---------------------------------------------------------------------------
loc_9847F:
les     bx, [bp+battle_unit]
add     bx, _DI_itr
mov     [es:bx+s_BATTLE_UNIT.damage], 0
inc     _DI_itr
loc_9848A:
cmp     _DI_itr, 3
jl      short loc_9847F
les     bx, [bp+battle_unit]
mov     [word ptr es:bx+(s_BATTLE_UNIT.enchantments+2)], 0
mov     [word ptr es:bx+s_BATTLE_UNIT.enchantments], 0
les     bx, [bp+battle_unit]
mov     [es:bx+s_BATTLE_UNIT.Suppression], 0
les     bx, [bp+battle_unit]
mov     [es:bx+s_BATTLE_UNIT.mana_max], 0
les     bx, [bp+battle_unit]
mov     [es:bx+s_BATTLE_UNIT.Item_Charges], 0
les     bx, [bp+battle_unit]
mov     [es:bx+s_BATTLE_UNIT.target_battle_unit_idx], e_ST_UNDEFINED_DB
les     bx, [bp+battle_unit]
mov     [es:bx+s_BATTLE_UNIT.Poison_Strength], 0
push    _SI_unit_idx
call    j_Unit_Gold_Upkeep
pop     cx
les     bx, [bp+battle_unit]
mov     [es:bx+s_BATTLE_UNIT.upkeep], al
les     bx, [bp+battle_unit]
test    [es:bx+s_BATTLE_UNIT.attack_attributes], Att_Poison
jz      short loc_984FF
les     bx, [bp+battle_unit]
mov     al, [es:bx+s_BATTLE_UNIT.Spec_Att_Attrib]
les     bx, [bp+battle_unit]
mov     [es:bx+s_BATTLE_UNIT.Poison_Strength], al
les     bx, [bp+battle_unit]
cmp     [es:bx+s_BATTLE_UNIT.ranged_type], srat_MultiGaze
jz      short loc_984FF
les     bx, [bp+battle_unit]
mov     [es:bx+s_BATTLE_UNIT.Spec_Att_Attrib], 0
loc_984FF:
push    [word ptr bp+battle_unit+2]
push    [word ptr bp+battle_unit]
nop
push    cs
call    near ptr BU_Init_Battle_Unit
pop     cx
pop     cx
les     bx, [bp+battle_unit]
mov     al, [es:bx+s_BATTLE_UNIT.mana_max]
les     bx, [bp+battle_unit]
mov     [es:bx+s_BATTLE_UNIT.mana], al
mov     ax, _SI_unit_idx
mov     cl, 5
shl     ax, cl
les     bx, [_UNITS]
add     bx, ax
mov     al, [es:bx+s_UNIT.Damage]
les     bx, [bp+battle_unit]
mov     [es:bx+s_BATTLE_UNIT.damage], al
mov     ax, _SI_unit_idx
mov     cl, 5
shl     ax, cl
les     bx, [_UNITS]
add     bx, ax
mov     al, [es:bx+s_UNIT.Damage]
cbw
les     bx, [bp+battle_unit]
push    ax
mov     al, [es:bx+s_BATTLE_UNIT.hits]
cbw
mov     bx, ax
pop     ax
cwd
idiv    bx
les     bx, [bp+battle_unit]
mov     [es:bx+s_BATTLE_UNIT.front_figure_damage], dl
mov     ax, _SI_unit_idx
mov     cl, 5
shl     ax, cl
les     bx, [_UNITS]
add     bx, ax
mov     al, [es:bx+s_UNIT.Damage]
cbw
les     bx, [bp+battle_unit]
push    ax
mov     al, [es:bx+s_BATTLE_UNIT.hits]
cbw
mov     bx, ax
pop     ax
cwd
idiv    bx
les     bx, [bp+battle_unit]
mov     dl, [es:bx+s_BATTLE_UNIT.Max_Figures]
sub     dl, al
les     bx, [bp+battle_unit]
mov     [es:bx+s_BATTLE_UNIT.Cur_Figures], dl
les     bx, [bp+battle_unit]
mov     [es:bx+s_BATTLE_UNIT.Atk_FigLoss], 0
les     bx, [bp+battle_unit]
mov     [es:bx+s_BATTLE_UNIT.Confusion_State], 0
les     bx, [bp+battle_unit]
mov     [es:bx+s_BATTLE_UNIT.gibs], 0
les     bx, [bp+battle_unit]
mov     [es:bx+s_BATTLE_UNIT.Unknown_5A], 0
les     bx, [bp+battle_unit]
mov     [es:bx+s_BATTLE_UNIT.Unknown_5A], 0
les     bx, [bp+battle_unit]
mov     [es:bx+s_BATTLE_UNIT.Melee_Anim], 0
les     bx, [bp+battle_unit]
mov     [es:bx+s_BATTLE_UNIT.outline_magic_realm], 0
les     bx, [bp+battle_unit]
mov     [es:bx+s_BATTLE_UNIT.move_anim_ctr], 0
les     bx, [bp+battle_unit]
mov     [es:bx+s_BATTLE_UNIT.Moving], 0
les     bx, [bp+battle_unit]
mov     [es:bx+s_BATTLE_UNIT.action], 0
les     bx, [bp+battle_unit]
mov     [es:bx+s_BATTLE_UNIT.Always_Animate], 0
les     bx, [bp+battle_unit]
mov     [es:bx+s_BATTLE_UNIT.Image_Effect], 0
les     bx, [bp+battle_unit]
mov     [es:bx+s_BATTLE_UNIT.Move_Bob], 0
mov     ax, _SI_unit_idx
mov     cl, 5
shl     ax, cl
les     bx, [_UNITS]
add     bx, ax
cmp     [es:bx+s_UNIT.Hero_Slot], -1
jg      short loc_98612
jmp     @@Done
; ---------------------------------------------------------------------------
loc_98612:
mov     ax, _SI_unit_idx
mov     cl, 5
shl     ax, cl
les     bx, [_UNITS]
add     bx, ax
mov     al, [es:bx+s_UNIT.owner_idx]
cbw
mov     dx, size s_WIZARD
imul    dx
mov     dx, _SI_unit_idx
mov     cl, 5
shl     dx, cl
les     bx, [_UNITS]
add     bx, dx
push    ax
mov     al, [es:bx+s_UNIT.Hero_Slot]
cbw
mov     dx, size s_OWNED_HERO
imul    dx
pop     bx
add     bx, ax
cmp     [_players.Heroes.Items+bx], -1
jle     short loc_98692
mov     ax, _SI_unit_idx
mov     cl, 5
shl     ax, cl
les     bx, [_UNITS]
add     bx, ax
mov     al, [es:bx+s_UNIT.owner_idx]
cbw
mov     dx, size s_WIZARD
imul    dx
mov     dx, _SI_unit_idx
mov     cl, 5
shl     dx, cl
les     bx, [_UNITS]
add     bx, dx
push    ax
mov     al, [es:bx+s_UNIT.Hero_Slot]
cbw
mov     dx, size s_OWNED_HERO
imul    dx
pop     bx
add     bx, ax
mov     ax, [_players.Heroes.Items+bx]
mov     dx, size s_ITEM
imul    dx
les     bx, [_ITEMS]
add     bx, ax
mov     al, [byte ptr es:bx+s_ITEM.embed_spell_cnt]
cbw
mov     [bp+Item_Charges], ax
jmp     short loc_98697
; ---------------------------------------------------------------------------
loc_98692:
mov     [bp+Item_Charges], 0
loc_98697:
cmp     [bp+Item_Charges], 0
jle     short @@Done
les     bx, [bp+battle_unit]
mov     al, [byte ptr bp+Item_Charges]
mov     [es:bx+s_BATTLE_UNIT.Item_Charges], al
@@Done:
pop     _DI_itr
pop     _SI_unit_idx
mov     sp, bp
pop     bp
retf
endp Load_Battle_Unit
