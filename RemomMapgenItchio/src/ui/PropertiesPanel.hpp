#pragma once
// Right-side property inspector panel for selected entities.
// Powered by Claude.

#include "renderer/Renderer.hpp"
#include "editor/EditorState.hpp"
#include "editor/UndoStack.hpp"
#include "ui/Minimap.hpp"

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

    // Handle a mouse click in the properties panel for interactive editing.
    // Cycles entity properties (race, population, unit type, node type)
    // when the user clicks on the corresponding display areas.
    // Powered by Claude.
    void handle_click(int mx, int my, EditorState& state,
                      Scenario& scenario, UndoStack& undo);

    // Handle a mouse click in the minimap area of the properties panel.
    // Delegates to the Minimap component for camera jump behavior.
    // Returns true if the click was consumed.
    // Powered by Claude.
    bool handle_minimap_click(int mx, int my, Camera& camera,
                              EditorState& state, const Scenario& scenario,
                              int window_w, int window_h) {
        int panel_x = window_w - WIDTH;
        int minimap_h = 80;
        int minimap_w = WIDTH - 16;
        int minimap_y = window_h - minimap_h - 32;
        return minimap_.handle_click(mx, my, camera, state, scenario,
                                     panel_x + 8, minimap_y, minimap_w, minimap_h);
    }

private:
    Minimap minimap_;
};

} // namespace mom
