#pragma once
// Wizard setup panel for editing wizard race, portrait, spell ranks, and retorts.
// Powered by Claude.

#include "renderer/Renderer.hpp"
#include "editor/EditorState.hpp"
#include "editor/UndoStack.hpp"

namespace mom {

struct Scenario;

// Wizard panel rendered below the tool panel when WIZARDS tool is active.
// Powered by Claude.
class WizardPanel {
public:
    static constexpr int WIDTH = 200;

    // Render the wizard panel.
    // Powered by Claude.
    void render(Renderer& renderer, EditorState& state, Scenario& scenario, int window_h);

    // Handle mouse click in the wizard panel. Returns true if click was consumed.
    // window_w is needed to compute the map area bounds.
    // Powered by Claude.
    bool handle_click(int mx, int my, EditorState& state, Scenario& scenario, UndoStack& undo, int window_w, int window_h);

private:
    // Whether the race dropdown list is currently open.
    // Powered by Claude.
    bool race_dropdown_open_ = false;

    // Y position where the race dropdown should render (set during render).
    // Powered by Claude.
    int race_dropdown_y_ = 0;
};

} // namespace mom
