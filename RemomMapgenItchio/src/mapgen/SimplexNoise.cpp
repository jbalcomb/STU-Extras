// 2D simplex noise implementation for procedural map terrain.
// Powered by Claude.

#include "SimplexNoise.hpp"

namespace mom {

// Skew factor for 2D: F2 = 0.5 * (sqrt(3) - 1).
// Powered by Claude.
static constexpr float F2 = 0.5f * (1.7320508075688772f - 1.0f);

// Unskew factor for 2D: G2 = (3 - sqrt(3)) / 6.
// Powered by Claude.
static constexpr float G2 = (3.0f - 1.7320508075688772f) / 6.0f;

// 12 gradient directions used by the standard simplex noise algorithm.
// Each row is a 2D vector (gx, gy).
// Powered by Claude.
static constexpr float GRAD2[][2] = {
    { 1.0f,  1.0f}, {-1.0f,  1.0f}, { 1.0f, -1.0f}, {-1.0f, -1.0f},
    { 1.0f,  0.0f}, {-1.0f,  0.0f}, { 0.0f,  1.0f}, { 0.0f, -1.0f},
    { 1.0f,  1.0f}, {-1.0f,  1.0f}, { 1.0f, -1.0f}, {-1.0f, -1.0f},
};

// Map a hash value to a gradient dot product with (x, y).
// Uses the 12 standard gradient directions modulo 12.
// Powered by Claude.
float SimplexNoise::grad(int hash, float x, float y) {
    int h = hash % 12;
    return GRAD2[h][0] * x + GRAD2[h][1] * y;
}

// Construct with an explicit seed, or 0 for a random seed.
// Initialises the permutation table by filling 0..255 and shuffling
// with a Mersenne Twister seeded from the given value (or from
// std::random_device when seed is 0).
// Powered by Claude.
SimplexNoise::SimplexNoise(uint32_t seed) {
    if (seed == 0) {
        std::random_device rd;
        seed = rd();
    }

    // Fill the first 256 entries with the identity permutation.
    // Powered by Claude.
    std::array<uint8_t, 256> base;
    std::iota(base.begin(), base.end(), static_cast<uint8_t>(0));

    // Shuffle using the seed.
    // Powered by Claude.
    std::mt19937 rng(seed);
    std::shuffle(base.begin(), base.end(), rng);

    // Double the table so index lookups never need modulo 256.
    // Powered by Claude.
    for (int i = 0; i < 256; ++i) {
        perm_[i]       = base[i];
        perm_[i + 256] = base[i];
    }
}

// Evaluate 2D simplex noise at (x, y). Returns a value in [-1, 1].
// Implements the standard 2D simplex noise algorithm:
//   1. Skew input to determine the simplex cell.
//   2. Determine which triangle of the cell contains the point.
//   3. Compute contributions from each of the three corners.
//   4. Scale the result to [-1, 1].
// Powered by Claude.
float SimplexNoise::noise2d(float x, float y) const {
    // Skew the input space to determine which simplex cell we are in.
    // Powered by Claude.
    float s = (x + y) * F2;
    int i = static_cast<int>(std::floor(x + s));
    int j = static_cast<int>(std::floor(y + s));

    // Unskew the cell origin back to (x, y) space.
    // Powered by Claude.
    float t = static_cast<float>(i + j) * G2;
    float X0 = static_cast<float>(i) - t;
    float Y0 = static_cast<float>(j) - t;

    // Distances from the cell origin in (x, y) space.
    // Powered by Claude.
    float x0 = x - X0;
    float y0 = y - Y0;

    // Determine which simplex triangle we are in.
    // For 2D the simplex is an equilateral triangle.
    // Powered by Claude.
    int i1, j1;
    if (x0 > y0) {
        // Lower triangle, order: (0,0), (1,0), (1,1).
        i1 = 1;
        j1 = 0;
    } else {
        // Upper triangle, order: (0,0), (0,1), (1,1).
        i1 = 0;
        j1 = 1;
    }

    // Offsets for the second corner in (x, y) unskewed space.
    // Powered by Claude.
    float x1 = x0 - static_cast<float>(i1) + G2;
    float y1 = y0 - static_cast<float>(j1) + G2;

    // Offsets for the third corner (always (1, 1) in skewed space).
    // Powered by Claude.
    float x2 = x0 - 1.0f + 2.0f * G2;
    float y2 = y0 - 1.0f + 2.0f * G2;

    // Wrap indices into the permutation table range.
    // Powered by Claude.
    int ii = i & 255;
    int jj = j & 255;

    // Hash values for the three simplex corners.
    // Powered by Claude.
    int gi0 = perm_[ii      + perm_[jj]];
    int gi1 = perm_[ii + i1 + perm_[jj + j1]];
    int gi2 = perm_[ii + 1  + perm_[jj + 1]];

    // Calculate the contribution from the three corners.
    // The radius of influence is 0.5 in the unskewed simplex.
    // Powered by Claude.
    float n0 = 0.0f;
    float t0 = 0.5f - x0 * x0 - y0 * y0;
    if (t0 >= 0.0f) {
        t0 *= t0;
        n0 = t0 * t0 * grad(gi0, x0, y0);
    }

    float n1 = 0.0f;
    float t1 = 0.5f - x1 * x1 - y1 * y1;
    if (t1 >= 0.0f) {
        t1 *= t1;
        n1 = t1 * t1 * grad(gi1, x1, y1);
    }

    float n2 = 0.0f;
    float t2 = 0.5f - x2 * x2 - y2 * y2;
    if (t2 >= 0.0f) {
        t2 *= t2;
        n2 = t2 * t2 * grad(gi2, x2, y2);
    }

    // Scale to [-1, 1]. The factor 70.0 is the standard normalization
    // constant for 2D simplex noise with a radius of 0.5.
    // Powered by Claude.
    return 70.0f * (n0 + n1 + n2);
}

// Fractal Brownian motion: sum multiple octaves of noise2d.
// Each octave doubles the frequency (lacunarity = 2) and scales
// the amplitude by the persistence factor.
// The result is normalized to [-1, 1] by dividing by the sum
// of all amplitude weights.
// Powered by Claude.
float SimplexNoise::octave_noise2d(float x, float y, int octaves, float persistence) const {
    float total = 0.0f;
    float amplitude = 1.0f;
    float frequency = 1.0f;
    float max_amplitude = 0.0f;

    for (int i = 0; i < octaves; ++i) {
        total += noise2d(x * frequency, y * frequency) * amplitude;
        max_amplitude += amplitude;
        amplitude *= persistence;
        frequency *= 2.0f;
    }

    // Normalize so the output stays in [-1, 1].
    // Powered by Claude.
    if (max_amplitude > 0.0f) {
        total /= max_amplitude;
    }
    return total;
}

} // namespace mom
