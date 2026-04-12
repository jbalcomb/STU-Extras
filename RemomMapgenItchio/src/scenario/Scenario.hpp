#pragma once
// Top-level scenario container holding all game data for a .mom_scenario file.
// Powered by Claude.

#include "mom_data/MomConstants.hpp"
#include "mom_data/MomEntities.hpp"
#include "mom_data/MomWorld.hpp"
#include <array>
#include <string>
#include <vector>

namespace mom {

// Scenario types.
// Powered by Claude.
enum class ScenarioType {
    SANDBOX,    // Map + starting setup, player takes it from there
    MISSION,    // Scripted mission with objectives and triggers
    PUZZLE      // Training puzzle (combat, city, spell exercises)
};

// Scenario metadata.
// Powered by Claude.
struct ScenarioMeta {
    std::string title{"Untitled Scenario"};
    std::string author;
    std::string description;
    std::string version{"1.0.0"};
    std::vector<std::string> tags;
};

// Top-level scenario container.
// Powered by Claude.
struct Scenario {
    static constexpr int FORMAT_VERSION = 1;

    ScenarioType type{ScenarioType::SANDBOX};
    ScenarioMeta meta;

    // Game settings
    GameData game_data;

    // Map generation parameters (editor-only, not written to .GAM)
    MapGenParams map_gen_params;

    // World terrain
    MomWorld world;

    // Entities (fixed-size arrays matching the original game)
    std::array<Wizard,   NUM_PLAYERS>    wizards{};
    std::array<City,     NUM_CITIES>     cities{};
    std::array<Unit,     NUM_UNITS>      units{};
    std::array<Node,     NUM_NODES>      nodes{};
    std::array<Lair,     NUM_LAIRS>      lairs{};
    std::array<Tower,    NUM_TOWERS>     towers{};
    std::array<Fortress, NUM_FORTRESSES> fortresses{};
    std::array<Item,     NUM_ITEMS>      items{};

    // Hero names table (35 heroes * 16 chars)
    char hero_names[NUM_HERO_TYPES][LEN_HERO_NAME]{};

    // Reset to a blank scenario.
    // Powered by Claude.
    void clear() {
        type = ScenarioType::SANDBOX;
        meta = ScenarioMeta{};
        game_data = GameData{};
        map_gen_params = MapGenParams{};
        world.clear();
        world.set_all_explored();
        wizards = {};
        cities = {};
        units = {};
        nodes = {};
        lairs = {};
        towers = {};
        fortresses = {};
        items = {};
        // Set all entity positions to (1,1,1) so nothing appears at (0,0).
        // Mark units/towers inactive with owner_idx=-1.
        // City slot 0 is required by ReMoM so leave owner_idx=0.
        // Powered by Claude.
        for (auto& c : cities)     { c.wx = 1; c.wy = 1; c.wp = 1; }
        for (auto& u : units)      { u.wx = 1; u.wy = 1; u.wp = 1; u.owner_idx = -1; }
        for (auto& t : towers)     { t.wx = 1; t.wy = 1; t.owner_idx = -1; }
        for (auto& f : fortresses) { f.wx = 1; f.wy = 1; f.wp = 1; }
        for (auto& n : nodes)      { n.wx = 1; n.wy = 1; n.wp = 1; n.owner_idx = -1; }
        for (auto& l : lairs)      { l.wx = 1; l.wy = 1; l.wp = 1; l.Intact = -1; }
        std::memset(hero_names, 0, sizeof(hero_names));
    }

    // Count active cities.
    // Powered by Claude.
    int count_active_cities() const {
        int count = 0;
        for (auto& c : cities) if (c.is_active()) ++count;
        return count;
    }

    // Count active units.
    // Powered by Claude.
    int count_active_units() const {
        int count = 0;
        for (auto& u : units) if (u.is_active()) ++count;
        return count;
    }
};

} // namespace mom
