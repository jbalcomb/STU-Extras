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

// ---- Terrain export mapping tests ----
// Verify that serialize_gam writes correct game tile indices (from TerrType.h)
// and that deserialize_gam converts them back to editor BaseTerrain values.

TEST(TerrainMapping, ExportWritesCorrectGameValues) {
    // Simple terrain types (not shore/river) export to fixed game values.
    // Powered by Claude.
    struct { BaseTerrain editor; uint16_t game; } simple_cases[] = {
        {TERRAIN_OCEAN,     0x00},
        {TERRAIN_GRASSLAND, 0xA2},
        {TERRAIN_FOREST,    0xA3},
        {TERRAIN_MOUNTAIN,  0xA4},
        {TERRAIN_DESERT,    0x134},  // isolated → _1Desert
        {TERRAIN_SWAMP,     0xA6},
        {TERRAIN_TUNDRA,    0x26A},  // isolated → _1Tundra
        {TERRAIN_HILL,      0xAB},
        {TERRAIN_VOLCANO,   0xB3},
        {TERRAIN_LAKE,      0x12},
    };

    for (auto& tc : simple_cases) {
        Scenario sc;
        sc.clear();
        sc.world.set_terrain(5, 5, 0, tc.editor);

        auto buf = serialize_gam(sc);
        ASSERT_EQ(buf.size(), static_cast<size_t>(SAVEGAM_RECORD_SIZE));

        int offset = 9880 + (5 * WORLD_WIDTH + 5) * 2;
        uint16_t gam_val = buf[offset] | (buf[offset + 1] << 8);
        EXPECT_EQ(gam_val, tc.game)
            << "Editor terrain " << static_cast<int>(tc.editor)
            << " should export as 0x" << std::hex << tc.game
            << " but got 0x" << gam_val;
    }
}

TEST(TerrainMapping, ExportShoreUsesAdjacency) {
    // Shore square with land only at SE (diagonal) → bitmask 00001000 = 0x08
    // → game value 0x02 (_Shore00001000).  Diagonal-only IS valid in MoM.
    // Powered by Claude.
    Scenario sc;
    sc.clear();
    sc.world.set_terrain(5, 5, 0, TERRAIN_SHORE);
    sc.world.set_terrain(6, 6, 0, TERRAIN_GRASSLAND); // SE neighbor (diagonal)

    auto buf = serialize_gam(sc);
    int offset = 9880 + (5 * WORLD_WIDTH + 5) * 2;
    uint16_t gam_val = buf[offset] | (buf[offset + 1] << 8);
    EXPECT_EQ(gam_val, 0x02);  // SE only = bitmask 00001000 → game 0x02
}

TEST(TerrainMapping, ExportShoreDiagonalForcing) {
    // Shore square with land at E and S → diagonal forcing sets SE too.
    // Bitmask becomes 00011100 (E+SE+S) = 0x1C → game value 0x26.
    // MoM requires SE when both E and S are land.
    // Powered by Claude.
    Scenario sc;
    sc.clear();
    sc.world.set_terrain(5, 5, 0, TERRAIN_SHORE);
    sc.world.set_terrain(6, 5, 0, TERRAIN_GRASSLAND); // E (cardinal)
    sc.world.set_terrain(5, 6, 0, TERRAIN_GRASSLAND); // S (cardinal)
    // SE is ocean, but diagonal forcing sets it because E+S are both land.

    auto buf = serialize_gam(sc);
    int offset = 9880 + (5 * WORLD_WIDTH + 5) * 2;
    uint16_t gam_val = buf[offset] | (buf[offset + 1] << 8);
    EXPECT_EQ(gam_val, 0x26);  // E+SE(forced)+S = bitmask 00011100 → game 0x26
}

TEST(TerrainMapping, ExportShoreAllLandBecomesLake) {
    // Shore square surrounded entirely by land is fully enclosed water —
    // must export as tt_Lake (0x12), not ocean (0x00).
    // Powered by Claude.
    Scenario sc;
    sc.clear();
    // Fill all neighbors of (5,5) with grassland.
    for (int dy = -1; dy <= 1; ++dy)
        for (int dx = -1; dx <= 1; ++dx)
            sc.world.set_terrain(5 + dx, 5 + dy, 0, TERRAIN_GRASSLAND);
    sc.world.set_terrain(5, 5, 0, TERRAIN_SHORE);

    auto buf = serialize_gam(sc);
    int offset = 9880 + (5 * WORLD_WIDTH + 5) * 2;
    uint16_t gam_val = buf[offset] | (buf[offset + 1] << 8);
    EXPECT_EQ(gam_val, 0x12)
        << "Shore with all land neighbors must become tt_Lake (0x12)";
}

