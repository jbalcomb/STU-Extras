// Left-side tool palette rendering and interaction.
// Powered by Claude.

#include "ui/ToolPanel.hpp"
#include "renderer/UIRenderer.hpp"
#include "renderer/MapRenderer.hpp"
#include <string>

namespace mom {

// Tool name lookup table.
// Powered by Claude.
static const char* tool_name(EditorTool t) {
    switch (t) {
        case EditorTool::SELECT:         return "Select";
        case EditorTool::PAINT_TERRAIN:  return "Terrain";
        case EditorTool::PAINT_SPECIAL:  return "Special";
        case EditorTool::PAINT_FLAGS:    return "Flags";
        case EditorTool::PLACE_CITY:     return "City";
        case EditorTool::PLACE_UNIT:     return "Unit";
        case EditorTool::PLACE_NODE:     return "Node";
        case EditorTool::PLACE_LAIR:     return "Lair";
        case EditorTool::PLACE_TOWER:    return "Tower";
        case EditorTool::PLACE_FORTRESS: return "Fortress";
        case EditorTool::ERASE:          return "Erase";
        case EditorTool::SETTINGS:       return "Settings";
        case EditorTool::WIZARDS:        return "Wizards";
    }
    return "???";
}

// Terrain type name lookup.
// Powered by Claude.
static const char* terrain_name(BaseTerrain t) {
    switch (t) {
        case TERRAIN_OCEAN:     return "Ocean";
        case TERRAIN_SHORE:     return "Shore";
        case TERRAIN_GRASSLAND: return "Grassland";
        case TERRAIN_FOREST:    return "Forest";
        case TERRAIN_MOUNTAIN:  return "Mountain";
        case TERRAIN_DESERT:    return "Desert";
        case TERRAIN_SWAMP:     return "Swamp";
        case TERRAIN_TUNDRA:    return "Tundra";
        case TERRAIN_HILL:      return "Hill";
        case TERRAIN_RIVER:     return "River";
        case TERRAIN_VOLCANO:   return "Volcano";
        case TERRAIN_LAKE:      return "Lake";
        default:                return "Unknown";
    }
}

// Terrain special name lookup.
// Powered by Claude.
static const char* special_name(TerrainSpecial s) {
    switch (s) {
        case TS_NONE:            return "None";
        case TS_IRON:            return "Iron";
        case TS_COAL:            return "Coal";
        case TS_SILVER:          return "Silver";
        case TS_GOLD:            return "Gold";
        case TS_GEMS:            return "Gems";
        case TS_MITHRIL:         return "Mithril";
        case TS_ADAMANTIUM:      return "Adamantium";
        case TS_QUORK_CRYSTALS:  return "Quork";
        case TS_CRYSX_CRYSTALS:  return "Crysx";
        case TS_WILD_GAME:       return "Wild Game";
        case TS_NIGHTSHADE:      return "Nightshade";
        default:                 return "Unknown";
    }
}

// Flag name lookup.
// Powered by Claude.
static const char* flag_name(uint8_t flag) {
    if (flag == MSF_ROAD)       return "Road";
    if (flag == MSF_ENC_ROAD)   return "Ench. Road";
    if (flag == MSF_CORRUPTION) return "Corruption";
    return "Unknown";
}

void ToolPanel::render(Renderer& renderer, const EditorState& state, int window_h) {
    // Panel background
    UIRenderer::draw_panel(renderer, 0, 0, WIDTH, window_h);

    int y = 8;
    int btn_w = WIDTH - 16;
    int btn_h = 20;
    int pad = 4;

    // Tool mode buttons with text labels.
    // Powered by Claude.
    EditorTool tools[] = {
        EditorTool::SELECT, EditorTool::PAINT_TERRAIN, EditorTool::PAINT_SPECIAL,
        EditorTool::PAINT_FLAGS,
        EditorTool::PLACE_CITY, EditorTool::PLACE_UNIT, EditorTool::PLACE_NODE,
        EditorTool::PLACE_LAIR, EditorTool::PLACE_TOWER, EditorTool::PLACE_FORTRESS,
        EditorTool::ERASE, EditorTool::SETTINGS, EditorTool::WIZARDS
    };

    for (auto t : tools) {
        bool sel = (state.tool == t);
        UIRenderer::draw_button_with_label(renderer, 8, y, btn_w, btn_h,
                                           tool_name(t), sel, false);
        y += btn_h + pad;
    }

    y += pad;
    UIRenderer::draw_separator(renderer, 8, y, btn_w);
    y += pad * 2;

    // Terrain type palette (only when terrain tool is active).
    // Powered by Claude.
    if (state.tool == EditorTool::PAINT_TERRAIN) {
        // Show selected terrain name above palette.
        // Powered by Claude.
        UIRenderer::draw_label(renderer, 8, y, terrain_name(state.paint_terrain),
                               120, 200, 120);
        y += 16;

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

    // Land proportion bar with text label.
    // Powered by Claude.
    {
        int bar_w = btn_w;
        int bar_h = 16;
        int pct = static_cast<int>(state.map_gen_params.land_proportion * 100.0f);
        int fill_w = static_cast<int>(bar_w * state.map_gen_params.land_proportion);
        renderer.draw_rect(8, y, bar_w, bar_h, 30, 60, 120);    // ocean bg
        renderer.draw_rect(8, y, fill_w, bar_h, 80, 160, 80);   // land fill
        renderer.draw_text(10, y + 2, "Land: " + std::to_string(pct) + "%", 240, 240, 240);
        y += bar_h + pad;
    }

    // Roughness selector with text labels.
    // Powered by Claude.
    {
        int block_w = (btn_w - 4) / 3;
        int block_h = 16;
        uint8_t colors[][3] = {{80,180,80}, {180,180,80}, {180,80,80}};
        const char* labels[] = {"Smooth", "Normal", "Rough"};
        for (int i = 0; i < 3; ++i) {
            int bx = 8 + i * (block_w + 2);
            bool sel = (static_cast<int>(state.map_gen_params.roughness) == i);
            renderer.draw_rect(bx, y, block_w, block_h, colors[i][0], colors[i][1], colors[i][2]);
            if (sel) renderer.draw_rect_outline(bx, y, block_w, block_h, 255, 255, 255);
            renderer.draw_text(bx + 2, y + 2, labels[i], 240, 240, 240);
        }
        y += block_h + pad;
    }

    // Continent count with text label.
    // Powered by Claude.
    {
        int dot_h = 16;
        int cnt = state.map_gen_params.continent_count;
        renderer.draw_text(8, y + 2, "Continents: " + std::to_string(cnt), 200, 200, 200);
        y += dot_h + pad;
    }

    // Generate button with text label.
    // Powered by Claude.
    {
        int gen_h = 24;
        renderer.draw_rect(8, y, btn_w, gen_h, 60, 120, 200);
        renderer.draw_rect_outline(8, y, btn_w, gen_h, 90, 90, 95);
        renderer.draw_text(btn_w / 2 - 20, y + 6, "Generate", 240, 240, 240);
        y += gen_h + pad;
    }

    // Generate Wizards button — populates all 6 wizard slots with preset data.
    // Powered by Claude.
    {
        int gen_h = 24;
        renderer.draw_rect(8, y, btn_w, gen_h, 120, 60, 200);
        renderer.draw_rect_outline(8, y, btn_w, gen_h, 90, 90, 95);
        renderer.draw_text(btn_w / 2 - 30, y + 6, "Gen Wizards", 240, 240, 240);
        y += gen_h + pad;
    }

    // Export .GAM button — saves scenario as binary .GAM file.
    // Powered by Claude.
    {
        int gen_h = 24;
        renderer.draw_rect(8, y, btn_w, gen_h, 60, 140, 80);
        renderer.draw_rect_outline(8, y, btn_w, gen_h, 90, 95, 90);
        renderer.draw_text(btn_w / 2 - 30, y + 6, "Export .GAM", 240, 240, 240);
        y += gen_h + pad;
    }

    y += pad;

    // Flag swatch palette (only when flag tool is active).
    // Powered by Claude.
    if (state.tool == EditorTool::PAINT_FLAGS) {
        // Show selected flag name above palette.
        // Powered by Claude.
        UIRenderer::draw_label(renderer, 8, y, flag_name(state.paint_flag),
                               200, 180, 120);
        y += 16;

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

    // Terrain special palette (only when special tool is active).
    // Powered by Claude.
    if (state.tool == EditorTool::PAINT_SPECIAL) {
        // Show selected special name above palette.
        // Powered by Claude.
        UIRenderer::draw_label(renderer, 8, y, special_name(state.paint_special),
                               200, 200, 120);
        y += 16;

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

    // Node type palette (only when node tool is active).
    // Powered by Claude.
    if (state.tool == EditorTool::PLACE_NODE) {
        static const char* node_names[] = {"Sorcery", "Nature", "Chaos"};
        UIRenderer::draw_label(renderer, 8, y, node_names[state.place_node_type],
                               200, 200, 120);
        y += 16;

        struct NodeSwatch { int8_t type; uint8_t r, g, b; };
        NodeSwatch swatches[] = {
            {NODE_SORCERY, 60,  60, 200},
            {NODE_NATURE,  60, 200,  60},
            {NODE_CHAOS,  200,  60,  60},
        };
        int swatch_sz = 22;
        int sx = 8;
        for (auto& ns : swatches) {
            bool sel = (state.place_node_type == ns.type);
            renderer.draw_rect(sx, y, swatch_sz, swatch_sz, ns.r, ns.g, ns.b);
            if (sel) {
                renderer.draw_rect_outline(sx, y, swatch_sz, swatch_sz, 255, 255, 255);
            }
            sx += swatch_sz + 2;
        }
        y += swatch_sz + 2;
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
            // Toggle off: clicking the active tool deselects back to SELECT.
            // Powered by Claude.
            state.tool = (state.tool == t) ? EditorTool::SELECT : t;
            return true;
        }
        y += btn_h + pad;
    }

    // After tool buttons: separator gap (matches render).
    // Powered by Claude.
    y += pad;           // y += pad before separator
    // separator drawn at y
    y += pad * 2;       // y += pad * 2 after separator

    // Terrain type swatches (with label offset matching render).
    // Powered by Claude.
    if (state.tool == EditorTool::PAINT_TERRAIN) {
        y += 16; // label above palette
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
        // Advance y past the palette rows.
        // Powered by Claude.
        int total_rows = (BASE_TERRAIN_COUNT + cols - 1) / cols;
        int last_row_count = BASE_TERRAIN_COUNT % cols;
        if (last_row_count > 0) {
            y += (total_rows - 1) * (swatch_sz + 2) + swatch_sz + 2;
        } else {
            y += total_rows * (swatch_sz + 2);
        }
    }

    // --- Map Generation Controls (same layout as render) ---
    // Separator between palette and map gen.
    // Powered by Claude.
    y += pad;           // y += pad before separator
    // separator drawn at y
    y += pad * 2;       // y += pad * 2 after separator

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
        y += gen_h + pad;
    }

    // Generate Wizards button — set generate_wizards_requested flag.
    // Powered by Claude.
    {
        int gen_h = 24;
        int btn_w_local = WIDTH - 16;
        if (my >= y && my < y + gen_h && mx >= 8 && mx < 8 + btn_w_local) {
            state.generate_wizards_requested = true;
            return true;
        }
        y += gen_h + pad;
    }

    // Export .GAM button — set export_gam_requested flag.
    // Powered by Claude.
    {
        int gen_h = 24;
        int btn_w_local = WIDTH - 16;
        if (my >= y && my < y + gen_h && mx >= 8 && mx < 8 + btn_w_local) {
            state.export_gam_requested = true;
            return true;
        }
        y += gen_h + pad;
    }

    y += pad; // extra pad after generate (matches render)

    // Flag swatches click handling (after map gen, matching render order).
    // Powered by Claude.
    if (state.tool == EditorTool::PAINT_FLAGS) {
        y += 16; // label above palette
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

    // Special swatches (after map gen, matching render order).
    // Powered by Claude.
    if (state.tool == EditorTool::PAINT_SPECIAL) {
        y += 16; // label above palette
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

    // Node type swatches (matching render order).
    // Powered by Claude.
    if (state.tool == EditorTool::PLACE_NODE) {
        y += 16; // label above palette
        int8_t node_types[] = {NODE_SORCERY, NODE_NATURE, NODE_CHAOS};
        int swatch_sz = 22;
        for (int i = 0; i < 3; ++i) {
            int sx = 8 + i * (swatch_sz + 2);
            if (mx >= sx && mx < sx + swatch_sz && my >= y && my < y + swatch_sz) {
                state.place_node_type = node_types[i];
                return true;
            }
        }
    }

    return false;
}

} // namespace mom
