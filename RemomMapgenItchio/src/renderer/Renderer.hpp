#pragma once
// SDL2 window, renderer, and camera/coordinate system.
// Powered by Claude.

#include "mom_data/MomConstants.hpp"
#include <SDL.h>
#include <SDL_ttf.h>
#include <cmath>
#include <string>
#include <unordered_map>

namespace mom {

// Camera state for scrolling and zooming the map viewport.
// Powered by Claude.
struct Camera {
    float offset_x{0.0f};  // scroll offset in pixels
    float offset_y{0.0f};
    float zoom{1.0f};      // 1.0 = default, higher = zoomed in
    int   tile_size{24};    // base tile width in pixels (height = tile_size * 0.9)

    // Get the rendered tile dimensions.
    // Powered by Claude.
    int tile_w() const { return static_cast<int>(tile_size * zoom); }
    int tile_h() const { return static_cast<int>(tile_size * 0.9f * zoom); }

    // Convert screen coordinates to world tile coordinates with horizontal wrapping.
    // Uses floor division so negative pixel offsets map to the correct tile.
    // Powered by Claude.
    void screen_to_world(int sx, int sy, int viewport_x, int viewport_y,
                         int& wx, int& wy) const {
        int tw = tile_w();
        int th = tile_h();
        if (tw <= 0 || th <= 0) { wx = -1; wy = -1; return; }
        float fx = (sx - viewport_x + offset_x) / static_cast<float>(tw);
        float fy = (sy - viewport_y + offset_y) / static_cast<float>(th);
        wx = static_cast<int>(std::floor(fx));
        wy = static_cast<int>(std::floor(fy));
        // Wrap x horizontally for seamless map.
        // Powered by Claude.
        wx = ((wx % WORLD_WIDTH) + WORLD_WIDTH) % WORLD_WIDTH;
        if (wy < 0 || wy >= WORLD_HEIGHT) {
            wx = -1; wy = -1;
        }
    }

    // Convert world tile coordinates to screen pixel position.
    // Powered by Claude.
    void world_to_screen(int wx, int wy, int viewport_x, int viewport_y,
                         int& sx, int& sy) const {
        sx = static_cast<int>(wx * tile_w() - offset_x) + viewport_x;
        sy = static_cast<int>(wy * tile_h() - offset_y) + viewport_y;
    }

    // Clamp scroll offset: wrap x horizontally, clamp y vertically.
    // Powered by Claude.
    void clamp(int viewport_w, int viewport_h) {
        int map_w = WORLD_WIDTH  * tile_w();
        int map_h = WORLD_HEIGHT * tile_h();
        // Wrap x horizontally for seamless scrolling.
        // Powered by Claude.
        float fmap_w = static_cast<float>(map_w);
        offset_x = std::fmod(offset_x, fmap_w);
        if (offset_x < 0) offset_x += fmap_w;
        // Clamp y vertically.
        // Powered by Claude.
        float max_y = static_cast<float>(map_h - viewport_h);
        if (max_y < 0) max_y = 0;
        if (offset_y < 0) offset_y = 0;
        if (offset_y > max_y) offset_y = max_y;
    }
};

// Main SDL2 renderer wrapper.
// Powered by Claude.
class Renderer {
public:
    // Initialize SDL2 and create window.
    // Powered by Claude.
    bool init(const std::string& title, int width = 1280, int height = 720);

    // Shutdown SDL2.
    // Powered by Claude.
    void shutdown();

    // Begin a new frame (clear screen).
    // Powered by Claude.
    void begin_frame();

    // End frame (present to screen).
    // Powered by Claude.
    void end_frame();

    // Draw a filled rectangle.
    // Powered by Claude.
    void draw_rect(int x, int y, int w, int h, uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);

    // Draw a rectangle outline.
    // Powered by Claude.
    void draw_rect_outline(int x, int y, int w, int h, uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);

    // Draw a line.
    // Powered by Claude.
    void draw_line(int x1, int y1, int x2, int y2, uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);

    // Draw text at the given position. Returns silently if font not loaded.
    // Powered by Claude.
    void draw_text(int x, int y, const std::string& text,
                   uint8_t r = 224, uint8_t g = 224, uint8_t b = 224);

    // Check if font rendering is available.
    // Powered by Claude.
    bool font_available() const { return font_available_; }

    SDL_Renderer* sdl_renderer() const { return renderer_; }
    SDL_Window*   sdl_window()   const { return window_; }
    int window_width()  const { return win_w_; }
    int window_height() const { return win_h_; }

    // Update cached window size (call on resize event).
    // Powered by Claude.
    void update_window_size();

    Camera camera;

private:
    SDL_Window*   window_   = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    TTF_Font*     font_     = nullptr;
    bool font_available_ = false;
    std::unordered_map<std::string, SDL_Texture*> text_cache_;
    int win_w_ = 1280;
    int win_h_ = 720;
};

} // namespace mom
