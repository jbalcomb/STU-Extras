#pragma once
// Renders the 60x40 tile map grid with terrain colors and entity icons.
// Powered by Claude.

#include "renderer/Renderer.hpp"
#include "mom_data/MomConstants.hpp"

namespace mom {

struct Scenario;
struct EditorState;

// Color for terrain rendering.
// Powered by Claude.
struct TerrainColor {
    uint8_t r, g, b;
};

// Map tile renderer using color-coded rectangles.
// Powered by Claude.
class MapRenderer {
public:
    // Render the map viewport area.
    // Powered by Claude.
    void render(Renderer& renderer, const Scenario& scenario,
                const EditorState& state, const SDL_Rect& viewport);

    // Get the terrain color for a base terrain type.
    // Powered by Claude.
    static TerrainColor terrain_color(BaseTerrain type);

    // Get the color for a terrain special overlay.
    // Powered by Claude.
    static TerrainColor special_color(TerrainSpecial special);

    // Banner colors for the 6 player slots.
    // Powered by Claude.
    static TerrainColor banner_color(int owner_idx);
};

} // namespace mom
