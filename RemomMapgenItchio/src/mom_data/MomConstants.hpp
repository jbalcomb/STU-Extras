#pragma once
// MoM game constants mirrored from ReMoM MOM_DEF.H for binary compatibility.
// Powered by Claude.

#include <cstdint>
#include <string>

namespace mom {

// World dimensions
constexpr int WORLD_WIDTH  = 60;
constexpr int WORLD_HEIGHT = 40;
constexpr int WORLD_SIZE   = WORLD_WIDTH * WORLD_HEIGHT; // 2400
constexpr int NUM_PLANES   = 2;

// Entity limits
constexpr int NUM_CITIES     = 100;
constexpr int NUM_UNITS      = 1009;
constexpr int NUM_NODES      = 30;
constexpr int NUM_LAIRS      = 102; // original game array size
constexpr int NUM_TOWERS     = 6;
constexpr int NUM_FORTRESSES = 6;
constexpr int NUM_PLAYERS    = 6;
constexpr int NUM_BUILDINGS  = 36;
constexpr int NUM_HEROES     = 6;
constexpr int NUM_HERO_TYPES = 35;
constexpr int NUM_ITEMS      = 138;
constexpr int NUM_CITY_ENCHANTMENTS = 25;
constexpr int NUM_OVERLAND_ENCHANTMENTS = 24;
constexpr int NUM_SPELLS     = 240;

// String lengths
constexpr int LEN_CITY_NAME   = 14;
constexpr int LEN_WIZARD_NAME = 20;
constexpr int LEN_HERO_NAME   = 16;

// Save file
constexpr int SAVEGAM_RECORD_SIZE = 123300;

// Tile pixel dimensions (original)
constexpr int TILE_PX_W = 20;
constexpr int TILE_PX_H = 18;

// Combat grid
constexpr int COMBAT_GRID_WIDTH  = 21;
constexpr int COMBAT_GRID_HEIGHT = 22;

// -- Enums --

enum Plane : int8_t {
    PLANE_ARCANUS = 0,
    PLANE_MYRROR  = 1
};

enum Difficulty : uint16_t {
    DIFF_INTRO      = 0,
    DIFF_EASY       = 1,
    DIFF_NORMAL     = 2,
    DIFF_HARD       = 3,
    DIFF_IMPOSSIBLE = 4
};

enum Race : int8_t {
    RACE_BARBARIAN = 0,
    RACE_BEASTMAN  = 1,
    RACE_DARK_ELF  = 2,
    RACE_DRACONIAN = 3,
    RACE_DWARF     = 4,
    RACE_GNOLL     = 5,
    RACE_HALFLING  = 6,
    RACE_HIGH_ELF  = 7,
    RACE_HIGH_MAN  = 8,
    RACE_KLACKON   = 9,
    RACE_LIZARDMAN = 10,
    RACE_NOMAD     = 11,
    RACE_ORC       = 12,
    RACE_TROLL     = 13,
    RACE_COUNT     = 14
};

enum TerrainSpecial : int8_t {
    TS_NONE            = 0,
    TS_IRON            = 1,
    TS_COAL            = 2,
    TS_SILVER          = 3,
    TS_GOLD            = 4,
    TS_GEMS            = 5,
    TS_MITHRIL         = 6,
    TS_ADAMANTIUM      = 7,
    TS_QUORK_CRYSTALS  = 8,
    TS_CRYSX_CRYSTALS  = 9,
    TS_HUNTERS_LODGE   = 16,
    TS_WILD_GAME       = 64,
    TS_NIGHTSHADE      = static_cast<int8_t>(128)
};

// Base terrain types used by the editor (simplified from 762 tile indices)
enum BaseTerrain : uint8_t {
    TERRAIN_OCEAN     = 0,
    TERRAIN_SHORE     = 1,
    TERRAIN_GRASSLAND = 2,
    TERRAIN_FOREST    = 3,
    TERRAIN_MOUNTAIN  = 4,
    TERRAIN_DESERT    = 5,
    TERRAIN_SWAMP     = 6,
    TERRAIN_TUNDRA    = 7,
    TERRAIN_HILL      = 8,
    TERRAIN_RIVER     = 9,
    TERRAIN_VOLCANO   = 10,
    TERRAIN_LAKE      = 11,
    BASE_TERRAIN_COUNT = 12
};

enum NodeType : int8_t {
    NODE_SORCERY = 0,
    NODE_NATURE  = 1,
    NODE_CHAOS   = 2
};

// Map square flags (bitfield)
enum MapSquareFlag : uint8_t {
    MSF_VOLCANO_MASK = 0x07,
    MSF_ROAD         = 0x08,
    MSF_ENC_ROAD     = 0x10,
    MSF_CORRUPTION   = 0x20,
    MSF_NO_ROAD      = 0x80
};

// -- Editor-only types (NOT part of packed game structs) --

// Map generation roughness level. Powered by Claude.
enum Roughness : uint8_t {
    ROUGHNESS_SMOOTH = 0,
    ROUGHNESS_MEDIUM = 1,
    ROUGHNESS_ROUGH  = 2
};

// Parameters for procedural map generation. Editor-only, not serialized to .GAM.
// Powered by Claude.
struct MapGenParams {
    float    land_proportion  = 0.4f;   // [0.05, 0.95]
    Roughness roughness      = ROUGHNESS_MEDIUM;
    int      continent_count = 3;       // [1, 6]
    uint32_t seed            = 0;       // 0 = random
    bool     mirror_planes   = false;   // copy Arcanus to Myrror
};

// Smoothing violation severity. Powered by Claude.
enum ViolationSeverity : uint8_t {
    SEVERITY_WARNING = 0,   // advisory, may render oddly in-game
    SEVERITY_ERROR   = 1    // will definitely cause rendering artifacts
};

// Smoothing validation rule identifiers. Powered by Claude.
enum ViolationRule : uint8_t {
    RULE_SHORE_NO_WATER     = 0,  // shore tile has no adjacent ocean/lake
    RULE_RIVER_ISOLATED     = 1,  // river tile has no adjacent river/shore/ocean/lake
    RULE_VOLCANO_ISOLATED   = 2,  // volcano has no adjacent mountain/hill
    RULE_TUNDRA_DESERT      = 3   // tundra is adjacent to desert
};

// A single smoothing rule violation. Transient, not serialized.
// Powered by Claude.
struct SmoothingViolation {
    int              plane    = 0;
    int              x        = 0;
    int              y        = 0;
    ViolationRule    rule     = RULE_SHORE_NO_WATER;
    ViolationSeverity severity = SEVERITY_WARNING;
    std::string      message;
};

} // namespace mom
