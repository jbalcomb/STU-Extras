// Left-side tool palette rendering and interaction.
// Powered by Claude.

#include "ui/ToolPanel.hpp"
#include "renderer/UIRenderer.hpp"
#include "renderer/MapRenderer.hpp"

namespace mom {

void ToolPanel::render(Renderer& renderer, const EditorState& state, int window_h) {
    // Panel background
    UIRenderer::draw_panel(renderer, 0, 0, WIDTH, window_h);

    int y = 8;
    int btn_w = WIDTH - 16;
    int btn_h = 20;
    int pad = 4;

    // Tool mode buttons
    struct ToolBtn {
        EditorTool tool;
        uint8_t r, g, b;
    };
    ToolBtn tool_buttons[] = {
        {EditorTool::SELECT,         180, 180, 180},
        {EditorTool::PAINT_TERRAIN,  100, 200, 100},
        {EditorTool::PAINT_SPECIAL,  200, 200, 100},
        {EditorTool::PLACE_CITY,     200, 150, 100},
        {EditorTool::PLACE_UNIT,     100, 150, 200},
        {EditorTool::PLACE_NODE,     150, 100, 200},
        {EditorTool::PLACE_LAIR,     200, 100, 100},
        {EditorTool::ERASE,          150, 150, 150},
    };

    for (auto& tb : tool_buttons) {
        bool sel = (state.tool == tb.tool);
        UIRenderer::draw_button(renderer, 8, y, btn_w, btn_h, sel, false);
        UIRenderer::draw_label_bar(renderer, 12, y, btn_w - 8, tb.r, tb.g, tb.b);
        y += btn_h + pad;
    }

    y += pad;
    UIRenderer::draw_separator(renderer, 8, y, btn_w);
    y += pad * 2;

    // Terrain type palette (only when terrain tool is active)
    if (state.tool == EditorTool::PAINT_TERRAIN) {
        int swatch_sz = 22;
        int cols = (WIDTH - 16) / (swatch_sz + 2);
        int col = 0;
        int sx = 8;
        for (int t = 0; t < BASE_TERRAIN_COUNT; ++t) {
            auto tc = MapRenderer::terrain_color(static_cast<BaseTerrain>(t));
            bool sel = (state.paint_terrain == static_cast<BaseTerrain>(t));
            renderer.draw_rect(sx, y, swatch_sz, swatch_sz, tc.r, tc.g, tc.b);
            if (sel) {
                renderer.draw_rect_outline(sx, y, swatch_sz, swatch_sz, 255, 255, 255);
            }
            sx += swatch_sz + 2;
            if (++col >= cols) {
                col = 0;
                sx = 8;
                y += swatch_sz + 2;
            }
        }
        if (col > 0) y += swatch_sz + 2;
    }

    // Terrain special palette (only when special tool is active)
    if (state.tool == EditorTool::PAINT_SPECIAL) {
        TerrainSpecial specials[] = {
            TS_NONE, TS_IRON, TS_COAL, TS_SILVER, TS_GOLD, TS_GEMS,
            TS_MITHRIL, TS_ADAMANTIUM, TS_QUORK_CRYSTALS, TS_CRYSX_CRYSTALS,
            TS_WILD_GAME, TS_NIGHTSHADE
        };
        int swatch_sz = 22;
        int cols = (WIDTH - 16) / (swatch_sz + 2);
        int col = 0;
        int sx = 8;
        for (auto sp : specials) {
            auto sc = MapRenderer::special_color(sp);
            if (sp == TS_NONE) { sc = {60, 60, 60}; }
            bool sel = (state.paint_special == sp);
            renderer.draw_rect(sx, y, swatch_sz, swatch_sz, sc.r, sc.g, sc.b);
            if (sel) {
                renderer.draw_rect_outline(sx, y, swatch_sz, swatch_sz, 255, 255, 255);
            }
            sx += swatch_sz + 2;
            if (++col >= cols) {
                col = 0;
                sx = 8;
                y += swatch_sz + 2;
            }
        }
    }
}

bool ToolPanel::handle_click(int mx, int my, EditorState& state, int window_h) {
    if (mx < 0 || mx >= WIDTH) return false;

    int y = 8;
    int btn_h = 20;
    int pad = 4;

    // Check tool mode buttons
    EditorTool tools[] = {
        EditorTool::SELECT, EditorTool::PAINT_TERRAIN, EditorTool::PAINT_SPECIAL,
        EditorTool::PLACE_CITY, EditorTool::PLACE_UNIT, EditorTool::PLACE_NODE,
        EditorTool::PLACE_LAIR, EditorTool::ERASE
    };

    for (auto t : tools) {
        if (my >= y && my < y + btn_h) {
            state.tool = t;
            return true;
        }
        y += btn_h + pad;
    }

    y += pad + pad * 2; // separator

    // Terrain type swatches
    if (state.tool == EditorTool::PAINT_TERRAIN) {
        int swatch_sz = 22;
        int cols = (WIDTH - 16) / (swatch_sz + 2);
        for (int t = 0; t < BASE_TERRAIN_COUNT; ++t) {
            int col = t % cols;
            int row = t / cols;
            int sx = 8 + col * (swatch_sz + 2);
            int sy = y + row * (swatch_sz + 2);
            if (mx >= sx && mx < sx + swatch_sz && my >= sy && my < sy + swatch_sz) {
                state.paint_terrain = static_cast<BaseTerrain>(t);
                return true;
            }
        }
    }

    // Special swatches
    if (state.tool == EditorTool::PAINT_SPECIAL) {
        TerrainSpecial specials[] = {
            TS_NONE, TS_IRON, TS_COAL, TS_SILVER, TS_GOLD, TS_GEMS,
            TS_MITHRIL, TS_ADAMANTIUM, TS_QUORK_CRYSTALS, TS_CRYSX_CRYSTALS,
            TS_WILD_GAME, TS_NIGHTSHADE
        };
        int swatch_sz = 22;
        int cols = (WIDTH - 16) / (swatch_sz + 2);
        int num_specials = sizeof(specials) / sizeof(specials[0]);
        for (int i = 0; i < num_specials; ++i) {
            int col = i % cols;
            int row = i / cols;
            int sx = 8 + col * (swatch_sz + 2);
            int sy = y + row * (swatch_sz + 2);
            if (mx >= sx && mx < sx + swatch_sz && my >= sy && my < sy + swatch_sz) {
                state.paint_special = specials[i];
                return true;
            }
        }
    }

    return false;
}

} // namespace mom
