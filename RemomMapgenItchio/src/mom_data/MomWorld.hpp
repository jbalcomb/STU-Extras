#pragma once
// World terrain grid for both planes (Arcanus and Myrror).
// Powered by Claude.

#include "mom_data/MomConstants.hpp"
#include <array>
#include <cstring>

namespace mom {

// Represents the 60x40x2 world terrain data.
// Powered by Claude.
struct MomWorld {
    // Terrain type per tile (2 bytes each in .GAM, stored as uint16_t)
    uint16_t terrain[NUM_PLANES][WORLD_SIZE]{};

    // Terrain specials per tile (mineral deposits, wild game, etc.)
    int8_t specials[NUM_PLANES][WORLD_SIZE]{};

    // Map square flags (roads, corruption, etc.)
    uint8_t flags[NUM_PLANES][WORLD_SIZE]{};

    // Landmass IDs
    uint8_t landmasses[NUM_PLANES][WORLD_SIZE]{};

    // Exploration state per tile
    uint8_t explored[NUM_PLANES][WORLD_SIZE]{};

    // Get/set terrain at world coordinates.
    // Powered by Claude.
    uint16_t get_terrain(int x, int y, int plane) const {
        if (x < 0 || x >= WORLD_WIDTH || y < 0 || y >= WORLD_HEIGHT || plane < 0 || plane >= NUM_PLANES)
            return 0;
        return terrain[plane][y * WORLD_WIDTH + x];
    }

    void set_terrain(int x, int y, int plane, uint16_t type) {
        if (x < 0 || x >= WORLD_WIDTH || y < 0 || y >= WORLD_HEIGHT || plane < 0 || plane >= NUM_PLANES)
            return;
        terrain[plane][y * WORLD_WIDTH + x] = type;
    }

    // Get/set terrain special at world coordinates.
    // Powered by Claude.
    int8_t get_special(int x, int y, int plane) const {
        if (x < 0 || x >= WORLD_WIDTH || y < 0 || y >= WORLD_HEIGHT || plane < 0 || plane >= NUM_PLANES)
            return 0;
        return specials[plane][y * WORLD_WIDTH + x];
    }

    void set_special(int x, int y, int plane, int8_t special) {
        if (x < 0 || x >= WORLD_WIDTH || y < 0 || y >= WORLD_HEIGHT || plane < 0 || plane >= NUM_PLANES)
            return;
        specials[plane][y * WORLD_WIDTH + x] = special;
    }

    // Get/set map square flags at world coordinates.
    // Powered by Claude.
    uint8_t get_flags(int x, int y, int plane) const {
        if (x < 0 || x >= WORLD_WIDTH || y < 0 || y >= WORLD_HEIGHT || plane < 0 || plane >= NUM_PLANES)
            return 0;
        return flags[plane][y * WORLD_WIDTH + x];
    }

    void set_flags(int x, int y, int plane, uint8_t f) {
        if (x < 0 || x >= WORLD_WIDTH || y < 0 || y >= WORLD_HEIGHT || plane < 0 || plane >= NUM_PLANES)
            return;
        flags[plane][y * WORLD_WIDTH + x] = f;
    }

    // Get/set landmass ID at world coordinates.
    // Powered by Claude.
    uint8_t get_landmass(int plane, int x, int y) const {
        if (x < 0 || x >= WORLD_WIDTH || y < 0 || y >= WORLD_HEIGHT || plane < 0 || plane >= NUM_PLANES)
            return 0;
        return landmasses[plane][y * WORLD_WIDTH + x];
    }

    void set_landmass(int plane, int x, int y, uint8_t id) {
        if (x < 0 || x >= WORLD_WIDTH || y < 0 || y >= WORLD_HEIGHT || plane < 0 || plane >= NUM_PLANES)
            return;
        landmasses[plane][y * WORLD_WIDTH + x] = id;
    }

    // Reset all terrain to ocean.
    // Powered by Claude.
    void clear() {
        std::memset(terrain,    0, sizeof(terrain));
        std::memset(specials,   0, sizeof(specials));
        std::memset(flags,      0, sizeof(flags));
        std::memset(landmasses, 0, sizeof(landmasses));
        std::memset(explored,   0, sizeof(explored));
    }

    // Set all tiles as fully explored.
    // Powered by Claude.
    void set_all_explored() {
        std::memset(explored, 0x0F, sizeof(explored));
    }
};

} // namespace mom
