// Tests for .GAM file read/write and struct binary compatibility.
// Powered by Claude.

#include <gtest/gtest.h>
#include "mom_data/MomConstants.hpp"
#include "mom_data/MomEntities.hpp"
#include "mom_data/MomWorld.hpp"
#include "mom_data/MomGamFile.hpp"
#include "scenario/Scenario.hpp"
#include <cstring>
#include <fstream>
#include <filesystem>

using namespace mom;

// -- Struct size tests (binary compatibility with ReMoM) --

TEST(StructSizes, CityIs114Bytes) {
    EXPECT_EQ(sizeof(City), 114);
}

TEST(StructSizes, UnitIs32Bytes) {
    EXPECT_EQ(sizeof(Unit), 32);
}

TEST(StructSizes, WizardIs1224Bytes) {
    EXPECT_EQ(sizeof(Wizard), 1224);
}

TEST(StructSizes, NodeIs48Bytes) {
    EXPECT_EQ(sizeof(Node), 48);
}

TEST(StructSizes, LairIs24Bytes) {
    EXPECT_EQ(sizeof(Lair), 24);
}

TEST(StructSizes, TowerIs4Bytes) {
    EXPECT_EQ(sizeof(Tower), 4);
}

TEST(StructSizes, FortressIs4Bytes) {
    EXPECT_EQ(sizeof(Fortress), 4);
}

TEST(StructSizes, GameDataIs16Bytes) {
    EXPECT_EQ(sizeof(GameData), 16);
}

TEST(StructSizes, ItemIs50Bytes) {
    EXPECT_EQ(sizeof(Item), 50);
}

// -- MomWorld tests --

TEST(MomWorld, GetSetTerrain) {
    MomWorld world;
    world.clear();

    world.set_terrain(10, 20, 0, TERRAIN_FOREST);
    EXPECT_EQ(world.get_terrain(10, 20, 0), TERRAIN_FOREST);
    EXPECT_EQ(world.get_terrain(10, 20, 1), 0); // other plane untouched
}

TEST(MomWorld, GetSetSpecial) {
    MomWorld world;
    world.clear();

    world.set_special(5, 15, 1, TS_GOLD);
    EXPECT_EQ(world.get_special(5, 15, 1), TS_GOLD);
    EXPECT_EQ(world.get_special(5, 15, 0), 0);
}

TEST(MomWorld, GetSetFlags) {
    MomWorld world;
    world.clear();

    world.set_flags(0, 0, 0, MSF_ROAD);
    EXPECT_EQ(world.get_flags(0, 0, 0), MSF_ROAD);
}

TEST(MomWorld, BoundsChecking) {
    MomWorld world;
    world.clear();

    // Out-of-bounds reads return 0
    EXPECT_EQ(world.get_terrain(-1, 0, 0), 0);
    EXPECT_EQ(world.get_terrain(60, 0, 0), 0);
    EXPECT_EQ(world.get_terrain(0, 40, 0), 0);
    EXPECT_EQ(world.get_terrain(0, 0, 2), 0);

    // Out-of-bounds writes are silently ignored
    world.set_terrain(-1, 0, 0, 999);
    world.set_terrain(60, 0, 0, 999);
}

TEST(MomWorld, ClearResetsAll) {
    MomWorld world;
    world.set_terrain(5, 5, 0, TERRAIN_MOUNTAIN);
    world.set_special(5, 5, 0, TS_IRON);
    world.set_flags(5, 5, 0, MSF_ROAD);

    world.clear();
    EXPECT_EQ(world.get_terrain(5, 5, 0), 0);
    EXPECT_EQ(world.get_special(5, 5, 0), 0);
    EXPECT_EQ(world.get_flags(5, 5, 0), 0);
}

TEST(MomWorld, SetAllExplored) {
    MomWorld world;
    world.clear();
    world.set_all_explored();

    for (int p = 0; p < NUM_PLANES; ++p) {
        for (int i = 0; i < WORLD_SIZE; ++i) {
            EXPECT_EQ(world.explored[p][i], 0x0F);
        }
    }
}

// -- Entity active state tests --

