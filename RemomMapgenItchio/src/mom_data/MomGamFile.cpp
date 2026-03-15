// Binary .GAM file read/write for Master of Magic save game compatibility.
// Powered by Claude.

#include "mom_data/MomGamFile.hpp"
#include "scenario/Scenario.hpp"
#include <cstring>
#include <fstream>
#include <vector>

namespace mom {

// .GAM file layout offsets (from LOADSAVE.C in ReMoM).
// Powered by Claude.
namespace gam_offsets {
    constexpr int HEROES       = 0;        // 2520 bytes (35 heroes * 12 bytes * 6 players)
    constexpr int GAME_DATA    = 2520;     // 16 bytes
    constexpr int WIZARDS      = 2536;     // 7344 bytes (6 * 1224)
    constexpr int WORLD_MAPS   = 9880;     // 9600 bytes (2 * 4800)
    constexpr int UU_TBL       = 19480;    // 384 bytes (2 * 192)
    constexpr int LANDMASSES   = 19864;    // 4800 bytes
    constexpr int NODES        = 24664;    // 1440 bytes (30 * 48)
    constexpr int FORTRESSES   = 26104;    // 24 bytes (6 * 4)
    constexpr int TOWERS       = 26128;    // 24 bytes (6 * 4)
    constexpr int LAIRS        = 26152;    // 2448 bytes (102 * 24)
    constexpr int ITEMS        = 28600;    // 6900 bytes (138 * 50)
    constexpr int CITIES       = 35500;    // 11400 bytes (100 * 114)
    constexpr int UNITS        = 46900;    // 32288 bytes (1009 * 32)
    constexpr int SPECIALS     = 79188;    // 4800 bytes
    constexpr int EXPLORED     = 83988;    // 4800 bytes
    constexpr int MOVE_COSTS   = 88788;    // 28800 bytes
    constexpr int EVENTS       = 117588;   // 100 bytes
    constexpr int MAP_FLAGS    = 117688;   // 4800 bytes
    constexpr int GRAND_VIZIER = 122488;   // 2 bytes
    constexpr int PREMADE_ITEMS= 122490;   // 250 bytes
    constexpr int HERO_NAMES   = 122740;   // 560 bytes (35 * 16)
    // Total: 123300 bytes
}

// Convert a game terrain tile index (from TerrType.h) to editor BaseTerrain.
// Uses range checks matching the ReMoM terrain type layout.
// Powered by Claude.
static uint16_t gam_terrain_to_editor(uint16_t val) {
    if (val == 0x00 || val == 0x01) return TERRAIN_OCEAN;
    if (val == 0x12) return TERRAIN_LAKE;
    if (val >= 0x02 && val <= 0xA1) return TERRAIN_SHORE;
    if (val == 0xA2) return TERRAIN_GRASSLAND;
    if (val == 0xA3) return TERRAIN_FOREST;
    if (val == 0xA4) return TERRAIN_MOUNTAIN;
    if (val == 0xA5) return TERRAIN_DESERT;
    if (val == 0xA6) return TERRAIN_SWAMP;
    if (val == 0xA7) return TERRAIN_TUNDRA;
    if (val == 0xAB) return TERRAIN_HILL;
    if (val == 0xB3) return TERRAIN_VOLCANO;
    // River variants
    if (val >= 0xB9 && val <= 0xC8) return TERRAIN_RIVER;
    // Shore variants (additional ranges)
    if (val >= 0xC9 && val <= 0xE8) return TERRAIN_SHORE;
    // River variants (additional)
    if (val >= 0xE9 && val <= 0x102) return TERRAIN_RIVER;
    // Mountain variants
    if (val >= 0x103 && val <= 0x112) return TERRAIN_MOUNTAIN;
    // Hill variants
    if (val >= 0x113 && val <= 0x123) return TERRAIN_HILL;
    // Desert variants
    if (val >= 0x124 && val <= 0x1C3) return TERRAIN_DESERT;
    // Shore variants (additional)
    if (val >= 0x1C4 && val <= 0x258) return TERRAIN_SHORE;
    // River variants (additional)
    if (val >= 0x1D4 && val <= 0x1D8) return TERRAIN_RIVER;
    // Tundra variants
    if (val >= 0x25A && val <= 0x2F9) return TERRAIN_TUNDRA;
    // Nodes → treat as grassland in editor
    if (val >= 0xA8 && val <= 0xAA) return TERRAIN_GRASSLAND;
    return TERRAIN_GRASSLAND; // fallback
}

// Helper to write a block of zeros.
// Powered by Claude.
static void write_zeros(std::ofstream& f, int count) {
    std::vector<uint8_t> zeros(count, 0);
    f.write(reinterpret_cast<const char*>(zeros.data()), count);
}

bool load_gam_file(const std::string& path, Scenario& sc) {
    std::ifstream f(path, std::ios::binary);
    if (!f.is_open()) return false;

    // Check file size
    f.seekg(0, std::ios::end);
    auto size = f.tellg();
    if (size < SAVEGAM_RECORD_SIZE) return false;
    f.seekg(0, std::ios::beg);

    std::vector<uint8_t> buf(SAVEGAM_RECORD_SIZE);
    f.read(reinterpret_cast<char*>(buf.data()), SAVEGAM_RECORD_SIZE);
    if (!f.good()) return false;

    auto at = [&](int offset) -> const uint8_t* { return buf.data() + offset; };

    // Game settings
    std::memcpy(&sc.game_data, at(gam_offsets::GAME_DATA), sizeof(GameData));

    // Wizards
    std::memcpy(sc.wizards.data(), at(gam_offsets::WIZARDS), NUM_PLAYERS * sizeof(Wizard));

    // World maps (terrain) — convert game tile indices to editor BaseTerrain (0-11).
    // Powered by Claude.
    {
        std::memcpy(sc.world.terrain, at(gam_offsets::WORLD_MAPS), NUM_PLANES * WORLD_SIZE * 2);
        for (int p = 0; p < NUM_PLANES; ++p) {
            for (int i = 0; i < WORLD_SIZE; ++i) {
                sc.world.terrain[p][i] = gam_terrain_to_editor(sc.world.terrain[p][i]);
            }
        }
    }

    // Landmasses
    std::memcpy(sc.world.landmasses, at(gam_offsets::LANDMASSES), NUM_PLANES * WORLD_SIZE);

    // Nodes
    std::memcpy(sc.nodes.data(), at(gam_offsets::NODES), NUM_NODES * sizeof(Node));

    // Fortresses
    std::memcpy(sc.fortresses.data(), at(gam_offsets::FORTRESSES), NUM_FORTRESSES * sizeof(Fortress));

    // Towers
    std::memcpy(sc.towers.data(), at(gam_offsets::TOWERS), NUM_TOWERS * sizeof(Tower));

    // Lairs
    std::memcpy(sc.lairs.data(), at(gam_offsets::LAIRS), NUM_LAIRS * sizeof(Lair));

    // Items
    std::memcpy(sc.items.data(), at(gam_offsets::ITEMS), NUM_ITEMS * sizeof(Item));

    // Cities
    std::memcpy(sc.cities.data(), at(gam_offsets::CITIES), NUM_CITIES * sizeof(City));

    // Units
    std::memcpy(sc.units.data(), at(gam_offsets::UNITS), NUM_UNITS * sizeof(Unit));

    // Terrain specials
    std::memcpy(sc.world.specials, at(gam_offsets::SPECIALS), NUM_PLANES * WORLD_SIZE);

    // Explored
    std::memcpy(sc.world.explored, at(gam_offsets::EXPLORED), NUM_PLANES * WORLD_SIZE);

    // Map flags
    std::memcpy(sc.world.flags, at(gam_offsets::MAP_FLAGS), NUM_PLANES * WORLD_SIZE);

    return true;
}

// Build the 123,300-byte .GAM buffer from a scenario (shared by file and buffer save).
// Powered by Claude.
static std::vector<uint8_t> build_gam_buffer(const Scenario& sc) {
    std::vector<uint8_t> buf(SAVEGAM_RECORD_SIZE, 0);

    auto put = [&](int offset, const void* data, int size) {
        std::memcpy(buf.data() + offset, data, size);
    };

    // Game settings
    put(gam_offsets::GAME_DATA, &sc.game_data, sizeof(GameData));

    // Wizards
    put(gam_offsets::WIZARDS, sc.wizards.data(), NUM_PLAYERS * sizeof(Wizard));

    // World maps (terrain) — convert editor BaseTerrain values (0-11) to game
    // tile indices from TerrType.h in ReMoM.
    // Powered by Claude.
    {
        static constexpr uint16_t EDITOR_TO_GAM[BASE_TERRAIN_COUNT] = {
            0x00,  // TERRAIN_OCEAN     → tOcean
            0x02,  // TERRAIN_SHORE     → tShore (first shore variant)
            0xA2,  // TERRAIN_GRASSLAND → tGrasslands
            0xA3,  // TERRAIN_FOREST    → tForest
            0xA4,  // TERRAIN_MOUNTAIN  → tMountain
            0xA5,  // TERRAIN_DESERT    → tDesert
            0xA6,  // TERRAIN_SWAMP     → tSwamp
            0xA7,  // TERRAIN_TUNDRA    → tTundra
            0xAB,  // TERRAIN_HILL      → tHills
            0xB9,  // TERRAIN_RIVER     → tRiver (first river variant)
            0xB3,  // TERRAIN_VOLCANO   → tVolcano
            0x12,  // TERRAIN_LAKE      → tLake
        };
        uint16_t gam_terrain[NUM_PLANES][WORLD_SIZE];
        for (int p = 0; p < NUM_PLANES; ++p) {
            for (int i = 0; i < WORLD_SIZE; ++i) {
                uint16_t val = sc.world.terrain[p][i];
                if (val < BASE_TERRAIN_COUNT) {
                    gam_terrain[p][i] = EDITOR_TO_GAM[val];
                } else {
                    gam_terrain[p][i] = val;
                }
            }
        }
        put(gam_offsets::WORLD_MAPS, gam_terrain, NUM_PLANES * WORLD_SIZE * 2);
    }

    // Landmasses
    put(gam_offsets::LANDMASSES, sc.world.landmasses, NUM_PLANES * WORLD_SIZE);

    // Nodes
    put(gam_offsets::NODES, sc.nodes.data(), NUM_NODES * sizeof(Node));

    // Fortresses
    put(gam_offsets::FORTRESSES, sc.fortresses.data(), NUM_FORTRESSES * sizeof(Fortress));

    // Towers
    put(gam_offsets::TOWERS, sc.towers.data(), NUM_TOWERS * sizeof(Tower));

    // Lairs
    put(gam_offsets::LAIRS, sc.lairs.data(), NUM_LAIRS * sizeof(Lair));

    // Items
    put(gam_offsets::ITEMS, sc.items.data(), NUM_ITEMS * sizeof(Item));

    // Cities
    put(gam_offsets::CITIES, sc.cities.data(), NUM_CITIES * sizeof(City));

    // Units
    put(gam_offsets::UNITS, sc.units.data(), NUM_UNITS * sizeof(Unit));

    // Terrain specials
    put(gam_offsets::SPECIALS, sc.world.specials, NUM_PLANES * WORLD_SIZE);

    // Explored (set all explored by default for scenarios)
    {
        uint8_t explored[NUM_PLANES][WORLD_SIZE];
        std::memcpy(explored, sc.world.explored, sizeof(explored));
        put(gam_offsets::EXPLORED, explored, NUM_PLANES * WORLD_SIZE);
    }

    // Map flags
    put(gam_offsets::MAP_FLAGS, sc.world.flags, NUM_PLANES * WORLD_SIZE);

    return buf;
}

bool save_gam_file(const std::string& path, const Scenario& sc) {
    std::ofstream f(path, std::ios::binary);
    if (!f.is_open()) return false;

    auto buf = build_gam_buffer(sc);
    f.write(reinterpret_cast<const char*>(buf.data()), SAVEGAM_RECORD_SIZE);
    return f.good();
}

// Serialize a scenario to a .GAM byte buffer for use with platform file dialogs.
// Powered by Claude.
std::vector<uint8_t> serialize_gam(const Scenario& sc) {
    return build_gam_buffer(sc);
}

// Deserialize a .GAM byte buffer into a scenario.
// Returns true on success, false if the buffer is too small.
// Powered by Claude.
bool deserialize_gam(const std::vector<uint8_t>& data, Scenario& sc) {
    if (static_cast<int>(data.size()) < SAVEGAM_RECORD_SIZE) return false;

    auto at = [&](int offset) -> const uint8_t* { return data.data() + offset; };

    // Game settings
    std::memcpy(&sc.game_data, at(gam_offsets::GAME_DATA), sizeof(GameData));

    // Wizards
    std::memcpy(sc.wizards.data(), at(gam_offsets::WIZARDS), NUM_PLAYERS * sizeof(Wizard));

    // World maps (terrain) — convert game tile indices to editor BaseTerrain (0-11).
    // Powered by Claude.
    {
        std::memcpy(sc.world.terrain, at(gam_offsets::WORLD_MAPS), NUM_PLANES * WORLD_SIZE * 2);
        for (int p = 0; p < NUM_PLANES; ++p) {
            for (int i = 0; i < WORLD_SIZE; ++i) {
                sc.world.terrain[p][i] = gam_terrain_to_editor(sc.world.terrain[p][i]);
            }
        }
    }

    // Landmasses
    std::memcpy(sc.world.landmasses, at(gam_offsets::LANDMASSES), NUM_PLANES * WORLD_SIZE);

    // Nodes
    std::memcpy(sc.nodes.data(), at(gam_offsets::NODES), NUM_NODES * sizeof(Node));

    // Fortresses
    std::memcpy(sc.fortresses.data(), at(gam_offsets::FORTRESSES), NUM_FORTRESSES * sizeof(Fortress));

    // Towers
    std::memcpy(sc.towers.data(), at(gam_offsets::TOWERS), NUM_TOWERS * sizeof(Tower));

    // Lairs
    std::memcpy(sc.lairs.data(), at(gam_offsets::LAIRS), NUM_LAIRS * sizeof(Lair));

    // Items
    std::memcpy(sc.items.data(), at(gam_offsets::ITEMS), NUM_ITEMS * sizeof(Item));

    // Cities
    std::memcpy(sc.cities.data(), at(gam_offsets::CITIES), NUM_CITIES * sizeof(City));

    // Units
    std::memcpy(sc.units.data(), at(gam_offsets::UNITS), NUM_UNITS * sizeof(Unit));

    // Terrain specials
    std::memcpy(sc.world.specials, at(gam_offsets::SPECIALS), NUM_PLANES * WORLD_SIZE);

    // Explored
    std::memcpy(sc.world.explored, at(gam_offsets::EXPLORED), NUM_PLANES * WORLD_SIZE);

    // Map flags
    std::memcpy(sc.world.flags, at(gam_offsets::MAP_FLAGS), NUM_PLANES * WORLD_SIZE);

    return true;
}

} // namespace mom
