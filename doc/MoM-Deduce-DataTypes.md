


s_BATTLE_UNIT.Extra_Hits
1-byte, signed
s_BATTLE_UNIT.Gold_Hits
1-byte, signed

BU_GetHitsPerFigure()
ovr116:0EC7 C4 5E 06                                        les     bx, [bp+battle_unit]
ovr116:0ECA 26 8A 47 32                                     mov     al, [es:bx+s_BATTLE_UNIT.Extra_Hits]
ovr116:0ECE 98                                              cbw
ovr116:0ECF 03 F8                                           add     _DI_IDK_hit_points, ax
ovr116:0ECF
ovr116:0ECF
ovr116:0ED1 C4 5E 06                                        les     bx, [bp+battle_unit]
ovr116:0ED4 26 8A 47 32                                     mov     al, [es:bx+s_BATTLE_UNIT.Extra_Hits]
ovr116:0ED8 C4 5E 06                                        les     bx, [bp+battle_unit]
ovr116:0EDB 26 00 47 68                                     add     [es:bx+s_BATTLE_UNIT.Gold_Hits], al








CTY_CatchmentRefresh()

mov     ax, [bp+itr_wp]
mov     dx, e_WORLD_SIZE_DW
imul    dx
les     bx, [city_area_bits]
add     bx, ax
add     bx, _SI_itr_cities
mov     [byte ptr es:bx], 0


mov     ax, [bp+itr_wp]
mov     dx, e_WORLD_SIZE_DW
imul    dx
les     bx, [city_area_shared_bits]
add     bx, ax
add     bx, _SI_itr_cities
mov     [byte ptr es:bx], 0

¿ les     bx, [city_area_bits] ... LES means it knows it's a Far Pointer ?
¿ mov     [byte ptr es:bx], 0  ... byte ptr means it knows it's a DB ?













push    [_screen_seg]                   ; base_seg
les     bx, [global_strategic_unit]
mov     al, [es:bx+s_STRATEGIC_UNIT.Unused_0Ch]
mov     ah, 0
push    ax                              ; entry_num
mov     ax, offset cnst_PORTRAIT_File4  ; should use dseg:326e
push    ax                              ; file_name
call    LBX_Reload_Next

add     sp, 6


mov     [IMG_USW_HeroPortrt@], ax









1-Byte, Unsigned
s_CITY.gold_units
s_CITY.building_maintenance
s_CITY.food_units
s_CITY.production_units
s_CITY.mana_units
s_CITY.research_units

1-Byte, Signed
s_CITY.population


1-Byte, Signed
s_FORTRESS.wx
s_FORTRESS.wy
s_FORTRESS.wp


1-Byte, Signed
s_UNIT_TYPE.Sight
s_UNIT_TYPE.Upkeep


1-Byte, Unsigned
USW_Ability.USW_IMG_Index
index into `special_seg` for SPECIAL.LBX and SPECIAL2.LBX, all {UNICON1, ..., UNICON7}
USW_Abilities.USW_IMG_Index
USW_Attributes_1.USW_IMG_Index
USW_Attributes_2.USW_IMG_Index
USW_ATK_Flags.USW_IMG_Index
USW_SR_Attacks.USW_IMG_Index
¿ USW_HeroAbl ?
¿ USW_Mutation?

s_STRATEGIC_UNIT.Unused_0Ch


1-Byte, Signed
s_STRATEGIC_UNIT.Spec_Att_Attrib
s_STRATEGIC_UNIT.Poison_Strength
s_STRATEGIC_UNIT.ranged
s_STRATEGIC_UNIT.TopFig_Dmg
s_STRATEGIC_UNIT.Weapon_Plus1
s_STRATEGIC_UNIT.melee
s_STRATEGIC_UNIT.Gold_Melee
s_STRATEGIC_UNIT.Grey_Melee


1-Byte
s_STRATEGIC_UNIT.ranged_type



1-Byte, Signed
s_SPELL_BOOK_PAGE.Spell_Count
ovr117:0BDB 8A 46 22                                        mov     al, [bp+Page.Spell_Count]
ovr117:0BDE 98                                              cbw
ovr117:0BDF 3B C6                                           cmp     ax, _SI_itr






s_UNIT  
s_UNITTYPE  
s_RACE  
s_STRATEGIC_UNIT  

For each structure member, ...  
...value is...
...1,2,4 byte...
...signed,unsigned...
...bitfield...
...
...min,max,range...

¿ Dasm shows `test` ?
F7
TEST r/m16,imm16 ; o16 F7 /7 iw [8086]

F7 87
F7 is TEST
¿ 8 ?
¿ 7 ? 
¿ BX ?

BA 01
0x01BA
_unit_type_table.Abilities

01 00
0x0001  Ab_Summoned

20 00
0x0020  Ab_CrOutpost

ovr121:02C8 F7 87 BA 01 20 00
test    [_unit_type_table.Abilities+bx], Ab_CrOutpost
F7 87 BA 01 20 00