TEST(Entity, CityActiveState) {
    City c{};
    EXPECT_FALSE(c.is_active()); // owner_idx defaults to -1

    c.owner_idx = 0;
    EXPECT_TRUE(c.is_active());
}

TEST(Entity, UnitActiveState) {
    Unit u{};
    EXPECT_FALSE(u.is_active());

    u.owner_idx = 2;
    EXPECT_TRUE(u.is_active());
}

TEST(Entity, UnitIsHero) {
    Unit u{};
    u.type = 10; // Within hero range (0-34)
    EXPECT_TRUE(u.is_hero());

    u.type = 35; // First non-hero
    EXPECT_FALSE(u.is_hero());
}

TEST(Entity, WizardActiveState) {
    Wizard w{};
    EXPECT_FALSE(w.is_active()); // name is empty

    std::strncpy(w.name, "Merlin", LEN_WIZARD_NAME);
    EXPECT_TRUE(w.is_active());
}

// -- Scenario tests --

TEST(Scenario, ClearResetsEverything) {
    Scenario sc;
    sc.meta.title = "Test";
    sc.cities[0].owner_idx = 0;
    sc.units[0].owner_idx = 0;

    sc.clear();
    EXPECT_EQ(sc.meta.title, "Untitled Scenario");
    EXPECT_FALSE(sc.cities[0].is_active());
    EXPECT_FALSE(sc.units[0].is_active());
    EXPECT_EQ(sc.type, ScenarioType::SANDBOX);
}

TEST(Scenario, CountActiveCities) {
    Scenario sc;
    sc.clear();
    EXPECT_EQ(sc.count_active_cities(), 0);

    sc.cities[0].owner_idx = 0;
    sc.cities[5].owner_idx = 1;
    EXPECT_EQ(sc.count_active_cities(), 2);
}

TEST(Scenario, CountActiveUnits) {
    Scenario sc;
    sc.clear();
    EXPECT_EQ(sc.count_active_units(), 0);

    sc.units[0].owner_idx = 0;
    sc.units[1].owner_idx = 0;
    sc.units[2].owner_idx = 1;
    EXPECT_EQ(sc.count_active_units(), 3);
}

// -- .GAM round-trip tests --

class GamFileTest : public ::testing::Test {
protected:
    std::string tmp_path;

    void SetUp() override {
        tmp_path = (std::filesystem::temp_directory_path() / "test_roundtrip.GAM").string();
    }

    void TearDown() override {
        std::filesystem::remove(tmp_path);
    }
};

TEST_F(GamFileTest, RoundTripEmptyScenario) {
    Scenario sc1;
    sc1.clear();

    ASSERT_TRUE(save_gam_file(tmp_path, sc1));

    // Verify file size
    std::ifstream f(tmp_path, std::ios::binary | std::ios::ate);
    ASSERT_TRUE(f.is_open());
    EXPECT_EQ(f.tellg(), SAVEGAM_RECORD_SIZE);
    f.close();

    Scenario sc2;
    ASSERT_TRUE(load_gam_file(tmp_path, sc2));

    // Game settings should match
    EXPECT_EQ(sc2.game_data.Difficulty, sc1.game_data.Difficulty);
    EXPECT_EQ(sc2.game_data.Magic, sc1.game_data.Magic);
    EXPECT_EQ(sc2.game_data.Land_Size, sc1.game_data.Land_Size);
}

TEST_F(GamFileTest, RoundTripWithTerrain) {
    Scenario sc1;
    sc1.clear();

    // Set some terrain
    sc1.world.set_terrain(10, 20, 0, TERRAIN_MOUNTAIN);
    sc1.world.set_terrain(30, 15, 1, TERRAIN_DESERT);
    sc1.world.set_special(10, 20, 0, TS_GOLD);
    sc1.world.set_flags(10, 20, 0, MSF_ROAD);

    ASSERT_TRUE(save_gam_file(tmp_path, sc1));

    Scenario sc2;
    ASSERT_TRUE(load_gam_file(tmp_path, sc2));

    EXPECT_EQ(sc2.world.get_terrain(10, 20, 0), TERRAIN_MOUNTAIN);
    EXPECT_EQ(sc2.world.get_terrain(30, 15, 1), TERRAIN_DESERT);
    EXPECT_EQ(sc2.world.get_special(10, 20, 0), TS_GOLD);
    EXPECT_EQ(sc2.world.get_flags(10, 20, 0), MSF_ROAD);
}

