// SDL2 renderer implementation.
// Powered by Claude.

#include "renderer/Renderer.hpp"
#include <cstdio>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

namespace mom {

bool Renderer::init(const std::string& title, int width, int height) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0)
        return false;

    // Under Emscripten, size the canvas to device-pixel resolution so the
    // rendering buffer matches the physical display 1:1. Without this,
    // window.innerWidth returns CSS pixels which may be smaller than the
    // actual screen, causing the browser to stretch a small canvas.
    // Minimum 1280x720 — targets desktop users at 1920x1080.
    // Powered by Claude.
#ifdef __EMSCRIPTEN__
    double dpr = EM_ASM_DOUBLE({ return window.devicePixelRatio || 1.0; });
    width  = static_cast<int>(EM_ASM_INT({ return window.innerWidth;  }) * dpr);
    height = static_cast<int>(EM_ASM_INT({ return window.innerHeight; }) * dpr);
    if (width  < 1280) width  = 1280;
    if (height < 720)  height = 720;
#endif

    window_ = SDL_CreateWindow(
        title.c_str(),
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN
    );
    if (!window_) return false;

    renderer_ = SDL_CreateRenderer(window_, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer_) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
        return false;
    }

    // Under Emscripten, SDL_GetWindowSize returns CSS display size which may
    // differ from the backing buffer. Use renderer output size for layout.
    // Powered by Claude.
#ifdef __EMSCRIPTEN__
    SDL_GetRendererOutputSize(renderer_, &win_w_, &win_h_);
#else
    win_w_ = width;
    win_h_ = height;
#endif

    // Initialize SDL_ttf and load the UI font.
    // Powered by Claude.
    if (TTF_Init() == 0) {
#ifdef __EMSCRIPTEN__
        const char* font_path = "/assets/Cousine-Regular.ttf";
#else
        const char* font_path = "assets/Cousine-Regular.ttf";
#endif
        font_ = TTF_OpenFont(font_path, 12);
        if (font_) {
            font_available_ = true;
        } else {
            std::fprintf(stderr, "Warning: Could not load font '%s': %s\n",
                         font_path, TTF_GetError());
        }
    } else {
        std::fprintf(stderr, "Warning: TTF_Init failed: %s\n", TTF_GetError());
    }

    return true;
}

void Renderer::shutdown() {
    // Clean up text texture cache.
    // Powered by Claude.
    for (auto& pair : text_cache_) {
        SDL_DestroyTexture(pair.second);
    }
    text_cache_.clear();

    if (font_)     { TTF_CloseFont(font_); font_ = nullptr; }
    font_available_ = false;
    TTF_Quit();

    if (renderer_) { SDL_DestroyRenderer(renderer_); renderer_ = nullptr; }
    if (window_)   { SDL_DestroyWindow(window_);     window_ = nullptr; }
    SDL_Quit();
}

void Renderer::begin_frame() {
    SDL_SetRenderDrawColor(renderer_, 30, 30, 30, 255);
    SDL_RenderClear(renderer_);
}

void Renderer::end_frame() {
    SDL_RenderPresent(renderer_);
}

void Renderer::draw_rect(int x, int y, int w, int h,
                          uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    SDL_SetRenderDrawColor(renderer_, r, g, b, a);
    SDL_Rect rect{x, y, w, h};
    SDL_RenderFillRect(renderer_, &rect);
}

void Renderer::draw_rect_outline(int x, int y, int w, int h,
                                  uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    SDL_SetRenderDrawColor(renderer_, r, g, b, a);
    SDL_Rect rect{x, y, w, h};
    SDL_RenderDrawRect(renderer_, &rect);
}

void Renderer::draw_line(int x1, int y1, int x2, int y2,
                          uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    SDL_SetRenderDrawColor(renderer_, r, g, b, a);
    SDL_RenderDrawLine(renderer_, x1, y1, x2, y2);
}

void Renderer::draw_text(int x, int y, const std::string& text,
                          uint8_t r, uint8_t g, uint8_t b) {
    if (!font_available_ || text.empty()) return;

    // Build cache key from text + color.
    // Powered by Claude.
    std::string key = text + '\0' + static_cast<char>(r)
                     + static_cast<char>(g) + static_cast<char>(b);

    // Evict cache when it grows too large (dynamic text like coordinates
    // creates new entries every mouse move).
    // Powered by Claude.
    if (text_cache_.size() > 256) {
        for (auto& pair : text_cache_) {
            SDL_DestroyTexture(pair.second);
        }
        text_cache_.clear();
    }

    SDL_Texture* tex = nullptr;
    auto it = text_cache_.find(key);
    if (it != text_cache_.end()) {
        tex = it->second;
    } else {
        SDL_Color color{r, g, b, 255};
        SDL_Surface* surf = TTF_RenderText_Blended(font_, text.c_str(), color);
        if (!surf) return;
        tex = SDL_CreateTextureFromSurface(renderer_, surf);
        SDL_FreeSurface(surf);
        if (!tex) return;
        text_cache_[key] = tex;
    }

    int tw = 0, th = 0;
    SDL_QueryTexture(tex, nullptr, nullptr, &tw, &th);
    SDL_Rect dst{x, y, tw, th};
    SDL_RenderCopy(renderer_, tex, nullptr, &dst);
}

void Renderer::update_window_size() {
    // Under Emscripten, SDL_GetWindowSize returns CSS display size which may
    // differ from the backing buffer. Use renderer output size for layout.
    // Powered by Claude.
#ifdef __EMSCRIPTEN__
    SDL_GetRendererOutputSize(renderer_, &win_w_, &win_h_);
#else
    SDL_GetWindowSize(window_, &win_w_, &win_h_);
#endif
    // DEBUG: log resize. Powered by Claude.
    int render_w = 0, render_h = 0;
    SDL_GetRendererOutputSize(renderer_, &render_w, &render_h);
    std::fprintf(stderr, "[DEBUG] resize: window=%dx%d renderer=%dx%d\n",
                 win_w_, win_h_, render_w, render_h);
#ifdef __EMSCRIPTEN__
    EM_ASM({
        var c = document.getElementById('canvas');
        console.log('[DEBUG] resize: SDL window=' + $0 + 'x' + $1 +
            ' renderer=' + $2 + 'x' + $3 +
            ' canvas=' + c.width + 'x' + c.height +
            ' CSS=' + c.clientWidth + 'x' + c.clientHeight);
    }, win_w_, win_h_, render_w, render_h);
#endif
}

} // namespace mom
