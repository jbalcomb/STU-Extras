; =============== S U B R O U T I N E =======================================
; Attributes: bp-based frame
proc Change_Relations__WIP far
Rel_Divisor= word ptr -4
NoCharisma_RelValue= word ptr -2
value= word ptr  6
attacker_idx= word ptr  8
defender_idx= word ptr  0Ah
type= word ptr  0Ch
city_idx= word ptr  0Eh
spell_idx= word ptr  10h
_DI_attacker_idx = di
_SI_defender_idx = si
push    bp
mov     bp, sp
sub     sp, 4
push    _SI_defender_idx
push    _DI_attacker_idx
mov     _DI_attacker_idx, [bp+attacker_idx]
mov     _SI_defender_idx, [bp+defender_idx]
mov     ax, _DI_attacker_idx
mov     dx, size s_WIZARD
imul    dx
add     ax, _SI_defender_idx
mov     bx, ax
cmp     [_players.Dipl.Contacted+bx], e_ST_FALSE
jnz     short loc_71076
@@JmpDone_Return_FALSE:
xor     ax, ax
jmp     @@Done
; ---------------------------------------------------------------------------
loc_71076:
cmp     [AI_Dipl_Unset_0], 3
jnz     short loc_710C3
or      _DI_attacker_idx, _DI_attacker_idx
jz      short @@JmpJmpDone_Return_FALSE__1
cmp     [bp+value], 0
jge     short @@JmpJmpDone_Return_FALSE__1
or      _SI_defender_idx, _SI_defender_idx
jnz     short @@JmpJmpDone_Return_FALSE__1
mov     ax, 100
push    ax
call    Random
pop     cx
push    ax
push    [bp+value]
call    abs
pop     cx
mov     bx, 10
cwd
idiv    bx
pop     dx
cmp     dx, ax
jg      short @@JmpJmpDone_Return_FALSE__1
mov     [_players.Dipl.DA_Strength], 100
mov     [_players.Dipl.Dipl_Action], 39
mov     ax, [bp+spell_idx]
mov     [_players.Dipl.DA_Spell], ax
mov     al, [byte ptr bp+city_idx]
mov     [_players.Dipl.DA_City], al
@@JmpJmpDone_Return_FALSE__1:
jmp     short @@JmpDone_Return_FALSE
; ---------------------------------------------------------------------------
loc_710C3:
cmp     _DI_attacker_idx, e_MOO_MONSTER_PLAYER_IDX
jge     short loc_710CD
cmp     _SI_defender_idx, e_MOO_MONSTER_PLAYER_IDX
jl      short loc_710CF
loc_710CD:
jmp     short @@JmpDone_Return_FALSE
; ---------------------------------------------------------------------------
loc_710CF:
cmp     _DI_attacker_idx, _SI_defender_idx
jz      short loc_710FB
cmp     [bp+value], 0
jz      short loc_710FB
or      _SI_defender_idx, _SI_defender_idx
jz      short loc_710FB
mov     ax, _DI_attacker_idx
mov     dx, size s_WIZARD
imul    dx
add     ax, _SI_defender_idx
mov     bx, ax
cmp     [_players.Dipl.Dipl_Status+bx], DIPL_Crusade
jz      short loc_710FB
cmp     _SI_defender_idx, [_num_players]
jg      short loc_710FB
cmp     _DI_attacker_idx, [_num_players]
jle     short loc_710FE
loc_710FB:
jmp     @@JmpDone_Return_FALSE
; ---------------------------------------------------------------------------
loc_710FE:
cmp     [bp+value], -10000
jnz     short loc_71135
or      _DI_attacker_idx, _DI_attacker_idx
jnz     short @@JmpJmpDone_Return_FALSE__2
cmp     [bp+type], do_None
jz      short @@JmpJmpDone_Return_FALSE__2
mov     bx, _SI_defender_idx
shl     bx, 1
mov     [_players.Dipl.DA_Strength+bx], 10000
mov     al, [byte ptr bp+type]
mov     [_players.Dipl.Dipl_Action+_SI_defender_idx], al
mov     bx, _SI_defender_idx
shl     bx, 1
mov     ax, [bp+spell_idx]
mov     [_players.Dipl.DA_Spell+bx], ax
mov     al, [byte ptr bp+city_idx]
mov     [_players.Dipl.DA_City+_SI_defender_idx], al
@@JmpJmpDone_Return_FALSE__2:
jmp     @@JmpDone_Return_FALSE
; ---------------------------------------------------------------------------
loc_71135:
cmp     [bp+value], 0
jle     short loc_711B8
mov     ax, _SI_defender_idx
mov     dx, size s_WIZARD
imul    dx
add     ax, _DI_attacker_idx
mov     bx, ax
cmp     [_players.Dipl.Visible_Rel+bx], 0
jge     short loc_71187
mov     ax, [bp+value]
shl     ax, 1
mov     [bp+value], ax
mov     ax, _SI_defender_idx
mov     dx, size s_WIZARD
imul    dx
add     ax, _DI_attacker_idx
mov     bx, ax
mov     al, [_players.Dipl.Visible_Rel+bx]
cbw
add     ax, [bp+value]
cmp     ax, 10
jle     short loc_71185
mov     ax, _SI_defender_idx
mov     dx, size s_WIZARD
imul    dx
add     ax, _DI_attacker_idx
mov     bx, ax
mov     al, [_players.Dipl.Visible_Rel+bx]
cbw
mov     dx, 10
sub     dx, ax
mov     [bp+value], dx
loc_71185:
jmp     short loc_711B6
; ---------------------------------------------------------------------------
loc_71187:
mov     ax, _SI_defender_idx
mov     dx, size s_WIZARD
imul    dx
add     ax, _DI_attacker_idx
mov     bx, ax
mov     al, [_players.Dipl.Visible_Rel+bx]
cbw
mov     bx, 25
cwd
idiv    bx
inc     ax
mov     [bp+Rel_Divisor], ax
cmp     [bp+Rel_Divisor], 0
jnz     short loc_711AC
mov     [bp+Rel_Divisor], 1
loc_711AC:
mov     ax, [bp+value]
cwd
idiv    [bp+Rel_Divisor]
mov     [bp+value], ax
loc_711B6:
jmp     short loc_71200
; ---------------------------------------------------------------------------
loc_711B8:
mov     ax, _SI_defender_idx
mov     dx, size s_WIZARD
imul    dx
add     ax, _DI_attacker_idx
mov     bx, ax
cmp     [_players.Dipl.Visible_Rel+bx], 0
jle     short loc_711D1
mov     ax, [bp+value]
shl     ax, 1
jmp     short loc_711FD
; ---------------------------------------------------------------------------
loc_711D1:
mov     ax, _SI_defender_idx
mov     dx, size s_WIZARD
imul    dx
add     ax, _DI_attacker_idx
mov     bx, ax
mov     al, [_players.Dipl.Visible_Rel+bx]
cbw
mov     bx, -25
cwd
idiv    bx
inc     ax
mov     [bp+Rel_Divisor], ax
cmp     [bp+Rel_Divisor], 0
jnz     short loc_711F6
mov     [bp+Rel_Divisor], 1
loc_711F6:
mov     ax, [bp+value]
cwd
idiv    [bp+Rel_Divisor]
loc_711FD:
mov     [bp+value], ax
loc_71200:
mov     ax, _SI_defender_idx
mov     dx, size s_WIZARD
imul    dx
mov     bx, ax
cmp     [_players.Personality+bx], PRS_Lawful
jnz     short loc_7121E
cmp     [bp+value], 0
jge     short loc_7121E
mov     ax, [bp+value]
shl     ax, 1
mov     [bp+value], ax
loc_7121E:
mov     ax, _SI_defender_idx
mov     dx, size s_WIZARD
imul    dx
mov     bx, ax
cmp     [_players.Personality+bx], PRS_Maniacal
jnz     short loc_71244
cmp     [bp+value], 0
jge     short loc_71244
mov     ax, [bp+value]
mov     dx, 3
imul    dx
cwd
sub     ax, dx
sar     ax, 1
mov     [bp+value], ax
loc_71244:
mov     ax, _SI_defender_idx
mov     dx, size s_WIZARD
imul    dx
mov     bx, ax
cmp     [_players.Personality+bx], PRS_Maniacal
jnz     short loc_7126B
cmp     [bp+value], 0
jle     short loc_7126B
mov     ax, [bp+value]
mov     dx, 3
imul    dx
mov     bx, 4
cwd
idiv    bx
mov     [bp+value], ax
loc_7126B:
mov     ax, [bp+value]
mov     [bp+NoCharisma_RelValue], ax
mov     ax, _DI_attacker_idx
mov     dx, size s_WIZARD
imul    dx
mov     bx, ax
cmp     [_players.charismatic+bx], e_ST_TRUE
jnz     short loc_7128F
cmp     [bp+value], 0
jle     short loc_7128F
mov     ax, [bp+value]
shl     ax, 1
mov     [bp+value], ax
loc_7128F:
mov     ax, _DI_attacker_idx
mov     dx, size s_WIZARD
imul    dx
mov     bx, ax
cmp     [_players.charismatic+bx], e_ST_TRUE
jnz     short loc_712B0
cmp     [bp+value], 0
jge     short loc_712B0
mov     ax, [bp+value]
cwd
sub     ax, dx
sar     ax, 1
mov     [bp+value], ax
loc_712B0:
mov     ax, _SI_defender_idx
mov     dx, size s_WIZARD
imul    dx
add     ax, _DI_attacker_idx
mov     bx, ax
cmp     [_players.Dipl.Dipl_Status+bx], DIPL_War
jnz     short loc_71321
cmp     [bp+type], dipact_Overextension
jg      short loc_71321
cmp     [bp+value], 0
jge     short loc_71321
mov     ax, _SI_defender_idx
mov     dx, size s_WIZARD
imul    dx
add     ax, _DI_attacker_idx
mov     bx, ax
cmp     [_players.Dipl.Dipl_Status+bx], DIPL_War
jnz     short loc_7131C
or      _DI_attacker_idx, _DI_attacker_idx
jnz     short loc_7131C
mov     ax, _SI_defender_idx
mov     dx, size s_WIZARD
imul    dx
mov     dx, _DI_attacker_idx
shl     dx, 1
add     ax, dx
push    ax
mov     ax, [bp+value]
mov     bx, 4
cwd
idiv    bx
pop     bx
add     [_players.Dipl.peace_modifier+bx], ax
mov     ax, _DI_attacker_idx
mov     dx, size s_WIZARD
imul    dx
mov     dx, _SI_defender_idx
shl     dx, 1
add     ax, dx
push    ax
mov     ax, [bp+value]
mov     bx, 4
cwd
idiv    bx
pop     bx
sub     [_players.Dipl.peace_modifier+bx], ax
loc_7131C:
mov     [bp+value], 0
loc_71321:
or      _DI_attacker_idx, _DI_attacker_idx
jz      short loc_7133F
or      _SI_defender_idx, _SI_defender_idx
jz      short loc_7133F
cmp     [bp+value], 0
jle     short loc_7133F
cmp     [_turn], 200
jle     short loc_7133F
mov     ax, [bp+value]
shl     ax, 1
mov     [bp+value], ax
loc_7133F:
or      _DI_attacker_idx, _DI_attacker_idx
jz      short loc_71369
or      _SI_defender_idx, _SI_defender_idx
jz      short loc_71369
cmp     [bp+value], 0
jge     short loc_71369
cmp     [_turn], 200
jle     short loc_71369
mov     ax, [_difficulty]
cwd
sub     ax, dx
sar     ax, 1
inc     ax
push    ax
mov     ax, [bp+value]
cwd
pop     bx
idiv    bx
mov     [bp+value], ax
loc_71369:
cmp     [bp+value], 0
jnz     short loc_71372
jmp     loc_71514
; ---------------------------------------------------------------------------
loc_71372:
mov     ax, _SI_defender_idx
mov     dx, size s_WIZARD
imul    dx
add     ax, _DI_attacker_idx
mov     bx, ax
mov     al, [_players.Dipl.Visible_Rel+bx]
add     al, [byte ptr bp+value]
push    ax
mov     ax, _SI_defender_idx
mov     dx, size s_WIZARD
imul    dx
add     ax, _DI_attacker_idx
mov     bx, ax
pop     ax
mov     [_players.Dipl.Visible_Rel+bx], al
mov     ax, _SI_defender_idx
mov     dx, size s_WIZARD
imul    dx
add     ax, _DI_attacker_idx
mov     bx, ax
cmp     [_players.Dipl.Visible_Rel+bx], -100
jge     short loc_713B7
mov     ax, _SI_defender_idx
mov     dx, size s_WIZARD
imul    dx
add     ax, _DI_attacker_idx
mov     bx, ax
mov     [_players.Dipl.Visible_Rel+bx], -100
loc_713B7:
mov     ax, _SI_defender_idx
mov     dx, size s_WIZARD
imul    dx
add     ax, _DI_attacker_idx
mov     bx, ax
cmp     [_players.Dipl.Visible_Rel+bx], 100
jle     short loc_713D9
mov     ax, _SI_defender_idx
mov     dx, size s_WIZARD
imul    dx
add     ax, _DI_attacker_idx
mov     bx, ax
mov     [_players.Dipl.Visible_Rel+bx], 100
loc_713D9:
mov     ax, _SI_defender_idx
mov     dx, size s_WIZARD
imul    dx
mov     dx, _DI_attacker_idx
shl     dx, 1
add     ax, dx
mov     dx, [bp+value]
mov     bx, ax
add     [_players.Dipl.treaty_modifier+bx], dx
mov     ax, _SI_defender_idx
mov     dx, size s_WIZARD
imul    dx
mov     dx, _DI_attacker_idx
shl     dx, 1
add     ax, dx
mov     dx, [bp+value]
mov     bx, ax
add     [_players.Dipl.exchange_modifier+bx], dx
mov     ax, _SI_defender_idx
mov     dx, size s_WIZARD
imul    dx
mov     dx, _DI_attacker_idx
shl     dx, 1
add     ax, dx
mov     bx, ax
mov     ax, [_players.Dipl.treaty_modifier+bx]
push    ax
mov     ax, _DI_attacker_idx
mov     dx, size s_WIZARD
imul    dx
mov     dx, _SI_defender_idx
shl     dx, 1
add     ax, dx
mov     bx, ax
pop     ax
mov     [_players.Dipl.treaty_modifier+bx], ax
mov     ax, _SI_defender_idx
mov     dx, size s_WIZARD
imul    dx
mov     dx, _DI_attacker_idx
shl     dx, 1
add     ax, dx
mov     bx, ax
mov     ax, [_players.Dipl.exchange_modifier+bx]
push    ax
mov     ax, _DI_attacker_idx
mov     dx, size s_WIZARD
imul    dx
mov     dx, _SI_defender_idx
shl     dx, 1
add     ax, dx
mov     bx, ax
pop     ax
mov     [_players.Dipl.exchange_modifier+bx], ax
cmp     [bp+NoCharisma_RelValue], 0
jge     short loc_71495
mov     ax, _DI_attacker_idx
mov     dx, size s_WIZARD
imul    dx
mov     dx, _SI_defender_idx
shl     dx, 1
add     ax, dx
push    ax
mov     ax, [bp+NoCharisma_RelValue]
mov     bx, 4
cwd
idiv    bx
pop     bx
sub     [_players.Dipl.peace_modifier+bx], ax
mov     ax, _SI_defender_idx
mov     dx, size s_WIZARD
imul    dx
mov     dx, _DI_attacker_idx
shl     dx, 1
add     ax, dx
push    ax
mov     ax, [bp+NoCharisma_RelValue]
mov     bx, 4
cwd
idiv    bx
pop     bx
add     [_players.Dipl.peace_modifier+bx], ax
jmp     short loc_714AB
; ---------------------------------------------------------------------------
loc_71495:
mov     ax, _SI_defender_idx
mov     dx, size s_WIZARD
imul    dx
mov     dx, _DI_attacker_idx
shl     dx, 1
add     ax, dx
mov     dx, [bp+NoCharisma_RelValue]
mov     bx, ax
add     [_players.Dipl.peace_modifier+bx], dx
loc_714AB:
cmp     [bp+spell_idx], spl_Earth_Lore
jnz     short loc_714C9
mov     ax, 100
push    ax
call    Random
pop     cx
cmp     ax, 11
jge     short loc_714C9
push    _SI_defender_idx
push    _DI_attacker_idx
nop
push    cs
call    near ptr Declare_War
pop     cx
pop     cx
loc_714C9:
or      _DI_attacker_idx, _DI_attacker_idx
jnz     short loc_71514
cmp     [bp+type], 0
jz      short loc_71514
push    [bp+value]
call    abs
pop     cx
push    ax
mov     bx, _SI_defender_idx
shl     bx, 1
push    [_players.Dipl.DA_Strength+bx]
call    abs
pop     cx
pop     dx
cmp     dx, ax
jle     short loc_71514
mov     bx, _SI_defender_idx
shl     bx, 1
mov     ax, [bp+value]
mov     [_players.Dipl.DA_Strength+bx], ax
mov     al, [byte ptr bp+type]
mov     [_players.Dipl.Dipl_Action+_SI_defender_idx], al
mov     bx, _SI_defender_idx
shl     bx, 1
mov     ax, [bp+spell_idx]
mov     [_players.Dipl.DA_Spell+bx], ax
mov     al, [byte ptr bp+city_idx]
mov     [_players.Dipl.DA_City+_SI_defender_idx], al
loc_71514:
mov     ax, _SI_defender_idx
mov     dx, size s_WIZARD
imul    dx
add     ax, _DI_attacker_idx
mov     bx, ax
cmp     [_players.Dipl.Dipl_Status+bx], DIPL_War
jl      short loc_71548
mov     ax, _SI_defender_idx
mov     dx, size s_WIZARD
imul    dx
add     ax, _DI_attacker_idx
mov     bx, ax
cmp     [_players.Dipl.Visible_Rel+bx], -25
jle     short loc_71548
mov     ax, _SI_defender_idx
mov     dx, size s_WIZARD
imul    dx
add     ax, _DI_attacker_idx
mov     bx, ax
mov     [_players.Dipl.Visible_Rel+bx], -25
loc_71548:
mov     ax, _SI_defender_idx
mov     dx, size s_WIZARD
imul    dx
add     ax, _DI_attacker_idx
mov     bx, ax
cmp     [_players.Dipl.Dipl_Status+bx], DIPL_Alliance
jz      short loc_7157C
mov     ax, _SI_defender_idx
mov     dx, size s_WIZARD
imul    dx
add     ax, _DI_attacker_idx
mov     bx, ax
cmp     [_players.Dipl.Visible_Rel+bx], 65
jle     short loc_7157C
mov     ax, _SI_defender_idx
mov     dx, size s_WIZARD
imul    dx
add     ax, _DI_attacker_idx
mov     bx, ax
mov     [_players.Dipl.Visible_Rel+bx], 65
loc_7157C:
mov     ax, _SI_defender_idx
mov     dx, size s_WIZARD
imul    dx
add     ax, _DI_attacker_idx
mov     bx, ax
mov     al, [_players.Dipl.Visible_Rel+bx]
push    ax
mov     ax, _DI_attacker_idx
mov     dx, size s_WIZARD
imul    dx
add     ax, _SI_defender_idx
mov     bx, ax
pop     ax
mov     [_players.Dipl.Visible_Rel+bx], al
mov     ax, _SI_defender_idx
mov     dx, size s_WIZARD
imul    dx
add     ax, _DI_attacker_idx
mov     bx, ax
cmp     [_players.Dipl.Dipl_Status+bx], DIPL_War
jl      short loc_715C3
mov     ax, _SI_defender_idx
mov     dx, size s_WIZARD
imul    dx
mov     dx, _DI_attacker_idx
shl     dx, 1
add     ax, dx
mov     bx, ax
mov     [_players.Hostility+bx], 3
loc_715C3:
mov     ax, _SI_defender_idx
mov     dx, size s_WIZARD
imul    dx
add     ax, _DI_attacker_idx
mov     bx, ax
cmp     [_players.Dipl.Dipl_Status+bx], DIPL_Alliance
jnz     short loc_715EA
mov     ax, _SI_defender_idx
mov     dx, size s_WIZARD
imul    dx
mov     dx, _DI_attacker_idx
shl     dx, 1
add     ax, dx
mov     bx, ax
mov     [_players.Hostility+bx], 0
loc_715EA:
mov     ax, 10
push    ax
call    Random
pop     cx
add     ax, 15
push    ax
mov     ax, _SI_defender_idx
mov     dx, size s_WIZARD
imul    dx
mov     bx, ax
pop     ax
mov     [_players.reevaluate_hostility_countdown+bx], ax
@@Done:
pop     _DI_attacker_idx
pop     _SI_defender_idx
mov     sp, bp
pop     bp
retf
endp Change_Relations__WIP
