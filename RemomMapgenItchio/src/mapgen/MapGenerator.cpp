// Procedural world map generator using simplex noise.
// Powered by Claude.

#include "mapgen/MapGenerator.hpp"
#include "mapgen/SimplexNoise.hpp"
#include <algorithm>
#include <cmath>
#include <queue>
#include <vector>

namespace mom {

// Construct with generation parameters.
// Powered by Claude.
MapGenerator::MapGenerator(const MapGenParams& params)
    : params_(params) {}

// Map roughness enum to octave count.
// Powered by Claude.
int MapGenerator::octave_count() const {
    switch (params_.roughness) {
        case ROUGHNESS_SMOOTH: return 2;
        case ROUGHNESS_MEDIUM: return 4;
        case ROUGHNESS_ROUGH:  return 6;
        default: return 4;
    }
}

// Wrap x-coordinate for cylindrical world.
// Powered by Claude.
int MapGenerator::wrap_x(int x) {
    return ((x % WORLD_WIDTH) + WORLD_WIDTH) % WORLD_WIDTH;
}

// Check if terrain type is land (not ocean, not shore).
// Powered by Claude.
bool MapGenerator::is_land(uint16_t terrain) {
    return terrain != TERRAIN_OCEAN && terrain != TERRAIN_SHORE;
}

// Fill both planes with procedurally generated terrain.
// Clamps extreme parameters before generating to prevent degenerate maps.
// Powered by Claude.
void MapGenerator::generate(MomWorld& world) {
    // Clamp generation parameters to safe ranges.
    // Powered by Claude.
    params_.land_proportion = std::clamp(params_.land_proportion, 0.05f, 0.95f);
    params_.continent_count = std::clamp(params_.continent_count, 1, 6);

    world.clear();

    generate_plane(world, PLANE_ARCANUS);

    if (params_.mirror_planes) {
        // Copy Arcanus terrain to Myrror.
        for (int y = 0; y < WORLD_HEIGHT; ++y) {
            for (int x = 0; x < WORLD_WIDTH; ++x) {
                int idx = y * WORLD_WIDTH + x;
                world.terrain[PLANE_MYRROR][idx] = world.terrain[PLANE_ARCANUS][idx];
                world.specials[PLANE_MYRROR][idx] = world.specials[PLANE_ARCANUS][idx];
                world.flags[PLANE_MYRROR][idx] = world.flags[PLANE_ARCANUS][idx];
                world.landmasses[PLANE_MYRROR][idx] = world.landmasses[PLANE_ARCANUS][idx];
            }
        }
    } else {
        generate_plane(world, PLANE_MYRROR);
    }

    world.set_all_explored();
}

// Generate terrain for one plane using simplex noise heightmap.
// Powered by Claude.
void MapGenerator::generate_plane(MomWorld& world, int plane) {
    // Use different noise seeds per plane so they differ.
    uint32_t seed = params_.seed;
    if (seed == 0) seed = 12345;
    uint32_t plane_seed = seed + static_cast<uint32_t>(plane) * 7919;

    SimplexNoise height_noise(plane_seed);
    SimplexNoise moisture_noise(plane_seed + 31337);

    int octaves = octave_count();
    float persistence = 0.5f;
    float scale = 0.08f; // controls feature size

    // Clamp land proportion to [0.05, 0.95].
    float land_prop = std::clamp(params_.land_proportion, 0.05f, 0.95f);

    // Generate height and moisture maps.
    std::vector<float> height_map(WORLD_SIZE);
    std::vector<float> moisture_map(WORLD_SIZE);

    for (int y = 0; y < WORLD_HEIGHT; ++y) {
        for (int x = 0; x < WORLD_WIDTH; ++x) {
            float fx = static_cast<float>(x) * scale;
            float fy = static_cast<float>(y) * scale;

            // Height noise — use cylindrical mapping for horizontal wrapping.
            float angle = static_cast<float>(x) / static_cast<float>(WORLD_WIDTH) * 6.2831853f;
            float cx = std::cos(angle) * 10.0f;
            float cy = std::sin(angle) * 10.0f;

            float h = height_noise.octave_noise2d(cx * scale, fy + cy * scale * 0.1f, octaves, persistence);

            // Push poles toward ocean (simple latitude falloff).
            float lat_factor = static_cast<float>(y) / static_cast<float>(WORLD_HEIGHT - 1);
            float pole_penalty = 0.0f;
            if (lat_factor < 0.1f) pole_penalty = (0.1f - lat_factor) * 3.0f;
            if (lat_factor > 0.9f) pole_penalty = (lat_factor - 0.9f) * 3.0f;
            h -= pole_penalty;

            height_map[y * WORLD_WIDTH + x] = h;

            float m = moisture_noise.octave_noise2d(fx * 1.3f, fy * 1.3f, 3, 0.5f);
            moisture_map[y * WORLD_WIDTH + x] = m;
        }
    }

    // Determine the height threshold for the desired land proportion.
    // Sort heights and pick the value at the (1 - land_prop) percentile.
    std::vector<float> sorted_heights = height_map;
    std::sort(sorted_heights.begin(), sorted_heights.end());
    int threshold_idx = static_cast<int>((1.0f - land_prop) * static_cast<float>(WORLD_SIZE));
    threshold_idx = std::clamp(threshold_idx, 0, WORLD_SIZE - 1);
    float threshold = sorted_heights[threshold_idx];

    // Assign base terrain types based on height and moisture.
    for (int y = 0; y < WORLD_HEIGHT; ++y) {
        for (int x = 0; x < WORLD_WIDTH; ++x) {
            int idx = y * WORLD_WIDTH + x;
            float h = height_map[idx];
            float m = moisture_map[idx];

            uint16_t t;
            if (h < threshold) {
                t = TERRAIN_OCEAN;
            } else {
                // Normalize height above threshold to [0, 1] for biome assignment.
                float range = sorted_heights[WORLD_SIZE - 1] - threshold;
                float norm_h = (range > 0.001f) ? (h - threshold) / range : 0.5f;

                if (norm_h > 0.85f) {
                    t = TERRAIN_MOUNTAIN;
                } else if (norm_h > 0.7f) {
                    t = TERRAIN_HILL;
                } else {
                    // Use moisture to decide between terrain types.
                    float lat = static_cast<float>(y) / static_cast<float>(WORLD_HEIGHT - 1);
                    bool polar = (lat < 0.15f || lat > 0.85f);
                    bool tropical = (lat > 0.35f && lat < 0.65f);

                    if (polar) {
                        t = TERRAIN_TUNDRA;
                    } else if (m > 0.3f) {
                        t = TERRAIN_FOREST;
                    } else if (m < -0.3f) {
                        t = tropical ? TERRAIN_DESERT : TERRAIN_GRASSLAND;
                    } else if (m > 0.0f && tropical) {
                        t = TERRAIN_SWAMP;
                    } else {
                        t = TERRAIN_GRASSLAND;
                    }
                }
            }

            world.set_terrain(x, y, plane, t);
        }
    }

    // Place shores, rivers, then compute landmass IDs.
    place_shores(world, plane);
    place_rivers(world, plane, height_map.data());
    compute_landmasses(world, plane);
}

// Place shore tiles between ocean and land.
// A shore replaces ocean tiles that have at least one cardinal land neighbor.
// Powered by Claude.
void MapGenerator::place_shores(MomWorld& world, int plane) {
    // Cardinal neighbor offsets (accounting for wrapping on x).
    static const int dx[] = {0, 0, -1, 1};
    static const int dy[] = {-1, 1, 0, 0};

    // Mark ocean tiles adjacent to land as shore.
    std::vector<std::pair<int, int>> shore_tiles;

    for (int y = 0; y < WORLD_HEIGHT; ++y) {
        for (int x = 0; x < WORLD_WIDTH; ++x) {
            if (world.get_terrain(x, y, plane) != TERRAIN_OCEAN) continue;

            bool has_land_neighbor = false;
            for (int d = 0; d < 4; ++d) {
                int nx = wrap_x(x + dx[d]);
                int ny = y + dy[d];
                if (ny < 0 || ny >= WORLD_HEIGHT) continue;

                uint16_t nt = world.get_terrain(nx, ny, plane);
                if (is_land(nt)) {
                    has_land_neighbor = true;
                    break;
                }
            }

            if (has_land_neighbor) {
                shore_tiles.emplace_back(x, y);
            }
        }
    }

    for (auto& [sx, sy] : shore_tiles) {
        world.set_terrain(sx, sy, plane, TERRAIN_SHORE);
    }
}

// Place river tiles starting from high-elevation land tiles flowing toward coast.
// Powered by Claude.
void MapGenerator::place_rivers(MomWorld& world, int plane,
                                 const float* height_map) {
    static const int dx[] = {0, 0, -1, 1};
    static const int dy[] = {-1, 1, 0, 0};

    // Seed a few rivers from the highest terrain points.
    struct HeightTile {
        float h;
        int x, y;
        bool operator<(const HeightTile& o) const { return h < o.h; }
    };

    std::vector<HeightTile> candidates;
    for (int y = 0; y < WORLD_HEIGHT; ++y) {
        for (int x = 0; x < WORLD_WIDTH; ++x) {
            uint16_t t = world.get_terrain(x, y, plane);
            if (t == TERRAIN_MOUNTAIN || t == TERRAIN_HILL) {
                candidates.push_back({height_map[y * WORLD_WIDTH + x], x, y});
            }
        }
    }

    std::sort(candidates.rbegin(), candidates.rend());

    // Place up to 8 rivers per plane.
    int max_rivers = std::min(8, static_cast<int>(candidates.size()));
    int river_spacing = (candidates.size() > 0) ?
        std::max(1, static_cast<int>(candidates.size()) / max_rivers) : 1;

    for (int r = 0; r < max_rivers; ++r) {
        int ci = r * river_spacing;
        if (ci >= static_cast<int>(candidates.size())) break;

        int cx = candidates[ci].x;
        int cy = candidates[ci].y;
        int max_length = 20;

        for (int step = 0; step < max_length; ++step) {
            uint16_t cur_t = world.get_terrain(cx, cy, plane);
            // Stop if we hit ocean, shore, or another river.
            if (cur_t == TERRAIN_OCEAN || cur_t == TERRAIN_SHORE ||
                cur_t == TERRAIN_RIVER || cur_t == TERRAIN_LAKE) {
                break;
            }

            // Don't overwrite mountains — start flowing from next tile.
            if (cur_t != TERRAIN_MOUNTAIN) {
                world.set_terrain(cx, cy, plane, TERRAIN_RIVER);
            }

            // Flow toward the lowest neighbor.
            float lowest_h = height_map[cy * WORLD_WIDTH + cx];
            int best_x = -1, best_y = -1;

            for (int d = 0; d < 4; ++d) {
                int nx = wrap_x(cx + dx[d]);
                int ny = cy + dy[d];
                if (ny < 0 || ny >= WORLD_HEIGHT) continue;

                float nh = height_map[ny * WORLD_WIDTH + nx];
                if (nh < lowest_h) {
                    lowest_h = nh;
                    best_x = nx;
                    best_y = ny;
                }
            }

            if (best_x < 0) break; // no lower neighbor, stop
            cx = best_x;
            cy = best_y;
        }
    }
}

// Flood-fill landmass IDs for contiguous land regions.
// Powered by Claude.
void MapGenerator::compute_landmasses(MomWorld& world, int plane) {
    static const int dx[] = {0, 0, -1, 1};
    static const int dy[] = {-1, 1, 0, 0};

    // Reset landmass IDs for this plane.
    for (int i = 0; i < WORLD_SIZE; ++i) {
        world.landmasses[plane][i] = 0;
    }

    uint8_t current_id = 1;

    for (int y = 0; y < WORLD_HEIGHT; ++y) {
        for (int x = 0; x < WORLD_WIDTH; ++x) {
            int idx = y * WORLD_WIDTH + x;
            if (world.landmasses[plane][idx] != 0) continue;
            if (!is_land(world.terrain[plane][idx])) continue;

            // BFS flood fill from this tile.
            std::queue<std::pair<int, int>> q;
            q.push({x, y});
            world.landmasses[plane][idx] = current_id;

            while (!q.empty()) {
                auto [qx, qy] = q.front();
                q.pop();

                for (int d = 0; d < 4; ++d) {
                    int nx = wrap_x(qx + dx[d]);
                    int ny = qy + dy[d];
                    if (ny < 0 || ny >= WORLD_HEIGHT) continue;

                    int nidx = ny * WORLD_WIDTH + nx;
                    if (world.landmasses[plane][nidx] != 0) continue;
                    if (!is_land(world.terrain[plane][nidx])) continue;

                    world.landmasses[plane][nidx] = current_id;
                    q.push({nx, ny});
                }
            }

            if (current_id < 255) ++current_id;
        }
    }
}

} // namespace mom
