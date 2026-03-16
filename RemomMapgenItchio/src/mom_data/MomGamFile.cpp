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

// Convert a game terrain square index (from TerrType.h) to editor BaseTerrain.
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
    // Directional lake variants (_1LakeRiv_W/N/E/S)
    if (val >= 0xC5 && val <= 0xC8) return TERRAIN_LAKE;
    // River variants
    if (val >= 0xB9 && val <= 0xC4) return TERRAIN_RIVER;
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

    // World maps (terrain) — convert game square indices to editor BaseTerrain (0-11).
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
    // square indices from TerrType.h in ReMoM.  Shore and river squares use
    // adjacency-based lookup to pick the correct variant.
    // Powered by Claude.
    {
        // Simple terrain types that don't need adjacency lookup.
        // Powered by Claude.
        static constexpr uint16_t EDITOR_TO_GAM[BASE_TERRAIN_COUNT] = {
            0x00,  // TERRAIN_OCEAN     → tOcean
            0x02,  // TERRAIN_SHORE     → fallback (overridden by adjacency)
            0xA2,  // TERRAIN_GRASSLAND → tGrasslands
            0xA3,  // TERRAIN_FOREST    → tForest
            0xA4,  // TERRAIN_MOUNTAIN  → tMountain
            0xA5,  // TERRAIN_DESERT    → tDesert
            0xA6,  // TERRAIN_SWAMP     → tSwamp
            0xA7,  // TERRAIN_TUNDRA    → tTundra
            0xAB,  // TERRAIN_HILL      → tHills
            0xB9,  // TERRAIN_RIVER     → fallback (overridden by adjacency)
            0xB3,  // TERRAIN_VOLCANO   → tVolcano
            0x12,  // TERRAIN_LAKE      → tLake
        };

        // Shore adjacency lookup: 8-bit bitmask → game square value.
        // Bitmask encodes which neighbors are land: NW=bit7 .. W=bit0.
        // Built from ReMoM TerrType.h Shore1 enum (0x02-0xA1).
        // Powered by Claude.
        static constexpr uint16_t SHORE_FROM_ADJ[256] = {
            0,      0x13,   0x06,   0x0C,   0x0B,   0,      0x05,   0x2E,  // 0x00-0x07
            0x02,   0x82,   0x07,   0x84,   0x03,   0,      0x04,   0x2F,  // 0x08-0x0F
            0x11,   0x4E,   0x6D,   0x4A,   0,      0,      0,      0,     // 0x10-0x17
            0x0A,   0x4F,   0x6C,   0x4B,   0x26,   0,      0x28,   0x52,  // 0x18-0x1F
            0x1B,   0x7A,   0x08,   0x7C,   0x79,   0,      0x76,   0x9E,  // 0x20-0x27
            0x0F,   0x7E,   0x19,   0x80,   0x73,   0,      0x70,   0x9F,  // 0x28-0x2F
            0x15,   0x50,   0x6B,   0x4C,   0,      0,      0,      0,     // 0x30-0x37
            0x10,   0x51,   0x6A,   0x4D,   0x27,   0,      0x29,   0x5A,  // 0x38-0x3F
            0x16,   0,      0x86,   0,      0x41,   0,      0x3F,   0,     // 0x40-0x47
            0x88,   0,      0x87,   0,      0x40,   0,      0x3E,   0,     // 0x48-0x4F
            0,      0,      0,      0,      0,      0,      0,      0,     // 0x50-0x57
            0,      0,      0,      0,      0,      0,      0,      0,     // 0x58-0x5F
            0x1C,   0,      0x8C,   0,      0x3D,   0,      0x3B,   0,     // 0x60-0x67
            0x8E,   0,      0x8D,   0,      0x3C,   0,      0x3A,   0,     // 0x68-0x6F
            0x2A,   0,      0x9A,   0,      0,      0,      0,      0,     // 0x70-0x77
            0x2B,   0,      0x9B,   0,      0x55,   0,      0x59,   0,     // 0x78-0x7F
            0x1F,   0x17,   0x09,   0x14,   0x77,   0,      0x74,   0x30,  // 0x80-0x87
            0x0E,   0x83,   0x1A,   0x85,   0x71,   0,      0x6E,   0x31,  // 0x88-0x8F
            0x65,   0x46,   0x69,   0x42,   0,      0,      0,      0,     // 0x90-0x97
            0x64,   0x47,   0x68,   0x43,   0x96,   0,      0x98,   0x56,  // 0x98-0x9F
            0x0D,   0x7B,   0x20,   0x7D,   0x78,   0,      0x75,   0xA0,  // 0xA0-0xA7
            0x18,   0x7F,   0x21,   0x81,   0x72,   0,      0x6F,   0xA1,  // 0xA8-0xAF
            0x63,   0x48,   0x67,   0x44,   0,      0,      0,      0,     // 0xB0-0xB7
            0x62,   0x49,   0x66,   0x45,   0x97,   0,      0x99,   0x5E,  // 0xB8-0xBF
            0x1E,   0x22,   0x89,   0x24,   0x39,   0,      0x37,   0x53,  // 0xC0-0xC7
            0x8B,   0x92,   0x8A,   0x94,   0x38,   0,      0x36,   0x5B,  // 0xC8-0xCF
            0,      0,      0,      0,      0,      0,      0,      0,     // 0xD0-0xD7
            0,      0,      0,      0,      0,      0,      0,      0,     // 0xD8-0xDF
            0x1D,   0x23,   0x8F,   0x25,   0x35,   0,      0x33,   0x57,  // 0xE0-0xE7
            0x91,   0x93,   0x90,   0x95,   0x34,   0,      0x32,   0x5F,  // 0xE8-0xEF
            0x2C,   0x54,   0x9C,   0x5C,   0,      0,      0,      0,     // 0xF0-0xF7
            0x2D,   0x58,   0x9D,   0x60,   0x5D,   0,      0x61,   0,     // 0xF8-0xFF
        };

        // River connection lookup: 4-bit NESW mask → game square value.
        // N=bit3, E=bit2, S=bit1, W=bit0. Uses variant 1 for each pattern.
        // Built from ReMoM TerrType.h River enums.
        // Powered by Claude.
        static constexpr uint16_t RIVER_FROM_CONN[16] = {
            0xB9,   // 0000 = isolated (fallback: S-only)
            0xBA,   // 0001 = W
            0xB9,   // 0010 = S
            0xBE,   // 0011 = S+W
            0xBC,   // 0100 = E
            0xF0,   // 0101 = E+W
            0xBF,   // 0110 = E+S
            0xF7,   // 0111 = E+S+W
            0xBB,   // 1000 = N
            0xC0,   // 1001 = N+W
            0xED,   // 1010 = N+S
            0xFF,   // 1011 = N+S+W
            0xBD,   // 1100 = N+E
            0xF3,   // 1101 = N+E+W
            0xFB,   // 1110 = N+E+S
            0x1D4,  // 1111 = N+E+S+W
        };

        // Mountain variants: 4-bit NESW mask where 1 = neighbor IS mountain.
        // Built from ReMoM TerrType.h Mountain enum (0x103-0x112).
        // Powered by Claude.
        static constexpr uint16_t MOUNTAIN_FROM_CONN[16] = {
            0xA4,   // 0000 = isolated
            0x107,  // 0001 = W
            0x103,  // 0010 = S
            0x10C,  // 0011 = S+W
            0x104,  // 0100 = E
            0x106,  // 0101 = E+W
            0x10A,  // 0110 = E+S
            0x10B,  // 0111 = E+S+W
            0x109,  // 1000 = N
            0x112,  // 1001 = N+W
            0x108,  // 1010 = N+S
            0x10F,  // 1011 = N+S+W
            0x110,  // 1100 = N+E
            0x111,  // 1101 = N+E+W
            0x10D,  // 1110 = N+E+S
            0x105,  // 1111 = N+E+S+W
        };

        // Hill variants: 4-bit NESW mask where 1 = neighbor IS hill.
        // Built from ReMoM TerrType.h Hills enum (0x113-0x122).
        // Powered by Claude.
        static constexpr uint16_t HILL_FROM_CONN[16] = {
            0xAB,   // 0000 = isolated
            0x117,  // 0001 = W
            0x113,  // 0010 = S
            0x11C,  // 0011 = S+W
            0x114,  // 0100 = E
            0x116,  // 0101 = E+W
            0x11A,  // 0110 = E+S
            0x11B,  // 0111 = E+S+W
            0x119,  // 1000 = N
            0x122,  // 1001 = N+W
            0x118,  // 1010 = N+S
            0x11F,  // 1011 = N+S+W
            0x120,  // 1100 = N+E
            0x121,  // 1101 = N+E+W
            0x11D,  // 1110 = N+E+S
            0x115,  // 1111 = N+E+S+W
        };

        // Helper: check if an editor terrain value is "land" for shore adjacency.
        // Everything except ocean and shore counts as land.
        // Powered by Claude.
        auto is_land = [](uint16_t t) -> bool {
            return t != TERRAIN_OCEAN && t != TERRAIN_SHORE;
        };

        // Helper: check if an editor terrain value connects to a river.
        // Rivers connect to other rivers, shores, oceans, and lakes.
        // Powered by Claude.
        auto river_connects = [](uint16_t t) -> bool {
            return t == TERRAIN_RIVER || t == TERRAIN_SHORE ||
                   t == TERRAIN_OCEAN || t == TERRAIN_LAKE;
        };

        // Wrap x for cylindrical world.
        // Powered by Claude.
        auto wrap_x = [](int x) -> int {
            return ((x % WORLD_WIDTH) + WORLD_WIDTH) % WORLD_WIDTH;
        };

        // 8-direction offsets: NW, N, NE, E, SE, S, SW, W (bit 7..0).
        // Powered by Claude.
        static constexpr int dx8[] = {-1,  0,  1, 1, 1, 0, -1, -1};
        static constexpr int dy8[] = {-1, -1, -1, 0, 1, 1,  1,  0};

        // 4-direction offsets: N, E, S, W (bit 3..0).
        // Powered by Claude.
        static constexpr int dx4[] = { 0, 1, 0, -1};
        static constexpr int dy4[] = {-1, 0, 1,  0};

        uint16_t gam_terrain[NUM_PLANES][WORLD_SIZE];
        for (int p = 0; p < NUM_PLANES; ++p) {
            for (int y = 0; y < WORLD_HEIGHT; ++y) {
                for (int x = 0; x < WORLD_WIDTH; ++x) {
                    int i = y * WORLD_WIDTH + x;
                    uint16_t val = sc.world.terrain[p][i];

                    if (val == TERRAIN_SHORE || val == TERRAIN_TUNDRA ||
                        val == TERRAIN_DESERT) {
                        // Compute 8-bit adjacency bitmask with diagonal forcing.
                        // Shore: bits indicate which neighbors are land.
                        // Tundra/Desert: bits indicate which neighbors are NOT that type.
                        // Same bitmask layout (NW=7..W=0) and same diagonal rules.
                        // Powered by Claude.
                        bool flagged[8] = {};
                        for (int d = 0; d < 8; ++d) {
                            int nx = wrap_x(x + dx8[d]);
                            int ny = y + dy8[d];
                            if (ny < 0 || ny >= WORLD_HEIGHT) continue;
                            uint16_t nb = sc.world.terrain[p][ny * WORLD_WIDTH + nx];
                            if (val == TERRAIN_SHORE) {
                                flagged[d] = is_land(nb);
                            } else {
                                flagged[d] = (nb != val); // tundra or desert: not-same-type
                            }
                        }
                        // Diagonal forcing: when both flanking cardinals are set,
                        // force the diagonal between them.
                        // Dirs: 0=NW 1=N 2=NE 3=E 4=SE 5=S 6=SW 7=W
                        // Powered by Claude.
                        if (flagged[1] && flagged[7]) flagged[0] = true; // N+W → NW
                        if (flagged[1] && flagged[3]) flagged[2] = true; // N+E → NE
                        if (flagged[3] && flagged[5]) flagged[4] = true; // E+S → SE
                        if (flagged[5] && flagged[7]) flagged[6] = true; // S+W → SW
                        uint8_t adj = 0;
                        for (int d = 0; d < 8; ++d) {
                            if (flagged[d]) adj |= (1 << (7 - d));
                        }
                        // If no direct table match, try clearing diagonals that
                        // lack both flanking cardinals (fallback masking).
                        // Powered by Claude.
                        uint16_t base_val = SHORE_FROM_ADJ[adj];
                        if (base_val == 0 && adj != 0) {
                            uint8_t masked = adj;
                            // Clear NW if neither N nor W is set.
                            if ((masked & 0x80) && !(masked & 0x40) && !(masked & 0x01))
                                masked &= ~0x80;
                            // Clear NE if neither N nor E is set.
                            if ((masked & 0x20) && !(masked & 0x40) && !(masked & 0x10))
                                masked &= ~0x20;
                            // Clear SE if neither E nor S is set.
                            if ((masked & 0x08) && !(masked & 0x10) && !(masked & 0x04))
                                masked &= ~0x08;
                            // Clear SW if neither S nor W is set.
                            if ((masked & 0x02) && !(masked & 0x04) && !(masked & 0x01))
                                masked &= ~0x02;
                            base_val = SHORE_FROM_ADJ[masked];
                        }

                        if (val == TERRAIN_SHORE) {
                            // No valid shore pattern → export as ocean.
                            // Powered by Claude.
                            gam_terrain[p][i] = (base_val != 0) ? base_val : 0x00;
                        } else if (val == TERRAIN_TUNDRA) {
                            // Tundra variants are shore values + 0x258.
                            // Bitmask 0 (all neighbors tundra) → base tundra 0xA7.
                            // Bitmask 0xFF (isolated) → _1Tundra 0x26A.
                            // Powered by Claude.
                            if (adj == 0) {
                                gam_terrain[p][i] = 0xA7;
                            } else if (base_val != 0) {
                                gam_terrain[p][i] = static_cast<uint16_t>(base_val + 0x258);
                            } else {
                                gam_terrain[p][i] = 0x26A; // _1Tundra (isolated)
                            }
                        } else {
                            // Desert variants are shore values + 0x122.
                            // Bitmask 0 (all neighbors desert) → base desert 0xA5.
                            // Bitmask 0xFF (isolated) → _1Desert 0x134.
                            // Powered by Claude.
                            if (adj == 0) {
                                gam_terrain[p][i] = 0xA5;
                            } else if (base_val != 0) {
                                gam_terrain[p][i] = static_cast<uint16_t>(base_val + 0x122);
                            } else {
                                gam_terrain[p][i] = 0x134; // _1Desert (isolated)
                            }
                        }

                    } else if (val == TERRAIN_MOUNTAIN) {
                        // Compute 4-bit NESW mask: 1 = neighbor IS mountain.
                        // Powered by Claude.
                        uint8_t conn = 0;
                        for (int d = 0; d < 4; ++d) {
                            int nx = wrap_x(x + dx4[d]);
                            int ny = y + dy4[d];
                            if (ny < 0 || ny >= WORLD_HEIGHT) continue;
                            if (sc.world.terrain[p][ny * WORLD_WIDTH + nx] == TERRAIN_MOUNTAIN) {
                                conn |= (1 << (3 - d));
                            }
                        }
                        gam_terrain[p][i] = MOUNTAIN_FROM_CONN[conn];

                    } else if (val == TERRAIN_HILL) {
                        // Compute 4-bit NESW mask: 1 = neighbor IS hill.
                        // Powered by Claude.
                        uint8_t conn = 0;
                        for (int d = 0; d < 4; ++d) {
                            int nx = wrap_x(x + dx4[d]);
                            int ny = y + dy4[d];
                            if (ny < 0 || ny >= WORLD_HEIGHT) continue;
                            if (sc.world.terrain[p][ny * WORLD_WIDTH + nx] == TERRAIN_HILL) {
                                conn |= (1 << (3 - d));
                            }
                        }
                        gam_terrain[p][i] = HILL_FROM_CONN[conn];

                    } else if (val == TERRAIN_RIVER) {
                        // Compute 4-bit NESW connection mask for river square.
                        // Powered by Claude.
                        uint8_t conn = 0;
                        for (int d = 0; d < 4; ++d) {
                            int nx = wrap_x(x + dx4[d]);
                            int ny = y + dy4[d];
                            if (ny < 0 || ny >= WORLD_HEIGHT) continue;
                            if (river_connects(sc.world.terrain[p][ny * WORLD_WIDTH + nx])) {
                                conn |= (1 << (3 - d));
                            }
                        }
                        gam_terrain[p][i] = RIVER_FROM_CONN[conn];

                    } else if (val == TERRAIN_LAKE) {
                        // Directional lake variants based on river inflow.
                        // N=0xC6, E=0xC7, S=0xC8, W=0xC5.
                        // If no river neighbor, export as base lake 0x12.
                        // Powered by Claude.
                        uint16_t lake_val = 0x12; // _1Lake
                        for (int d = 0; d < 4; ++d) {
                            int nx = wrap_x(x + dx4[d]);
                            int ny = y + dy4[d];
                            if (ny < 0 || ny >= WORLD_HEIGHT) continue;
                            if (sc.world.terrain[p][ny * WORLD_WIDTH + nx] == TERRAIN_RIVER) {
                                // Use first river neighbor found for direction.
                                // N=0xC6, E=0xC7, S=0xC8, W=0xC5.
                                // Powered by Claude.
                                static constexpr uint16_t LAKE_DIR[] = {0xC6, 0xC7, 0xC8, 0xC5};
                                lake_val = LAKE_DIR[d];
                                break;
                            }
                        }
                        gam_terrain[p][i] = lake_val;

                    } else if (val < BASE_TERRAIN_COUNT) {
                        gam_terrain[p][i] = EDITOR_TO_GAM[val];
                    } else {
                        gam_terrain[p][i] = val;
                    }
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

    // World maps (terrain) — convert game square indices to editor BaseTerrain (0-11).
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