ovr121:0467 F7 87 BA 01 01 00
test    [_unit_type_table.Abilities+bx], Ab_Summoned
F7 87 BA 01 01 00


seg022:04F0
85 87 4A 78
test    [bit_field_test_bits+bx], ax
85 87
TEST
4A 78
0x784A  dseg:784A  bit_field_test_bits


ovr122:1720 8B 46 0A                                        
mov     ax, [bp+Imm_Flags]
ovr122:1723 26 23 47 18                                     
and     ax, [es:bx+BU_REC.Attribs_1]
ovr122:1727 A9 00 01                                        
test    ax, Imm_Weapon

A9 00 01
A9
TEST
00 01
0x0100  Imm_Weapon











drake178 uses file offset
there ain't no short-cut key


8BFED
95A0D

95A0D - 8BFED = 9A20



ovr057:10BD 26 8A 47 12                                     
mov     al, [es:bx+s_CITY.owner_idx]
ovr057:10C1 98                                              
cbw



s_UNIT
Draw_Priority
4DE36
5A266

ovr067:0685 26 8A 47 12
mov     al, [es:bx+s_UNIT.Draw_Priority]
ovr067:0696 26 3A 47 12
cmp     al, [es:bx+s_UNIT.Draw_Priority]

 is defined as 8 bit signed.
Anything else would result in different compiled code at e.g. 
$4DE3A
(where we have 8 bit signed comparison).

s_UNIT.Level
is also defined as 8 bit signed
$97636
reads 8 bits and then sign-extends it to 16 bits

ovr120:1A96 26 8A 47 0C                                     
mov     al, [es:bx+s_UNIT.Level]
ovr120:1A9A 98                                              
cbw



s_UNIT.type

26 8A 47 05

ovr055:07E6 26 8A 47 05                                     
mov     al, [es:bx+s_UNIT.type]
ovr055:07EA B4 00                                           
mov     ah, 0



For unsigned byte fields (e.g. unit type), the extension is naturally different. 


s_UNITTYPE

/* 18 */  uint16_t Move_Flags;

TODO: check the data-type - UnitHasFlying() uses `byte ptr` and `CBW` */

51BE8
5DDA8
ovr071:06A8 8A 87 B4 01                                     
mov     al, [byte ptr _unit_type_table.Move_Flags+bx]
ovr071:06AC 98                                              
cbw

ovr071:06A8 8A 87 B4 01
mov     al, [bx+1B4h]

1B4 - 019C = 18

1B5 - 019C = 

8A 87 B5 01



42420
4F130
ovr056:0B70 8A 87 AA 01
mov     al, [byte ptr _unit_type_table.u_Bldng1_or_Portrait+bx]
ovr056:0B74 B4 00                                           
mov     ah, 0

01AA - 019C = 0E
019C + 0E = 1AA

01AB - 019C = 0F
8A 87 AB 01

019C + 0A = 1A6
8A 87 A6 01

8A 87 A7 01

8A 87 A5 01

ovr121:0094 UNIT_Create           mov     al, [_unit_type_table.Move_Halves+bx]
ovr121:0A82 UNIT_GetHalfMoves_WIP mov     al, [_unit_type_table.Move_Halves+bx]
ovr124:0DD8 BU_GetHalfMoves       mov     al, [_unit_type_table.Move_Halves+bx]

97BC4
A0F54
ovr121:0094 8A 87 A5 01                                     
mov     al, [_unit_type_table.Move_Halves+bx]
ovr121:00A6 26 88 47 04                                     
mov     [es:bx+s_UNIT.HalfMoves_Max], al


ovr121:0111 8A 87 AF 01                                     
mov     al, [_unit_type_table.Sight+bx]

ovr121:0123 26 88 47 16                                     
mov     [es:bx+s_UNIT.Sight_Range], al

ovr121:01B3 26 C7 47 1A 00 00                               
mov     [es:bx+s_UNIT.Enchants_HI], 0
ovr121:01B9 26 C7 47 18 00 00                               
mov     [es:bx+s_UNIT.Enchants_LO], 0


ovr121:02A6 9A A2 00 84 36                                  
call    j_CTY_GetWeaponQuality
    ; returns the highest weapon quality available to units
    ; produced in the city (normal = 0, adamantium = 3)
...
ovr121:02BA 26 88 47 17                                     
mov     [es:bx+s_UNIT.Mutations], al



5DEBE
6967E
ovr078:1CD9 8A 87 AF 01                                     mov     al, [_unit_type_table.Sight+bx]
ovr078:1CDD 98                                              cbw


ovr121:02C8 F7 87 BA 01 20 00
test    [_unit_type_table.Abilities+bx], Ab_CrOutpost
F7 87 BA 01 20 00

ovr121:0467 F7 87 BA 01 01 00
test    [_unit_type_table.Abilities+bx], Ab_Summoned
F7 87 BA 01 01 00







