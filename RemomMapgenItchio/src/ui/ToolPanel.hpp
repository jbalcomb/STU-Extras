#pragma once
// Left-side tool palette panel for terrain types, specials, and entities.
// Powered by Claude.

#include "renderer/Renderer.hpp"
#include "editor/EditorState.hpp"

namespace mom {

// Tool palette panel rendered on the left side of the editor.
// Powered by Claude.
class ToolPanel {
public:
    static constexpr int WIDTH = 200;

    // Render the tool palette.
    // Powered by Claude.
    void render(Renderer& renderer, const EditorState& state, int window_h);

    // Handle mouse click in the tool panel. Returns true if click was consumed.
    // Powered by Claude.
    bool handle_click(int mx, int my, EditorState& state, int window_h);
};

} // namespace mom
