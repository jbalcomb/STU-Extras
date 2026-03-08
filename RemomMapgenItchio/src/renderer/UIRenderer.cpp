// Simple immediate-mode UI rendering.
// Powered by Claude.

#include "renderer/UIRenderer.hpp"

namespace mom {

void UIRenderer::draw_panel(Renderer& r, int x, int y, int w, int h,
                            uint8_t bg_r, uint8_t bg_g, uint8_t bg_b) {
    r.draw_rect(x, y, w, h, bg_r, bg_g, bg_b);
    r.draw_rect_outline(x, y, w, h, 80, 80, 85);
}

void UIRenderer::draw_button(Renderer& r, int x, int y, int w, int h,
                             bool selected, bool hovered) {
    if (selected) {
        r.draw_rect(x, y, w, h, 70, 130, 180);
    } else if (hovered) {
        r.draw_rect(x, y, w, h, 60, 60, 70);
    } else {
        r.draw_rect(x, y, w, h, 50, 50, 55);
    }
    r.draw_rect_outline(x, y, w, h, 90, 90, 95);
}

void UIRenderer::draw_label_bar(Renderer& r, int x, int y, int w,
                                uint8_t cr, uint8_t cg, uint8_t cb) {
    r.draw_rect(x, y + 4, w, 4, cr, cg, cb);
}

void UIRenderer::draw_separator(Renderer& r, int x, int y, int w) {
    r.draw_line(x, y, x + w, y, 80, 80, 85);
}

void UIRenderer::draw_status_bar(Renderer& r, int window_w, int window_h,
                                  int cursor_wx, int cursor_wy, int plane,
                                  float zoom) {
    int bar_h = 24;
    int bar_y = window_h - bar_h;
    r.draw_rect(0, bar_y, window_w, bar_h, 35, 35, 40);
    r.draw_line(0, bar_y, window_w, bar_y, 80, 80, 85);

    // Coordinate indicator (colored blocks as placeholder for text)
    if (cursor_wx >= 0 && cursor_wy >= 0) {
        // X coordinate indicator
        r.draw_rect(8, bar_y + 6, 12, 12, 120, 180, 120);
        // Y coordinate indicator
        r.draw_rect(28, bar_y + 6, 12, 12, 180, 120, 120);
    }

    // Plane indicator
    if (plane == 0) {
        r.draw_rect(window_w / 2 - 20, bar_y + 6, 40, 12, 80, 160, 80); // green=Arcanus
    } else {
        r.draw_rect(window_w / 2 - 20, bar_y + 6, 40, 12, 160, 80, 160); // purple=Myrror
    }

    // Zoom indicator
    int zoom_w = static_cast<int>(zoom * 20);
    r.draw_rect(window_w - 60, bar_y + 6, zoom_w, 12, 100, 100, 200);
}

} // namespace mom
