// Tests for procedural map generation.
// Powered by Claude.

#include <gtest/gtest.h>
#include "mapgen/MapGenerator.hpp"
#include "mom_data/MomWorld.hpp"
#include "mom_data/MomConstants.hpp"

// Test fixture for MapGenerator tests.
// Powered by Claude.
class MapGeneratorTest : public ::testing::Test {
protected:
    mom::MomWorld world;
    mom::MapGenParams params;

    void SetUp() override {
        params.seed = 12345;  // deterministic for tests
    }
};

// Verify every tile on both planes has a valid BaseTerrain enum value.
// Powered by Claude.
TEST_F(MapGeneratorTest, GenerateProducesValidTerrain) {
    mom::MapGenerator gen(params);
    gen.generate(world);

    for (int plane = 0; plane < mom::NUM_PLANES; ++plane) {
        for (int y = 0; y < mom::WORLD_HEIGHT; ++y) {
            for (int x = 0; x < mom::WORLD_WIDTH; ++x) {
                uint16_t t = world.get_terrain(x, y, plane);
                EXPECT_LT(t, mom::BASE_TERRAIN_COUNT)
                    << "Invalid terrain " << t
                    << " at (" << x << ", " << y << ", " << plane << ")";
            }
        }
    }
}

// Verify that land proportion approximately matches the requested value.
// Powered by Claude.
TEST_F(MapGeneratorTest, LandProportionApproximatelyMatchesInput) {
    params.land_proportion = 0.5f;
    mom::MapGenerator gen(params);
    gen.generate(world);

    int land_count = 0;
    for (int y = 0; y < mom::WORLD_HEIGHT; ++y) {
        for (int x = 0; x < mom::WORLD_WIDTH; ++x) {
            uint16_t t = world.get_terrain(x, y, mom::PLANE_ARCANUS);
            if (t != mom::TERRAIN_OCEAN && t != mom::TERRAIN_SHORE) {
                ++land_count;
            }
        }
    }

    double proportion = static_cast<double>(land_count) / mom::WORLD_SIZE;
    EXPECT_GE(proportion, 0.35)
        << "Land proportion " << proportion << " is below lower bound 0.35";
    EXPECT_LE(proportion, 0.65)
        << "Land proportion " << proportion << " is above upper bound 0.65";
}

// Verify that both planes contain at least one non-ocean tile.
// Powered by Claude.
TEST_F(MapGeneratorTest, BothPlanesHaveTerrain) {
    mom::MapGenerator gen(params);
    gen.generate(world);

    for (int plane = 0; plane < mom::NUM_PLANES; ++plane) {
        int non_ocean = 0;
        for (int y = 0; y < mom::WORLD_HEIGHT; ++y) {
            for (int x = 0; x < mom::WORLD_WIDTH; ++x) {
                if (world.get_terrain(x, y, plane) != mom::TERRAIN_OCEAN) {
                    ++non_ocean;
                }
            }
        }
        EXPECT_GT(non_ocean, 0)
            << "Plane " << plane << " has no non-ocean tiles";
    }
}

// Verify that shore tiles exist at land/ocean boundaries.
// Powered by Claude.
TEST_F(MapGeneratorTest, ShoresAtLandOceanBoundaries) {
    mom::MapGenerator gen(params);
    gen.generate(world);

    int shore_count = 0;
    for (int plane = 0; plane < mom::NUM_PLANES; ++plane) {
        for (int y = 0; y < mom::WORLD_HEIGHT; ++y) {
            for (int x = 0; x < mom::WORLD_WIDTH; ++x) {
                if (world.get_terrain(x, y, plane) == mom::TERRAIN_SHORE) {
                    ++shore_count;
                }
            }
        }
    }
    EXPECT_GT(shore_count, 0) << "No shore tiles found on either plane";
}

// Verify that at least one land tile has a nonzero landmass ID.
// Powered by Claude.
TEST_F(MapGeneratorTest, LandmassIDsComputedForLand) {
    mom::MapGenerator gen(params);
    gen.generate(world);

    bool found_nonzero_landmass = false;
    for (int plane = 0; plane < mom::NUM_PLANES && !found_nonzero_landmass; ++plane) {
        for (int y = 0; y < mom::WORLD_HEIGHT && !found_nonzero_landmass; ++y) {
            for (int x = 0; x < mom::WORLD_WIDTH && !found_nonzero_landmass; ++x) {
                uint16_t t = world.get_terrain(x, y, plane);
                if (t != mom::TERRAIN_OCEAN && t != mom::TERRAIN_SHORE) {
                    if (world.get_landmass(plane, x, y) != 0) {
                        found_nonzero_landmass = true;
                    }
                }
            }
        }
    }
    EXPECT_TRUE(found_nonzero_landmass)
        << "No land tile has a nonzero landmass ID";
}

// Verify that different seeds produce different maps.
// Powered by Claude.
TEST_F(MapGeneratorTest, DifferentSeedsProduceDifferentMaps) {
    mom::MomWorld world1;
    mom::MomWorld world2;

    mom::MapGenParams params1;
    params1.seed = 1;
    mom::MapGenerator gen1(params1);
    gen1.generate(world1);

    mom::MapGenParams params2;
    params2.seed = 2;
    mom::MapGenerator gen2(params2);
    gen2.generate(world2);

    bool found_difference = false;
    for (int i = 0; i < 100 && !found_difference; ++i) {
        int x = i % mom::WORLD_WIDTH;
        int y = i / mom::WORLD_WIDTH;
        if (world1.get_terrain(x, y, mom::PLANE_ARCANUS)
            != world2.get_terrain(x, y, mom::PLANE_ARCANUS)) {
            found_difference = true;
        }
    }
    EXPECT_TRUE(found_difference)
        << "Seeds 1 and 2 produced identical terrain for the first 100 tiles";
}

// Verify that the same seed produces identical maps.
// Powered by Claude.
TEST_F(MapGeneratorTest, SameSeedProducesSameMap) {
    mom::MomWorld world1;
    mom::MomWorld world2;

    mom::MapGenParams seed_params;
    seed_params.seed = 42;

    mom::MapGenerator gen1(seed_params);
    gen1.generate(world1);

    mom::MapGenerator gen2(seed_params);
    gen2.generate(world2);

    for (int plane = 0; plane < mom::NUM_PLANES; ++plane) {
        for (int y = 0; y < mom::WORLD_HEIGHT; ++y) {
            for (int x = 0; x < mom::WORLD_WIDTH; ++x) {
                EXPECT_EQ(world1.get_terrain(x, y, plane),
                          world2.get_terrain(x, y, plane))
                    << "Mismatch at (" << x << ", " << y << ", " << plane
                    << ") with seed 42";
            }
        }
    }
}
