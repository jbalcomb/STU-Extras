#pragma once
// Game entity structs matching ReMoM's binary layout for .GAM compatibility.
// All structs use #pragma pack(2) to match the original 8086 alignment.
// Powered by Claude.

#include "mom_data/MomConstants.hpp"
#include <cstdint>
#include <cstring>

namespace mom {

#pragma pack(push, 2)

// City data -- 114 bytes (0x72). Matches s_CITY in MOM_Data.H.
// Powered by Claude.
struct City {
    char     name[LEN_CITY_NAME]{};  // 0x00
    int8_t   race{};                 // 0x0E
    int8_t   wx{};                   // 0x0F
    int8_t   wy{};                   // 0x10
    int8_t   wp{};                   // 0x11
    int8_t   owner_idx{-1};          // 0x12
    int8_t   size{};                 // 0x13
    int8_t   population{};           // 0x14
    int8_t   farmer_count{};         // 0x15
    int8_t   did_sell_building{};    // 0x16
    int8_t   pad_17{};               // 0x17
    int16_t  Pop_10s{};              // 0x18
    uint8_t  PlayerBits{};           // 0x1A
    int8_t   pad_1B{};               // 0x1B
    int16_t  construction{};         // 0x1C
    int8_t   bldg_cnt{};             // 0x1E
    int8_t   bldg_status[NUM_BUILDINGS]{}; // 0x1F  (36 bytes)
    uint8_t  enchantments[NUM_CITY_ENCHANTMENTS]{}; // 0x43 (25 bytes)
    int8_t   production_units{};     // 0x5C
    int8_t   pad_5D{};               // 0x5D
    int16_t  Prod_Accu{};            // 0x5E
    uint8_t  gold_units{};           // 0x60
    int8_t   building_maintenance{}; // 0x61
    int8_t   mana_units{};           // 0x62
    int8_t   research_units{};       // 0x63
    int8_t   food_units{};           // 0x64
    int8_t   road_connections[13]{}; // 0x65
    // total: 0x72 = 114 bytes

    // Check if this city slot is active.
    // Powered by Claude.
    bool is_active() const { return owner_idx >= 0; }
};

// Unit data -- 32 bytes (0x20). Matches s_UNIT in MOM_Data.H.
// Powered by Claude.
struct Unit {
    int8_t   wx{};                   // 0x00
    int8_t   wy{};                   // 0x01
    int8_t   wp{};                   // 0x02
    int8_t   owner_idx{-1};          // 0x03
    int8_t   moves2_max{};           // 0x04
    uint8_t  type{};                 // 0x05
    int8_t   Hero_Slot{-1};          // 0x06
    int8_t   Finished{};             // 0x07
    int8_t   moves2{};               // 0x08
    int8_t   dst_wx{};               // 0x09
    int8_t   dst_wy{};               // 0x0A
    int8_t   Status{};               // 0x0B
    int8_t   Level{};                // 0x0C
    int8_t   pad_0D{};               // 0x0D
    int16_t  XP{};                   // 0x0E
    int8_t   Move_Failed{};          // 0x10
    int8_t   Damage{};               // 0x11
    int8_t   Draw_Priority{};        // 0x12
    int8_t   pad_13{};               // 0x13
    int16_t  in_tower{};             // 0x14
    int8_t   Sight_Range{};          // 0x16
    int8_t   mutations{};            // 0x17
    uint32_t enchantments{};         // 0x18
    int8_t   Rd_Constr_Left{};       // 0x1C
    int8_t   Rd_From_X{};            // 0x1D
    int8_t   Rd_From_Y{};            // 0x1E
    int8_t   pad_1F{};               // 0x1F
    // total: 0x20 = 32 bytes