TEST(TerrainMapping, ExportRiverUsesConnections) {
    // River square with river to the south → game value 0xB9 (River0010 = S).
    // Powered by Claude.
    Scenario sc;
    sc.clear();
    sc.world.set_terrain(5, 5, 0, TERRAIN_RIVER);
    sc.world.set_terrain(5, 6, 0, TERRAIN_RIVER);  // S neighbor is river
    // Fill other neighbors with land so they don't connect.
    sc.world.set_terrain(5, 4, 0, TERRAIN_GRASSLAND); // N
    sc.world.set_terrain(6, 5, 0, TERRAIN_GRASSLAND); // E
    sc.world.set_terrain(4, 5, 0, TERRAIN_GRASSLAND); // W

    auto buf = serialize_gam(sc);
    int offset = 9880 + (5 * WORLD_WIDTH + 5) * 2;
    uint16_t gam_val = buf[offset] | (buf[offset + 1] << 8);
    EXPECT_EQ(gam_val, 0xB9);  // River connecting S = NESW 0010

    // Check the south river square connects N only → 0xBB (River1000 = N).
    // Must also surround it with land so E/S/W don't connect to ocean.
    // Powered by Claude.
    sc.world.set_terrain(6, 6, 0, TERRAIN_GRASSLAND); // E of south river
    sc.world.set_terrain(4, 6, 0, TERRAIN_GRASSLAND); // W of south river
    sc.world.set_terrain(5, 7, 0, TERRAIN_GRASSLAND); // S of south river
    buf = serialize_gam(sc);
    int offset2 = 9880 + (6 * WORLD_WIDTH + 5) * 2;
    uint16_t gam_val2 = buf[offset2] | (buf[offset2 + 1] << 8);
    EXPECT_EQ(gam_val2, 0xBB);  // River connecting N = NESW 1000
}

TEST(TerrainMapping, RoundTripAllBaseTerrains) {
    // Every BaseTerrain value must survive an export→import round-trip.
    // Shore and river squares need proper neighbors for adjacency export.
    // Powered by Claude.
    Scenario sc1;
    sc1.clear();

    // Place each terrain type at a known row-5 location, spaced apart.
    for (int t = 0; t < BASE_TERRAIN_COUNT; ++t) {
        sc1.world.set_terrain(t * 3 + 1, 5, 0, static_cast<uint16_t>(t));
    }

    // Give the shore square (TERRAIN_SHORE=1) a land neighbor so it exports
    // as a valid shore variant that round-trips back to TERRAIN_SHORE.
    // Powered by Claude.
    sc1.world.set_terrain(1 * 3 + 2, 6, 0, TERRAIN_GRASSLAND); // SE of shore

    // Give the river square (TERRAIN_RIVER=9) a river neighbor so it exports
    // with a connection pattern that round-trips back to TERRAIN_RIVER.
    // Surround both river squares with land so ocean doesn't create extra connections.
    // Powered by Claude.
    int rx = 9 * 3 + 1;
    sc1.world.set_terrain(rx, 6, 0, TERRAIN_RIVER);      // S of river
    sc1.world.set_terrain(rx - 1, 5, 0, TERRAIN_GRASSLAND); // W of river
    sc1.world.set_terrain(rx + 1, 5, 0, TERRAIN_GRASSLAND); // E of river
    sc1.world.set_terrain(rx, 4, 0, TERRAIN_GRASSLAND);     // N of river
    sc1.world.set_terrain(rx - 1, 6, 0, TERRAIN_GRASSLAND); // W of S river
    sc1.world.set_terrain(rx + 1, 6, 0, TERRAIN_GRASSLAND); // E of S river
    sc1.world.set_terrain(rx, 7, 0, TERRAIN_GRASSLAND);     // S of S river

    auto buf = serialize_gam(sc1);

    Scenario sc2;
    ASSERT_TRUE(deserialize_gam(buf, sc2));

    for (int t = 0; t < BASE_TERRAIN_COUNT; ++t) {
        EXPECT_EQ(sc2.world.get_terrain(t * 3 + 1, 5, 0), static_cast<uint16_t>(t))
            << "BaseTerrain " << t << " did not round-trip correctly";
    }
}

