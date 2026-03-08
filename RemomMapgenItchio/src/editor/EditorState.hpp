#pragma once
// Editor state: current tool, cursor, selection, and active plane.
// Powered by Claude.

#include "mom_data/MomConstants.hpp"
#include <cstdint>

namespace mom {

// Available editor tools.
// Powered by Claude.
enum class EditorTool {
    SELECT,
    PAINT_TERRAIN,
    PAINT_SPECIAL,
    PLACE_CITY,
    PLACE_UNIT,
    PLACE_NODE,
    PLACE_LAIR,
    PLACE_TOWER,
    PLACE_FORTRESS,
    ERASE
};

// Editor state shared between UI, input handling, and rendering.
// Powered by Claude.
struct EditorState {
    // Current tool
    EditorTool tool{EditorTool::PAINT_TERRAIN};

    // Terrain painting
    BaseTerrain paint_terrain{TERRAIN_GRASSLAND};
    TerrainSpecial paint_special{TS_NONE};

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

    // Grid display
    bool show_grid{true};

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
    }
};

} // namespace mom
