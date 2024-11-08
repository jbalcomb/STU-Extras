// #include <inja/inja.hpp>
// NOTE  Inja uses the string_view feature of the C++17 STL.
#include <nlohmann/json.hpp>

// as of C++11
// std::to_string();  ...is safe, requires C++11 (to_string() is already included in #include <string>)

#include <STUlib/STUlib.hpp>

#include "STU-Extras.hpp"


enum e_Range_Attack_Type
{
    rt_NONE = -1,
    rt_Rock = 10
};

// 19: { -1, 10, 20, 21, 30, 31, 32, 33, 34, 35, 36, 37, 38, 100, 101, 102, 103, 104, 105 }
int8_t ranged_attack_type_xref[256] =
{
    /* -128 + 128 = 0 */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0,
    /* -1 + 128 = 127 */
    1, 
    0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    /* 10 + 128 = 138 */
    2,
    0,
    0, 0, 0, 0, 0, 0, 0, 0,
    /* 20 + 128 = 148 */
    3,
    /* 21 + 128 = 149 */
    4,
    0, 0, 0, 0, 0, 0, 0, 0,
    /* 30 + 128 = 158 */
    5,
    6,
    7,
    8,
    9,
    10,
    11,
    12,
    13,
    0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    /* 100 + 128 = 228 */
    14,
    15,
    16,
    17,
    18,
    19,
    0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0
};

char ranged_attack_type_name[20][22] =
{
    "INVALID",  /* Does Not Exist (DNE) */
    "NONE",  /* -1 */
    "Rock",  /* 10 */
    "Missile",  /* 20 */
    "Missile-Slingers",  /* 21 */
    "Magic-",                 /*       30  Warlock, Chaos Warrior, Storm Giant ("Lightning Attack") */
    "Magic-Fireballs",        /*       31  Warrior Mage, Magician, Witch, Golden One, Unknown, Magicians, Warlocks, Efreet (Chaos) */
    "Magic-SageIllusionist",  /*       32  Sage, Illusionist */
    "Magic-Death",            /*       33  Necromancer (?), Shadow Demons (Death), Demon Lord (Death) */
    "Magic-WindmageDjinn",    /*       34  Wind Mage (?), Djinn (Sorcery) */
    "Magic-PriestShaman",     /*       35  Healer, Priestess, Shaman, Priests */
    "Magic-DarkElves",        /*       36  Dark Elves  Spearmen, Swordsmen, Halberdies, Cavalry, Nightmares (NOT Priests, Warlocks (31), Nightblades) */
    "Magic-Sprites",          /* OON   37  Sprites (Nature) */
    "Magic-Druid",            /* OON   38  Druid (?) */
    "Thrown",                 /*      100  */
    "Fiery Breath",           /*      101  "Fiery Breath" */
    "Sky Drake",              /* OON  102  Sky Drake  Breath  Lightning Breath */
    "Stoning Gaze",           /*      103  Basilisk, Gorgons  "Stoning Gaze" */
    "Chaos Spawn",            /* OON  104  Chaos Spawn  Death Gaze, Doom Gaze, Stoning Gaze */
    "Night Stalker"           /* OON  105  Night Stalker  Death Gaze */
};

char ranged_attack_type_name_00[] = "NONE";

void Get_Ranged_Attack_Type_Name(int8_t ranged_attack_type, char ** ranged_attack_type_name)
{
    switch (ranged_attack_type)
    {
    case -1:
    {
        *ranged_attack_type_name = ranged_attack_type_name_00;
    } break;

    }
}


void proggen_unittype(void);




void Dedu_UTT_(void)
{
    int itr;
    int8_t ut[NUM_UNIT_TYPES] = { 0 };
    int8_t uth[NUM_HERO_UNITS] = { 0 };
    int8_t utn[NUM_NORMAL_UNITS] = { 0 };
    int8_t utf[NUM_FANTASTIC_UNITS] = { 0 };
    uint8_t* ut_set;
    uint64_t set_cnt;
    for (itr = 0; itr < NUM_UNIT_TYPES; itr++) { ut[itr] = unit_type_table[itr].melee; }
    sortset_uint8(ut, NUM_UNIT_TYPES, &ut_set, &set_cnt);
    for (itr = 0; itr < set_cnt; itr++) { printf("%u", ut_set[itr]); if (itr < (set_cnt - 1)) { printf(", "); } }
    printf("\n");
}

