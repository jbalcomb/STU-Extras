// Tests for .mom_scenario JSON serialization round-trip.
// Powered by Claude.

#include <gtest/gtest.h>
#include "scenario/Scenario.hpp"
#include "scenario/ScenarioIO.hpp"
#include "mom_data/MomConstants.hpp"
#include <cstring>
#include <filesystem>

using namespace mom;

class ScenarioIOTest : public ::testing::Test {
protected:
    std::string tmp_path;

    void SetUp() override {
        tmp_path = (std::filesystem::temp_directory_path() / "test_scenario.mom_scenario").string();
    }

    void TearDown() override {
        std::filesystem::remove(tmp_path);
    }
};

TEST_F(ScenarioIOTest, RoundTripEmptyScenario) {
    Scenario sc1;
    sc1.clear();
    sc1.meta.title = "Test Scenario";
    sc1.meta.author = "Tester";
    sc1.meta.description = "A test scenario";
    sc1.meta.tags = {"test", "sandbox"};

    ASSERT_TRUE(save_scenario(tmp_path, sc1));

    Scenario sc2;
    ASSERT_TRUE(load_scenario(tmp_path, sc2));

    EXPECT_EQ(sc2.meta.title, "Test Scenario");
    EXPECT_EQ(sc2.meta.author, "Tester");
    EXPECT_EQ(sc2.meta.description, "A test scenario");
    EXPECT_EQ(sc2.meta.tags.size(), 2u);
    EXPECT_EQ(sc2.meta.tags[0], "test");
    EXPECT_EQ(sc2.meta.tags[1], "sandbox");
    EXPECT_EQ(sc2.type, ScenarioType::SANDBOX);
}

TEST_F(ScenarioIOTest, RoundTripScenarioType) {
    Scenario sc1;
    sc1.clear();
    sc1.type = ScenarioType::MISSION;

    ASSERT_TRUE(save_scenario(tmp_path, sc1));

    Scenario sc2;
    ASSERT_TRUE(load_scenario(tmp_path, sc2));
    EXPECT_EQ(sc2.type, ScenarioType::MISSION);
}

TEST_F(ScenarioIOTest, RoundTripGameSettings) {
    Scenario sc1;
    sc1.clear();
    sc1.game_data.Difficulty = DIFF_HARD;
    sc1.game_data.Magic = 2;
    sc1.game_data.Land_Size = 0;
    sc1.game_data.Total_Wizards = 4;

    ASSERT_TRUE(save_scenario(tmp_path, sc1));

    Scenario sc2;
    ASSERT_TRUE(load_scenario(tmp_path, sc2));

    EXPECT_EQ(sc2.game_data.Difficulty, DIFF_HARD);
    EXPECT_EQ(sc2.game_data.Magic, 2);
    EXPECT_EQ(sc2.game_data.Land_Size, 0);
    EXPECT_EQ(sc2.game_data.Total_Wizards, 4);
}

TEST_F(ScenarioIOTest, RoundTripTerrain) {
    Scenario sc1;
    sc1.clear();

    // Set terrain across both planes
    sc1.world.set_terrain(0, 0, 0, TERRAIN_OCEAN);
    sc1.world.set_terrain(30, 20, 0, TERRAIN_FOREST);
    sc1.world.set_terrain(59, 39, 1, TERRAIN_MOUNTAIN);
    sc1.world.set_special(30, 20, 0, TS_MITHRIL);
    sc1.world.set_flags(30, 20, 0, MSF_ROAD);

    ASSERT_TRUE(save_scenario(tmp_path, sc1));

    Scenario sc2;
    ASSERT_TRUE(load_scenario(tmp_path, sc2));

    EXPECT_EQ(sc2.world.get_terrain(0, 0, 0), TERRAIN_OCEAN);
    EXPECT_EQ(sc2.world.get_terrain(30, 20, 0), TERRAIN_FOREST);
    EXPECT_EQ(sc2.world.get_terrain(59, 39, 1), TERRAIN_MOUNTAIN);
    EXPECT_EQ(sc2.world.get_special(30, 20, 0), TS_MITHRIL);
    EXPECT_EQ(sc2.world.get_flags(30, 20, 0), MSF_ROAD);
}

