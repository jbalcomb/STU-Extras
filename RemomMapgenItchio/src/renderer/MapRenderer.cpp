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

    // Determine visible tile range with horizontal wrapping support.
    // Powered by Claude.
    int start_x = static_cast<int>(cam.offset_x / tw);
    int start_y = static_cast<int>(cam.offset_y / th);
    int end_x = start_x + (viewport.w / tw) + 2;
    int end_y = start_y + (viewport.h / th) + 2;
    if (start_y < 0) start_y = 0;
    if (end_y > WORLD_HEIGHT) end_y = WORLD_HEIGHT;

    // Render terrain tiles with horizontal wrapping.
    // Powered by Claude.
    for (int wy = start_y; wy < end_y; ++wy) {
        for (int ix = start_x; ix < end_x; ++ix) {
            // Wrap x coordinate horizontally.
            // Powered by Claude.
            int wx = ((ix % WORLD_WIDTH) + WORLD_WIDTH) % WORLD_WIDTH;
            int sx, sy;
            cam.world_to_screen(ix, wy, viewport.x, viewport.y, sx, sy);

            // Skip if outside viewport
            if (sx + tw < viewport.x || sx > viewport.x + viewport.w) continue;
            if (sy + th < viewport.y || sy > viewport.y + viewport.h) continue;

            uint16_t terrain_val = scenario.world.get_terrain(wx, wy, plane);
            BaseTerrain base = terrain_to_base(terrain_val);
            auto tc = terrain_color(base);
            renderer.draw_rect(sx, sy, tw, th, tc.r, tc.g, tc.b);

            // Draw terrain special overlay (small dot in corner).
            // Powered by Claude.
            int8_t special = scenario.world.get_special(wx, wy, plane);
            if (special != TS_NONE) {
                auto sc = special_color(static_cast<TerrainSpecial>(special));
                int dot_sz = tw / 4;
                if (dot_sz < 3) dot_sz = 3;
                renderer.draw_rect(sx + tw - dot_sz - 1, sy + 1, dot_sz, dot_sz,
                                   sc.r, sc.g, sc.b);
            }

            // Draw map square flag overlays (road, enchanted road, corruption).
            // Powered by Claude.
            uint8_t tile_flags = scenario.world.get_flags(wx, wy, plane);
            if (tile_flags & MSF_ROAD) {
                int line_h = 2;
                if (line_h < 1) line_h = 1;
                renderer.draw_rect(sx, sy + th / 2 - line_h / 2, tw, line_h,
                                   160, 120, 80);
            }
            if (tile_flags & MSF_ENC_ROAD) {
                int line_h = 2;
                if (line_h < 1) line_h = 1;
                renderer.draw_rect(sx, sy + th / 2 - line_h / 2, tw, line_h,
                                   220, 200, 80);
            }
            if (tile_flags & MSF_CORRUPTION) {
                renderer.draw_rect(sx, sy, tw, th, 120, 60, 140, 100);
            }

            // Grid lines
            if (state.show_grid) {
                renderer.draw_rect_outline(sx, sy, tw, th, 60, 60, 60, 80);
            }
        }
    }

    // Render smoothing violation overlays.
    // Powered by Claude.
    if (state.show_all_violations) {
        for (const auto& v : state.violations) {
            if (v.plane != plane) continue;
            int sx, sy;
            cam.world_to_screen(v.x, v.y, viewport.x, viewport.y, sx, sy);
            if (sx + tw < viewport.x || sx > viewport.x + viewport.w) continue;
            if (sy + th < viewport.y || sy > viewport.y + viewport.h) continue;
            renderer.draw_rect_outline(sx + 2, sy + 2, tw - 4, th - 4,
                                       255, 255, 0);
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

    // Render towers as white diamond shapes (two overlapping rotated rects).
    // Towers have no plane field; they appear on both planes.
    // Active when wx != 0 || wy != 0.
    // Powered by Claude.
    for (int i = 0; i < NUM_TOWERS; ++i) {
        const auto& t = scenario.towers[i];
        if (t.wx == 0 && t.wy == 0) continue;
        int sx, sy;
        cam.world_to_screen(t.wx, t.wy, viewport.x, viewport.y, sx, sy);
        if (sx + tw < viewport.x || sx > viewport.x + viewport.w) continue;
        if (sy + th < viewport.y || sy > viewport.y + viewport.h) continue;
        // Diamond shape: draw a small horizontal bar and a small vertical bar
        // centered on the tile to approximate a diamond/rotated square.
        int sz = tw / 3;
        if (sz < 3) sz = 3;
        int cx = sx + tw / 2;
        int cy = sy + th / 2;
        // Horizontal bar
        renderer.draw_rect(cx - sz, cy - sz / 3, sz * 2, sz * 2 / 3,
                           220, 220, 240);
        // Vertical bar
        renderer.draw_rect(cx - sz / 3, cy - sz, sz * 2 / 3, sz * 2,
                           220, 220, 240);
        renderer.draw_rect_outline(cx - sz, cy - sz, sz * 2, sz * 2,
                                   255, 255, 255);
        // Selection highlight
        if (state.selected_tower == i) {
            renderer.draw_rect_outline(sx, sy, tw, th, 255, 255, 0);
        }
    }

    // Render fortresses as bright colored star shapes using the wizard banner color.
    // Uses overlapping horizontal and vertical rects to approximate a star/cross.
    // Powered by Claude.
    for (int i = 0; i < NUM_FORTRESSES; ++i) {
        const auto& f = scenario.fortresses[i];
        if (f.active == 0) continue;
        if (f.wp != plane) continue;
        int sx, sy;
        cam.world_to_screen(f.wx, f.wy, viewport.x, viewport.y, sx, sy);
        if (sx + tw < viewport.x || sx > viewport.x + viewport.w) continue;
        if (sy + th < viewport.y || sy > viewport.y + viewport.h) continue;
        auto bc = banner_color(i); // fortress index maps to wizard slot
        int sz = tw / 2;
        if (sz < 3) sz = 3;
        int cx = sx + tw / 2;
        int cy = sy + th / 2;
        // Horizontal bar of the star
        renderer.draw_rect(cx - sz, cy - sz / 3, sz * 2, sz * 2 / 3,
                           bc.r, bc.g, bc.b);
        // Vertical bar of the star
        renderer.draw_rect(cx - sz / 3, cy - sz, sz * 2 / 3, sz * 2,
                           bc.r, bc.g, bc.b);
        renderer.draw_rect_outline(cx - sz, cy - sz, sz * 2, sz * 2,
                                   255, 255, 0);
        // Selection highlight
        if (state.selected_fortress == i) {
            renderer.draw_rect_outline(sx, sy, tw, th, 255, 255, 0);
        }
    }

    // Render lairs as dark red triangle approximations (filled rect with outline).
    // Active when wx != 0 || wy != 0.
    // Powered by Claude.
    for (int i = 0; i < NUM_LAIRS; ++i) {
        const auto& l = scenario.lairs[i];
        if (l.wx == 0 && l.wy == 0) continue;
        if (l.wp != plane) continue;
        int sx, sy;
        cam.world_to_screen(l.wx, l.wy, viewport.x, viewport.y, sx, sy);
        if (sx + tw < viewport.x || sx > viewport.x + viewport.w) continue;
        if (sy + th < viewport.y || sy > viewport.y + viewport.h) continue;
        // Triangle approximation: draw a tall narrow rect as the peak and
        // a wider rect as the base to suggest a triangular shape.
        int sz = tw / 3;
        if (sz < 3) sz = 3;
        int cx = sx + tw / 2;
        int cy = sy + th / 2;
        // Wide base
        renderer.draw_rect(cx - sz, cy, sz * 2, sz / 2 + 1, 140, 30, 30);
        // Narrow peak
        renderer.draw_rect(cx - sz / 3, cy - sz, sz * 2 / 3, sz, 180, 40, 40);
        renderer.draw_rect_outline(cx - sz, cy - sz, sz * 2, sz + sz / 2 + 1,
                                   200, 80, 80);
        // Selection highlight
        if (state.selected_lair == i) {
            renderer.draw_rect_outline(sx, sy, tw, th, 255, 255, 0);
        }
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
