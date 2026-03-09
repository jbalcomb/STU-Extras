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
        {EditorTool::PAINT_FLAGS,    160, 120,  80},
        {EditorTool::PLACE_CITY,     200, 150, 100},
        {EditorTool::PLACE_UNIT,     100, 150, 200},
        {EditorTool::PLACE_NODE,     150, 100, 200},
        {EditorTool::PLACE_LAIR,     200, 100, 100},
        {EditorTool::PLACE_TOWER,    200, 200, 220},
        {EditorTool::PLACE_FORTRESS, 220, 180, 100},
        {EditorTool::ERASE,          150, 150, 150},
        {EditorTool::SETTINGS,       200, 180,  60},
        {EditorTool::WIZARDS,        180, 100, 200},
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

    // --- Map Generation Controls ---
    y += pad;
    UIRenderer::draw_separator(renderer, 8, y, btn_w);
    y += pad * 2;

    // Land proportion bar (colored gradient, labeled by percentage).
    // Powered by Claude.
    {
        int bar_w = btn_w;
        int bar_h = 16;
        int pct = static_cast<int>(state.map_gen_params.land_proportion * 100.0f);
        int fill_w = static_cast<int>(bar_w * state.map_gen_params.land_proportion);
        renderer.draw_rect(8, y, bar_w, bar_h, 30, 60, 120);    // ocean bg
        renderer.draw_rect(8, y, fill_w, bar_h, 80, 160, 80);   // land fill
        y += bar_h + pad;
    }

    // Roughness selector (3 colored blocks).
    // Powered by Claude.
    {
        int block_w = (btn_w - 4) / 3;
        int block_h = 16;
        uint8_t colors[][3] = {{80,180,80}, {180,180,80}, {180,80,80}};
        for (int i = 0; i < 3; ++i) {
            int bx = 8 + i * (block_w + 2);
            bool sel = (static_cast<int>(state.map_gen_params.roughness) == i);
            renderer.draw_rect(bx, y, block_w, block_h, colors[i][0], colors[i][1], colors[i][2]);
            if (sel) renderer.draw_rect_outline(bx, y, block_w, block_h, 255, 255, 255);
        }
        y += block_h + pad;
    }

    // Continent count (dots).
    // Powered by Claude.
    {
        int dot_h = 16;
        int cnt = state.map_gen_params.continent_count;
        int dot_r = 5;
        for (int i = 0; i < cnt && i < 6; ++i) {
            int dx = 8 + i * (dot_r * 2 + 4) + dot_r;
            int dy = y + dot_h / 2;
            renderer.draw_rect(dx - dot_r, dy - dot_r, dot_r * 2, dot_r * 2, 200, 200, 200);
        }
        y += dot_h + pad;
    }

    // Generate button.
    // Powered by Claude.
    {
        int gen_h = 24;
        renderer.draw_rect(8, y, btn_w, gen_h, 60, 120, 200);
        y += gen_h + pad;
    }

    y += pad;

    // Flag swatch palette (only when flag tool is active).
    // Powered by Claude.
    if (state.tool == EditorTool::PAINT_FLAGS) {
        struct FlagSwatch {
            uint8_t flag;
            uint8_t r, g, b;
        };
        FlagSwatch flag_swatches[] = {
            {MSF_ROAD,       160, 120,  80},  // Road (brown)
            {MSF_ENC_ROAD,   220, 200,  80},  // EnchRoad (gold)
            {MSF_CORRUPTION, 120,  60, 140},  // Corruption (purple)
        };
        int swatch_sz = 22;
        int sx = 8;
        for (auto& fs : flag_swatches) {
            bool sel = (state.paint_flag == fs.flag);
            renderer.draw_rect(sx, y, swatch_sz, swatch_sz, fs.r, fs.g, fs.b);
            if (sel) {
                renderer.draw_rect_outline(sx, y, swatch_sz, swatch_sz, 255, 255, 255);
            }
            sx += swatch_sz + 2;
        }
        y += swatch_sz + 2;
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
        EditorTool::PAINT_FLAGS,
        EditorTool::PLACE_CITY, EditorTool::PLACE_UNIT, EditorTool::PLACE_NODE,
        EditorTool::PLACE_LAIR, EditorTool::PLACE_TOWER, EditorTool::PLACE_FORTRESS,
        EditorTool::ERASE,
        EditorTool::SETTINGS, EditorTool::WIZARDS
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

    // Flag swatches click handling.
    // Powered by Claude.
    if (state.tool == EditorTool::PAINT_FLAGS) {
        uint8_t flag_values[] = {MSF_ROAD, MSF_ENC_ROAD, MSF_CORRUPTION};
        int swatch_sz = 22;
        int num_flags = 3;
        for (int i = 0; i < num_flags; ++i) {
            int sx = 8 + i * (swatch_sz + 2);
            if (mx >= sx && mx < sx + swatch_sz && my >= y && my < y + swatch_sz) {
                state.paint_flag = flag_values[i];
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

    // --- Map Generation Controls (same layout as render) ---
    // Skip terrain/special palette area and separator.
    y += pad + pad * 2;

    // Land proportion bar — click to cycle 20/40/60/80%.
    // Powered by Claude.
    {
        int btn_w_local = WIDTH - 16;
        int bar_h = 16;
        if (my >= y && my < y + bar_h && mx >= 8 && mx < 8 + btn_w_local) {
            float vals[] = {0.2f, 0.4f, 0.6f, 0.8f};
            float cur = state.map_gen_params.land_proportion;
            int next = 0;
            for (int i = 0; i < 4; ++i) {
                if (cur < vals[i] + 0.01f) { next = (i + 1) % 4; break; }
            }
            state.map_gen_params.land_proportion = vals[next];
            return true;
        }
        y += bar_h + pad;
    }

    // Roughness selector — click to cycle.
    // Powered by Claude.
    {
        int btn_w_local = WIDTH - 16;
        int block_w = (btn_w_local - 4) / 3;
        int block_h = 16;
        if (my >= y && my < y + block_h) {
            for (int i = 0; i < 3; ++i) {
                int bx = 8 + i * (block_w + 2);
                if (mx >= bx && mx < bx + block_w) {
                    state.map_gen_params.roughness = static_cast<Roughness>(i);
                    return true;
                }
            }
        }
        y += block_h + pad;
    }

    // Continent count — click to cycle 1-6.
    // Powered by Claude.
    {
        int dot_h = 16;
        if (my >= y && my < y + dot_h) {
            state.map_gen_params.continent_count =
                (state.map_gen_params.continent_count % 6) + 1;
            return true;
        }
        y += dot_h + pad;
    }

    // Generate button — set generate_requested flag.
    // Powered by Claude.
    {
        int gen_h = 24;
        int btn_w_local = WIDTH - 16;
        if (my >= y && my < y + gen_h && mx >= 8 && mx < 8 + btn_w_local) {
            state.generate_requested = true;
            return true;
        }
    }

    return false;
}

} // namespace mom
