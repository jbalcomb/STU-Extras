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

void UIRenderer::draw_label(Renderer& r, int x, int y, const std::string& text,
                             uint8_t cr, uint8_t cg, uint8_t cb) {
    if (r.font_available()) {
        r.draw_text(x, y, text, cr, cg, cb);
    } else {
        int w = static_cast<int>(text.size()) * 6;
        draw_label_bar(r, x, y, w, cr, cg, cb);
    }
}

void UIRenderer::draw_button_with_label(Renderer& r, int x, int y, int w, int h,
                                         const std::string& label,
                                         bool selected, bool hovered) {
    draw_button(r, x, y, w, h, selected, hovered);
    if (r.font_available()) {
        // Center text vertically in button, left-padded.
        // Powered by Claude.
        r.draw_text(x + 4, y + (h - 12) / 2, label, 224, 224, 224);
    } else {
        int bar_w = static_cast<int>(label.size()) * 6;
        if (bar_w > w - 8) bar_w = w - 8;
        draw_label_bar(r, x + 4, y, bar_w, 180, 180, 180);
    }
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

    // Coordinate text.
    // Powered by Claude.
    if (cursor_wx >= 0 && cursor_wy >= 0) {
        std::string coords = "X: " + std::to_string(cursor_wx)
                           + "  Y: " + std::to_string(cursor_wy);
        r.draw_text(8, bar_y + 6, coords, 180, 220, 180);
    }

    // Plane name text.
    // Powered by Claude.
    const char* plane_name = (plane == 0) ? "Arcanus" : "Myrror";
    uint8_t pr = (plane == 0) ? 120 : 200;
    uint8_t pg = (plane == 0) ? 200 : 120;
    uint8_t pb = (plane == 0) ? 120 : 200;
    r.draw_text(window_w / 2 - 24, bar_y + 6, plane_name, pr, pg, pb);

    // Zoom level text.
    // Powered by Claude.
    char zoom_buf[32];
    std::snprintf(zoom_buf, sizeof(zoom_buf), "Zoom: %.1fx", zoom);
    r.draw_text(window_w - 100, bar_y + 6, zoom_buf, 140, 140, 220);
}

void UIRenderer::draw_status_message(Renderer& r, int window_w, int window_h,
                                      const std::string& message) {
    // Draw a message overlay above the status bar with text.
    // Powered by Claude.
    int bar_h = 20;
    int bar_y = window_h - 24 - bar_h - 4;
    int msg_w = static_cast<int>(message.size()) * 7 + 16;
    if (msg_w > window_w - 16) msg_w = window_w - 16;
    if (msg_w < 60) msg_w = 60;

    // Semi-transparent dark background.
    // Powered by Claude.
    r.draw_rect(window_w / 2 - msg_w / 2 - 4, bar_y - 2,
                msg_w + 8, bar_h + 4, 30, 30, 35, 220);
    r.draw_rect_outline(window_w / 2 - msg_w / 2 - 4, bar_y - 2,
                        msg_w + 8, bar_h + 4, 200, 140, 30);
    r.draw_text(window_w / 2 - msg_w / 2 + 4, bar_y + 2, message, 240, 200, 80);
}

} // namespace mom