    // Check if this unit slot is active.
    // Powered by Claude.
    bool is_active() const { return owner_idx >= 0; }
    bool is_hero() const { return type < NUM_HERO_TYPES; }
};

// Owned hero slot -- 28 bytes. Part of Wizard struct.
// Powered by Claude.
struct OwnedHero {
    uint8_t  data[28]{};
};

// Wizard diplomacy data -- 306 bytes (0x132).
// Powered by Claude.
struct WizDipl {
    uint8_t  data[306]{};
};

// Astrology data -- 6 bytes.
// Powered by Claude.
struct AstrData {
    uint8_t  data[6]{};
};

// Wizard data -- 1224 bytes (0x4C8). Matches s_WIZARD in MOM_Data.H.
// Powered by Claude.
struct Wizard {
    uint8_t  wizard_id{};               // 0x0000
    char     name[LEN_WIZARD_NAME]{};   // 0x0001
    uint8_t  capital_race{};            // 0x0015
    uint8_t  banner_id{};               // 0x0016
    uint8_t  pad_17{};                  // 0x0017
    uint16_t Personality{};             // 0x0018
    uint16_t Objective{};               // 0x001A
    uint8_t  Unknown_01C[6]{};          // 0x001C
    uint16_t SoM_RC{};                  // 0x0022
    uint16_t fame{};                    // 0x0024
    uint16_t Power_Base{};              // 0x0026
    uint16_t volcanoes{};               // 0x0028
    uint8_t  research_ratio{};          // 0x002A
    uint8_t  mana_ratio{};              // 0x002B
    uint8_t  skill_ratio{};             // 0x002C
    uint8_t  Volcano_Power{};           // 0x002D
    int16_t  summon_wx{};               // 0x002E
    int16_t  summon_wy{};               // 0x0030
    int16_t  summon_wp{};               // 0x0032
    uint16_t research_spells[8]{};      // 0x0034
    uint8_t  Unknown_044[4]{};          // 0x0044
    uint16_t average_unit_cost{};       // 0x0048
    uint16_t Unknown_04A{};             // 0x004A
    uint16_t Cmbt_Skill_Left{};         // 0x004C
    uint16_t casting_cost_remaining{};  // 0x004E
    uint16_t casting_cost_original{};   // 0x0050
    uint16_t casting_spell_idx{};       // 0x0052
    uint16_t Skill_Left{};              // 0x0054
    uint16_t Nominal_Skill{};           // 0x0056
    uint16_t tax_rate{};                // 0x0058
    int16_t  spellranks[5]{};           // 0x005A  (Nature, Sorcery, Chaos, Life, Death)
    // Retorts 0x0064-0x0075
    int8_t   alchemy{};
    int8_t   warlord{};
    int8_t   chaos_mastery{};
    int8_t   nature_mastery{};
    int8_t   sorcery_mastery{};
    int8_t   infernal_power{};
    int8_t   divine_power{};
    int8_t   sage_master{};
    int8_t   channeler{};
    int8_t   myrran{};
    int8_t   archmage{};
    int8_t   mana_focusing{};
    int8_t   node_mastery{};
    int8_t   famous{};
    int8_t   runemaster{};
    int8_t   conjurer{};
    int8_t   charismatic{};
    int8_t   artificer{};
    OwnedHero Heroes[NUM_HEROES]{};     // 0x0076  (6 * 28 = 168 bytes)
    uint16_t Unknown_11E{};             // 0x011E
    int16_t  Vault_Items[4]{};          // 0x0120
    WizDipl  Dipl{};                    // 0x0128  (306 bytes)
    uint16_t research_cost_remaining{}; // 0x025A
    uint16_t mana_reserve{};            // 0x025C
    int32_t  spell_casting_skill{};     // 0x025E
    int16_t  researching_spell_idx{};   // 0x0262
    uint8_t  spells_list[NUM_SPELLS]{}; // 0x0264  (240 bytes)
    uint16_t Defeated_Wizards{};        // 0x0354
    uint16_t gold_reserve{};            // 0x0356
    uint16_t Unknown_358{};             // 0x0358
    AstrData Astr{};                    // 0x035A
    uint16_t Pop_div_10k{};             // 0x0360
    uint8_t  Historian[288]{};          // 0x0362
    uint8_t  Globals[NUM_OVERLAND_ENCHANTMENTS]{}; // 0x0482
    uint16_t Magic_Strategy{};          // 0x049A
    uint16_t Unknown_49C{};             // 0x049C
    uint16_t Hostility[NUM_PLAYERS]{};  // 0x049E
    uint16_t reevaluate_hostility_countdown{};       // 0x04AA
    uint16_t reevaluate_magic_strategy_countdown{};  // 0x04AC
    uint16_t reevaluate_magic_power_countdown{};     // 0x04AE
    uint8_t  peace_duration[NUM_PLAYERS]{};          // 0x04B0
    uint8_t  field_4B6{};               // 0x04B6
    uint8_t  field_4B7{};               // 0x04B7
    uint16_t field_4B8{};               // 0x04B8
    uint16_t OVL_TargetWiz{};           // 0x04BA
    uint8_t  field_4BC{};               // 0x04BC
    uint8_t  field_4BD{};               // 0x04BD
    uint8_t  Unknown_4BE[6]{};          // 0x04BE
    uint16_t Prim_Realm{};              // 0x04C4
    uint16_t Sec_Realm{};               // 0x04C6
    // total: 0x04C8 = 1224 bytes