42434
4F144
ovr056:0B84 8A 87 AB 01                                     
mov     al, [_unit_type_table.Bldng2_or_HeroType+bx]
ovr056:0B88 B4 00                                           
mov     ah, 0


ovr056:0B4A 8A 87 A9 01                                     
mov     al, [_unit_type_table.Race+bx]
ovr056:0B4E 98                                              
cbw


53934
5FAE4
ovr072:18EF 8A 87 A8 01                                     
mov     al, [_unit_type_table.Upkeep+bx]
ovr072:18F3 98                                              
cbw




## s_CITY.enchantments
looks to definitely be 1 byte
    but maybe also signed
more below / later
    is definitely signed OR just casting to signed, because C, and the casting is to the type for the assignemnt

3FCD9
4CC69
ovr055:2879 C4 5E F8                                        les     bx, [bp+city_enchantments]
ovr055:287C 03 DE                                           add     bx, _SI_itr_City_Enchantments
ovr055:287E 26 80 3F 00                                     cmp     [byte ptr es:bx], e_ST_FALSE
ovr055:2882 7E 47                                           jle     short @@NextCityEnchantment_1

ovr055:28B5 C4 5E F8                                        les     bx, [bp+city_enchantments]
ovr055:28B8 03 DE                                           add     bx, _SI_itr_City_Enchantments
ovr055:28BA 26 8A 07                                        mov     al, [es:bx]
ovr055:28BD 98                                              cbw



## s_CITY.farmer_count
definitely a byte

3AA54
47D14
ovr054:0154 8A 46 E4                                        mov     al, [byte ptr bp+required_farmer_count]
ovr054:0157 26 88 47 15                                     mov     [es:bx+s_CITY.farmer_count], al





IDK_CityScreen_AddResourcesFields()

ovr055:1C31 26 8A 47 64                                     mov     al, [es:bx+s_CITY.food_units]
ovr055:1C35 B4 00                                           mov     ah, 0

ovr055:1C45 26 8A 47 14                                     mov     al, [es:bx+s_CITY.population]
ovr055:1C49 98                                              cbw

3F0C4
4C054
ovr055:1C64 26 8A 47 60                                     mov     al, [es:bx+s_CITY.gold_units]
ovr055:1C68 B4 00                                           mov     ah, 0

ovr055:1C78 26 8A 47 61                                     mov     al, [es:bx+s_CITY.building_maintenance]
ovr055:1C7C B4 00                                           mov     ah, 0

ovr055:1C98 26 8A 47 64                                     mov     al, [es:bx+s_CITY.food_units]
ovr055:1C9C B4 00                                           mov     ah, 0

ovr055:1CAF 26 8A 47 5D                                     mov     al, [es:bx+s_CITY.production_units]
ovr055:1CB3 B4 00                                           mov     ah, 0

ovr055:1CC5 26 8A 47 62                                     mov     al, [es:bx+s_CITY.mana_units]
ovr055:1CC9 B4 00                                           mov     ah, 0

ovr055:1CDB 26 8A 47 63                                     mov     al, [es:bx+s_CITY.research_units]
ovr055:1CDF B4 00                                           mov     ah, 0


ovr055:1CF1 26 8A 47 60                                     mov     al, [es:bx+s_CITY.gold_units]
ovr055:1CF5 50                                              push    ax
...
ovr055:1D04 26 3A 47 61                                     cmp     al, [es:bx+s_CITY.building_maintenance]


cmp s_CITY.population, s_CITY.food_units :: can not see if they are signed or unsigned
mov Current, s_CITY.food_units  :: can see that food_units is unsigned, 1-byte
mov Current, s_CITY.population  :: can see that population is   signed, 1-byte

ovr055:1DBD 26 8A 47 14                                     mov     al, [es:bx+s_CITY.population]
...
ovr055:1DD0 26 3A 47 64                                     cmp     al, [es:bx+s_CITY.food_units]
...
ovr055:1DE3 26 8A 47 64                                     mov     al, [es:bx+s_CITY.food_units]
ovr055:1DE7 B4 00                                           mov     ah, 0
...
ovr055:1DF8 26 8A 47 14                                     mov     al, [es:bx+s_CITY.population]
ovr055:1DFC 98                                              cbw
...
mov     [bp+Current], ax






GAME_Overland_Init()

3895A
45E9A
ovr051:013A 26 8A 47 02                                     mov     al, [es:bx+s_FORTRESS.world_plane]
ovr051:013E 98                                              cbw
ovr051:013F A3 86 BD                                        mov     [_map_plane], ax

ovr051:0154 26 8A 47 01                                     mov     al, [es:bx+s_FORTRESS.world_y]
ovr051:0158 98                                              cbw

ovr051:015E 26 8A 07                                        mov     al, [es:bx+s_FORTRESS.world_x]
ovr051:0161 98                                              cbw

Center_Map()