TEST_F(ScenarioIOTest, RoundTripWizards) {
    Scenario sc1;
    sc1.clear();

    auto& w = sc1.wizards[0];
    w.wizard_id = 0;
    std::strncpy(w.name, "Rjak", LEN_WIZARD_NAME);
    w.banner_id = 2;
    w.capital_race = RACE_DARK_ELF;
    w.gold_reserve = 1000;
    w.mana_reserve = 500;
    w.fame = 42;
    w.tax_rate = 2;
    w.spellranks[0] = 3; // Nature
    w.spellranks[4] = 5; // Death
    w.myrran = 1;
    w.archmage = 1;

    ASSERT_TRUE(save_scenario(tmp_path, sc1));

    Scenario sc2;
    ASSERT_TRUE(load_scenario(tmp_path, sc2));

    auto& w2 = sc2.wizards[0];
    EXPECT_STREQ(w2.name, "Rjak");
    EXPECT_EQ(w2.banner_id, 2);
    EXPECT_EQ(w2.capital_race, RACE_DARK_ELF);
    EXPECT_EQ(w2.gold_reserve, 1000);
    EXPECT_EQ(w2.mana_reserve, 500);
    EXPECT_EQ(w2.fame, 42);
    EXPECT_EQ(w2.spellranks[0], 3);
    EXPECT_EQ(w2.spellranks[4], 5);
    EXPECT_EQ(w2.myrran, 1);
    EXPECT_EQ(w2.archmage, 1);
}

TEST_F(ScenarioIOTest, RoundTripCities) {
    Scenario sc1;
    sc1.clear();

    auto& c = sc1.cities[0];
    std::strncpy(c.name, "Mirion", LEN_CITY_NAME);
    c.race = RACE_HIGH_ELF;
    c.wx = 25;
    c.wy = 18;
    c.wp = PLANE_ARCANUS;
    c.owner_idx = 0;
    c.population = 8;
    c.size = 2;
    c.bldg_status[0] = 1; // First building built
    c.bldg_status[5] = 1; // Another building

    ASSERT_TRUE(save_scenario(tmp_path, sc1));

    Scenario sc2;
    ASSERT_TRUE(load_scenario(tmp_path, sc2));

    auto& c2 = sc2.cities[0];
    EXPECT_STREQ(c2.name, "Mirion");
    EXPECT_EQ(c2.race, RACE_HIGH_ELF);
    EXPECT_EQ(c2.wx, 25);
    EXPECT_EQ(c2.wy, 18);
    EXPECT_EQ(c2.wp, PLANE_ARCANUS);
    EXPECT_EQ(c2.owner_idx, 0);
    EXPECT_EQ(c2.population, 8);
    EXPECT_EQ(c2.bldg_status[0], 1);
    EXPECT_EQ(c2.bldg_status[5], 1);
    EXPECT_EQ(c2.bldg_status[1], 0); // Unset building
}

TEST_F(ScenarioIOTest, RoundTripUnits) {
    Scenario sc1;
    sc1.clear();

    auto& u = sc1.units[0];
    u.type = 36;
    u.wx = 10;
    u.wy = 20;
    u.wp = PLANE_MYRROR;
    u.owner_idx = 1;
    u.Level = 3;
    u.XP = 150;
    u.Hero_Slot = -1;
    u.enchantments = 0x0004;

    ASSERT_TRUE(save_scenario(tmp_path, sc1));

    Scenario sc2;
    ASSERT_TRUE(load_scenario(tmp_path, sc2));

    auto& u2 = sc2.units[0];
    EXPECT_EQ(u2.type, 36);
    EXPECT_EQ(u2.wx, 10);
    EXPECT_EQ(u2.wy, 20);
    EXPECT_EQ(u2.wp, PLANE_MYRROR);
    EXPECT_EQ(u2.owner_idx, 1);
    EXPECT_EQ(u2.Level, 3);
    EXPECT_EQ(u2.XP, 150);
    EXPECT_EQ(u2.enchantments, 0x0004u);
}

