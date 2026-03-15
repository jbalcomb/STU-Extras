// SmoothingValidator — validates terrain smoothing rules for the world map.
// Powered by Claude.

#include "validation/SmoothingValidator.hpp"

namespace mom {

// Validate a single tile against all smoothing rules (V-009 through V-012).
// Returns a vector of violations found at the given tile coordinates.
// Powered by Claude.
std::vector<SmoothingViolation> SmoothingValidator::validate_tile(
    const MomWorld& world, int plane, int x, int y)
{
    std::vector<SmoothingViolation> violations;

    uint16_t tile = world.get_terrain(x, y, plane);

    // Cardinal direction offsets: N, S, W, E
    static constexpr int dx[] = { 0,  0, -1, 1};
    static constexpr int dy[] = {-1,  1,  0, 0};

    // V-009: SHORE_NO_WATER (SEVERITY_ERROR)
    // Shore tile must have at least one cardinal neighbor that is ocean or lake.
    // Powered by Claude.
    if (tile == TERRAIN_SHORE) {
        bool has_water = false;
        for (int d = 0; d < 4; ++d) {
            int ny = y + dy[d];
            if (ny < 0 || ny >= WORLD_HEIGHT)
                continue;
            int nx = wrap_x(x + dx[d]);
            uint16_t neighbor = world.get_terrain(nx, ny, plane);
            if (neighbor == TERRAIN_OCEAN || neighbor == TERRAIN_LAKE) {
                has_water = true;
                break;
            }
        }
        if (!has_water) {
            SmoothingViolation v;
            v.plane    = plane;
            v.x        = x;
            v.y        = y;
            v.rule     = RULE_SHORE_NO_WATER;
            v.severity = SEVERITY_ERROR;
            v.message  = "Shore tile has no adjacent water (ocean or lake)";
            violations.push_back(v);
        }
    }

    // V-010: RIVER_ISOLATED (SEVERITY_ERROR)
    // River tile must have at least one cardinal neighbor that is
    // river, shore, ocean, or lake.
    // Powered by Claude.
    if (tile == TERRAIN_RIVER) {
        bool has_connection = false;
        for (int d = 0; d < 4; ++d) {
            int ny = y + dy[d];
            if (ny < 0 || ny >= WORLD_HEIGHT)
                continue;
            int nx = wrap_x(x + dx[d]);
            uint16_t neighbor = world.get_terrain(nx, ny, plane);
            if (neighbor == TERRAIN_RIVER || neighbor == TERRAIN_SHORE ||
                neighbor == TERRAIN_OCEAN || neighbor == TERRAIN_LAKE) {
                has_connection = true;
                break;
            }
        }
        if (!has_connection) {
            SmoothingViolation v;
            v.plane    = plane;
            v.x        = x;
            v.y        = y;
            v.rule     = RULE_RIVER_ISOLATED;
            v.severity = SEVERITY_ERROR;
            v.message  = "River tile is isolated — no adjacent river, shore, ocean, or lake";
            violations.push_back(v);
        }
    }

    // V-011: VOLCANO_ISOLATED (SEVERITY_WARNING)
    // Volcano tile should have at least one cardinal neighbor that is
    // mountain or hill.
    // Powered by Claude.
    if (tile == TERRAIN_VOLCANO) {
        bool has_highland = false;
        for (int d = 0; d < 4; ++d) {
            int ny = y + dy[d];
            if (ny < 0 || ny >= WORLD_HEIGHT)
                continue;
            int nx = wrap_x(x + dx[d]);
            uint16_t neighbor = world.get_terrain(nx, ny, plane);
            if (neighbor == TERRAIN_MOUNTAIN || neighbor == TERRAIN_HILL) {
                has_highland = true;
                break;
            }
        }
        if (!has_highland) {
            SmoothingViolation v;
            v.plane    = plane;
            v.x        = x;
            v.y        = y;
            v.rule     = RULE_VOLCANO_ISOLATED;
            v.severity = SEVERITY_WARNING;
            v.message  = "Volcano has no adjacent mountain or hill";
            violations.push_back(v);
        }
    }

    // V-012: TUNDRA_DESERT (SEVERITY_WARNING)
    // Tundra tile should not have any cardinal neighbor that is desert.
    // Powered by Claude.
    if (tile == TERRAIN_TUNDRA) {
        for (int d = 0; d < 4; ++d) {
            int ny = y + dy[d];
            if (ny < 0 || ny >= WORLD_HEIGHT)
                continue;
            int nx = wrap_x(x + dx[d]);
            uint16_t neighbor = world.get_terrain(nx, ny, plane);
            if (neighbor == TERRAIN_DESERT) {
                SmoothingViolation v;
                v.plane    = plane;
                v.x        = x;
                v.y        = y;
                v.rule     = RULE_TUNDRA_DESERT;
                v.severity = SEVERITY_WARNING;
                v.message  = "Tundra is adjacent to desert";
                violations.push_back(v);
                break;  // one violation per tile is sufficient
            }
        }
    }

    return violations;
}

// Validate all tiles on a plane, collecting every violation.
// Powered by Claude.
std::vector<SmoothingViolation> SmoothingValidator::validate_plane(
    const MomWorld& world, int plane)
{
    std::vector<SmoothingViolation> violations;

    for (int y = 0; y < WORLD_HEIGHT; ++y) {
        for (int x = 0; x < WORLD_WIDTH; ++x) {
            auto tile_violations = validate_tile(world, plane, x, y);
            violations.insert(violations.end(),
                              tile_violations.begin(),
                              tile_violations.end());
        }
    }

    return violations;
}

} // namespace mom
