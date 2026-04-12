// Tests for terrain smoothing validation rules.
// Powered by Claude.

#include <gtest/gtest.h>
#include "validation/SmoothingValidator.hpp"
#include "mom_data/MomWorld.hpp"
#include "mom_data/MomConstants.hpp"

// Test fixture for SmoothingValidator tests.
// Powered by Claude.
class SmoothingValidatorTest : public ::testing::Test {
protected:
    mom::MomWorld world;

    void SetUp() override {
        world.clear(); // All tiles start as TERRAIN_OCEAN (0)
    }
};

// Shore tile surrounded entirely by grassland should produce a violation.
// Powered by Claude.
TEST_F(SmoothingValidatorTest, ShoreNoWaterReturnsViolation) {
    world.set_terrain(5, 5, 0, mom::TERRAIN_SHORE);
    world.set_terrain(4, 5, 0, mom::TERRAIN_GRASSLAND);
    world.set_terrain(6, 5, 0, mom::TERRAIN_GRASSLAND);
    world.set_terrain(5, 4, 0, mom::TERRAIN_GRASSLAND);
    world.set_terrain(5, 6, 0, mom::TERRAIN_GRASSLAND);

    auto violations = mom::SmoothingValidator::validate_tile(world, 0, 5, 5);

    EXPECT_EQ(violations.size(), 1u);
    EXPECT_EQ(violations[0].rule, mom::RULE_SHORE_NO_WATER);
}

// Shore tile with at least one adjacent ocean tile should produce no violation.
// Powered by Claude.
TEST_F(SmoothingValidatorTest, ShoreWithAdjacentOceanNoViolation) {
    world.set_terrain(5, 5, 0, mom::TERRAIN_SHORE);
    world.set_terrain(5, 4, 0, mom::TERRAIN_GRASSLAND);
    world.set_terrain(5, 6, 0, mom::TERRAIN_GRASSLAND);
    world.set_terrain(4, 5, 0, mom::TERRAIN_OCEAN);
    world.set_terrain(6, 5, 0, mom::TERRAIN_GRASSLAND);

    auto violations = mom::SmoothingValidator::validate_tile(world, 0, 5, 5);

    EXPECT_EQ(violations.size(), 0u);
}

// River tile with no adjacent river, shore, ocean, or lake should produce a violation.
// Powered by Claude.
TEST_F(SmoothingValidatorTest, RiverIsolatedReturnsViolation) {
    world.set_terrain(10, 10, 0, mom::TERRAIN_RIVER);
    world.set_terrain(9,  10, 0, mom::TERRAIN_GRASSLAND);
    world.set_terrain(11, 10, 0, mom::TERRAIN_GRASSLAND);
    world.set_terrain(10,  9, 0, mom::TERRAIN_GRASSLAND);
    world.set_terrain(10, 11, 0, mom::TERRAIN_GRASSLAND);

    auto violations = mom::SmoothingValidator::validate_tile(world, 0, 10, 10);

    EXPECT_EQ(violations.size(), 1u);
    EXPECT_EQ(violations[0].rule, mom::RULE_RIVER_ISOLATED);
}

// River tile adjacent to another river should produce no violation.
// Powered by Claude.
TEST_F(SmoothingValidatorTest, RiverAdjacentToRiverNoViolation) {
    world.set_terrain(10, 10, 0, mom::TERRAIN_RIVER);
    world.set_terrain(10, 11, 0, mom::TERRAIN_RIVER);
    world.set_terrain(9,  10, 0, mom::TERRAIN_GRASSLAND);
    world.set_terrain(11, 10, 0, mom::TERRAIN_GRASSLAND);
    world.set_terrain(10,  9, 0, mom::TERRAIN_GRASSLAND);

    auto violations = mom::SmoothingValidator::validate_tile(world, 0, 10, 10);

    EXPECT_EQ(violations.size(), 0u);
}

// Volcano tile with no adjacent mountain or hill should produce a violation.
// Powered by Claude.
TEST_F(SmoothingValidatorTest, VolcanoIsolatedReturnsViolation) {
    world.set_terrain(20, 20, 0, mom::TERRAIN_VOLCANO);
    world.set_terrain(19, 20, 0, mom::TERRAIN_GRASSLAND);
    world.set_terrain(21, 20, 0, mom::TERRAIN_GRASSLAND);
    world.set_terrain(20, 19, 0, mom::TERRAIN_GRASSLAND);
    world.set_terrain(20, 21, 0, mom::TERRAIN_GRASSLAND);

    auto violations = mom::SmoothingValidator::validate_tile(world, 0, 20, 20);

    EXPECT_EQ(violations.size(), 1u);
    EXPECT_EQ(violations[0].rule, mom::RULE_VOLCANO_ISOLATED);
}