TEST(TerrainMapping, ImportGameVariantsToBaseType) {
    // Game terrain variants (autotile indices) must import as the correct
    // editor BaseTerrain. Verifies the gam_terrain_to_editor conversion.
    // Powered by Claude.
    struct { uint16_t game; BaseTerrain editor; } cases[] = {
        // Base values
        {0x00, TERRAIN_OCEAN},
        {0x01, TERRAIN_OCEAN},
        {0x02, TERRAIN_SHORE},
        {0x12, TERRAIN_LAKE},
        {0xA2, TERRAIN_GRASSLAND},
        {0xA3, TERRAIN_FOREST},
        {0xA4, TERRAIN_MOUNTAIN},
        {0xA5, TERRAIN_DESERT},
        {0xA6, TERRAIN_SWAMP},
        {0xA7, TERRAIN_TUNDRA},
        {0xAB, TERRAIN_HILL},
        {0xB3, TERRAIN_VOLCANO},
        {0xB9, TERRAIN_RIVER},
        // Autotile variants
        {0x50, TERRAIN_SHORE},     // shore variant mid-range
        {0xA1, TERRAIN_SHORE},     // shore variant end
        {0xC0, TERRAIN_RIVER},     // river variant
        {0xD0, TERRAIN_SHORE},     // shore variant (0xC9-0xE8 range)
        {0xF0, TERRAIN_RIVER},     // river variant (0xE9-0x102 range)
        {0x110, TERRAIN_MOUNTAIN}, // mountain variant
        {0x120, TERRAIN_HILL},     // hill variant
        {0x150, TERRAIN_DESERT},   // desert variant
        {0x200, TERRAIN_SHORE},    // shore variant (0x1C4-0x258 range)
        {0x280, TERRAIN_TUNDRA},   // tundra variant
    };

    for (auto& tc : cases) {
        // Build a .GAM buffer with the game value at tile (0,0) plane 0.
        Scenario sc_write;
        sc_write.clear();
        auto buf = serialize_gam(sc_write);

        // Manually poke the game terrain value into the buffer.
        int offset = 9880; // gam_offsets::WORLD_MAPS
        buf[offset]     = tc.game & 0xFF;
        buf[offset + 1] = (tc.game >> 8) & 0xFF;

        Scenario sc_read;
        ASSERT_TRUE(deserialize_gam(buf, sc_read));
        EXPECT_EQ(sc_read.world.get_terrain(0, 0, 0),
                  static_cast<uint16_t>(tc.editor))
            << "Game value 0x" << std::hex << tc.game
            << " should import as BaseTerrain " << std::dec
            << static_cast<int>(tc.editor);
    }
}

// -- Export smoothing validation tests --

// Helper: fill entire plane with a base terrain type.
// Powered by Claude.
static void fill_plane(Scenario& sc, int plane, uint16_t terrain) {
    for (int y = 0; y < WORLD_HEIGHT; ++y)
        for (int x = 0; x < WORLD_WIDTH; ++x)
            sc.world.set_terrain(x, y, plane, terrain);
}

TEST(ExportSmoothing, NoOrphanedOceanInsideLand) {
    // A single ocean square surrounded by forest on all 8 sides must
    // not remain as ocean after smoothing — it should become shore.
    // Powered by Claude.
    Scenario original;
    original.clear();
    fill_plane(original, 0, TERRAIN_FOREST);
    original.world.set_terrain(5, 5, 0, TERRAIN_OCEAN);

    Scenario export_sc = original;
    int changed = smooth_terrain_for_export(export_sc, original);

    EXPECT_NE(export_sc.world.get_terrain(5, 5, 0), TERRAIN_OCEAN)
        << "Orphaned ocean square inside landmass must not survive smoothing";
    EXPECT_EQ(export_sc.world.get_terrain(5, 5, 0), TERRAIN_SHORE);
    EXPECT_GT(changed, 0);
}

TEST(ExportSmoothing, NoOrphanedOceanIn2x2Pocket) {
    // A 2x2 ocean pocket inside land: all 4 squares should become shore.
    // This tests the iterative peeling in Pass 2.
    // Powered by Claude.
    Scenario original;
    original.clear();
    fill_plane(original, 0, TERRAIN_GRASSLAND);
    original.world.set_terrain(10, 10, 0, TERRAIN_OCEAN);
    original.world.set_terrain(11, 10, 0, TERRAIN_OCEAN);
    original.world.set_terrain(10, 11, 0, TERRAIN_OCEAN);
    original.world.set_terrain(11, 11, 0, TERRAIN_OCEAN);

    Scenario export_sc = original;
    smooth_terrain_for_export(export_sc, original);

    for (int dy = 0; dy < 2; ++dy) {
        for (int dx = 0; dx < 2; ++dx) {
            EXPECT_NE(export_sc.world.get_terrain(10 + dx, 10 + dy, 0), TERRAIN_OCEAN)
                << "Ocean at (" << 10 + dx << "," << 10 + dy
                << ") in 2x2 pocket must not survive smoothing";
        }
    }
}

