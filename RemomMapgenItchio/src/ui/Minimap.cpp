// Minimap widget rendering and click handling.
// Shows both Arcanus and Myrror planes side-by-side with terrain colors.
// Powered by Claude.

#include "ui/Minimap.hpp"
#include "renderer/UIRenderer.hpp"
#include "renderer/MapRenderer.hpp"
#include "scenario/Scenario.hpp"

namespace mom {

// Render the minimap at the given position and size.
// Draws both planes side-by-side with terrain colors sampled from the world.
// Powered by Claude.
void Minimap::render(Renderer& renderer, const EditorState& state,
                     const Scenario& scenario,
                     int x, int y, int w, int h) {
    UIRenderer::draw_panel(renderer, x, y, w, h, 20, 20, 25);

    // Each plane gets half the width, with a small gap between them.
    int half_w = (w - 4) / 2;

    for (int p = 0; p < NUM_PLANES; ++p) {
        int mx = x + p * (half_w + 4);
        int my = y;
        float sx_scale = static_cast<float>(half_w) / WORLD_WIDTH;
        float sy_scale = static_cast<float>(h) / WORLD_HEIGHT;

        // Sample every few tiles for minimap.
        // Powered by Claude.
        int step_x = WORLD_WIDTH / half_w + 1;
        int step_y = WORLD_HEIGHT / h + 1;
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

        // Highlight the active plane with a yellow outline.
        // Powered by Claude.
        if (p == state.current_plane) {
            renderer.draw_rect_outline(mx, my, half_w, h, 255, 255, 0);
        } else {
            renderer.draw_rect_outline(mx, my, half_w, h, 80, 80, 80);
        }
    }
}

// Handle a mouse click within the minimap area.
// Determines which plane and world coordinate was clicked, then updates the
// camera offset to center on that location. Returns true if consumed.
// Powered by Claude.
bool Minimap::handle_click(int mx, int my, Camera& camera,
                           EditorState& state, const Scenario& scenario,
                           int minimap_x, int minimap_y,
                           int minimap_w, int minimap_h) {
    // Check if click is within the minimap bounding box.
    if (mx < minimap_x || mx >= minimap_x + minimap_w ||
        my < minimap_y || my >= minimap_y + minimap_h) {
        return false;
    }

    int half_w = (minimap_w - 4) / 2;

    for (int p = 0; p < NUM_PLANES; ++p) {
        int plane_x = minimap_x + p * (half_w + 4);

        if (mx >= plane_x && mx < plane_x + half_w) {
            // Compute world coordinates from the click position within this plane.
            // Powered by Claude.
            float sx_scale = static_cast<float>(half_w) / WORLD_WIDTH;
            float sy_scale = static_cast<float>(minimap_h) / WORLD_HEIGHT;

            int wx = static_cast<int>((mx - plane_x) / sx_scale);
            int wy = static_cast<int>((my - minimap_y) / sy_scale);

            // Clamp to world bounds.
            if (wx < 0) wx = 0;
            if (wx >= WORLD_WIDTH) wx = WORLD_WIDTH - 1;
            if (wy < 0) wy = 0;
            if (wy >= WORLD_HEIGHT) wy = WORLD_HEIGHT - 1;

            // Switch to the clicked plane.
            state.current_plane = p;

            // Center the camera on the clicked world tile.
            // Powered by Claude.
            camera.offset_x = static_cast<float>(wx * camera.tile_w());
            camera.offset_y = static_cast<float>(wy * camera.tile_h());

            return true;
        }
    }

    return false;
}

} // namespace mom
