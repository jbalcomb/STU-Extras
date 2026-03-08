#pragma once
// Right-side property inspector panel for selected entities.
// Powered by Claude.

#include "renderer/Renderer.hpp"
#include "editor/EditorState.hpp"

namespace mom {

struct Scenario;

// Properties panel rendered on the right side of the editor.
// Powered by Claude.
class PropertiesPanel {
public:
    static constexpr int WIDTH = 250;

    // Render the properties panel.
    // Powered by Claude.
    void render(Renderer& renderer, const EditorState& state,
                const Scenario& scenario, int window_w, int window_h);
};

} // namespace mom