TEST_F(ScenarioIOTest, RoundTripNodes) {
    Scenario sc1;
    sc1.clear();

    sc1.nodes[0].wx = 40;
    sc1.nodes[0].wy = 30;
    sc1.nodes[0].wp = 0;
    sc1.nodes[0].type = NODE_CHAOS;
    sc1.nodes[0].power = 12;
    sc1.nodes[0].owner_idx = 2;

    ASSERT_TRUE(save_scenario(tmp_path, sc1));

    Scenario sc2;
    ASSERT_TRUE(load_scenario(tmp_path, sc2));

    EXPECT_EQ(sc2.nodes[0].wx, 40);
    EXPECT_EQ(sc2.nodes[0].wy, 30);
    EXPECT_EQ(sc2.nodes[0].type, NODE_CHAOS);
    EXPECT_EQ(sc2.nodes[0].power, 12);
    EXPECT_EQ(sc2.nodes[0].owner_idx, 2);
}

TEST_F(ScenarioIOTest, RoundTripLairs) {
    Scenario sc1;
    sc1.clear();

    auto& l = sc1.lairs[0];
    l.wx = 15;
    l.wy = 22;
    l.wp = 0;
    l.type = 4; // Lair type
    l.Intact = 1;
    l.guard1_unit_type = 50;
    l.guard1_count = 3;
    l.Loot_Gold = 200;
    l.Loot_Mana = 50;

    ASSERT_TRUE(save_scenario(tmp_path, sc1));

    Scenario sc2;
    ASSERT_TRUE(load_scenario(tmp_path, sc2));

    auto& l2 = sc2.lairs[0];
    EXPECT_EQ(l2.wx, 15);
    EXPECT_EQ(l2.wy, 22);
    EXPECT_EQ(l2.type, 4);
    EXPECT_EQ(l2.Intact, 1);
    EXPECT_EQ(l2.guard1_unit_type, 50);
    EXPECT_EQ(l2.guard1_count, 3);
    EXPECT_EQ(l2.Loot_Gold, 200);
    EXPECT_EQ(l2.Loot_Mana, 50);
}

TEST_F(ScenarioIOTest, ActiveCountsUpdatedOnLoad) {
    Scenario sc1;
    sc1.clear();
    sc1.cities[0].owner_idx = 0;
    std::strncpy(sc1.cities[0].name, "City1", LEN_CITY_NAME);
    sc1.cities[1].owner_idx = 1;
    std::strncpy(sc1.cities[1].name, "City2", LEN_CITY_NAME);
    sc1.units[0].owner_idx = 0;
    sc1.units[0].type = 36;

    ASSERT_TRUE(save_scenario(tmp_path, sc1));

    Scenario sc2;
    ASSERT_TRUE(load_scenario(tmp_path, sc2));

    EXPECT_EQ(sc2.game_data.Total_Cities, 2);
    EXPECT_EQ(sc2.game_data.Total_Units, 1);
}

TEST_F(ScenarioIOTest, OnlyActiveCitiesSerialized) {
    Scenario sc1;
    sc1.clear();

    // Active city at index 5
    sc1.cities[5].owner_idx = 0;
    std::strncpy(sc1.cities[5].name, "TestCity", LEN_CITY_NAME);
    sc1.cities[5].wx = 30;

    ASSERT_TRUE(save_scenario(tmp_path, sc1));

    Scenario sc2;
    ASSERT_TRUE(load_scenario(tmp_path, sc2));

    // Should be loaded at index 0 (first slot)
    EXPECT_STREQ(sc2.cities[0].name, "TestCity");
    EXPECT_EQ(sc2.cities[0].wx, 30);
    EXPECT_EQ(sc2.count_active_cities(), 1);
}

TEST_F(ScenarioIOTest, LoadNonexistentFileFails) {
    Scenario sc;
    EXPECT_FALSE(load_scenario("nonexistent.mom_scenario", sc));
}

TEST_F(ScenarioIOTest, LoadInvalidJsonFails) {
    // Write invalid JSON
    std::ofstream f(tmp_path);
    f << "this is not json {{{";
    f.close();

    Scenario sc;
    EXPECT_FALSE(load_scenario(tmp_path, sc));
}
