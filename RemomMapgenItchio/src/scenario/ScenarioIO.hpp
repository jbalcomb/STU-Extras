#pragma once
// JSON serialization for .mom_scenario files using nlohmann/json.
// Powered by Claude.

#include "scenario/Scenario.hpp"
#include <cstdint>
#include <string>
#include <vector>

namespace mom {

// Save a scenario to a .mom_scenario JSON file.
// Returns true on success.
// Powered by Claude.
bool save_scenario(const std::string& path, const Scenario& scenario);

// Load a scenario from a .mom_scenario JSON file.
// Returns true on success.
// Powered by Claude.
bool load_scenario(const std::string& path, Scenario& scenario);

// Serialize a scenario to a JSON byte buffer for use with platform file dialogs.
// Returns the serialized bytes; empty on failure.
// Powered by Claude.
std::vector<uint8_t> serialize_scenario(const Scenario& scenario);

// Deserialize a scenario from a JSON byte buffer loaded via platform file dialogs.
// Returns true on success.
// Powered by Claude.
bool deserialize_scenario(const std::vector<uint8_t>& data, Scenario& scenario);

} // namespace mom