void Dedu_UTHU(void)
{
    int itr;
    int8_t ut[NUM_HERO_UNITS] = { 0 };
    uint8_t* ut_set;
    uint64_t set_cnt;
    for (itr = FST_HERO_UNIT; itr <= LST_HERO_UNIT; itr++) { ut[itr - FST_HERO_UNIT] = unit_type_table[itr].melee; }
    sortset_uint8(ut, NUM_HERO_UNITS, &ut_set, &set_cnt);
    for (itr = 0; itr < set_cnt; itr++) { printf("%u", ut_set[itr]); if (itr < (set_cnt - 1)) { printf(", "); } }
    printf("\n");
}

void Dedu_UTNU(void)
{
    int itr;
    int8_t ut[NUM_NORMAL_UNITS] = { 0 };
    uint8_t* ut_set;
    uint64_t set_cnt;
    for (itr = FST_NORMAL_UNIT; itr <= LST_NORMAL_UNIT; itr++) { ut[itr - FST_NORMAL_UNIT] = unit_type_table[itr].melee; }
    sortset_uint8(ut, NUM_NORMAL_UNITS, &ut_set, &set_cnt);
    for (itr = 0; itr < set_cnt; itr++) { printf("%u", ut_set[itr]); if (itr < (set_cnt - 1)) { printf(", "); } }
    printf("\n");
}

void Dedu_UTFU(void)
{
    int itr;
    int8_t ut[NUM_FANTASTIC_UNITS] = { 0 };
    uint8_t* ut_set;
    uint64_t set_cnt;
    for (itr = FST_FANTASTIC_UNIT; itr <= LST_FANTASTIC_UNIT; itr++) { ut[itr - FST_FANTASTIC_UNIT] = unit_type_table[itr].melee; }
    sortset_uint8(ut, NUM_FANTASTIC_UNITS, &ut_set, &set_cnt);
    for (itr = 0; itr < set_cnt; itr++) { printf("%u", ut_set[itr]); if (itr < (set_cnt - 1)) { printf(", "); } }
    printf("\n");
}

