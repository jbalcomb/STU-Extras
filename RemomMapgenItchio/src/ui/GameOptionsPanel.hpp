#pragma once
// Game options panel for difficulty, magic level, land size, and wizard count.
// Powered by Claude.

#include "renderer/Renderer.hpp"
#include "editor/EditorState.hpp"
#include "editor/UndoStack.hpp"

namespace mom {

struct Scenario;

// Game options panel rendered below the tool panel when SETTINGS tool is active.
// Powered by Claude.
class GameOptionsPanel {
public:
    static constexpr int WIDTH = 200;

    // Render the game options panel.
    // Powered by Claude.
    void render(Renderer& renderer, EditorState& state, Scenario& scenario, int window_h);

    // Handle mouse click in the game options panel. Returns true if click was consumed.
    // Powered by Claude.
    bool handle_click(int mx, int my, EditorState& state, Scenario& scenario, UndoStack& undo);
};

} // namespace mom
