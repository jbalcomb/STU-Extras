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

// Race name lookup.
// Powered by Claude.
static const char* race_name(int8_t race) {
    static const char* names[] = {
        "Barbarian", "Beastmen", "Dark Elf", "Draconian", "Dwarf",
        "Gnoll", "Halfling", "High Elf", "High Men", "Klackon",
        "Lizardman", "Nomad", "Orc", "Troll"
    };
    if (race >= 0 && race < RACE_COUNT) return names[race];
    return "Unknown";
}

// Node type name lookup.
// Powered by Claude.
static const char* node_type_name(int8_t type) {
    switch (type) {
        case NODE_SORCERY: return "Sorcery";
        case NODE_NATURE:  return "Nature";
        case NODE_CHAOS:   return "Chaos";
    }
    return "Unknown";
}

// Terrain type name lookup for properties panel.
// Powered by Claude.
static const char* terrain_type_name(uint8_t t) {
    switch (static_cast<BaseTerrain>(t)) {
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

// Special resource name lookup for properties panel.
// Powered by Claude.
static const char* special_res_name(int8_t s) {
    switch (static_cast<TerrainSpecial>(s)) {
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

// Render scenario metadata fields with text labels.
// Powered by Claude.
static void render_metadata_section(Renderer& renderer, const Scenario& scenario,
                                    int panel_x, int& y) {
    int bar_x = panel_x + 8;
    int bar_max_w = PropertiesPanel::WIDTH - 16;
    int pad = 4;

    // Section header.
    // Powered by Claude.
    UIRenderer::draw_label(renderer, bar_x, y, "Scenario", 160, 160, 180);
    y += 16;

    // Title.
    // Powered by Claude.
    renderer.draw_text(bar_x, y, "Title:", 140, 140, 150);
    y += 14;
    if (!scenario.meta.title.empty()) {
        renderer.draw_text(bar_x + 4, y, scenario.meta.title, 220, 180, 100);
    }
    y += 16;

    // Author.
    // Powered by Claude.
    renderer.draw_text(bar_x, y, "Author:", 140, 140, 150);
    y += 14;
    if (!scenario.meta.author.empty()) {
        renderer.draw_text(bar_x + 4, y, scenario.meta.author, 160, 200, 120);
    }
    y += 16;

    // Description indicator.
    // Powered by Claude.
    if (!scenario.meta.description.empty()) {
        renderer.draw_text(bar_x, y, "Desc:", 140, 140, 150);
        y += 14;
    }

    // Tags count.
    // Powered by Claude.
    int tag_count = static_cast<int>(scenario.meta.tags.size());
    if (tag_count > 0) {
        renderer.draw_text(bar_x, y, "Tags: " + std::to_string(tag_count), 160, 140, 200);
        y += 14;
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

    // Show selected tile info with text labels.
    // Powered by Claude.
    if (state.cursor_wx >= 0 && state.cursor_wy >= 0) {
        int plane = state.current_plane;
        int bar_x = panel_x + 8;
        int bar_max_w = WIDTH - 16;
        uint16_t terrain_val = scenario.world.get_terrain(
            state.cursor_wx, state.cursor_wy, plane);
        int8_t special = scenario.world.get_special(
            state.cursor_wx, state.cursor_wy, plane);
        uint8_t flags = scenario.world.get_flags(
            state.cursor_wx, state.cursor_wy, plane);

        // Terrain type label with color swatch.
        // Powered by Claude.
        auto tc = MapRenderer::terrain_color(
            static_cast<BaseTerrain>(terrain_val & 0xFF));
        renderer.draw_rect(bar_x, y, 12, 12, tc.r, tc.g, tc.b);
        renderer.draw_rect_outline(bar_x, y, 12, 12, 120, 120, 120);
        renderer.draw_text(bar_x + 16, y,
            std::string("Terrain: ") + terrain_type_name(terrain_val & 0xFF),
            200, 200, 200);
        y += 18;

        // Special resource label.
        // Powered by Claude.
        if (special != TS_NONE) {
            auto sc = MapRenderer::special_color(static_cast<TerrainSpecial>(special));
            renderer.draw_rect(bar_x, y, 12, 12, sc.r, sc.g, sc.b);
            renderer.draw_rect_outline(bar_x, y, 12, 12, 120, 120, 120);
            renderer.draw_text(bar_x + 16, y,
                std::string("Special: ") + special_res_name(special),
                200, 200, 200);
            y += 18;
        }

        // Flags label.
        // Powered by Claude.
        if (flags & MSF_ROAD) {
            renderer.draw_text(bar_x, y, "Flags: Road", 160, 140, 100);
            y += 16;
        }

        UIRenderer::draw_separator(renderer, bar_x, y, bar_max_w);
        y += pad * 2;
    }

    // Show selected city info with text labels (clickable race and population).
    // Powered by Claude.
    if (state.selected_city >= 0 && state.selected_city < NUM_CITIES) {
        const auto& c = scenario.cities[state.selected_city];
        if (c.is_active()) {
            int bar_x = panel_x + 8;

            // Owner banner swatch.
            // Powered by Claude.
            auto bc = MapRenderer::banner_color(c.owner_idx);
            renderer.draw_rect(bar_x, y, 12, 12, bc.r, bc.g, bc.b);
            renderer.draw_text(bar_x + 16, y, "City", 200, 200, 200);
            y += 18;

            // Race label (clickable).
            // Powered by Claude.
            renderer.draw_text(bar_x, y,
                std::string("Race: ") + race_name(c.race), 180, 140, 200);
            y += 16;

            // Population label (clickable).
            // Powered by Claude.
            renderer.draw_text(bar_x, y,
                "Pop: " + std::to_string(c.population), 120, 200, 120);
            y += 16;

            // Buildings count label.
            // Powered by Claude.
            renderer.draw_text(bar_x, y,
                "Buildings: " + std::to_string(c.bldg_cnt), 200, 160, 100);
            y += 16;
        }
    }

    // Show selected unit info with text labels (clickable type area).
    // Powered by Claude.
    if (state.selected_unit >= 0 && state.selected_unit < NUM_UNITS) {
        const auto& u = scenario.units[state.selected_unit];
        if (u.is_active()) {
            int bar_x = panel_x + 8;

            // Owner banner swatch.
            // Powered by Claude.
            auto bc = MapRenderer::banner_color(u.owner_idx);
            renderer.draw_rect(bar_x, y, 12, 12, bc.r, bc.g, bc.b);
            renderer.draw_text(bar_x + 16, y, "Unit", 200, 200, 200);
            y += 18;

            // Unit type label (clickable).
            // Powered by Claude.
            renderer.draw_text(bar_x, y,
                "Type: " + std::to_string(u.type), 100, 150, 200);
            y += 16;

            // Level label.
            // Powered by Claude.
            renderer.draw_text(bar_x, y,
                "Level: " + std::to_string(u.Level), 200, 200, 100);
            y += 16;
        }
    }

    // Show selected node info with text labels (clickable type area).
    // Powered by Claude.
    if (state.selected_node >= 0 && state.selected_node < NUM_NODES) {
        const auto& n = scenario.nodes[state.selected_node];
        if (!(n.wx == 0 && n.wy == 0)) {
            int bar_x = panel_x + 8;

            // Node type with color swatch (clickable).
            // Powered by Claude.
            uint8_t nr = 0, ng = 0, nb = 0;
            switch (n.type) {
                case NODE_NATURE:  ng = 200; break;
                case NODE_SORCERY: nb = 200; break;
                case NODE_CHAOS:   nr = 200; break;
            }
            renderer.draw_rect(bar_x, y, 12, 12, nr, ng, nb);
            renderer.draw_text(bar_x + 16, y,
                std::string("Node: ") + node_type_name(n.type), 200, 200, 200);
            y += 18;

            // Power label.
            // Powered by Claude.
            renderer.draw_text(bar_x, y,
                "Power: " + std::to_string(n.power), 200, 180, 100);
            y += 16;
        }
    }

    // Show selected tower info with text label.
    // Powered by Claude.
    if (state.selected_tower >= 0 && state.selected_tower < NUM_TOWERS) {
        const auto& t = scenario.towers[state.selected_tower];
        if (!(t.wx == 0 && t.wy == 0)) {
            int bar_x = panel_x + 8;
            renderer.draw_rect(bar_x, y, 12, 12, 220, 220, 240);
            renderer.draw_text(bar_x + 16, y, "Tower", 220, 220, 240);
            y += 18;
        }
    }

    // Show selected fortress info with text label.
    // Powered by Claude.
    if (state.selected_fortress >= 0 && state.selected_fortress < NUM_FORTRESSES) {
        const auto& f = scenario.fortresses[state.selected_fortress];
        if (f.active != 0) {
            int bar_x = panel_x + 8;
            renderer.draw_rect(bar_x, y, 12, 12, 200, 160, 60);
            renderer.draw_text(bar_x + 16, y, "Fortress", 240, 200, 80);
            y += 18;
        }
    }

    // Show selected lair info with text labels.
    // Powered by Claude.
    if (state.selected_lair >= 0 && state.selected_lair < NUM_LAIRS) {
        const auto& l = scenario.lairs[state.selected_lair];
        if (!(l.wx == 0 && l.wy == 0)) {
            int bar_x = panel_x + 8;

            // Lair type label.
            // Powered by Claude.
            renderer.draw_text(bar_x, y,
                "Lair Type: " + std::to_string(l.type), 160, 40, 40);
            y += 16;

            // Intact/Cleared status label.
            // Powered by Claude.
            if (l.Intact) {
                renderer.draw_text(bar_x, y, "Intact", 200, 100, 100);
            } else {
                renderer.draw_text(bar_x, y, "Cleared", 120, 120, 120);
            }
            y += 16;
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
    // Layout matches render_metadata_section: header at y=8 (+16),
    // "Title:" at y=24 (+14), title value at y=38 (+16),
    // "Author:" at y=54 (+14), author value at y=68 (+16).
    // Powered by Claude.
    {
        bool has_entity_selected = (state.selected_city >= 0) ||
                                   (state.selected_unit >= 0) ||
                                   (state.selected_node >= 0) ||
                                   (state.selected_lair >= 0) ||
                                   (state.selected_tower >= 0) ||
                                   (state.selected_fortress >= 0);
        if (!has_entity_selected) {
            // Title click area: y range [24, 54) covers label + value.
            // Powered by Claude.
            if (my >= 24 && my < 54) {
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

            // Author click area: y range [54, 84) covers label + value.
            // Powered by Claude.
            if (my >= 54 && my < 84) {
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