void Dedu_UTT_RT(void)
{
    int itr;
    int8_t ut[NUM_UNIT_TYPES] = { 0 };
    // uint8_t* ut_set;
    // uint64_t set_cnt;
    char* unittype_name;
    for (itr = 0; itr < NUM_UNIT_TYPES; itr++)
    {

        // if (unit_type_table[itr].ranged_type != -1)
        // if (unit_type_table[itr].ranged_type ==  21)  // OON  Slingers
        // if (unit_type_table[itr].ranged_type ==  30)  // Warlock, Chaos Warrior, Storm Giant
        // if (unit_type_table[itr].ranged_type ==  31)  // Warrior Mage, Magician, Witch, Golden One, Unknown, Magicians, Warlocks, Efreet
        // if (unit_type_table[itr].ranged_type ==  32)  // Sage, Illusionist
        // if (unit_type_table[itr].ranged_type ==  33)  // Necromancer, Shadow Demons, Demon Lord
        // if (unit_type_table[itr].ranged_type ==  34)  // Wind Mage, Djinn
        // if (unit_type_table[itr].ranged_type ==  35)  // Healer, Priestess, Shaman, Priests
        // if (unit_type_table[itr].ranged_type ==  36)  // Dark Elves
        // if (unit_type_table[itr].ranged_type ==  37)  // OON  Sprites
        // if (unit_type_table[itr].ranged_type ==  38)  // OON  Druid
        // if (unit_type_table[itr].ranged_type == 102)  // OON  "Sky Drake"
        // if (unit_type_table[itr].ranged_type == 103)  // Basilisk, Gorgons  "Stoning Gaze"
        // if (unit_type_table[itr].ranged_type == 104)  // OON  "Chaos Spawn"
        // if (unit_type_table[itr].ranged_type == 105)  // OON  "Night Stalker"
        if ((unit_type_table[itr].ranged_type != -1) && (unit_type_table[itr].ammo == 0))
        {

            Get_UnitType_Name(itr, &unittype_name);

            // printf("%20s  %3d  %3d\n", unittype_name, unit_type_table[itr].ranged_type, (unit_type_table[itr].ranged_type + 128));
            //printf("%20s  %3d  %3d  %3d  %22s\n",
            //    unittype_name,
            //    unit_type_table[itr].ranged_type,
            //    (unit_type_table[itr].ranged_type + 128),
            //    ranged_attack_type_xref[(unit_type_table[itr].ranged_type + 128)],
            //    ranged_attack_type_name[ranged_attack_type_xref[(unit_type_table[itr].ranged_type + 128)]]
            //);
            printf("%20s  %3d  %3d  %3d  %22s\n",
                unittype_name,
                unit_type_table[itr].ranged_type,
                unit_type_table[itr].ranged,
                unit_type_table[itr].ammo,
                ranged_attack_type_name[ranged_attack_type_xref[(unit_type_table[itr].ranged_type + 128)]]
            );

            // printf("%s  %d  %d  %d\n", unittype_name, unit_type_table[itr].ranged_type, unit_type_table[itr].ranged, unit_type_table[itr].ammo);

            //printf("%s  %d  %d  %d  %s  %d  %d\n",
            //    unittype_name,
            //    unit_type_table[itr].ranged_type,
            //    unit_type_table[itr].ranged,
            //    unit_type_table[itr].ammo,
            //    ranged_attack_type_name[ranged_attack_type_xref[(unit_type_table[itr].ranged_type + 128)]],
            //    (unit_type_table[itr].ranged_type + 128),
            //    ranged_attack_type_xref[(unit_type_table[itr].ranged_type + 128)]
            //);
        }
    }

    // sortset_uint8(ut, NUM_UNIT_TYPES, &ut_set, &set_cnt);
    // for (itr = 0; itr < set_cnt; itr++) { printf("%u", ut_set[itr]); if (itr < (set_cnt - 1)) { printf(", "); } }
    // printf("\n");
}


void Dedu_UTT_NU(void)
{
    int itr;
    int8_t ut[NUM_UNIT_TYPES] = { 0 };
    // uint8_t* ut_set;
    // uint64_t set_cnt;
    char* unittype_name;
    for (itr = 0; itr < NUM_UNIT_TYPES; itr++)
    {

        // if ( (unit_type_table[itr].abilities & 0x0001) != 0)  // attr_FantasticUnit  AKA Ab_Summoned
        // if (((unit_type_table[itr].abilities & 0x0001) != 0) && itr < FST_FANTASTIC_UNIT)  // none
        // if (((unit_type_table[itr].abilities & 0x0001) == 0) && itr >= FST_FANTASTIC_UNIT)  // none
        // if ((unit_type_table[itr].attribs_1 & 0x8000) != 0)  // attr_RaceSpecificUnit  AKA Std_Unit
        // if (((unit_type_table[itr].attribs_1 & 0x8000) != 0) && ((itr < FST_NORMAL_UNIT) || (itr > LST_NORMAL_UNIT)))  // none
        if (((unit_type_table[itr].attribs_1 & 0x8000) == 0) && ((itr >= FST_NORMAL_UNIT) && (itr <= LST_NORMAL_UNIT)))  // none
        {

            Get_UnitType_Name(itr, &unittype_name);

            printf("%20s  %3d  %3d  %3d  %22s\n",
                unittype_name,
                unit_type_table[itr].ranged_type,
                unit_type_table[itr].ranged,
                unit_type_table[itr].ammo,
                ranged_attack_type_name[ranged_attack_type_xref[(unit_type_table[itr].ranged_type + 128)]]
            );

        }
    }

    // sortset_uint8(ut, NUM_UNIT_TYPES, &ut_set, &set_cnt);
    // for (itr = 0; itr < set_cnt; itr++) { printf("%u", ut_set[itr]); if (itr < (set_cnt - 1)) { printf(", "); } }
    // printf("\n");
}


