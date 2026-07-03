; WZD-o145p04-City_Gold_Production.asm
; WZD o145p04

proc City_Gold_Production far
wy_array= word ptr -72h
wx_array= word ptr -40h
gold_modifier= word ptr	-0Eh
are_dwarf= word	ptr -0Ch
have_miners_guild= word	ptr -0Ah
city_wp= word ptr -8
city_owner_idx=	word ptr -6
itr= word ptr -4
useable_map_squares= word ptr -2
_city_idx= word	ptr  6
city_idx = si
gold_units = di
push	bp
mov	bp, sp
sub	sp, 72h
push	city_idx
push	gold_units


mov	city_idx, [bp+_city_idx]


mov	ax, city_idx
mov	dx, size s_CITY
imul	dx
les	bx, [_CITIES]
add	bx, ax
cmp	[es:bx+s_CITY.population], 0
jnz	short loc_C8011
@@JmpJmpDone__Return_ZERO:
xor	ax, ax
@@JmpDone:
jmp	@@Done
; ---------------------------------------------------------------------------
loc_C8011:
mov	ax, city_idx
mov	dx, size s_CITY
imul	dx
les	bx, [_CITIES]
add	bx, ax
mov	al, [es:bx+s_CITY.owner_idx]
cbw
mov	[bp+city_owner_idx], ax


mov	ax, city_idx
mov	dx, size s_CITY
imul	dx
les	bx, [_CITIES]
add	bx, ax
mov	al, [es:bx+s_CITY.wp]
cbw
mov	[bp+city_wp], ax


lea	ax, [bp+wy_array]
push	ax				; wy_array@
lea	ax, [bp+wx_array]
push	ax				; wx_array@
push	[bp+city_wp]			; wp
mov	ax, city_idx
mov	dx, size s_CITY
imul	dx
les	bx, [_CITIES]
add	bx, ax
mov	al, [es:bx+s_CITY.wy]
cbw
push	ax				; wy
mov	ax, city_idx
mov	dx, size s_CITY
imul	dx
les	bx, [_CITIES]
add	bx, ax
mov	al, [es:bx+s_CITY.wx]
cbw
push	ax				; wx
push	cs
call	near ptr Get_Useable_City_Area
add	sp, 0Ah


mov	[bp+useable_map_squares], ax


mov	ax, city_idx
mov	dx, size s_CITY
imul	dx
les	bx, [_CITIES]
add	bx, ax
mov	al, [es:bx+s_CITY.wp]
cbw
push	ax				; Plane
mov	ax, city_idx
mov	dx, size s_CITY
imul	dx
les	bx, [_CITIES]
add	bx, ax
mov	al, [es:bx+s_CITY.wy]
cbw
push	ax				; YPos
mov	ax, city_idx
mov	dx, size s_CITY
imul	dx
les	bx, [_CITIES]
add	bx, ax
mov	al, [es:bx+s_CITY.wx]
cbw
push	ax				; XPos
call	j_Map_Square_Gold_Bonus		; returns the percentage of trade gold that a city
					; on the tile gets or would from oceans	and rivers
add	sp, 6


push	ax


push	city_idx			; City_Index
call	j_City_Road_Trade_Bonus		; calculates the road gold percentage based on the
					; active road connections; 1% per connected town pop,
					; half that if same race, max 3% per source city pop
pop	cx


pop	dx


add	ax, dx


mov	[bp+gold_modifier], ax