TEST(ExportSmoothing, NoOrphanedOceanIn3x3Pocket) {
    // A 3x3 ocean pocket: the center square has all ocean neighbors
    // initially, but after peeling it should also become shore.
    // Powered by Claude.
    Scenario original;
    original.clear();
    fill_plane(original, 0, TERRAIN_FOREST);
    for (int dy = 0; dy < 3; ++dy)
        for (int dx = 0; dx < 3; ++dx)
            original.world.set_terrain(20 + dx, 20 + dy, 0, TERRAIN_OCEAN);

    Scenario export_sc = original;
    smooth_terrain_for_export(export_sc, original);

    for (int dy = 0; dy < 3; ++dy) {
        for (int dx = 0; dx < 3; ++dx) {
            EXPECT_NE(export_sc.world.get_terrain(20 + dx, 20 + dy, 0), TERRAIN_OCEAN)
                << "Ocean at (" << 20 + dx << "," << 20 + dy
                << ") in 3x3 pocket must not survive smoothing";
        }
    }
}

TEST(ExportSmoothing, ShoresAddedAtLandOceanBoundary) {
    // Ocean squares adjacent to land become shore; land squares stay land.
    // Powered by Claude.
    Scenario original;
    original.clear();
    fill_plane(original, 0, TERRAIN_OCEAN);
    // Place a single land square at (30, 20).
    original.world.set_terrain(30, 20, 0, TERRAIN_GRASSLAND);

    Scenario export_sc = original;
    smooth_terrain_for_export(export_sc, original);

    // The land square stays grassland.
    EXPECT_EQ(export_sc.world.get_terrain(30, 20, 0), TERRAIN_GRASSLAND);

    // All 8 neighbors should now be shore.
    static constexpr int dx8[] = {-1,  0,  1, 1, 1, 0, -1, -1};
    static constexpr int dy8[] = {-1, -1, -1, 0, 1, 1,  1,  0};
    for (int d = 0; d < 8; ++d) {
        int nx = 30 + dx8[d];
        int ny = 20 + dy8[d];
        EXPECT_EQ(export_sc.world.get_terrain(nx, ny, 0), TERRAIN_SHORE)
            << "Ocean neighbor at (" << nx << "," << ny
            << ") of land square should become shore";
    }
}

TEST(ExportSmoothing, IsolatedRiverBecomesGrassland) {
    // A river square with no adjacent water connection becomes grassland.
    // Powered by Claude.
    Scenario original;
    original.clear();
    fill_plane(original, 0, TERRAIN_FOREST);
    original.world.set_terrain(15, 15, 0, TERRAIN_RIVER);

    Scenario export_sc = original;
    smooth_terrain_for_export(export_sc, original);

    EXPECT_EQ(export_sc.world.get_terrain(15, 15, 0), TERRAIN_GRASSLAND)
        << "Isolated river with no water neighbors should become grassland";
}

TEST(ExportSmoothing, ConnectedRiverSurvives) {
    // A river connected to shore should survive smoothing.
    // Powered by Claude.
    Scenario original;
    original.clear();
    fill_plane(original, 0, TERRAIN_FOREST);
    original.world.set_terrain(25, 15, 0, TERRAIN_RIVER);
    original.world.set_terrain(25, 16, 0, TERRAIN_SHORE);

    Scenario export_sc = original;
    smooth_terrain_for_export(export_sc, original);

    EXPECT_EQ(export_sc.world.get_terrain(25, 15, 0), TERRAIN_RIVER)
        << "River connected to shore should survive smoothing";
}

TEST(ExportSmoothing, BothPlanesSmoothed) {
    // Smoothing applies to both Arcanus (0) and Myrror (1).
    // Powered by Claude.
    Scenario original;
    original.clear();
    fill_plane(original, 0, TERRAIN_FOREST);
    fill_plane(original, 1, TERRAIN_FOREST);
    original.world.set_terrain(5, 5, 0, TERRAIN_OCEAN);
    original.world.set_terrain(5, 5, 1, TERRAIN_OCEAN);

    Scenario export_sc = original;
    smooth_terrain_for_export(export_sc, original);

    EXPECT_NE(export_sc.world.get_terrain(5, 5, 0), TERRAIN_OCEAN)
        << "Orphaned ocean on Arcanus must be fixed";
    EXPECT_NE(export_sc.world.get_terrain(5, 5, 1), TERRAIN_OCEAN)
        << "Orphaned ocean on Myrror must be fixed";
}
