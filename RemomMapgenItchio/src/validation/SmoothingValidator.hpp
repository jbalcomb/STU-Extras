#pragma once
// SmoothingValidator — validates terrain smoothing rules for the world map.
// Powered by Claude.

#include <vector>
#include <string>
#include "mom_data/MomConstants.hpp"
#include "mom_data/MomWorld.hpp"

namespace mom {

// Validates terrain tiles against smoothing rules (V-009 through V-012).
// Powered by Claude.
class SmoothingValidator {
public:
    // Validate a single tile, return violations for that tile.
    // Powered by Claude.
    static std::vector<SmoothingViolation> validate_tile(
        const MomWorld& world, int plane, int x, int y);

    // Validate all tiles on a plane, return all violations.
    // Powered by Claude.
    static std::vector<SmoothingViolation> validate_plane(
        const MomWorld& world, int plane);

private:
    // Wrap x coordinate for cylindrical world topology.
    // Powered by Claude.
    static int wrap_x(int x) {
        return ((x % WORLD_WIDTH) + WORLD_WIDTH) % WORLD_WIDTH;
    }
};

} // namespace mom