mov	ax, city_idx
mov	dx, size s_CITY
imul	dx
les	bx, [_CITIES]
add	bx, ax
cmp	[es:bx+s_CITY.race], rt_Nomad
jnz	short loc_C80DD
add	[bp+gold_modifier], 50
loc_C80DD:
mov	ax, city_idx
mov	dx, size s_CITY
imul	dx
les	bx, [_CITIES]
add	bx, ax
mov	al, [es:bx+s_CITY.population]
cbw
mov	dx, 3
imul	dx
cmp	ax, [bp+gold_modifier]
jge	short loc_C8113
mov	ax, city_idx
mov	dx, size s_CITY
imul	dx
les	bx, [_CITIES]
add	bx, ax
mov	al, [es:bx+s_CITY.population]
cbw
mov	dx, 3
imul	dx
mov	[bp+gold_modifier], ax
loc_C8113:
mov	ax, city_idx
mov	dx, size s_CITY
imul	dx
les	bx, [_CITIES]
add	bx, ax
cmp	[es:bx+(s_CITY.bldg_status+1Ch)], bs_Built ; {-1:NotBuilt,0:Replaced,1:Built,2:Removed}
jz	short loc_C813B
mov	ax, city_idx
mov	dx, size s_CITY
imul	dx
les	bx, [_CITIES]
add	bx, ax
cmp	[es:bx+(s_CITY.bldg_status+1Ch)], bs_Replaced ;	{-1:NotBuilt,0:Replaced,1:Built,2:Removed}
jnz	short loc_C813F
loc_C813B:
add	[bp+gold_modifier], 100
loc_C813F:
mov	ax, city_idx
mov	dx, size s_CITY
imul	dx
les	bx, [_CITIES]
add	bx, ax
cmp	[es:bx+(s_CITY.bldg_status+22h)], bs_Built ; {-1:NotBuilt,0:Replaced,1:Built,2:Removed}
jz	short loc_C8167
mov	ax, city_idx
mov	dx, size s_CITY
imul	dx
les	bx, [_CITIES]
add	bx, ax
cmp	[es:bx+(s_CITY.bldg_status+22h)], bs_Replaced ;	{-1:NotBuilt,0:Replaced,1:Built,2:Removed}
jnz	short loc_C816E
loc_C8167:
mov	[bp+have_miners_guild],	e_ST_TRUE
jmp	short loc_C8173
; ---------------------------------------------------------------------------
loc_C816E:
mov	[bp+have_miners_guild],	e_ST_FALSE
loc_C8173:
mov	ax, city_idx
mov	dx, size s_CITY
imul	dx
les	bx, [_CITIES]
add	bx, ax
cmp	[es:bx+(s_CITY.bldg_status+1Bh)], bs_Built ; {-1:NotBuilt,0:Replaced,1:Built,2:Removed}
jz	short loc_C819B
mov	ax, city_idx
mov	dx, size s_CITY
imul	dx
les	bx, [_CITIES]
add	bx, ax
cmp	[es:bx+(s_CITY.bldg_status+1Bh)], bs_Replaced ;	{-1:NotBuilt,0:Replaced,1:Built,2:Removed}
jnz	short loc_C819F
loc_C819B:
add	[bp+gold_modifier], 50
loc_C819F:
mov	ax, city_idx
mov	dx, size s_CITY
imul	dx
les	bx, [_CITIES]
add	bx, ax
cmp	[es:bx+(s_CITY.bldg_status+1Ah)], bs_Built ; {-1:NotBuilt,0:Replaced,1:Built,2:Removed}
jz	short loc_C81C7
mov	ax, city_idx
mov	dx, size s_CITY
imul	dx
les	bx, [_CITIES]
add	bx, ax
cmp	[es:bx+(s_CITY.bldg_status+1Ah)], bs_Replaced ;	{-1:NotBuilt,0:Replaced,1:Built,2:Removed}
jnz	short loc_C81CB
loc_C81C7:
add	[bp+gold_modifier], 50
loc_C81CB:
mov	ax, city_idx
mov	dx, size s_CITY
imul	dx
les	bx, [_CITIES]
add	bx, ax
loc_C81D8:
cmp	[es:bx+s_CITY.enchantments.PROSPERITY],	0
jle	short loc_C81E3
add	[bp+gold_modifier], 100
loc_C81E3:
cmp	[bp+city_owner_idx], e_NEUTRAL_PLAYER_IDX
jnz	short loc_C820A
mov	ax, city_idx
mov	dx, size s_CITY
imul	dx
les	bx, [_CITIES]
add	bx, ax
mov	al, [es:bx+s_CITY.population]
cbw


