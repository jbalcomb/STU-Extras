// Map tile rendering with terrain colors and entity icons.
// Powered by Claude.

#include "renderer/MapRenderer.hpp"
#include "scenario/Scenario.hpp"
#include "editor/EditorState.hpp"

namespace mom {

TerrainColor MapRenderer::terrain_color(BaseTerrain type) {
    switch (type) {
        case TERRAIN_OCEAN:     return {26, 82, 118};
        case TERRAIN_SHORE:     return {93, 173, 226};
        case TERRAIN_GRASSLAND: return {39, 174, 96};
        case TERRAIN_FOREST:    return {30, 132, 73};
        case TERRAIN_MOUNTAIN:  return {125, 102, 8};
        case TERRAIN_DESERT:    return {244, 208, 63};
        case TERRAIN_SWAMP:     return {108, 122, 0};
        case TERRAIN_TUNDRA:    return {213, 216, 220};
        case TERRAIN_HILL:      return {160, 82, 45};
        case TERRAIN_RIVER:     return {41, 128, 185};
        case TERRAIN_VOLCANO:   return {180, 30, 30};
        case TERRAIN_LAKE:      return {52, 152, 219};
        default:                return {100, 100, 100};
    }
}

TerrainColor MapRenderer::special_color(TerrainSpecial special) {
    switch (special) {
        case TS_IRON:           return {150, 150, 150};
        case TS_COAL:           return {40, 40, 40};
        case TS_SILVER:         return {220, 220, 220};
        case TS_GOLD:           return {255, 215, 0};
        case TS_GEMS:           return {200, 0, 200};
        case TS_MITHRIL:        return {0, 220, 220};
        case TS_ADAMANTIUM:     return {128, 0, 255};
        case TS_QUORK_CRYSTALS: return {255, 100, 255};
        case TS_CRYSX_CRYSTALS: return {100, 255, 255};
        case TS_WILD_GAME:      return {180, 120, 60};
        case TS_NIGHTSHADE:     return {80, 0, 80};
        default:                return {255, 255, 255};
    }
}

TerrainColor MapRenderer::banner_color(int owner_idx) {
    switch (owner_idx) {
        case 0: return {0, 128, 0};     // Green
        case 1: return {0, 0, 200};     // Blue
        case 2: return {200, 0, 0};     // Red
        case 3: return {180, 0, 180};   // Purple
        case 4: return {200, 200, 0};   // Yellow
        case 5: return {30, 30, 30};    // Dark (neutral/independent)
        default: return {128, 128, 128};
    }
}

// Map a terrain uint16 value to a base terrain type for rendering.
// The original game uses complex tile indices with edge transitions;
// this simplified mapping covers the base types.
// Powered by Claude.
static BaseTerrain terrain_to_base(uint16_t terrain_val) {
    // The low byte typically indicates the base terrain type
    // This is a simplified mapping; exact values depend on ReMoM's terrain tables
    uint8_t base = static_cast<uint8_t>(terrain_val & 0xFF);
    if (base <= 3)  return TERRAIN_OCEAN;
    if (base <= 7)  return TERRAIN_SHORE;
    if (base <= 11) return TERRAIN_GRASSLAND;
    if (base <= 15) return TERRAIN_FOREST;
    if (base <= 19) return TERRAIN_MOUNTAIN;
    if (base <= 23) return TERRAIN_DESERT;
    if (base <= 27) return TERRAIN_SWAMP;
    if (base <= 31) return TERRAIN_TUNDRA;
    if (base <= 35) return TERRAIN_HILL;
    if (base <= 39) return TERRAIN_RIVER;
    if (base <= 43) return TERRAIN_VOLCANO;
    return TERRAIN_GRASSLAND; // fallback
}

void MapRenderer::render(Renderer& renderer, const Scenario& scenario,
                         const EditorState& state, const SDL_Rect& viewport) {
    const auto& cam = renderer.camera;
    int tw = cam.tile_w();
    int th = cam.tile_h();
    if (tw <= 0 || th <= 0) return;

    int plane = state.current_plane;

    // Determine visible tile range
    int start_x = static_cast<int>(cam.offset_x / tw);
    int start_y = static_cast<int>(cam.offset_y / th);
    int end_x = start_x + (viewport.w / tw) + 2;
    int end_y = start_y + (viewport.h / th) + 2;
    if (start_x < 0) start_x = 0;
    if (start_y < 0) start_y = 0;
    if (end_x > WORLD_WIDTH)  end_x = WORLD_WIDTH;
    if (end_y > WORLD_HEIGHT) end_y = WORLD_HEIGHT;

    // Render terrain tiles
    for (int wy = start_y; wy < end_y; ++wy) {
        for (int wx = start_x; wx < end_x; ++wx) {
            int sx, sy;
            cam.world_to_screen(wx, wy, viewport.x, viewport.y, sx, sy);

            // Skip if outside viewport
            if (sx + tw < viewport.x || sx > viewport.x + viewport.w) continue;
            if (sy + th < viewport.y || sy > viewport.y + viewport.h) continue;

            uint16_t terrain_val = scenario.world.get_terrain(wx, wy, plane);
            BaseTerrain base = terrain_to_base(terrain_val);
            auto tc = terrain_color(base);
            renderer.draw_rect(sx, sy, tw, th, tc.r, tc.g, tc.b);

            // Draw terrain special overlay (small dot in corner)
            int8_t special = scenario.world.get_special(wx, wy, plane);
            if (special != TS_NONE) {
                auto sc = special_color(static_cast<TerrainSpecial>(special));
                int dot_sz = tw / 4;
                if (dot_sz < 3) dot_sz = 3;
                renderer.draw_rect(sx + tw - dot_sz - 1, sy + 1, dot_sz, dot_sz,
                                   sc.r, sc.g, sc.b);
            }

            // Grid lines
            if (state.show_grid) {
                renderer.draw_rect_outline(sx, sy, tw, th, 60, 60, 60, 80);
            }
        }
    }

    // Render cities
    for (int i = 0; i < NUM_CITIES; ++i) {
        const auto& c = scenario.cities[i];
        if (!c.is_active() || c.wp != plane) continue;
        int sx, sy;
        cam.world_to_screen(c.wx, c.wy, viewport.x, viewport.y, sx, sy);
        if (sx + tw < viewport.x || sx > viewport.x + viewport.w) continue;
        if (sy + th < viewport.y || sy > viewport.y + viewport.h) continue;
        auto bc = banner_color(c.owner_idx);
        int inset = tw / 6;
        renderer.draw_rect(sx + inset, sy + inset, tw - inset * 2, th - inset * 2,
                           bc.r, bc.g, bc.b);
        // Highlight selected
        if (state.selected_city == i) {
            renderer.draw_rect_outline(sx, sy, tw, th, 255, 255, 0);
        }
    }

    // Render units (show as small triangles via a filled rect for now)
    for (int i = 0; i < NUM_UNITS; ++i) {
        const auto& u = scenario.units[i];
        if (!u.is_active() || u.wp != plane) continue;
        int sx, sy;
        cam.world_to_screen(u.wx, u.wy, viewport.x, viewport.y, sx, sy);
        if (sx + tw < viewport.x || sx > viewport.x + viewport.w) continue;
        if (sy + th < viewport.y || sy > viewport.y + viewport.h) continue;
        auto bc = banner_color(u.owner_idx);
        int sz = tw / 3;
        if (sz < 3) sz = 3;
        renderer.draw_rect(sx + tw / 2 - sz / 2, sy + th / 2 - sz / 2,
                           sz, sz, bc.r, bc.g, bc.b);
    }

    // Render nodes
    for (int i = 0; i < NUM_NODES; ++i) {
        const auto& n = scenario.nodes[i];
        if (n.wp != plane) continue;
        int sx, sy;
        cam.world_to_screen(n.wx, n.wy, viewport.x, viewport.y, sx, sy);
        if (sx + tw < viewport.x || sx > viewport.x + viewport.w) continue;
        if (sy + th < viewport.y || sy > viewport.y + viewport.h) continue;
        // Color by node type: Nature=green, Sorcery=blue, Chaos=red
        uint8_t nr = 0, ng = 0, nb = 0;
        switch (n.type) {
            case NODE_NATURE:  ng = 200; break;
            case NODE_SORCERY: nb = 200; break;
            case NODE_CHAOS:   nr = 200; break;
        }
        int sz = tw / 2;
        renderer.draw_rect(sx + tw / 2 - sz / 2, sy + th / 2 - sz / 2,
                           sz, sz, nr, ng, nb);
        renderer.draw_rect_outline(sx + tw / 2 - sz / 2, sy + th / 2 - sz / 2,
                                   sz, sz, 255, 255, 255, 128);
    }

    // Cursor highlight
    if (state.cursor_wx >= 0 && state.cursor_wy >= 0) {
        int sx, sy;
        cam.world_to_screen(state.cursor_wx, state.cursor_wy,
                            viewport.x, viewport.y, sx, sy);
        renderer.draw_rect_outline(sx, sy, tw, th, 255, 255, 255);
    }
}

} // namespace mom
