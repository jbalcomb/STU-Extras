
Page 95  (PDF Page 100)
Stoning Gaze and First Strike attacks are conducted simultaneously



Special Unit Abilities

Unit Special Abilities

Hero Special Abilities


unit_type_table
s_UNIT_TYPE

s_UNIT_TYPE.Construction > 0                "Construction"
s_UNIT_TYPE.Sight > 1                       "Scouting"

s_UNIT_TYPE.Move_Flags & 0x10               "Teleporting"
s_UNIT_TYPE.Move_Flags & 0x20               "Forester"
s_UNIT_TYPE.Move_Flags & 0x40               "Mountaineer"

Abilities  (15)
    "Large Shield";
    "Plane Shift";
    "Wall Crusher";
    "Healer";
    "Create Outpost";
    "Invisibility";
    "Create Undead";
    "Undead";
    "Long Range";
    "Land Corruption";
    "Meld With Node";
    "Non Corporeal";
    "Wind Walking";
    "Regeneration";
    "Purify";
    "Negate First Strike";
    "First Strike";

Attribs_1  (10)
    "Lucky";
    "Poison Immunity";
    "Fire Immunity";
    "Stoning Immunity";
    "Weapon Immunity";
    "Missiles Immunity";
    "Illusions Immunity";
    "Cold Immunity";
    "Magic Immunity";
    "Death Immunity";

Attribs_2  (9)
    "Healing Spell";
    "Spell";
    "Fire Ball Spell";
    "Doombolt Spell";
    "Immolation";
    "Cause Fear Spell";
    "Web Spell";
    "Resistance To All";
    "Holy Bonus";

Attack_Flags  (11)
    "Armor Piercing";
    "Poison";
    "Life Steal";
    "Automatic Damage";
    "Destruction";
    "Illusion";
    "Stoning Touch";
    "Death Touch";
    "Power Drain";
    "Dispel Evil";

Spec_Att_Attrib

Ranged Attack Type
    100 + itr
    {100,101,102,103,104,105}
    default is to use ranged attack strength  s_COMBAT_UNIT.ranged
    but, 103 and 105 use s_COMBAT_UNIT.Spec_Att_Attrib
    and, 104 *includes* 103 and 105

Attack_Flags
    default is no value
    but, 0x04 and 0x80 use Spec_Att_Attr

¿ is it meaningful that 'Poison Attack' and 'Stoning Touch' are post-melee ?
"Defender must resist for each strength unit of the poison attack..."
    .Attack_Flag & 0x04  /* Att_Poison */
    .Attack_Flag & 0x80  /* Att_StnTouch */
Active_Unit->Poison_Strength;
CombatUnit->Poison_Strength = _unit_type_table[unit_type].Spec_Att_Attr;











Elsewhere, ...
    USW_Build_Effect_List()
    UV_Build_Effect_List__WIP()
        loops for 6
        checks ranged attack type
        specifically against USW_SR_Attacks[].Ranged_Type flags
        in order
        special extra checks for 103, 105, 104
        if 103 or 105
            uses Spec_Att_Attrib for the *attack strength*
        if 104

    "Thrown",                 /*      100  */
    "Fiery Breath",           /*      101  "Fiery Breath" */
    "Sky Drake",              /* OON  102  Sky Drake  Breath  Lightning Breath */
    "Stoning Gaze",           /*      103  Basilisk, Gorgons  "Stoning Gaze" */
    "Chaos Spawn",            /* OON  104  Chaos Spawn  Death Gaze, Doom Gaze, Stoning Gaze */
    "Night Stalker"           /* OON  105  Night Stalker  Death Gaze */

// WZD dseg:42B5
char cnst_Thrown[] = "Thrown";
char cnst_FireBreath[] = "Fire Breath";
char cnst_Lightning[] = "Lightning";
char cnst_StoningGaze[] = "Stoning Gaze";
char cnst_DoomGaze[] = "Doom Gaze";
char cnst_DeathGaze[] = "Death Gaze";

char * unit_sr_attack_names[] =
{
    cnst_Thrown,
    cnst_FireBreath,
    cnst_Lightning,
    cnst_StoningGaze,
    cnst_DoomGaze,
    cnst_DeathGaze
};

