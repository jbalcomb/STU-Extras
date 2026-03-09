#pragma once
// 2D simplex noise generator for procedural map terrain.
// Powered by Claude.

#include <cstdint>
#include <cmath>
#include <array>
#include <random>
#include <algorithm>
#include <numeric>

namespace mom {

// Simplex noise generator with seed-based permutation table.
// Produces coherent noise suitable for terrain heightmaps.
// Powered by Claude.
class SimplexNoise {
public:
    // Construct with an explicit seed, or 0 for a random seed.
    // Powered by Claude.
    explicit SimplexNoise(uint32_t seed = 0);

    // Evaluate 2D simplex noise at (x, y). Returns a value in [-1, 1].
    // Powered by Claude.
    float noise2d(float x, float y) const;

    // Fractal Brownian motion: sum multiple octaves of noise2d with
    // increasing frequency and decreasing amplitude.
    // Returns a normalized value in [-1, 1].
    // Powered by Claude.
    float octave_noise2d(float x, float y, int octaves, float persistence) const;

private:
    // Permutation table doubled for wrap-around indexing.
    // Powered by Claude.
    std::array<uint8_t, 512> perm_;

    // Map a hash value to a gradient dot product with (x, y).
    // Powered by Claude.
    static float grad(int hash, float x, float y);
};

} // namespace mom
