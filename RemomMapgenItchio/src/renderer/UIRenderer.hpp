#pragma once
// Simple immediate-mode UI rendering helpers for panels, buttons, and text.
// Powered by Claude.

#include "renderer/Renderer.hpp"
#include <string>

namespace mom {

// Simple UI drawing helpers built on top of Renderer.
// Powered by Claude.
class UIRenderer {
public:
    // Draw a panel background.
    // Powered by Claude.
    static void draw_panel(Renderer& r, int x, int y, int w, int h,
                           uint8_t bg_r = 45, uint8_t bg_g = 45, uint8_t bg_b = 50);

    // Draw a button (highlighted if hovered or selected).
    // Powered by Claude.
    static void draw_button(Renderer& r, int x, int y, int w, int h,
                            bool selected, bool hovered);

    // Draw a label using colored rectangle (fallback for no font).
    // Powered by Claude.
    static void draw_label_bar(Renderer& r, int x, int y, int w,
                               uint8_t cr, uint8_t cg, uint8_t cb);

    // Draw a text label at the given position.
    // Falls back to draw_label_bar if font is not available.
    // Powered by Claude.
    static void draw_label(Renderer& r, int x, int y, const std::string& text,
                           uint8_t cr = 224, uint8_t cg = 224, uint8_t cb = 224);

    // Draw a button with a centered text label.
    // Powered by Claude.
    static void draw_button_with_label(Renderer& r, int x, int y, int w, int h,
                                       const std::string& label,
                                       bool selected, bool hovered);

    // Draw a horizontal separator line.
    // Powered by Claude.
    static void draw_separator(Renderer& r, int x, int y, int w);

    // Draw the status bar at the bottom of the window.
    // Powered by Claude.
    static void draw_status_bar(Renderer& r, int window_w, int window_h,
                                int cursor_wx, int cursor_wy, int plane,
                                float zoom);

    // Draw a temporary status message overlay above the status bar.
    // Powered by Claude.
    static void draw_status_message(Renderer& r, int window_w, int window_h,
                                    const std::string& message);
};

} // namespace mom