TEST_F(GamFileTest, RoundTripWithEntities) {
    Scenario sc1;
    sc1.clear();

    // Set up a wizard
    std::strncpy(sc1.wizards[0].name, "Merlin", LEN_WIZARD_NAME);
    sc1.wizards[0].wizard_id = 0;
    sc1.wizards[0].gold_reserve = 500;
    sc1.wizards[0].mana_reserve = 200;
    sc1.wizards[0].capital_race = RACE_HIGH_ELF;

    // Set up a city
    std::strncpy(sc1.cities[0].name, "Arcadia", LEN_CITY_NAME);
    sc1.cities[0].race = RACE_HIGH_ELF;
    sc1.cities[0].wx = 15;
    sc1.cities[0].wy = 25;
    sc1.cities[0].wp = PLANE_ARCANUS;
    sc1.cities[0].owner_idx = 0;
    sc1.cities[0].population = 5;

    // Set up a unit
    sc1.units[0].type = 36; // Spearmen
    sc1.units[0].wx = 15;
    sc1.units[0].wy = 25;
    sc1.units[0].wp = PLANE_ARCANUS;
    sc1.units[0].owner_idx = 0;
    sc1.units[0].Level = 2;

    // Set up a node
    sc1.nodes[0].wx = 30;
    sc1.nodes[0].wy = 10;
    sc1.nodes[0].wp = PLANE_ARCANUS;
    sc1.nodes[0].type = NODE_NATURE;
    sc1.nodes[0].power = 8;

    sc1.game_data.Total_Cities = 1;
    sc1.game_data.Total_Units = 1;

    ASSERT_TRUE(save_gam_file(tmp_path, sc1));

    Scenario sc2;
    ASSERT_TRUE(load_gam_file(tmp_path, sc2));

    // Wizard
    EXPECT_STREQ(sc2.wizards[0].name, "Merlin");
    EXPECT_EQ(sc2.wizards[0].gold_reserve, 500);
    EXPECT_EQ(sc2.wizards[0].mana_reserve, 200);
    EXPECT_EQ(sc2.wizards[0].capital_race, RACE_HIGH_ELF);

    // City
    EXPECT_STREQ(sc2.cities[0].name, "Arcadia");
    EXPECT_EQ(sc2.cities[0].race, RACE_HIGH_ELF);
    EXPECT_EQ(sc2.cities[0].wx, 15);
    EXPECT_EQ(sc2.cities[0].wy, 25);
    EXPECT_EQ(sc2.cities[0].owner_idx, 0);
    EXPECT_EQ(sc2.cities[0].population, 5);

    // Unit
    EXPECT_EQ(sc2.units[0].type, 36);
    EXPECT_EQ(sc2.units[0].wx, 15);
    EXPECT_EQ(sc2.units[0].owner_idx, 0);
    EXPECT_EQ(sc2.units[0].Level, 2);

    // Node
    EXPECT_EQ(sc2.nodes[0].wx, 30);
    EXPECT_EQ(sc2.nodes[0].wy, 10);
    EXPECT_EQ(sc2.nodes[0].type, NODE_NATURE);
    EXPECT_EQ(sc2.nodes[0].power, 8);

    // Game data
    EXPECT_EQ(sc2.game_data.Total_Cities, 1);
    EXPECT_EQ(sc2.game_data.Total_Units, 1);
}

TEST_F(GamFileTest, LoadNonexistentFileFails) {
    Scenario sc;
    EXPECT_FALSE(load_gam_file("nonexistent.GAM", sc));
}

TEST_F(GamFileTest, LoadTruncatedFileFails) {
    // Create a file that's too small
    std::ofstream f(tmp_path, std::ios::binary);
    f.write("short", 5);
    f.close();

    Scenario sc;
    EXPECT_FALSE(load_gam_file(tmp_path, sc));
}
