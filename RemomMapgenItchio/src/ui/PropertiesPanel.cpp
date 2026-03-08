// Right-side property inspector panel.
// Powered by Claude.

#include "ui/PropertiesPanel.hpp"
#include "renderer/UIRenderer.hpp"
#include "renderer/MapRenderer.hpp"
#include "scenario/Scenario.hpp"

namespace mom {

void PropertiesPanel::render(Renderer& renderer, const EditorState& state,
                             const Scenario& scenario, int window_w, int window_h) {
    int panel_x = window_w - WIDTH;
    UIRenderer::draw_panel(renderer, panel_x, 0, WIDTH, window_h);

    int y = 8;
    int pad = 4;

    // Show selected tile info
    if (state.cursor_wx >= 0 && state.cursor_wy >= 0) {
        int plane = state.current_plane;
        uint16_t terrain_val = scenario.world.get_terrain(
            state.cursor_wx, state.cursor_wy, plane);
        int8_t special = scenario.world.get_special(
            state.cursor_wx, state.cursor_wy, plane);
        uint8_t flags = scenario.world.get_flags(
            state.cursor_wx, state.cursor_wy, plane);

        // Terrain type color indicator
        // (In Phase 1 without text rendering, we show colored bars)
        UIRenderer::draw_label_bar(renderer, panel_x + 8, y, WIDTH - 16, 180, 180, 180);
        y += 16;

        // Terrain color swatch
        auto tc = MapRenderer::terrain_color(
            static_cast<BaseTerrain>(terrain_val & 0xFF));
        renderer.draw_rect(panel_x + 8, y, 40, 20, tc.r, tc.g, tc.b);
        renderer.draw_rect_outline(panel_x + 8, y, 40, 20, 120, 120, 120);
        y += 28;

        // Special indicator
        if (special != TS_NONE) {
            auto sc = MapRenderer::special_color(static_cast<TerrainSpecial>(special));
            renderer.draw_rect(panel_x + 8, y, 20, 20, sc.r, sc.g, sc.b);
            renderer.draw_rect_outline(panel_x + 8, y, 20, 20, 120, 120, 120);
        }
        y += 28;

        // Flags indicator
        if (flags & MSF_ROAD) {
            renderer.draw_rect(panel_x + 8, y, 40, 8, 160, 140, 100);
        }
        y += 16;

        UIRenderer::draw_separator(renderer, panel_x + 8, y, WIDTH - 16);
        y += pad * 2;
    }

    // Show selected city info
    if (state.selected_city >= 0 && state.selected_city < NUM_CITIES) {
        const auto& c = scenario.cities[state.selected_city];
        if (c.is_active()) {
            auto bc = MapRenderer::banner_color(c.owner_idx);
            renderer.draw_rect(panel_x + 8, y, 30, 20, bc.r, bc.g, bc.b);
            y += 28;

            // Population indicator bar
            int pop_w = c.population * 8;
            if (pop_w > WIDTH - 16) pop_w = WIDTH - 16;
            renderer.draw_rect(panel_x + 8, y, pop_w, 12, 120, 200, 120);
            y += 20;

            // Buildings count indicator
            int bldg_w = c.bldg_cnt * 4;
            if (bldg_w > WIDTH - 16) bldg_w = WIDTH - 16;
            renderer.draw_rect(panel_x + 8, y, bldg_w, 12, 200, 160, 100);
            y += 20;
        }
    }

    // Minimap area at bottom
    int minimap_h = 80;
    int minimap_w = WIDTH - 16;
    int minimap_y = window_h - minimap_h - 32; // above status bar
    UIRenderer::draw_panel(renderer, panel_x + 8, minimap_y, minimap_w, minimap_h, 20, 20, 25);

    // Render minimap: each plane side by side
    int half_w = (minimap_w - 4) / 2;
    for (int p = 0; p < NUM_PLANES; ++p) {
        int mx = panel_x + 8 + p * (half_w + 4);
        int my = minimap_y;
        float sx_scale = static_cast<float>(half_w) / WORLD_WIDTH;
        float sy_scale = static_cast<float>(minimap_h) / WORLD_HEIGHT;

        // Sample every few tiles for minimap
        int step_x = WORLD_WIDTH / half_w + 1;
        int step_y = WORLD_HEIGHT / minimap_h + 1;
        if (step_x < 1) step_x = 1;
        if (step_y < 1) step_y = 1;

        for (int wy = 0; wy < WORLD_HEIGHT; wy += step_y) {
            for (int wx = 0; wx < WORLD_WIDTH; wx += step_x) {
                uint16_t tv = scenario.world.get_terrain(wx, wy, p);
                auto tc = MapRenderer::terrain_color(
                    static_cast<BaseTerrain>(tv & 0xFF));
                int px = mx + static_cast<int>(wx * sx_scale);
                int py = my + static_cast<int>(wy * sy_scale);
                int pw = static_cast<int>(step_x * sx_scale);
                int ph = static_cast<int>(step_y * sy_scale);
                if (pw < 1) pw = 1;
                if (ph < 1) ph = 1;
                renderer.draw_rect(px, py, pw, ph, tc.r, tc.g, tc.b);
            }
        }

        // Highlight active plane
        if (p == state.current_plane) {
            renderer.draw_rect_outline(mx, my, half_w, minimap_h, 255, 255, 0);
        } else {
            renderer.draw_rect_outline(mx, my, half_w, minimap_h, 80, 80, 80);
        }
    }
}

} // namespace mom
