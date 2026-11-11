; =============== S U B R O U T I N E =======================================
; Attributes: bp-based frame
proc Diplomacy_Growth far
Too_Strong_Treshold= word ptr -0Ch
human_army_strength= word ptr -0Ah
Gravitation= word ptr -8
human_player_city_count= word ptr -6
Dipl_182h_Field= word ptr -4
Reaction_Value= word ptr -2
itr1 = si
itr2 = di
push    bp
mov     bp, sp
sub     sp, 0Ch
push    itr1
push    itr2
xor     itr1, itr1
jmp     short loc_70685
; ---------------------------------------------------------------------------
loc_70607:
mov     ax, itr1
inc     ax
mov     itr2, ax
jmp     short loc_7067E
; ---------------------------------------------------------------------------
loc_7060E:
mov     ax, 2
push    ax
call    Random
pop     cx
cmp     ax, 1
jnz     short loc_7067D
mov     ax, itr1
mov     dx, size s_WIZARD
imul    dx
add     ax, itr2
mov     bx, ax
cmp     [_players.Dipl.Dipl_Status+bx], DIPL_WizardPact
jnz     short loc_7064D
xor     ax, ax
push    ax
xor     ax, ax
push    ax
xor     ax, ax
push    ax
push    itr2
push    itr1
mov     ax, 3
push    ax
call    Random
pop     cx
push    ax
nop
push    cs
call    near ptr Change_Relations__WIP
add     sp, 0Ch
loc_7064D:
mov     ax, itr1
mov     dx, size s_WIZARD
imul    dx
add     ax, itr2
mov     bx, ax
cmp     [_players.Dipl.Dipl_Status+bx], DIPL_Alliance
jnz     short loc_7067D
xor     ax, ax
push    ax
xor     ax, ax
push    ax
xor     ax, ax
push    ax
push    itr2
push    itr1
mov     ax, 6
push    ax
call    Random
pop     cx
push    ax
nop
push    cs
call    near ptr Change_Relations__WIP
add     sp, 0Ch
loc_7067D:
inc     itr2
loc_7067E:
cmp     itr2, [_num_players]
jl      short loc_7060E
inc     itr1
loc_70685:
cmp     itr1, [_num_players]
jge     short loc_7068E
jmp     loc_70607
; ---------------------------------------------------------------------------
loc_7068E:
mov     ax, [_players.Astr.Army_Strength]
mov     [bp+human_army_strength], ax
cmp     [bp+human_army_strength], 0
jg      short loc_7069D
jmp     loc_70732
; ---------------------------------------------------------------------------
loc_7069D:
xor     itr1, itr1
jmp     loc_70729
; ---------------------------------------------------------------------------
loc_706A2:
mov     ax, itr1
mov     dx, size s_WIZARD
imul    dx
mov     bx, ax
mov     ax, [_players.Astr.Army_Strength+bx]
cmp     ax, [bp+human_army_strength]
jge     short loc_70728
mov     ax, itr1
mov     dx, size s_WIZARD
imul    dx
mov     bx, ax
cmp     [_players.Astr.Army_Strength+bx], 0
jle     short loc_70728
mov     ax, itr1
mov     dx, size s_WIZARD
imul    dx
mov     bx, ax
mov     ax, [_players.Astr.Army_Strength+bx]
cwd
push    dx
push    ax
mov     ax, [bp+human_army_strength]
cwd
push    ax
push    dx
xor     dx, dx
mov     ax, 50
pop     cx
pop     bx
call    LXMUL@
push    dx
push    ax
call    LDIV@
mov     [bp+Too_Strong_Treshold], ax
mov     ax, 100
push    ax
call    Random
pop     cx
cmp     ax, [bp+Too_Strong_Treshold]
jl      short loc_70728
mov     ax, 20
push    ax
call    Random
pop     cx
cmp     ax, 1
jnz     short loc_70728
xor     ax, ax
push    ax
xor     ax, ax
push    ax
mov     ax, 7
push    ax
push    itr1
xor     ax, ax
push    ax
mov     ax, -10
push    ax
loc_70720:
nop
push    cs
call    near ptr Change_Relations__WIP
add     sp, 0Ch
loc_70728:
inc     itr1
loc_70729:
cmp     itr1, [_num_players]
jge     short loc_70732
jmp     loc_706A2
; ---------------------------------------------------------------------------
loc_70732:
mov     ax, [_turn]
mov     bx, 2
cwd
idiv    bx
or      dx, dx
jz      short loc_70742
jmp     loc_7083F
; ---------------------------------------------------------------------------
loc_70742:
xor     itr1, itr1
jmp     loc_70836
; ---------------------------------------------------------------------------
loc_70747:
mov     [bp+human_player_city_count], 0
xor     itr2, itr2
jmp     short loc_70768
; ---------------------------------------------------------------------------
loc_70750:
mov     ax, itr2
mov     dx, size s_CITY
imul    dx
les     bx, [_CITIES]
add     bx, ax
cmp     [es:bx+s_CITY.owner_idx], e_HUMAN_PLAYER_IDX
jnz     short loc_70767
inc     [bp+human_player_city_count]
loc_70767:
inc     itr2
loc_70768:
cmp     itr2, [_cities]
jl      short loc_70750
mov     ax, [_landsize]
inc     ax
mov     dx, 3
imul    dx
cmp     ax, [bp+human_player_city_count]
jl      short loc_7077F
jmp     loc_70835
; ---------------------------------------------------------------------------
loc_7077F:
mov     ax, 4
push    ax
call    Random
pop     cx
cmp     ax, 1
jz      short loc_70791
jmp     loc_70835
; ---------------------------------------------------------------------------
loc_70791:
mov     ax, 4
push    ax
call    Random
pop     cx
push    ax
mov     ax, [_landsize]
inc     ax
mov     dx, 3
imul    dx
push    ax
mov     ax, [bp+human_player_city_count]
pop     dx
sub     ax, dx
neg     ax
mov     bx, [_landsize]
add     bx, 6
sub     bx, [_difficulty]
cwd
idiv    bx
mov     dx, ax
pop     ax
imul    dx
mov     bx, 3
cwd
idiv    bx
mov     [bp+Reaction_Value], ax
mov     ax, [bp+Reaction_Value]
mov     cl, 2
shl     ax, cl
mov     [bp+Reaction_Value], ax
or      itr1, itr1
jz      short loc_707E3
mov     ax, [bp+Reaction_Value]
cwd
sub     ax, dx
sar     ax, 1
mov     [bp+Reaction_Value], ax
loc_707E3:
cmp     [bp+Reaction_Value], -15
jge     short loc_707EE
mov     [bp+Reaction_Value], -15
loc_707EE:
mov     itr2, 1
jmp     short loc_7082F
; ---------------------------------------------------------------------------
loc_707F3:
mov     ax, itr1
mov     dx, size s_WIZARD
imul    dx
add     ax, itr2
mov     bx, ax
cmp     [_players.Dipl.Contacted+bx], e_ST_TRUE
jnz     short loc_7082E
mov     ax, itr1
mov     dx, size s_WIZARD
imul    dx
add     ax, itr2
mov     bx, ax
cmp     [_players.Dipl.Dipl_Status+bx], DIPL_Alliance
jz      short loc_7082E
xor     ax, ax
push    ax
xor     ax, ax
push    ax
mov     ax, 14
push    ax
push    itr2
push    itr1
push    [bp+Reaction_Value]
nop
push    cs
call    near ptr Change_Relations__WIP
add     sp, 0Ch
loc_7082E:
inc     itr2
loc_7082F:
cmp     itr2, [_num_players]
jl      short loc_707F3
loc_70835:
inc     itr1
loc_70836:
cmp     itr1, [_num_players]
jge     short loc_7083F
jmp     loc_70747
; ---------------------------------------------------------------------------
loc_7083F:
mov     itr1, 1
jmp     loc_709BF
; ---------------------------------------------------------------------------
loc_70845:
xor     itr2, itr2
jmp     loc_709B5
; ---------------------------------------------------------------------------
loc_7084A:
mov     ax, 105
push    ax
call    Random
pop     cx
push    ax
mov     ax, itr1
mov     dx, size s_WIZARD
imul    dx
add     ax, itr2
mov     bx, ax
mov     al, [_players.Dipl.Visible_Rel+bx]
cbw
push    ax
call    abs
pop     cx
pop     dx
cmp     dx, ax
jle     short loc_70880
mov     ax, 2
push    ax
call    Random
pop     cx
mov     [bp+Gravitation], ax
jmp     short loc_70885
; ---------------------------------------------------------------------------
loc_70880:
mov     [bp+Gravitation], 0
loc_70885:
cmp     itr2, itr1
jnz     short loc_7088C
jmp     loc_709B4
; ---------------------------------------------------------------------------
loc_7088C:
mov     ax, itr1
mov     dx, size s_WIZARD
imul    dx
mov     dx, itr2
shl     dx, 1
add     ax, dx
mov     bx, ax
cmp     [_players.Dipl.DA_Strength+bx], 0
jz      short loc_708A5
jmp     loc_709B4
; ---------------------------------------------------------------------------
loc_708A5:
mov     ax, itr1
mov     dx, size s_WIZARD
imul    dx
add     ax, itr2
mov     bx, ax
mov     al, [_players.Dipl.Default_Rel+bx]
cbw
mov     [bp+Dipl_182h_Field], ax
mov     ax, itr1
mov     dx, size s_WIZARD
imul    dx
add     ax, itr2
mov     bx, ax
mov     al, [_players.Dipl.Visible_Rel+bx]
cbw
cmp     ax, [bp+Dipl_182h_Field]
jge     short loc_70919
mov     ax, itr1
mov     dx, size s_WIZARD
imul    dx
add     ax, itr2
mov     bx, ax
mov     al, [_players.Dipl.Visible_Rel+bx]
add     al, [byte ptr bp+Gravitation]
push    ax
mov     ax, itr1
mov     dx, size s_WIZARD
imul    dx
add     ax, itr2
mov     bx, ax
pop     ax
mov     [_players.Dipl.Visible_Rel+bx], al
mov     ax, itr1
mov     dx, size s_WIZARD
imul    dx
add     ax, itr2
mov     bx, ax
mov     al, [_players.Dipl.Visible_Rel+bx]
cbw
cmp     ax, [bp+Dipl_182h_Field]
jle     short loc_70917
mov     ax, itr1
mov     dx, size s_WIZARD
imul    dx
add     ax, itr2
mov     dl, [byte ptr bp+Dipl_182h_Field]
mov     bx, ax
mov     [_players.Dipl.Visible_Rel+bx], dl
loc_70917:
jmp     short loc_70970
; ---------------------------------------------------------------------------
loc_70919:
mov     ax, [_turn]
mov     bx, 10
cwd
idiv    bx
or      dx, dx
jnz     short loc_70970
mov     ax, itr1
mov     dx, size s_WIZARD
imul    dx
add     ax, itr2
mov     bx, ax
mov     al, [_players.Dipl.Visible_Rel+bx]
sub     al, [byte ptr bp+Gravitation]
push    ax
mov     ax, itr1
mov     dx, size s_WIZARD
imul    dx
add     ax, itr2
mov     bx, ax
pop     ax
mov     [_players.Dipl.Visible_Rel+bx], al
mov     ax, itr1
mov     dx, size s_WIZARD
imul    dx
add     ax, itr2
mov     bx, ax
mov     al, [_players.Dipl.Visible_Rel+bx]
cbw
cmp     ax, [bp+Dipl_182h_Field]
jge     short loc_70970
mov     ax, itr1
mov     dx, size s_WIZARD
imul    dx
add     ax, itr2
mov     dl, [byte ptr bp+Dipl_182h_Field]
mov     bx, ax
mov     [_players.Dipl.Visible_Rel+bx], dl
loc_70970:
mov     ax, itr1
mov     dx, size s_WIZARD
imul    dx
add     ax, itr2
mov     bx, ax
cmp     [_players.Dipl.Visible_Rel+bx], -100
jge     short loc_70992
mov     ax, itr1
mov     dx, size s_WIZARD
imul    dx
add     ax, itr2
mov     bx, ax
mov     [_players.Dipl.Visible_Rel+bx], -100
loc_70992:
mov     ax, itr1
mov     dx, size s_WIZARD
imul    dx
add     ax, itr2
mov     bx, ax
cmp     [_players.Dipl.Visible_Rel+bx], 100
jle     short loc_709B4
mov     ax, itr1
mov     dx, size s_WIZARD
imul    dx
add     ax, itr2
mov     bx, ax
mov     [_players.Dipl.Visible_Rel+bx], 100
loc_709B4:
inc     itr2
loc_709B5:
cmp     itr2, [_num_players]
jge     short loc_709BE
jmp     loc_7084A
; ---------------------------------------------------------------------------
loc_709BE:
inc     itr1
loc_709BF:
cmp     itr1, [_num_players]
jge     short loc_709C8
jmp     loc_70845
; ---------------------------------------------------------------------------
loc_709C8:
xor     itr1, itr1
jmp     short loc_70A0E
; ---------------------------------------------------------------------------
loc_709CC:
xor     itr2, itr2
jmp     short loc_70A07
; ---------------------------------------------------------------------------
loc_709D0:
cmp     itr1, itr2
jnz     short loc_709E6
mov     ax, itr1
mov     dx, size s_WIZARD
imul    dx
add     ax, itr2
mov     bx, ax
mov     [_players.Dipl.Visible_Rel+bx], 0
jmp     short loc_70A06
; ---------------------------------------------------------------------------
loc_709E6:
mov     ax, itr2
mov     dx, size s_WIZARD
imul    dx
add     ax, itr1
mov     bx, ax
mov     al, [_players.Dipl.Visible_Rel+bx]
push    ax
mov     ax, itr1
mov     dx, size s_WIZARD
imul    dx
add     ax, itr2
mov     bx, ax
pop     ax
mov     [_players.Dipl.Visible_Rel+bx], al
loc_70A06:
inc     itr2
loc_70A07:
cmp     itr2, [_num_players]
jl      short loc_709D0
inc     itr1
loc_70A0E:
cmp     itr1, [_num_players]
jl      short loc_709CC
pop     itr2
pop     itr1
mov     sp, bp
pop     bp
retf
endp Diplomacy_Growth