    bool is_active() const { return name[0] != '\0'; }
};

// Node -- 48 bytes (0x30). Matches s_NODE in MOM_Data.H.
// Powered by Claude.
struct Node {
    int8_t  wx{};                    // 0x00
    int8_t  wy{};                    // 0x01
    int8_t  wp{};                    // 0x02
    int8_t  owner_idx{-1};           // 0x03
    int8_t  power{};                 // 0x04
    int8_t  Aura_Xs[20]{};           // 0x05
    int8_t  Aura_Ys[20]{};           // 0x19
    int8_t  type{};                  // 0x2D  (0=Sorcery, 1=Nature, 2=Chaos)
    int8_t  Meld_Flags{};            // 0x2E
    int8_t  pad_2F{};                // 0x2F
    // total: 0x30 = 48 bytes
};

// Lair -- 24 bytes (0x18). Matches s_LAIR in MOM_Data.H.
// Powered by Claude.
struct Lair {
    int8_t   wx{};                   // 0x00
    int8_t   wy{};                   // 0x01
    int8_t   wp{};                   // 0x02
    int8_t   Intact{};               // 0x03
    int8_t   type{};                 // 0x04
    uint8_t  guard1_unit_type{};     // 0x05
    uint8_t  guard1_count{};         // 0x06
    uint8_t  guard2_unit_type{};     // 0x07
    uint8_t  guard2_count{};         // 0x08
    int8_t   pad_09{};               // 0x09
    int16_t  Loot_Gold{};            // 0x0A
    int16_t  Loot_Mana{};            // 0x0C
    int8_t   Spell_n_Special{};      // 0x0E
    uint8_t  Misc_Flags{};           // 0x0F
    int8_t   Item_Count{};           // 0x10
    int8_t   pad_11{};               // 0x11
    int16_t  Item_Values[3]{};       // 0x12
    // total: 0x18 = 24 bytes
};

// Tower -- 4 bytes. Matches s_TOWER in MOM_Data.H.
// Powered by Claude.
struct Tower {
    int8_t wx{};
    int8_t wy{};
    int8_t owner_idx{-1};
    int8_t pad{};
};

// Fortress -- 4 bytes. Matches s_FORTRESS in MOM_Data.H.
// Powered by Claude.
struct Fortress {
    int8_t wx{};
    int8_t wy{};
    int8_t wp{};
    int8_t active{};
};

// Game settings -- 16 bytes (0x10). Matches s_GAME_DATA in MOM_Data.H.
// Powered by Claude.
struct GameData {
    uint16_t Current_Unit{};
    uint16_t Turn{1};
    uint16_t Total_Units{};
    uint16_t Total_Cities{};
    uint16_t Difficulty{DIFF_NORMAL};
    uint16_t Magic{1};
    uint16_t Land_Size{1};
    uint16_t Total_Wizards{2};
};

// Item -- 50 bytes. Matches s_ITEM in MOM_Data.H.
// Powered by Claude.
struct Item {
    uint8_t data[50]{};
};

#pragma pack(pop)

// Static assertions to verify struct sizes match ReMoM.
// Powered by Claude.
static_assert(sizeof(City)     == 114, "City must be 114 bytes");
static_assert(sizeof(Unit)     ==  32, "Unit must be 32 bytes");
static_assert(sizeof(Wizard)   == 1224, "Wizard must be 1224 bytes");
static_assert(sizeof(Node)     ==  48, "Node must be 48 bytes");
static_assert(sizeof(Lair)     ==  24, "Lair must be 24 bytes");
static_assert(sizeof(Tower)    ==   4, "Tower must be 4 bytes");
static_assert(sizeof(Fortress) ==   4, "Fortress must be 4 bytes");
static_assert(sizeof(GameData) ==  16, "GameData must be 16 bytes");
static_assert(sizeof(Item)     ==  50, "Item must be 50 bytes");

} // namespace mom
