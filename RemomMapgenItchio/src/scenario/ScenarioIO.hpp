#pragma once
// JSON serialization for .mom_scenario files using nlohmann/json.
// Powered by Claude.

#include "scenario/Scenario.hpp"
#include <string>

namespace mom {

// Save a scenario to a .mom_scenario JSON file.
// Returns true on success.
// Powered by Claude.
bool save_scenario(const std::string& path, const Scenario& scenario);

// Load a scenario from a .mom_scenario JSON file.
// Returns true on success.
// Powered by Claude.
bool load_scenario(const std::string& path, Scenario& scenario);

} // namespace mom
