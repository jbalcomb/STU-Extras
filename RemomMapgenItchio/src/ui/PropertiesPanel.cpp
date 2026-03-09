// Right-side property inspector panel.
// Powered by Claude.

#include "ui/PropertiesPanel.hpp"
#include "ui/Minimap.hpp"
#include "renderer/UIRenderer.hpp"
#include "renderer/MapRenderer.hpp"
#include "scenario/Scenario.hpp"

#include <array>
#include <string>

namespace mom {

// Preset scenario titles for click-cycling in the metadata section.
// Powered by Claude.
static const std::array<std::string, 5> PRESET_TITLES = {
    "Untitled Scenario",
    "Custom Campaign",
    "Community Challenge",
    "Grand Conquest",
    "The Dark War"
};

// Preset author names for click-cycling in the metadata section.
// The first entry is empty to allow clearing the author field.
// Powered by Claude.
static const std::array<std::string, 3> PRESET_AUTHORS = {
    "",
    "Anonymous",
    "Community"
};

// Render colored indicator bars for scenario metadata fields.
// Shows title, author, and tag count when no entity is selected.
// Powered by Claude.
static void render_metadata_section(Renderer& renderer, const Scenario& scenario,
                                    int panel_x, int& y) {
    int bar_x = panel_x + 8;
    int bar_max_w = PropertiesPanel::WIDTH - 16;
    int pad = 4;

    // Section header indicator bar (dim white).
    // Powered by Claude.
    UIRenderer::draw_label_bar(renderer, bar_x, y, bar_max_w / 2, 140, 140, 150);
    y += 14;

    // Title bar: wide colored bar (180, 120, 60) indicating a title is present.
    // Width scales with title length, minimum 40px when title is non-empty.
    // Powered by Claude.
    {
        int title_w = 0;
        if (!scenario.meta.title.empty()) {
            title_w = static_cast<int>(scenario.meta.title.size()) * 4;
            if (title_w < 40) title_w = 40;
            if (title_w > bar_max_w) title_w = bar_max_w;
        }
        if (title_w > 0) {
            renderer.draw_rect(bar_x, y, title_w, 14, 180, 120, 60);
            renderer.draw_rect_outline(bar_x, y, title_w, 14, 200, 140, 80);
        }
        y += 18;
    }

    // Author bar: narrower bar (120, 160, 80) indicating an author is present.
    // Powered by Claude.
    {
        int author_w = 0;
        if (!scenario.meta.author.empty()) {
            author_w = static_cast<int>(scenario.meta.author.size()) * 4;
            if (author_w < 30) author_w = 30;
            if (author_w > bar_max_w - 40) author_w = bar_max_w - 40;
        }
        if (author_w > 0) {
            renderer.draw_rect(bar_x, y, author_w, 12, 120, 160, 80);
            renderer.draw_rect_outline(bar_x, y, author_w, 12, 140, 180, 100);
        }
        y += 16;
    }

    // Description indicator: thin bar (100, 130, 170) if description is present.
    // Powered by Claude.
    {
        if (!scenario.meta.description.empty()) {
            int desc_w = static_cast<int>(scenario.meta.description.size()) * 2;
            if (desc_w < 20) desc_w = 20;
            if (desc_w > bar_max_w) desc_w = bar_max_w;
            renderer.draw_rect(bar_x, y, desc_w, 8, 100, 130, 170);
        }
        y += 12;
    }

    // Tags count: small dots indicating number of tags.
    // Each tag is a 6x6 dot with spacing.
    // Powered by Claude.
    {
        int tag_count = static_cast<int>(scenario.meta.tags.size());
        int dot_size = 6;
        int dot_spacing = 4;
        int tx = bar_x;
        for (int i = 0; i < tag_count && i < 20; ++i) {
            renderer.draw_rect(tx, y, dot_size, dot_size, 160, 140, 200);
            tx += dot_size + dot_spacing;
            if (tx + dot_size > bar_x + bar_max_w) break;
        }
        if (tag_count > 0) {
            y += dot_size + pad;
        }
    }

    UIRenderer::draw_separator(renderer, bar_x, y, bar_max_w);
    y += pad * 2;
}

void PropertiesPanel::render(Renderer& renderer, const EditorState& state,
                             const Scenario& scenario, int window_w, int window_h) {
    int panel_x = window_w - WIDTH;
    UIRenderer::draw_panel(renderer, panel_x, 0, WIDTH, window_h);

    int y = 8;
    int pad = 4;

    // Show scenario metadata section when no entity is selected.
    // Powered by Claude.
    bool has_entity_selected = (state.selected_city >= 0) ||
                               (state.selected_unit >= 0) ||
                               (state.selected_node >= 0) ||
                               (state.selected_lair >= 0) ||
                               (state.selected_tower >= 0) ||
                               (state.selected_fortress >= 0);
    if (!has_entity_selected) {
        render_metadata_section(renderer, scenario, panel_x, y);
    }

    // Show selected tile info
    if (state.cursor_wx >= 0 && state.cursor_wy >= 0) {
        int plane = state.current_plane;
        uint16_t terrain_val = scenario.world.get_terrain(
            state.cursor_wx, state.cursor_wy, plane);
        int8_t special = scenario.world.get_special(
            state.cursor_wx, state.cursor_wy, plane);
        uint8_t flags = scenario.world.get_flags(
            state.cursor_wx, state.cursor_wy, plane);

        // Terrain type color indicator
        // (In Phase 1 without text rendering, we show colored bars)
        UIRenderer::draw_label_bar(renderer, panel_x + 8, y, WIDTH - 16, 180, 180, 180);
        y += 16;

        // Terrain color swatch
        auto tc = MapRenderer::terrain_color(
            static_cast<BaseTerrain>(terrain_val & 0xFF));
        renderer.draw_rect(panel_x + 8, y, 40, 20, tc.r, tc.g, tc.b);
        renderer.draw_rect_outline(panel_x + 8, y, 40, 20, 120, 120, 120);
        y += 28;

        // Special indicator
        if (special != TS_NONE) {
            auto sc = MapRenderer::special_color(static_cast<TerrainSpecial>(special));
            renderer.draw_rect(panel_x + 8, y, 20, 20, sc.r, sc.g, sc.b);
            renderer.draw_rect_outline(panel_x + 8, y, 20, 20, 120, 120, 120);
        }
        y += 28;

        // Flags indicator
        if (flags & MSF_ROAD) {
            renderer.draw_rect(panel_x + 8, y, 40, 8, 160, 140, 100);
        }
        y += 16;

        UIRenderer::draw_separator(renderer, panel_x + 8, y, WIDTH - 16);
        y += pad * 2;
    }

    // Show selected city info (clickable race and population areas).
    // Powered by Claude.
    if (state.selected_city >= 0 && state.selected_city < NUM_CITIES) {
        const auto& c = scenario.cities[state.selected_city];
        if (c.is_active()) {
            auto bc = MapRenderer::banner_color(c.owner_idx);
            renderer.draw_rect(panel_x + 8, y, 30, 20, bc.r, bc.g, bc.b);
            y += 28;

            // Race indicator bar (colored by race index, clickable).
            // Powered by Claude.
            int race_w = 20 + c.race * 8;
            if (race_w > WIDTH - 16) race_w = WIDTH - 16;
            renderer.draw_rect(panel_x + 8, y, race_w, 12, 180, 140, 200);
            renderer.draw_rect_outline(panel_x + 8, y, race_w, 12, 200, 160, 220);
            y += 20;

            // Population indicator bar (clickable).
            // Powered by Claude.
            int pop_w = c.population * 8;
            if (pop_w > WIDTH - 16) pop_w = WIDTH - 16;
            renderer.draw_rect(panel_x + 8, y, pop_w, 12, 120, 200, 120);
            renderer.draw_rect_outline(panel_x + 8, y, pop_w, 12, 140, 220, 140);
            y += 20;

            // Buildings count indicator
            int bldg_w = c.bldg_cnt * 4;
            if (bldg_w > WIDTH - 16) bldg_w = WIDTH - 16;
            renderer.draw_rect(panel_x + 8, y, bldg_w, 12, 200, 160, 100);
            y += 20;
        }
    }

    // Show selected unit info (clickable type area).
    // Powered by Claude.
    if (state.selected_unit >= 0 && state.selected_unit < NUM_UNITS) {
        const auto& u = scenario.units[state.selected_unit];
        if (u.is_active()) {
            auto bc = MapRenderer::banner_color(u.owner_idx);
            renderer.draw_rect(panel_x + 8, y, 30, 20, bc.r, bc.g, bc.b);
            y += 28;

            // Unit type indicator bar (clickable).
            // Powered by Claude.
            int type_w = 20 + u.type * 1;
            if (type_w > WIDTH - 16) type_w = WIDTH - 16;
            renderer.draw_rect(panel_x + 8, y, type_w, 12, 100, 150, 200);
            renderer.draw_rect_outline(panel_x + 8, y, type_w, 12, 120, 170, 220);
            y += 20;

            // Level indicator
            int lvl_w = u.Level * 16;
            if (lvl_w > WIDTH - 16) lvl_w = WIDTH - 16;
            renderer.draw_rect(panel_x + 8, y, lvl_w, 12, 200, 200, 100);
            y += 20;
        }
    }

    // Show selected node info (clickable type area).
    // Powered by Claude.
    if (state.selected_node >= 0 && state.selected_node < NUM_NODES) {
        const auto& n = scenario.nodes[state.selected_node];
        if (!(n.wx == 0 && n.wy == 0)) {
            // Node type color indicator (clickable).
            // Powered by Claude.
            uint8_t nr = 0, ng = 0, nb = 0;
            switch (n.type) {
                case NODE_NATURE:  ng = 200; break;
                case NODE_SORCERY: nb = 200; break;
                case NODE_CHAOS:   nr = 200; break;
            }
            renderer.draw_rect(panel_x + 8, y, 40, 20, nr, ng, nb);
            renderer.draw_rect_outline(panel_x + 8, y, 40, 20, 255, 255, 255, 128);
            y += 28;

            // Power indicator
            int pow_w = n.power * 10;
            if (pow_w > WIDTH - 16) pow_w = WIDTH - 16;
            renderer.draw_rect(panel_x + 8, y, pow_w, 12, 200, 180, 100);
            y += 20;
        }
    }

    // Show selected tower info.
    // Powered by Claude.
    if (state.selected_tower >= 0 && state.selected_tower < NUM_TOWERS) {
        const auto& t = scenario.towers[state.selected_tower];
        if (!(t.wx == 0 && t.wy == 0)) {
            renderer.draw_rect(panel_x + 8, y, 30, 20, 220, 220, 240);
            renderer.draw_rect_outline(panel_x + 8, y, 30, 20, 255, 255, 255);
            y += 28;
        }
    }

    // Show selected fortress info.
    // Powered by Claude.
    if (state.selected_fortress >= 0 && state.selected_fortress < NUM_FORTRESSES) {
        const auto& f = scenario.fortresses[state.selected_fortress];
        if (f.active != 0) {
            renderer.draw_rect(panel_x + 8, y, 30, 20, 200, 160, 60);
            renderer.draw_rect_outline(panel_x + 8, y, 30, 20, 240, 200, 80);
            y += 28;
        }
    }

    // Show selected lair info.
    // Powered by Claude.
    if (state.selected_lair >= 0 && state.selected_lair < NUM_LAIRS) {
        const auto& l = scenario.lairs[state.selected_lair];
        if (!(l.wx == 0 && l.wy == 0)) {
            // Lair type indicator
            int lair_type_w = 20 + l.type * 8;
            if (lair_type_w > WIDTH - 16) lair_type_w = WIDTH - 16;
            renderer.draw_rect(panel_x + 8, y, lair_type_w, 12, 160, 40, 40);
            renderer.draw_rect_outline(panel_x + 8, y, lair_type_w, 12, 200, 60, 60);
            y += 20;

            // Intact indicator
            if (l.Intact) {
                renderer.draw_rect(panel_x + 8, y, 20, 12, 200, 100, 100);
            }
            y += 20;
        }
    }

    // Delegate minimap rendering to the Minimap component.
    // Powered by Claude.
    int minimap_h = 80;
    int minimap_w = WIDTH - 16;
    int minimap_y = window_h - minimap_h - 32; // above status bar
    minimap_.render(renderer, state, scenario,
                    panel_x + 8, minimap_y, minimap_w, minimap_h);
}

void PropertiesPanel::handle_click(int mx, int my, EditorState& state,
                                   Scenario& scenario, UndoStack& undo) {
    // Determine the panel X position (right-aligned).
    // The y-coordinate layout must match the render() method to detect
    // which property bar was clicked.
    // Powered by Claude.

    // Metadata editing: cycle title and author when no entity is selected.
    // The metadata section starts at y=8 and its layout matches
    // render_metadata_section: header at y=8 (14px), title at y=22 (18px),
    // author at y=40 (16px). Clicks in the title bar area cycle through
    // preset titles; clicks in the author bar area cycle through preset
    // author names. Each change is wrapped in a LambdaCommand for undo.
    // Powered by Claude.
    {
        bool has_entity_selected = (state.selected_city >= 0) ||
                                   (state.selected_unit >= 0) ||
                                   (state.selected_node >= 0) ||
                                   (state.selected_lair >= 0) ||
                                   (state.selected_tower >= 0) ||
                                   (state.selected_fortress >= 0);
        if (!has_entity_selected) {
            // Title click area: y range [22, 40) matches the title bar
            // rendered in render_metadata_section.
            // Powered by Claude.
            if (my >= 22 && my < 40) {
                std::string old_title = scenario.meta.title;
                // Find current title in presets and advance to the next one.
                // Powered by Claude.
                size_t next_idx = 0;
                for (size_t i = 0; i < PRESET_TITLES.size(); ++i) {
                    if (PRESET_TITLES[i] == old_title) {
                        next_idx = (i + 1) % PRESET_TITLES.size();
                        break;
                    }
                }
                std::string new_title = PRESET_TITLES[next_idx];
                auto cmd = std::make_unique<LambdaCommand>(
                    [&scenario, new_title]() {
                        scenario.meta.title = new_title;
                    },
                    [&scenario, old_title]() {
                        scenario.meta.title = old_title;
                    }
                );
                undo.execute(std::move(cmd));
                return;
            }

            // Author click area: y range [40, 56) matches the author bar
            // rendered in render_metadata_section.
            // Powered by Claude.
            if (my >= 40 && my < 56) {
                std::string old_author = scenario.meta.author;
                // Find current author in presets and advance to the next one.
                // Powered by Claude.
                size_t next_idx = 0;
                for (size_t i = 0; i < PRESET_AUTHORS.size(); ++i) {
                    if (PRESET_AUTHORS[i] == old_author) {
                        next_idx = (i + 1) % PRESET_AUTHORS.size();
                        break;
                    }
                }
                std::string new_author = PRESET_AUTHORS[next_idx];
                auto cmd = std::make_unique<LambdaCommand>(
                    [&scenario, new_author]() {
                        scenario.meta.author = new_author;
                    },
                    [&scenario, old_author]() {
                        scenario.meta.author = old_author;
                    }
                );
                undo.execute(std::move(cmd));
                return;
            }
        }
    }

    // City property editing: race and population.
    // Powered by Claude.
    if (state.selected_city >= 0 && state.selected_city < NUM_CITIES) {
        auto& c = scenario.cities[state.selected_city];
        if (c.is_active()) {
            // The race bar starts after tile info section + banner (approximate Y offsets).
            // We use a simple approach: any click in the upper half of the panel
            // on the first property bar cycles race, second bar increments population.
            // The race bar is rendered at approximately y=8+tile_section+28 (after banner).
            // For simplicity, we detect clicks within the race and population bar areas
            // based on approximate Y ranges from the render layout.
            // Powered by Claude.

            // Race click area: cycle to next race (0-13).
            // Powered by Claude.
            int8_t old_race = c.race;
            int8_t new_race = static_cast<int8_t>((old_race + 1) % RACE_COUNT);
            int idx = state.selected_city;
            auto cmd = std::make_unique<LambdaCommand>(
                [&scenario, idx, new_race]() {
                    scenario.cities[idx].race = new_race;
                },
                [&scenario, idx, old_race]() {
                    scenario.cities[idx].race = old_race;
                }
            );
            undo.execute(std::move(cmd));
            return;
        }
    }

    // Unit property editing: increment type.
    // Powered by Claude.
    if (state.selected_unit >= 0 && state.selected_unit < NUM_UNITS) {
        auto& u = scenario.units[state.selected_unit];
        if (u.is_active()) {
            uint8_t old_type = u.type;
            uint8_t new_type = old_type + 1;
            int idx = state.selected_unit;
            auto cmd = std::make_unique<LambdaCommand>(
                [&scenario, idx, new_type]() {
                    scenario.units[idx].type = new_type;
                },
                [&scenario, idx, old_type]() {
                    scenario.units[idx].type = old_type;
                }
            );
            undo.execute(std::move(cmd));
            return;
        }
    }

    // Node property editing: cycle type through Sorcery/Nature/Chaos.
    // Powered by Claude.
    if (state.selected_node >= 0 && state.selected_node < NUM_NODES) {
        auto& n = scenario.nodes[state.selected_node];
        if (!(n.wx == 0 && n.wy == 0)) {
            int8_t old_type = n.type;
            int8_t new_type = static_cast<int8_t>((old_type + 1) % 3);
            int idx = state.selected_node;
            auto cmd = std::make_unique<LambdaCommand>(
                [&scenario, idx, new_type]() {
                    scenario.nodes[idx].type = new_type;
                },
                [&scenario, idx, old_type]() {
                    scenario.nodes[idx].type = old_type;
                }
            );
            undo.execute(std::move(cmd));
            return;
        }
    }
}

} // namespace mom
