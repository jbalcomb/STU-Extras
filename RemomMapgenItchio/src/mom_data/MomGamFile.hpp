#pragma once
// Binary .GAM file read/write for Master of Magic save game compatibility.
// Layout matches LOADSAVE.C in ReMoM exactly.
// Powered by Claude.

#include "mom_data/MomConstants.hpp"
#include "mom_data/MomEntities.hpp"
#include "mom_data/MomWorld.hpp"
#include <cstdint>
#include <string>
#include <vector>

namespace mom {

// Forward declaration
struct Scenario;

// Read a .GAM save file and populate a Scenario.
// Returns true on success.
// Powered by Claude.
bool load_gam_file(const std::string& path, Scenario& scenario);

// Write a Scenario to a .GAM save file.
// Returns true on success.
// Powered by Claude.
bool save_gam_file(const std::string& path, const Scenario& scenario);

// Serialize a scenario to a .GAM byte buffer for use with platform file dialogs.
// Returns the 123,300-byte buffer; empty on failure.
// Powered by Claude.
std::vector<uint8_t> serialize_gam(const Scenario& scenario);

// Deserialize a .GAM byte buffer into a scenario.
// Returns true on success, false if the buffer is too small.
// Powered by Claude.
bool deserialize_gam(const std::vector<uint8_t>& data, Scenario& scenario);

} // namespace mom