// Tundra tile adjacent to a desert tile should produce a violation.
// Powered by Claude.
TEST_F(SmoothingValidatorTest, TundraDesertReturnsViolation) {
    world.set_terrain(30, 30, 0, mom::TERRAIN_TUNDRA);
    world.set_terrain(31, 30, 0, mom::TERRAIN_DESERT);

    auto violations = mom::SmoothingValidator::validate_tile(world, 0, 30, 30);

    EXPECT_EQ(violations.size(), 1u);
    EXPECT_EQ(violations[0].rule, mom::RULE_TUNDRA_DESERT);
}

// Shore tile at x=0 with ocean at x=59 (wrap-around) should produce no violation.
// Powered by Claude.
TEST_F(SmoothingValidatorTest, WrapAroundCheck) {
    world.set_terrain(0,  5, 0, mom::TERRAIN_SHORE);
    world.set_terrain(59, 5, 0, mom::TERRAIN_OCEAN);
    world.set_terrain(1,  5, 0, mom::TERRAIN_GRASSLAND);
    world.set_terrain(0,  4, 0, mom::TERRAIN_GRASSLAND);
    world.set_terrain(0,  6, 0, mom::TERRAIN_GRASSLAND);

    auto violations = mom::SmoothingValidator::validate_tile(world, 0, 0, 5);

    EXPECT_EQ(violations.size(), 0u);
}

// Ocean square with no adjacent ocean (landlocked) should produce a violation.
// Powered by Claude.
TEST_F(SmoothingValidatorTest, OceanLandlockedReturnsViolation) {
    // Single ocean square surrounded by grassland on all 8 sides.
    world.set_terrain(15, 15, 0, mom::TERRAIN_OCEAN);
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            if (dx == 0 && dy == 0) continue;
            world.set_terrain(15 + dx, 15 + dy, 0, mom::TERRAIN_GRASSLAND);
        }
    }

    auto violations = mom::SmoothingValidator::validate_tile(world, 0, 15, 15);

    EXPECT_EQ(violations.size(), 1u);
    EXPECT_EQ(violations[0].rule, mom::RULE_OCEAN_LANDLOCKED);
}

// Ocean square with at least one adjacent ocean should produce no violation.
// Powered by Claude.
TEST_F(SmoothingValidatorTest, OceanWithOceanNeighborNoViolation) {
    world.set_terrain(15, 15, 0, mom::TERRAIN_OCEAN);
    world.set_terrain(16, 15, 0, mom::TERRAIN_OCEAN);
    // Other neighbors are land.
    world.set_terrain(14, 15, 0, mom::TERRAIN_GRASSLAND);
    world.set_terrain(15, 14, 0, mom::TERRAIN_GRASSLAND);
    world.set_terrain(15, 16, 0, mom::TERRAIN_GRASSLAND);

    auto violations = mom::SmoothingValidator::validate_tile(world, 0, 15, 15);

    EXPECT_EQ(violations.size(), 0u);
}

// validate_plane should collect violations from all tiles on the plane.
// Powered by Claude.
TEST_F(SmoothingValidatorTest, ValidatePlaneReturnsAllViolations) {
    // Shore at (5,5) with no adjacent water
    world.set_terrain(5, 5, 0, mom::TERRAIN_SHORE);
    world.set_terrain(4, 5, 0, mom::TERRAIN_GRASSLAND);
    world.set_terrain(6, 5, 0, mom::TERRAIN_GRASSLAND);
    world.set_terrain(5, 4, 0, mom::TERRAIN_GRASSLAND);
    world.set_terrain(5, 6, 0, mom::TERRAIN_GRASSLAND);

    // Isolated river at (10,10)
    world.set_terrain(10, 10, 0, mom::TERRAIN_RIVER);
    world.set_terrain(9,  10, 0, mom::TERRAIN_GRASSLAND);
    world.set_terrain(11, 10, 0, mom::TERRAIN_GRASSLAND);
    world.set_terrain(10,  9, 0, mom::TERRAIN_GRASSLAND);
    world.set_terrain(10, 11, 0, mom::TERRAIN_GRASSLAND);

    auto violations = mom::SmoothingValidator::validate_plane(world, 0);

    EXPECT_GE(violations.size(), 2u);
}
