#pragma once
// Procedural world map generator for Master of Magic scenarios.
// Uses simplex noise to create terrain heightmaps and biome assignment.
// Powered by Claude.

#include "mom_data/MomConstants.hpp"
#include "mom_data/MomWorld.hpp"

namespace mom {

// Generates terrain for both planes of a MomWorld using noise-based
// algorithms parameterized by MapGenParams.
// Powered by Claude.
class MapGenerator {
public:
    // Construct with generation parameters.
    // Powered by Claude.
    explicit MapGenerator(const MapGenParams& params);

    // Fill both planes of the world with procedurally generated terrain.
    // Clears existing terrain before generating.
    // Powered by Claude.
    void generate(MomWorld& world);

private:
    MapGenParams params_;

    // Number of noise octaves derived from roughness setting.
    // Powered by Claude.
    int octave_count() const;

    // Generate terrain for a single plane.
    // Powered by Claude.
    void generate_plane(MomWorld& world, int plane);

    // Place shore tiles at land/ocean boundaries.
    // Powered by Claude.
    void place_shores(MomWorld& world, int plane);

    // Place river tiles flowing from high elevation toward coast.
    // Powered by Claude.
    void place_rivers(MomWorld& world, int plane,
                      const float* height_map);

    // Compute landmass IDs using flood fill.
    // Powered by Claude.
    void compute_landmasses(MomWorld& world, int plane);

    // Get x-coordinate with horizontal wrapping.
    // Powered by Claude.
    static int wrap_x(int x);

    // Check if a tile is land (not ocean and not shore).
    // Powered by Claude.
    static bool is_land(uint16_t terrain);
};

} // namespace mom