push	ax


push	city_idx			; city_idx
nop
push	cs
call	near ptr City_Rebel_Count
pop	cx


pop	dx


sub	dx, ax
mov	gold_units, dx


jmp	short loc_C823E
; ---------------------------------------------------------------------------
loc_C820A:
mov	ax, city_idx
mov	dx, size s_CITY
imul	dx
les	bx, [_CITIES]
add	bx, ax
mov	al, [es:bx+s_CITY.population]
cbw


push	ax


push	city_idx			; city_idx
nop
push	cs
call	near ptr City_Rebel_Count
pop	cx


pop	dx


sub	dx, ax
mov	ax, [bp+city_owner_idx]
mov	bx, 4C8h


push	dx


imul	bx
mov	bx, ax


pop	ax


imul	[_players.tax_rate+bx]
cwd
sub	ax, dx
sar	ax, 1
mov	gold_units, ax
loc_C823E:
mov	ax, city_idx
mov	dx, size s_CITY
imul	dx
les	bx, [_CITIES]
add	bx, ax
cmp	[es:bx+s_CITY.race], rt_Dwarf
jnz	short loc_C825B
add	gold_units, gold_units
mov	[bp+are_dwarf],	e_ST_TRUE
jmp	short loc_C8260
; ---------------------------------------------------------------------------
loc_C825B:
mov	[bp+are_dwarf],	e_ST_FALSE
loc_C8260:
mov	[bp+itr], 0
jmp	short loc_C8295
; ---------------------------------------------------------------------------
loc_C8267:				; Dwarves
push	[bp+are_dwarf]
push	[bp+have_miners_guild]		; Miners
push	[bp+city_wp]			; Plane
mov	bx, [bp+itr]
shl	bx, 1				; sizeof
lea	ax, [bp+wy_array]
add	bx, ax
push	[word ptr bx]			; YPos
mov	bx, [bp+itr]
shl	bx, 1				; sizeof
lea	ax, [bp+wx_array]
add	bx, ax
push	[word ptr bx]			; XPos
call	j_Square_Gold_Income
add	sp, 0Ah


add	gold_units, ax


inc	[bp+itr]
loc_C8295:
mov	ax, [bp+itr]
cmp	ax, [bp+useable_map_squares]
jl	short loc_C8267
xor	ax, ax
mov	dx, 100
push	ax
push	dx				; divisor
mov	ax, gold_units
cwd
push	ax
mov	ax, [bp+gold_modifier]
push	dx
cwd
add	ax, 100
adc	dx, 0
pop	cx
pop	bx
call	LXMUL@				; #2 * #3 / #1
push	dx
push	ax				; dividend
call	LDIV@				; #2 * #3 / #1
mov	gold_units, ax


mov	ax, city_idx
mov	dx, size s_CITY
imul	dx
les	bx, [_CITIES]
add	bx, ax
cmp	[es:bx+s_CITY.construction], bt_TradeGoods
jnz	short loc_C82F1
mov	ax, city_idx
mov	dx, size s_CITY
imul	dx
les	bx, [_CITIES]
add	bx, ax
mov	al, [es:bx+s_CITY.production_units]
mov	ah, 0
cwd
sub	ax, dx
sar	ax, 1
add	gold_units, ax
loc_C82F1:
cmp	gold_units, 255
jle	short @@JmpJmpDone__Return_DI
mov	gold_units, 255
@@JmpJmpDone__Return_DI:
mov	ax, gold_units
jmp	@@JmpDone
; ---------------------------------------------------------------------------
@@Done:
pop	gold_units
pop	city_idx
mov	sp, bp
pop	bp
retf
endp City_Gold_Production
