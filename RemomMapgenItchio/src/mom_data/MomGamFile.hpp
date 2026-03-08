#pragma once
// Binary .GAM file read/write for Master of Magic save game compatibility.
// Layout matches LOADSAVE.C in ReMoM exactly.
// Powered by Claude.

#include "mom_data/MomConstants.hpp"
#include "mom_data/MomEntities.hpp"
#include "mom_data/MomWorld.hpp"
#include <string>

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

} // namespace mom