// WZD dseg:3C7A
// drake178: struct USW_SR_Attack USW_SR_Attacks
struct USW_SR_Attack USW_SR_Attacksp[] =
{
    { &unit_sr_attack_names[0], 0x64,  0x82,  0x01BB },
    { &unit_sr_attack_names[1], 0x65,  0x8A,  0x01BC },
    { &unit_sr_attack_names[2], 0x66,  0x89,  0x01BD },
    { &unit_sr_attack_names[3], 0x67,  0x1A,  0x01BE },
    { &unit_sr_attack_names[4], 0x68,  0x86,  0x01BF },
    { &unit_sr_attack_names[5], 0x69,  0x87,  0x01C0 }
};






(unit_type_table[itr].attribs_1 & 0x8000)
is "Race-Specific Unit"



Unit Attributes

Melee Attack Strength

Hit Points
Defense
Resistance

Cost
    Production
    Gold
    Mana

Upkeep
    Food
    Gold
    Mana

Type
    Hero
    Non-Hero
        Normal
            Standard
            Race-Specific
        Fantastic  (Summoned)

Unit Type  [  0, 34]   35
Unit Type  [ 35,153]  119
Unit Type  [154,197]   44

Ranged Attack Type
Ranged Attack Strength
Ranged Attack Ammo

¿ ammo != 0 ?
¿ rt == 0 ?

¿ meaning, if any, of ranges of ranged attack types ?
order of operation?

Every Unit with rt == -1 has ranged == 0 and ammo == 0

Unlimited Ammo?
rt != -1 && ammo == 0
bunches

rang_type vs ammo ?
no relationship









melee
ranged
ranged_type
ammo
to_hit
defense
resist
move_halves
cost
upkeep
race
bldng1_or_portrait
bldng2_or_herotype
pict_seg
hits
sight
transport
figures
construction
spec_att_attr
move_flags
attribs_1
attribs_2
abilities
attack_flags
sound


melee
{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 13, 14, 15, 18, 20, 25, 30 }  

0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 13, 14, 15, 18, 20, 25, 30
1, 2, 3, 4, 5, 6, 7, 8, 9, 12
0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 15
0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 13, 14, 15, 18, 20, 25, 30


int8_t   Ranged_Type  
{ -1, 10, 20, 21, 30, 31, 32, 33, 34, 35, 36, 37, 38, 100, 101, 102, 103, 104, 105 }  

{  
    -1,  
    10,  
    20, 21,  
    30, 31, 32, 33, 34, 35, 36, 37, 38,  
    100, 101, 102, 103, 104, 105,  
}  

int8_t   Race  
{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20 }  

int8_t   Spec_Att_Attr  
{ 0, 1, 2, 4, 5, 6, 15, 251, 252, 253, 254, 255 }  

uint16_t Move_Flags  
{ 0, 1, 2, 4, 5, 6, 15, 65531, 65532, 65533, 65534, 65535 }  

uint16_t Attribs_1  
{ 0, 1, 4, 32, 36, 128, 130, 131, 192, 194, 216, 220, 472, 512, 520, 552, 898, 984, 988, 1024, 7128, 8704, 8705, 16904, 32768, 32772, 33280, 33284, 33792 }  

uint16_t Attribs_2
{ 0, 1, 2, 4, 8, 16, 32, 64, 128 }  

uint16_t Abilities  
{ 0, 1, 2, 8, 9, 16, 32, 64, 65, 129, 256, 264, 2049, 3073, 4096, 4097, 8192, 8193, 8194, 8224, 10245, 16384, 16400, 24592, 32768, 40960 }  

uint16_t Attack_Flags
{ 0, 1, 2, 3, 4, 6, 8, 11, 64, 128, 2048 }  

Move_Flags
65531  ||  -5
FFFB
1111111111111011



Abilities
40960
A000
1010000000000000

32768
8000
1000000000000000

8192
2000
0010000000000000

Attack_Flags
2048
800
0000100000000000


Heuristics?
FIIK
But, maybe, ...
Yay Nay Power of 2
combination thereof?

e.g., Attribs_2
all powers of 2, all distinct values

e.g., Attack_Flags
    ...3, 6, 11, ...
     3 == 03 == 01 | 02      == 0001 | 0010
     6 == 06 == 02 | 04      == 0010 | 0100
    11 == 0B == 01 | 02 | 08 == 0001 | 0010 | 1000

00000001 Att_ArmorPrc  = 1
00000002 Att_1stStrike  = 2

