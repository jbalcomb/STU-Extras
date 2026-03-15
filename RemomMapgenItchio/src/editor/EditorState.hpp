#pragma once
// Editor state: current tool, cursor, selection, and active plane.
// Powered by Claude.

#include "mom_data/MomConstants.hpp"
#include <cstdint>
#include <string>
#include <vector>

namespace mom {

// Available editor tools.
// Powered by Claude.
enum class EditorTool {
    SELECT,
    PAINT_TERRAIN,
    PAINT_SPECIAL,
    PAINT_FLAGS,
    PLACE_CITY,
    PLACE_UNIT,
    PLACE_NODE,
    PLACE_LAIR,
    PLACE_TOWER,
    PLACE_FORTRESS,
    ERASE,
    SETTINGS,
    WIZARDS
};

// Editor state shared between UI, input handling, and rendering.
// Powered by Claude.
struct EditorState {
    // Current tool
    EditorTool tool{EditorTool::PAINT_TERRAIN};

    // Terrain painting
    BaseTerrain paint_terrain{TERRAIN_GRASSLAND};
    TerrainSpecial paint_special{TS_NONE};

    // Flag painting (default = MSF_ROAD).
    // Powered by Claude.
    uint8_t paint_flag{0x08};

    // Entity placement defaults
    int8_t  place_owner{0};
    uint8_t place_unit_type{35}; // first non-hero unit
    int8_t  place_race{RACE_HIGH_MAN};
    int8_t  place_node_type{NODE_NATURE};

    // Current plane being edited
    int current_plane{PLANE_ARCANUS};

    // Cursor position in world coordinates (-1 = off map)
    int cursor_wx{-1};
    int cursor_wy{-1};

    // Currently selected entity index (-1 = none)
    int selected_city{-1};
    int selected_unit{-1};
    int selected_node{-1};
    int selected_lair{-1};
    int selected_tower{-1};
    int selected_fortress{-1};

    // Status message bar for slot limit feedback and notifications.
    // Powered by Claude.
    std::string status_message;
    float status_timer{0.0f}; // seconds remaining

    // Map generation parameters
    MapGenParams map_gen_params;
    bool generate_requested{false};

    // Smoothing validation
    std::vector<SmoothingViolation> violations;
    bool show_all_violations{false};

    // Grid display
    bool show_grid{true};

    // Wizard panel tab selection
    int wizard_tab{0};

    // Wizard generation: per-wizard dirty flag (session-only, resets on save/load).
    // Powered by Claude.
    bool wizard_dirty[NUM_PLAYERS]{};

    // Wizard generation: modal confirmation dialog state.
    // Powered by Claude.
    bool wizard_gen_confirm_open{};
    bool wizard_gen_confirm_global{};
    int  wizard_gen_confirm_idx{0};

    // Wizard generation: request flag (set by ToolPanel, consumed by main loop).
    // Powered by Claude.
    bool generate_wizards_requested{false};

    // Wizard generation: confirmed flag (set by modal dialog, consumed by main loop).
    // Powered by Claude.
    bool wizard_gen_confirmed{false};

    // .GAM export: request flag (set by ToolPanel button, consumed by main loop).
    // Powered by Claude.
    bool export_gam_requested{false};

    // Toggle active plane.
    // Powered by Claude.
    void toggle_plane() {
        current_plane = (current_plane == PLANE_ARCANUS) ? PLANE_MYRROR : PLANE_ARCANUS;
    }

    // Clear selection.
    // Powered by Claude.
    void deselect() {
        selected_city = -1;
        selected_unit = -1;
        selected_node = -1;
        selected_lair = -1;
        selected_tower = -1;
        selected_fortress = -1;
    }

    // Set a temporary status message with a display duration.
    // Powered by Claude.
    void set_status(const std::string& msg, float duration = 3.0f) {
        status_message = msg;
        status_timer = duration;
    }
};

} // namespace mom