int main()
{
    Load_Game_Files("SAVETEST.GAM");



    //Dedu_UTT_();
    //Dedu_UTHU();
    //Dedu_UTNU();
    //Dedu_UTFU();

    // Dedu_UTT_RT();
    Dedu_UTT_NU();

    // proggen_unittype();



    Unload_Game_Files();
    return 0;
}


int print_unittype_json()
{
    Load_Game_Files("SAVETEST.GAM");

    std::string unittype_obj = "unittype_";
    std::string unittype_str;
    int itr;
    nlohmann::json unittype_json;
    bool flag = false;
    char* unittype_name = nullptr;

    //std::string unittype_03 = "unittype_03";
    //std::string unittype_03_name = "Dark Elf";
    //// unittype_json = inja::json::object();
    //unittype_json["unittype_01"]["name"] = "NONE";
    //unittype_json["unittype_01"]["name"] = "Barbarian";
    //unittype_json["unittype_02"]["name"] = "Beastmen";
    //// unittype_json["unittype_03"]["name"] = Get_UnitType_Name(2);
    //unittype_json[unittype_03]["name"] = unittype_03_name;
    //// unittype_json["unittype_00"] = "Barbarian";

    // for (itr = 0; itr < NUM_UNIT_TYPES; itr++)
    for (itr = 0; itr < 3; itr++)
    {
        unittype_str = unittype_obj + std::to_string(itr);
        Get_UnitType_Name(itr, &unittype_name);
        unittype_json[unittype_str]["name"] = std::string(unittype_name);
        unittype_json[unittype_str]["melee"] = unit_type_table[itr].melee;
        unittype_json[unittype_str]["ranged"] = unit_type_table[itr].ranged;
        unittype_json[unittype_str]["ranged_type"] = unit_type_table[itr].ranged_type;
        unittype_json[unittype_str]["ammo"] = unit_type_table[itr].ammo;
        unittype_json[unittype_str]["to_hit"] = unit_type_table[itr].to_hit;
        unittype_json[unittype_str]["defense"] = unit_type_table[itr].defense;
        unittype_json[unittype_str]["resistance"] = unit_type_table[itr].resistance;
        unittype_json[unittype_str]["move2_points"] = unit_type_table[itr].move2_points;
        unittype_json[unittype_str]["cost"] = unit_type_table[itr].cost;
        unittype_json[unittype_str]["upkeep"] = unit_type_table[itr].upkeep;
        unittype_json[unittype_str]["race_idx"] = unit_type_table[itr].race_idx;

        unittype_json[unittype_str]["required_building_1_idx"] = unit_type_table[itr].required_building_1_idx;
        unittype_json[unittype_str]["required_building_2_idx"] = unit_type_table[itr].required_building_2_idx;

        unittype_json[unittype_str]["hero_portrait"] = unit_type_table[itr].hero_portrait;
        unittype_json[unittype_str]["hero_type"] = unit_type_table[itr].hero_type;

        unittype_json[unittype_str]["pict_seg"] = unit_type_table[itr].pict_seg;
        unittype_json[unittype_str]["hits"] = unit_type_table[itr].hits;
        unittype_json[unittype_str]["scout_range"] = unit_type_table[itr].scout_range;
        unittype_json[unittype_str]["carry_capacity"] = unit_type_table[itr].carry_capacity;
        unittype_json[unittype_str]["figure_count"] = unit_type_table[itr].figure_count;
        unittype_json[unittype_str]["construction"] = unit_type_table[itr].construction;

        unittype_json[unittype_str]["spec_att_attr"] = unit_type_table[itr].spec_att_attr;

        unittype_json[unittype_str]["move_flags"] = unit_type_table[itr].move_flags;

        unittype_json[unittype_str]["attribs_1"] = unit_type_table[itr].attribs_1;

        unittype_json[unittype_str]["attribs_2"] = unit_type_table[itr].attribs_2;

        unittype_json[unittype_str]["abilities"] = unit_type_table[itr].abilities;

        unittype_json[unittype_str]["attack_flags"] = unit_type_table[itr].attack_flags;

        unittype_json[unittype_str]["sound"] = unit_type_table[itr].sound;

    }


    std::cout << std::setw(4) << unittype_json << std::endl;


    Unload_Game_Files();
    return 0;
}