00000002 Att_1stStrike  = 2
00000004 Att_Poison  = 4

00000001 Att_ArmorPrc   = 1
00000002 Att_1stStrike  = 2
00000008 Att_LifeSteal  = 8

/*  172  20  Death  Death Knights  */
/* { Name,      mel,  rng,  rt,  amm,  hit,  def, res,  mov, cos,  upk,  rac, bl1,  bl2,   pict,  hp, sco, tra, fig, con, spec,    mvfl,    att1,    att2,     abl,    atfl, snd } */
{"Death Knights",     9,   0,  -1,   0,   3,   8,  10,   6,  600,   8,   20,  6,   0, 0x00,  8,  1,  0,  4,  0,   -4,  0x0008,  0x03D8,  0x0000,  0x0001,  0x000B,  0}, 

Armor Piercing Attack;
+2 Bonus to Hit;
First Strike;
Lifestealing Touch (-4 Save)

spec of -4 is for the save on lifestealing touch
So, ...
    +1, ..., +6
    -1, ..., -5
     0
    15


any spec != 0 where atfl == 0?


/*  182  16  Nature  Basilisk  */
/* { Name,      mel,  rng,  rt,  amm,  hit,  def, res,  mov, cos,  upk,  rac, bl1,  bl2,   pict,  hp, sco, tra, fig, con, spec,    mvfl,    att1,    att2,     abl,    atfl, snd } */
{"Basilisk",     15,   1,  103,   0,   1,   4,  7,   4,  325,   7,   16,  6,   0, 0x00,  30,  1,  0,  1,  0,   -1,  0x0000,  0x0000,  0x0000,  0x0001,  0x0000,  0}, 

Stoning Gaze  (-1 Save)
rng       1
rt      103
spec     -1
abl  0x0001

So, ...
    range 1 ranged attack of Stoning Gaze
    must know that spec is -1 Save



/*  159  18  Chaos  Chaos Spawn  */
/* { Name,      mel,  rng,  rt,  amm,  hit,  def, res,  mov, cos,  upk,  rac, bl1,  bl2,   pict,  hp, sco, tra, fig, con, spec,    mvfl,    att1,    att2,     abl,    atfl, snd } */
{"Chaos Spawn",     1,   4,  104,   0,   0,   6,  10,   2,  400,   12,   18,  6,   0, 0x00,  15,  1,  0,  1,  0,   4,  0x0008,  0x0200,  0x0020,  0x0001,  0x0004,  0}, 


Death Gaze (-4 Save);
Doom Gaze (Strength 4);
Stoning Gaze (-4 Save);
Strength 4 Poisonous Bite

rng          4
rt         104
spec         4
att1    0x0200
att2    0x0020
abl     0x0001
atfl    0x0004

FFFFFFFF SR_MultiGaze  = 104
00000200 Ab_Flying  = 200h
00000020 Ab_CauseFear  = 20h
00000001 Ab_Summoned  = 1
00000004 Att_Poison  = 4

So, ...
rt of 104 is Doom, Death, & Stone Gaze
rng of 4 is "Strength 4" & "(-4 Save)"
atfl of 0x0004 is "Poison Bite"
spec of 4 is "Strength 4"

Death Gaze and Stone Gaze had their own flags, Doom Gaze does not




att1  ATTRIB_1
00008000 Std_Unit  = 8000h

atfl  ABL_FLAGS
00000001 Ab_Summoned  = 1

unit type hero
normal  AKA  standard and race-specific units
Fantastic units  AKA  summoned creatures

normal units that have become undead creatures
nonhero mercenaries are always some normal (i.e., non-fantastic) type

Special Unit Abilities






uint16_t Name_Offset;
int8_t   Melee
int8_t   Ranged
int8_t   Ranged_Type
int8_t   Ammo
int8_t   To_Hit
int8_t   Defense
int8_t   Resist
int8_t   Move_Halves
int16_t  Cost
int8_t   Upkeep
int8_t   Race
int8_t   Bldng1_or_Portrait
int8_t   Bldng2_or_HeroType
uint16_t pict_seg;
int8_t   Hits
int8_t   Sight
int8_t   Transport
int8_t   Figures
int8_t   Construction
int8_t   Spec_Att_Attr
uint16_t Move_Flags
uint16_t Attribs_1
uint16_t Attribs_2
uint16_t Abilities
uint16_t Attack_Flags
int16_t  Sound
