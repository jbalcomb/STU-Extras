#pragma once
// Minimap widget showing both Arcanus and Myrror planes side-by-side.
// Powered by Claude.

#include "renderer/Renderer.hpp"
#include "editor/EditorState.hpp"

namespace mom {

struct Scenario;

// Standalone minimap component displayed at the bottom of the properties panel.
// Shows both planes side-by-side with the active plane highlighted.
// Powered by Claude.
class Minimap {
public:
    // Render the minimap at the given position and size.
    // Draws both planes side-by-side with terrain colors.
    // Powered by Claude.
    void render(Renderer& renderer, const EditorState& state,
                const Scenario& scenario,
                int x, int y, int w, int h);

    // Handle a mouse click within the minimap area.
    // If the click falls inside either plane, jumps the camera to the
    // corresponding world location. Returns true if the click was consumed.
    // Powered by Claude.
    bool handle_click(int mx, int my, Camera& camera,
                      EditorState& state, const Scenario& scenario,
                      int minimap_x, int minimap_y, int minimap_w, int minimap_h);
};

} // namespace mom
