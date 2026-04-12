// MoM Community Scenario Editor -- main entry point.
// SDL2 application with map editor, tool palette, and properties panel.
// Powered by Claude.

#include "renderer/Renderer.hpp"
#include "renderer/MapRenderer.hpp"
#include "renderer/UIRenderer.hpp"
#include "editor/EditorState.hpp"
#include "editor/UndoStack.hpp"
#include "scenario/Scenario.hpp"
#include "scenario/ScenarioIO.hpp"
#include "mom_data/MomGamFile.hpp"
#include "mapgen/MapGenerator.hpp"
#include "mom_data/WizardPresets.hpp"
#include "validation/SmoothingValidator.hpp"
#include "ui/ToolPanel.hpp"
#include "ui/PropertiesPanel.hpp"
#include "ui/GameOptionsPanel.hpp"
#include "ui/WizardPanel.hpp"
#include "platform/FileDialog.hpp"
#include <SDL.h>
#include <algorithm>
#include <cassert>
#include <cstdio>
#include <string>
#include <vector>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

using namespace mom;

// Generate a new map from params, wrapped in an undo command.
// Powered by Claude.
static void do_generate_map(EditorState& state, Scenario& scenario, UndoStack& undo) {
    // Capture the entire world state for undo.
    MomWorld before = scenario.world;
    MapGenParams params = state.map_gen_params;

    // The execute lambda generates the map; undo restores the previous state.
    auto cmd = std::make_unique<LambdaCommand>(
        [&scenario, params]() {
            MapGenerator gen(params);
            gen.generate(scenario.world);
        },
        [&scenario, before]() { scenario.world = before; }
    );
    undo.execute(std::move(cmd));
}

// Generate all 6 wizard profiles from preset data, wrapped in an undo command.
// Powered by Claude.
static void do_generate_wizards(EditorState& state, Scenario& scenario, UndoStack& undo) {
    // Capture all 6 wizard structs for undo.
    Wizard before[NUM_PLAYERS];
    for (int i = 0; i < NUM_PLAYERS; ++i)
        before[i] = scenario.wizards[i];

    auto cmd = std::make_unique<LambdaCommand>(
        [&scenario, &state]() {
            for (int i = 0; i < NUM_PLAYERS; ++i) {
                apply_preset(scenario.wizards[i], i, WIZARD_PRESETS[i]);
                state.wizard_dirty[i] = false;
            }
        },
        [&scenario, &state, before]() {
            for (int i = 0; i < NUM_PLAYERS; ++i) {
                scenario.wizards[i] = before[i];
                state.wizard_dirty[i] = false;
            }
        }
    );
    undo.execute(std::move(cmd));
    state.set_status("Generated 6 wizard profiles");
    printf("[main] do_generate_wizards: applied 6 presets\n");
}

// Generate a single wizard profile from preset data, wrapped in an undo command.
// Powered by Claude.
static void do_generate_wizard(EditorState& state, Scenario& scenario, UndoStack& undo, int idx) {
    Wizard before = scenario.wizards[idx];

    auto cmd = std::make_unique<LambdaCommand>(
        [&scenario, &state, idx]() {
            apply_preset(scenario.wizards[idx], idx, WIZARD_PRESETS[idx]);
            state.wizard_dirty[idx] = false;
        },
        [&scenario, &state, idx, before]() {
            scenario.wizards[idx] = before;
            state.wizard_dirty[idx] = false;
        }
    );
    undo.execute(std::move(cmd));
}

// Export the current scenario as a .GAM binary file with status feedback.
// Updates entity counts, serializes, saves via platform dialog, and shows
// a status message with entity summary and any warnings.
// Powered by Claude.
static void do_export_gam(EditorState& state, Scenario& scenario) {
    // Update entity counts before export.
    int city_count = scenario.count_active_cities();
    int unit_count = scenario.count_active_units();
    int wiz_count = 0;
    for (auto& w : scenario.wizards) if (w.is_active()) ++wiz_count;

    scenario.game_data.Total_Cities  = static_cast<uint16_t>(city_count);
    scenario.game_data.Total_Units   = static_cast<uint16_t>(unit_count);
    scenario.game_data.Total_Wizards = static_cast<uint16_t>(wiz_count > 1 ? wiz_count : 1);

    // Log entity state before export for diagnostics.
    // Powered by Claude.
    printf("[export] %d wizards, %d cities, %d units\n",
           wiz_count, city_count, unit_count);
    for (int i = 0; i < NUM_TOWERS; ++i) {
        auto& t = scenario.towers[i];
        printf("[export] tower[%d]: wx=%d wy=%d owner=%d\n",
               i, t.wx, t.wy, t.owner_idx);
    }
    for (int i = 0; i < NUM_FORTRESSES; ++i) {
        auto& f = scenario.fortresses[i];
        printf("[export] fortress[%d]: wx=%d wy=%d wp=%d active=%d\n",
               i, f.wx, f.wy, f.wp, f.active);
    }
    for (int i = 0; i < NUM_CITIES; ++i) {
        auto& c = scenario.cities[i];
        if (c.is_active()) {
            printf("[export] city[%d]: \"%s\" wx=%d wy=%d wp=%d owner=%d size=%d\n",
                   i, c.name, c.wx, c.wy, c.wp, c.owner_idx, c.size);
        }
    }

    // Smooth terrain on a temporary copy before export.
    // Powered by Claude.
    Scenario export_sc = scenario;
    int smoothed = smooth_terrain_for_export(export_sc, scenario);
    if (smoothed > 0) {
        printf("[main] export smoothing: fixed %d squares\n", smoothed);
    }

    // Serialize and save the smoothed copy.
    auto gam_buf = serialize_gam(export_sc);
    platform::save_file("scenario.GAM", "GAM Save Files", "*.GAM", gam_buf);

    // Build status message with entity counts and warnings.
    std::string msg = "Exported scenario.GAM (" +
        std::to_string(wiz_count) + " wizards, " +
        std::to_string(city_count) + " cities, " +
        std::to_string(unit_count) + " units)";

    if (wiz_count == 0) {
        msg += " — no active wizards";
    }
    if (!state.violations.empty()) {
        msg += " — " + std::to_string(state.violations.size()) + " violations";
    }

    state.set_status(msg);
}

// Apply the current editor tool at the cursor position.
// Powered by Claude.
static void apply_tool(EditorState& state, Scenario& scenario, UndoStack& undo) {
    int wx = state.cursor_wx;
    int wy = state.cursor_wy;
    int plane = state.current_plane;
    if (wx < 0 || wy < 0) return;

    switch (state.tool) {
        case EditorTool::PAINT_TERRAIN: {
            uint16_t old_val = scenario.world.get_terrain(wx, wy, plane);
            uint16_t new_val = static_cast<uint16_t>(state.paint_terrain);
            if (old_val == new_val) return;
            auto cmd = std::make_unique<LambdaCommand>(
                [&scenario, wx, wy, plane, new_val]() {
                    scenario.world.set_terrain(wx, wy, plane, new_val);
                },
                [&scenario, wx, wy, plane, old_val]() {
                    scenario.world.set_terrain(wx, wy, plane, old_val);
                }
            );
            undo.execute(std::move(cmd));

            // Revalidate edited tile and cardinal neighbors.
            // Powered by Claude.
            {
                auto& v = state.violations;
                auto remove_at = [&v](int p, int tx, int ty) {
                    v.erase(std::remove_if(v.begin(), v.end(),
                        [p, tx, ty](const SmoothingViolation& sv) {
                            return sv.plane == p && sv.x == tx && sv.y == ty;
                        }), v.end());
                };
                int dirs[][2] = {{0,0},{0,-1},{0,1},{-1,0},{1,0}};
                for (auto& d : dirs) {
                    int nx = ((wx + d[0]) % WORLD_WIDTH + WORLD_WIDTH) % WORLD_WIDTH;
                    int ny = wy + d[1];
                    if (ny < 0 || ny >= WORLD_HEIGHT) continue;
                    remove_at(plane, nx, ny);
                    auto new_violations = SmoothingValidator::validate_tile(
                        scenario.world, plane, nx, ny);
                    v.insert(v.end(), new_violations.begin(), new_violations.end());
                }
            }

            break;
        }
        case EditorTool::PAINT_SPECIAL: {
            int8_t old_val = scenario.world.get_special(wx, wy, plane);
            int8_t new_val = static_cast<int8_t>(state.paint_special);
            if (old_val == new_val) return;
            auto cmd = std::make_unique<LambdaCommand>(
                [&scenario, wx, wy, plane, new_val]() {
                    scenario.world.set_special(wx, wy, plane, new_val);
                },
                [&scenario, wx, wy, plane, old_val]() {
                    scenario.world.set_special(wx, wy, plane, old_val);
                }
            );
            undo.execute(std::move(cmd));
            break;
        }
        case EditorTool::PAINT_FLAGS: {
            uint8_t old_flags = scenario.world.get_flags(wx, wy, plane);
            uint8_t flag = state.paint_flag;
            uint8_t new_flags;
            if (old_flags & flag) {
                new_flags = old_flags & ~flag; // clear the flag
            } else {
                new_flags = old_flags | flag;  // set the flag
            }
            if (old_flags == new_flags) return;
            auto cmd = std::make_unique<LambdaCommand>(
                [&scenario, wx, wy, plane, new_flags]() {
                    scenario.world.set_flags(wx, wy, plane, new_flags);
                },
                [&scenario, wx, wy, plane, old_flags]() {
                    scenario.world.set_flags(wx, wy, plane, old_flags);
                }
            );
            undo.execute(std::move(cmd));
            break;
        }
        case EditorTool::PLACE_CITY: {
            // Find first empty city slot
            for (int i = 0; i < NUM_CITIES; ++i) {
                if (!scenario.cities[i].is_active()) {
                    int idx = i;
                    auto cmd = std::make_unique<LambdaCommand>(
                        [&scenario, &state, idx, wx, wy, plane]() {
                            auto& c = scenario.cities[idx];
                            std::snprintf(c.name, LEN_CITY_NAME, "City %d", idx);
                            c.race = state.place_race;
                            c.wx = static_cast<int8_t>(wx);
                            c.wy = static_cast<int8_t>(wy);
                            c.wp = static_cast<int8_t>(plane);
                            c.owner_idx = state.place_owner;
                            c.population = 1;
                            c.size = 1;
                        },
                        [&scenario, idx]() {
                            scenario.cities[idx] = City{};
                        }
                    );
                    undo.execute(std::move(cmd));
                    break;
                }
            }
            break;
        }
        case EditorTool::PLACE_UNIT: {
            // Find first empty unit slot
            for (int i = 0; i < NUM_UNITS; ++i) {
                if (!scenario.units[i].is_active()) {
                    int idx = i;
                    auto cmd = std::make_unique<LambdaCommand>(
                        [&scenario, &state, idx, wx, wy, plane]() {
                            auto& u = scenario.units[idx];
                            u.type = state.place_unit_type;
                            u.wx = static_cast<int8_t>(wx);
                            u.wy = static_cast<int8_t>(wy);
                            u.wp = static_cast<int8_t>(plane);
                            u.owner_idx = state.place_owner;
                            u.Level = 1;
                        },
                        [&scenario, idx]() {
                            scenario.units[idx] = Unit{};
                        }
                    );
                    undo.execute(std::move(cmd));
                    break;
                }
            }
            break;
        }
        case EditorTool::PLACE_NODE: {
            // Find first inactive node slot (owner_idx <= -1 means empty).
            // Powered by Claude.
            for (int i = 0; i < NUM_NODES; ++i) {
                if (scenario.nodes[i].owner_idx <= -1) {
                    int idx = i;
                    int8_t node_type = state.place_node_type;
                    auto cmd = std::make_unique<LambdaCommand>(
                        [&scenario, idx, wx, wy, plane, node_type]() {
                            auto& n = scenario.nodes[idx];
                            n.wx = static_cast<int8_t>(wx);
                            n.wy = static_cast<int8_t>(wy);
                            n.wp = static_cast<int8_t>(plane);
                            n.type = node_type;
                            n.power = 5;
                            n.owner_idx = 0; // mark active
                        },
                        [&scenario, idx]() {
                            scenario.nodes[idx] = Node{};
                            scenario.nodes[idx].wx = 1;
                            scenario.nodes[idx].wy = 1;
                            scenario.nodes[idx].wp = 1;
                            scenario.nodes[idx].owner_idx = -1;
                        }
                    );
                    undo.execute(std::move(cmd));
                    return;
                }
            }
            {
                int used = 0;
                for (int i = 0; i < NUM_NODES; ++i)
                    if (!(scenario.nodes[i].owner_idx <= -1)) ++used;
                printf("[DEBUG] node slots: %d/%d used\n", used, NUM_NODES);
                state.set_status("All 30 node slots are full");
            }
            break;
        }
        case EditorTool::PLACE_TOWER: {
            // Find first inactive tower slot (owner_idx <= -1 means empty).
            // Powered by Claude.
            for (int i = 0; i < NUM_TOWERS; ++i) {
                auto& t = scenario.towers[i];
                if (t.owner_idx <= -1) {
                    int idx = i;
                    auto cmd = std::make_unique<LambdaCommand>(
                        [&scenario, idx, wx, wy]() {
                            auto& tw = scenario.towers[idx];
                            tw.wx = static_cast<int8_t>(wx);
                            tw.wy = static_cast<int8_t>(wy);
                            tw.owner_idx = -1;
                        },
                        [&scenario, idx]() {
                            scenario.towers[idx] = Tower{};
                            scenario.towers[idx].wx = 1;
                            scenario.towers[idx].wy = 1;
                            scenario.towers[idx].owner_idx = -1;
                        }
                    );
                    undo.execute(std::move(cmd));
                    return;
                }
            }
            state.set_status("All 6 tower slots are full");
            break;
        }
        case EditorTool::PLACE_FORTRESS: {
            // Find first inactive fortress slot (active==0 means empty).
            // Fortress index == wizard index (1:1 mapping).
            // Auto-activates the corresponding wizard if inactive.
            // Powered by Claude.
            for (int i = 0; i < NUM_FORTRESSES; ++i) {
                auto& f = scenario.fortresses[i];
                if (f.active == 0) {
                    int idx = i;
                    bool wiz_was_inactive = !scenario.wizards[idx].is_active();
                    Wizard wiz_backup = scenario.wizards[idx];
                    auto cmd = std::make_unique<LambdaCommand>(
                        [&scenario, idx, wx, wy, plane, wiz_was_inactive]() {
                            auto& ft = scenario.fortresses[idx];
                            ft.wx = static_cast<int8_t>(wx);
                            ft.wy = static_cast<int8_t>(wy);
                            ft.wp = static_cast<int8_t>(plane);
                            ft.active = 1;
                            // Auto-activate wizard stub if not already active.
                            // Powered by Claude.
                            if (wiz_was_inactive) {
                                std::snprintf(scenario.wizards[idx].name,
                                              LEN_WIZARD_NAME, "Wizard %d", idx);
                                scenario.wizards[idx].wizard_id =
                                    static_cast<uint8_t>(idx);
                                scenario.wizards[idx].banner_id =
                                    static_cast<uint8_t>(idx);
                            }
                        },
                        [&scenario, idx, wiz_backup]() {
                            scenario.fortresses[idx] = Fortress{};
                            // Restore wizard to previous state on undo.
                            // Powered by Claude.
                            scenario.wizards[idx] = wiz_backup;
                        }
                    );
                    undo.execute(std::move(cmd));
                    if (wiz_was_inactive) {
                        state.wizard_dirty[idx] = true;
                        state.set_status("Fortress placed — Wizard " +
                                         std::to_string(idx) + " activated");
                    }
                    return;
                }
            }
            state.set_status("All 6 fortress slots are full");
            break;
        }
        case EditorTool::PLACE_LAIR: {
            // Find first inactive lair slot (Intact <= -1 means empty).
            // Powered by Claude.
            for (int i = 0; i < NUM_LAIRS; ++i) {
                auto& l = scenario.lairs[i];
                if (l.Intact <= -1) {
                    int idx = i;
                    auto cmd = std::make_unique<LambdaCommand>(
                        [&scenario, idx, wx, wy, plane]() {
                            auto& lr = scenario.lairs[idx];
                            lr.wx = static_cast<int8_t>(wx);
                            lr.wy = static_cast<int8_t>(wy);
                            lr.wp = static_cast<int8_t>(plane);
                            lr.type = 1;
                            lr.Intact = 1;
                        },
                        [&scenario, idx]() {
                            scenario.lairs[idx] = Lair{};
                            scenario.lairs[idx].wx = 1;
                            scenario.lairs[idx].wy = 1;
                            scenario.lairs[idx].wp = 1;
                            scenario.lairs[idx].Intact = -1;
                        }
                    );
                    undo.execute(std::move(cmd));
                    return;
                }
            }
            state.set_status("All 102 lair slots are full");
            break;
        }
        case EditorTool::ERASE: {
            // Erase any entity at cursor position.
            // Powered by Claude.
            for (int i = 0; i < NUM_CITIES; ++i) {
                auto& c = scenario.cities[i];
                if (c.is_active() && c.wx == wx && c.wy == wy && c.wp == plane) {
                    City backup = c;
                    int idx = i;
                    auto cmd = std::make_unique<LambdaCommand>(
                        [&scenario, idx]() { scenario.cities[idx] = City{}; },
                        [&scenario, idx, backup]() { scenario.cities[idx] = backup; }
                    );
                    undo.execute(std::move(cmd));
                    return;
                }
            }
            for (int i = 0; i < NUM_UNITS; ++i) {
                auto& u = scenario.units[i];
                if (u.is_active() && u.wx == wx && u.wy == wy && u.wp == plane) {
                    Unit backup = u;
                    int idx = i;
                    auto cmd = std::make_unique<LambdaCommand>(
                        [&scenario, idx]() { scenario.units[idx] = Unit{}; },
                        [&scenario, idx, backup]() { scenario.units[idx] = backup; }
                    );
                    undo.execute(std::move(cmd));
                    return;
                }
            }
            // Erase nodes at cursor.
            // Powered by Claude.
            for (int i = 0; i < NUM_NODES; ++i) {
                auto& n = scenario.nodes[i];
                if (!(n.owner_idx <= -1) && n.wx == wx && n.wy == wy && n.wp == plane) {
                    Node backup = n;
                    int idx = i;
                    auto cmd = std::make_unique<LambdaCommand>(
                        [&scenario, idx]() {
                            scenario.nodes[idx] = Node{};
                            scenario.nodes[idx].wx = 1;
                            scenario.nodes[idx].wy = 1;
                            scenario.nodes[idx].wp = 1;
                            scenario.nodes[idx].owner_idx = -1;
                        },
                        [&scenario, idx, backup]() { scenario.nodes[idx] = backup; }
                    );
                    undo.execute(std::move(cmd));
                    return;
                }
            }
            // Erase towers at cursor.
            // Powered by Claude.
            for (int i = 0; i < NUM_TOWERS; ++i) {
                auto& t = scenario.towers[i];
                if (!(t.owner_idx <= -1) && t.wx == wx && t.wy == wy) {
                    Tower backup = t;
                    int idx = i;
                    auto cmd = std::make_unique<LambdaCommand>(
                        [&scenario, idx]() {
                            scenario.towers[idx] = Tower{};
                            scenario.towers[idx].wx = 1;
                            scenario.towers[idx].wy = 1;
                            scenario.towers[idx].owner_idx = -1;
                        },
                        [&scenario, idx, backup]() { scenario.towers[idx] = backup; }
                    );
                    undo.execute(std::move(cmd));
                    return;
                }
            }
            // Erase fortresses at cursor.
            // Powered by Claude.
            for (int i = 0; i < NUM_FORTRESSES; ++i) {
                auto& f = scenario.fortresses[i];
                if (f.active != 0 && f.wx == wx && f.wy == wy && f.wp == plane) {
                    Fortress backup = f;
                    int idx = i;
                    auto cmd = std::make_unique<LambdaCommand>(
                        [&scenario, idx]() { scenario.fortresses[idx] = Fortress{}; },
                        [&scenario, idx, backup]() { scenario.fortresses[idx] = backup; }
                    );
                    undo.execute(std::move(cmd));
                    return;
                }
            }
            // Erase lairs at cursor.
            // Powered by Claude.
            for (int i = 0; i < NUM_LAIRS; ++i) {
                auto& l = scenario.lairs[i];
                if (!(l.Intact <= -1) && l.wx == wx && l.wy == wy && l.wp == plane) {
                    Lair backup = l;
                    int idx = i;
                    auto cmd = std::make_unique<LambdaCommand>(
                        [&scenario, idx]() {
                            scenario.lairs[idx] = Lair{};
                            scenario.lairs[idx].wx = 1;
                            scenario.lairs[idx].wy = 1;
                            scenario.lairs[idx].wp = 1;
                            scenario.lairs[idx].Intact = -1;
                        },
                        [&scenario, idx, backup]() { scenario.lairs[idx] = backup; }
                    );
                    undo.execute(std::move(cmd));
                    return;
                }
            }
            break;
        }
        case EditorTool::SELECT: {
            state.deselect();
            // Select city at cursor.
            // Powered by Claude.
            for (int i = 0; i < NUM_CITIES; ++i) {
                auto& c = scenario.cities[i];
                if (c.is_active() && c.wx == wx && c.wy == wy && c.wp == plane) {
                    state.selected_city = i;
                    return;
                }
            }
            // Select unit at cursor.
            // Powered by Claude.
            for (int i = 0; i < NUM_UNITS; ++i) {
                auto& u = scenario.units[i];
                if (u.is_active() && u.wx == wx && u.wy == wy && u.wp == plane) {
                    state.selected_unit = i;
                    return;
                }
            }
            // Select node at cursor.
            // Powered by Claude.
            for (int i = 0; i < NUM_NODES; ++i) {
                auto& n = scenario.nodes[i];
                if (!(n.owner_idx <= -1) && n.wx == wx && n.wy == wy && n.wp == plane) {
                    state.selected_node = i;
                    return;
                }
            }
            // Select tower at cursor.
            // Powered by Claude.
            for (int i = 0; i < NUM_TOWERS; ++i) {
                auto& t = scenario.towers[i];
                if (!(t.owner_idx <= -1) && t.wx == wx && t.wy == wy) {
                    state.selected_tower = i;
                    return;
                }
            }
            // Select fortress at cursor.
            // Powered by Claude.
            for (int i = 0; i < NUM_FORTRESSES; ++i) {
                auto& f = scenario.fortresses[i];
                if (f.active != 0 && f.wx == wx && f.wy == wy && f.wp == plane) {
                    state.selected_fortress = i;
                    return;
                }
            }
            // Select lair at cursor.
            // Powered by Claude.
            for (int i = 0; i < NUM_LAIRS; ++i) {
                auto& l = scenario.lairs[i];
                if (!(l.Intact <= -1) && l.wx == wx && l.wy == wy && l.wp == plane) {
                    state.selected_lair = i;
                    return;
                }
            }
            break;
        }
        default:
            break;
    }
}

// Application state struct for Emscripten main loop callback.
// Powered by Claude.
struct AppState {
    Renderer renderer;
    Scenario scenario;
    EditorState editor_state;
    UndoStack undo_stack;
    ToolPanel tool_panel;
    PropertiesPanel props_panel;
    GameOptionsPanel game_options_panel;
    WizardPanel wizard_panel;
    MapRenderer map_renderer;
    bool running{true};
    bool mouse_down{false};
    int prev_mouse_x{-1};  // previous mouse pixel position for drag interpolation
    int prev_mouse_y{-1};
    Uint32 last_tick{0};
};

static void main_loop_iteration(void* arg);

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
#else
int main(int, char**) {
#endif
    static AppState app;
    printf("=== MoM Scenario Editor v%s (%s %s) ===\n",
           EDITOR_VERSION, __DATE__, __TIME__);

    if (!app.renderer.init("MoM Scenario Editor", 1280, 720)) {
        std::fprintf(stderr, "Failed to initialize SDL2\n");
        return 1;
    }

    // Log renderer info for WASM builds to aid diagnostics.
    // Powered by Claude.
#ifdef __EMSCRIPTEN__
    // Build timestamp for diagnostics. Powered by Claude.
    {
        // __DATE__ = "Mar  9 2026", __TIME__ = "14:30:05"
        // Convert to ISO 8601: 2026-03-09T14:30:05Z
        static const char* months[] = {
            "Jan","Feb","Mar","Apr","May","Jun",
            "Jul","Aug","Sep","Oct","Nov","Dec"
        };
        const char* d = __DATE__;
        int mon = 0;
        for (int i = 0; i < 12; ++i) {
            if (d[0]==months[i][0] && d[1]==months[i][1] && d[2]==months[i][2]) {
                mon = i + 1; break;
            }
        }
        int day = (d[4]==' ' ? 0 : d[4]-'0') * 10 + (d[5]-'0');
        char ts[32];
        std::snprintf(ts, sizeof(ts), "%.4s-%02d-%02dT%sZ", d+7, mon, day, __TIME__);
        EM_ASM({
            console.log("MoM Scenario Editor loaded - WASM build");
            console.log("[DEBUG] Build timestamp (UTC): " + UTF8ToString($0));
        }, ts);
    }
#endif

    app.scenario.clear();

    // Debug: dump entity state at startup for diagnostics.
    printf("[DEBUG] cursor: wx=%d wy=%d\n",
           app.editor_state.cursor_wx, app.editor_state.cursor_wy);
    for (int i = 0; i < NUM_TOWERS; ++i) {
        auto& t = app.scenario.towers[i];
        if (t.wx != 0 || t.wy != 0)
            printf("[DEBUG] tower[%d]: wx=%d wy=%d owner=%d\n",
                   i, t.wx, t.wy, t.owner_idx);
    }
    for (int i = 0; i < NUM_FORTRESSES; ++i) {
        auto& f = app.scenario.fortresses[i];
        if (f.active != 0)
            printf("[DEBUG] fortress[%d]: wx=%d wy=%d active=%d\n",
                   i, f.wx, f.wy, f.active);
    }
    for (int i = 0; i < NUM_NODES; ++i) {
        auto& n = app.scenario.nodes[i];
        if (n.wx != 0 || n.wy != 0)
            printf("[DEBUG] node[%d]: wx=%d wy=%d\n", i, n.wx, n.wy);
    }

    // Fill with grassland by default
    for (int p = 0; p < NUM_PLANES; ++p) {
        for (int i = 0; i < WORLD_SIZE; ++i) {
            app.scenario.world.terrain[p][i] = TERRAIN_GRASSLAND;
        }
    }

    app.last_tick = SDL_GetTicks();

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(main_loop_iteration, &app, 0, 1);
#else
    while (app.running) {
        main_loop_iteration(&app);
    }
    app.renderer.shutdown();
#endif
    return 0;
}

// One iteration of the main event/render loop.
// Powered by Claude.
static void main_loop_iteration(void* arg) {
    AppState& a = *static_cast<AppState*>(arg);
    auto& renderer = a.renderer;
    auto& scenario = a.scenario;
    auto& editor_state = a.editor_state;
    auto& undo_stack = a.undo_stack;
    auto& tool_panel = a.tool_panel;
    auto& props_panel = a.props_panel;
    auto& game_options_panel = a.game_options_panel;
    auto& wizard_panel = a.wizard_panel;
    auto& map_renderer = a.map_renderer;
    auto& running = a.running;
    auto& mouse_down = a.mouse_down;
    auto& prev_mouse_x = a.prev_mouse_x;
    auto& prev_mouse_y = a.prev_mouse_y;

        // Calculate frame delta time for status timer.
        // Powered by Claude.
        Uint32 now_tick = SDL_GetTicks();
        float dt = (now_tick - a.last_tick) / 1000.0f;
        a.last_tick = now_tick;

        // Decrement status message timer.
        // Powered by Claude.
        if (editor_state.status_timer > 0.0f) {
            editor_state.status_timer -= dt;
            if (editor_state.status_timer <= 0.0f) {
                editor_state.status_timer = 0.0f;
                editor_state.status_message.clear();
            }
        }

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = false;
                    break;

                case SDL_WINDOWEVENT:
                    if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                        renderer.update_window_size();
                    }
                    break;

                case SDL_KEYDOWN: {
                    auto key = event.key.keysym;
                    bool ctrl = (key.mod & KMOD_CTRL) != 0;

                    if (key.sym == SDLK_TAB) {
                        editor_state.toggle_plane();
                    } else if (key.sym == SDLK_g) {
                        editor_state.show_grid = !editor_state.show_grid;
                    } else if (ctrl && key.sym == SDLK_z) {
                        undo_stack.undo();
                    } else if (ctrl && key.sym == SDLK_y) {
                        undo_stack.redo();
                    } else if (ctrl && key.sym == SDLK_s) {
                        // Save scenario via platform file dialog.
                        // Powered by Claude.
                        auto buf = serialize_scenario(scenario);
                        if (!buf.empty()) {
                            platform::save_file("scenario.mom_scenario",
                                                "MoM Scenario Files",
                                                "*.mom_scenario",
                                                buf);
                        }
                    } else if (ctrl && key.sym == SDLK_o) {
                        // Load scenario via platform file dialog.
                        // Powered by Claude.
                        std::vector<uint8_t> buf;
                        if (platform::load_file("MoM Scenario Files",
                                                "*.mom_scenario", buf)) {
                            if (deserialize_scenario(buf, scenario)) {
                                // Clear undo stack after loading a new scenario.
                                // Powered by Claude.
                                undo_stack.clear();

                                // Revalidate smoothing violations for both planes.
                                // Powered by Claude.
                                editor_state.violations.clear();
                                for (int p = 0; p < NUM_PLANES; ++p) {
                                    auto pv = SmoothingValidator::validate_plane(
                                        scenario.world, p);
                                    editor_state.violations.insert(
                                        editor_state.violations.end(),
                                        pv.begin(), pv.end());
                                }
                            }
                        }
                    } else if (ctrl && key.sym == SDLK_n) {
                        do_generate_map(editor_state, scenario, undo_stack);
                    } else if (ctrl && key.sym == SDLK_e) {
                        // Export .GAM via shared helper (Ctrl+E shortcut).
                        // Powered by Claude.
                        do_export_gam(editor_state, scenario);
                    }
                    // Import an external .GAM save file (Ctrl+I).
                    // Reads the binary save via file dialog, populates the scenario,
                    // and clears undo history.
                    // Powered by Claude.
                    else if (ctrl && key.sym == SDLK_i) {
                        std::vector<uint8_t> gam_data;
                        if (platform::load_file("GAM Save Files", "*.GAM", gam_data)) {
                            if (deserialize_gam(gam_data, scenario)) {
                                undo_stack.clear();
                                editor_state.deselect();
                                editor_state.set_status("Imported .GAM save file");

                                // Revalidate smoothing violations after import.
                                // Powered by Claude.
                                editor_state.violations.clear();
                                for (int p = 0; p < NUM_PLANES; ++p) {
                                    auto pv = SmoothingValidator::validate_plane(
                                        scenario.world, p);
                                    editor_state.violations.insert(
                                        editor_state.violations.end(),
                                        pv.begin(), pv.end());
                                }
                            } else {
                                editor_state.set_status("Failed to import .GAM file");
                            }
                        }
                    }
                    // Toggle smoothing violation overlay.
                    // Powered by Claude.
                    else if (key.sym == SDLK_v) {
                        editor_state.show_all_violations = !editor_state.show_all_violations;
                    }
                    // Arrow key scrolling
                    else if (key.sym == SDLK_LEFT)  renderer.camera.offset_x -= 40;
                    else if (key.sym == SDLK_RIGHT) renderer.camera.offset_x += 40;
                    else if (key.sym == SDLK_UP)    renderer.camera.offset_y -= 40;
                    else if (key.sym == SDLK_DOWN)  renderer.camera.offset_y += 40;
                    break;
                }

                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        mouse_down = true;
                        int mx = event.button.x;
                        int my = event.button.y;

                        // Check tool panel first
                        if (tool_panel.handle_click(mx, my, editor_state, renderer.window_height())) {
                            break;
                        }

                        // Check minimap click in the properties panel.
                        // Clicking a minimap while the wizard editor is open
                        // closes it so the map becomes active. Powered by Claude.
                        if (props_panel.handle_minimap_click(
                                mx, my, renderer.camera, editor_state, scenario,
                                renderer.window_width(), renderer.window_height())) {
                            if (editor_state.tool == EditorTool::WIZARDS) {
                                editor_state.tool = EditorTool::SELECT;
                            }
                            break;
                        }

                        // Check properties panel click for interactive editing.
                        // Powered by Claude.
                        {
                            int props_x = renderer.window_width() - PropertiesPanel::WIDTH;
                            if (mx >= props_x) {
                                props_panel.handle_click(mx, my, editor_state, scenario, undo_stack);
                                break;
                            }
                        }

                        // Check game options panel when SETTINGS tool is active.
                        // Powered by Claude.
                        if (editor_state.tool == EditorTool::SETTINGS) {
                            if (game_options_panel.handle_click(mx, my, editor_state, scenario, undo_stack)) {
                                break;
                            }
                        }

                        // Check wizard panel when WIZARDS tool is active.
                        // Powered by Claude.
                        if (editor_state.tool == EditorTool::WIZARDS) {
                            if (wizard_panel.handle_click(mx, my, editor_state, scenario, undo_stack, renderer.window_width(), renderer.window_height())) {
                                break;
                            }
                        }

                        // Check if in map viewport
                        int vp_x = ToolPanel::WIDTH;
                        int vp_w = renderer.window_width() - ToolPanel::WIDTH - PropertiesPanel::WIDTH;
                        if (mx >= vp_x && mx < vp_x + vp_w) {
                            renderer.camera.screen_to_world(
                                mx, my, vp_x, 0,
                                editor_state.cursor_wx, editor_state.cursor_wy);

                            // Double-click on a fortress opens the wizard editor
                            // for that fortress's wizard. Powered by Claude.
                            if (event.button.clicks >= 2) {
                                int cwx = editor_state.cursor_wx;
                                int cwy = editor_state.cursor_wy;
                                int cplane = editor_state.current_plane;
                                for (int i = 0; i < NUM_FORTRESSES; ++i) {
                                    const auto& f = scenario.fortresses[i];
                                    if (f.active != 0 && f.wx == cwx &&
                                        f.wy == cwy && f.wp == cplane) {
                                        editor_state.wizard_tab = i;
                                        editor_state.tool = EditorTool::WIZARDS;
                                        break;
                                    }
                                }
                            } else {
                                apply_tool(editor_state, scenario, undo_stack);
                            }
                            prev_mouse_x = mx;
                            prev_mouse_y = my;
                        }
                    }
                    break;

                case SDL_MOUSEBUTTONUP:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        mouse_down = false;
                        prev_mouse_x = -1;
                        prev_mouse_y = -1;
                    }
                    break;

                case SDL_MOUSEMOTION: {
                    int mx = event.motion.x;
                    int my = event.motion.y;
                    int vp_x = ToolPanel::WIDTH;
                    int vp_w = renderer.window_width() - ToolPanel::WIDTH - PropertiesPanel::WIDTH;

                    if (mx >= vp_x && mx < vp_x + vp_w) {
                        renderer.camera.screen_to_world(
                            mx, my, vp_x, 0,
                            editor_state.cursor_wx, editor_state.cursor_wy);

                        // Drag-painting with pixel-space interpolation to fill gaps.
                        // Walk the pixel line from previous mouse position to current,
                        // converting each sample to tile coords and painting new tiles.
                        // Powered by Claude.
                        if (mouse_down && (editor_state.tool == EditorTool::PAINT_TERRAIN ||
                                           editor_state.tool == EditorTool::PAINT_SPECIAL ||
                                           editor_state.tool == EditorTool::PAINT_FLAGS ||
                                           editor_state.tool == EditorTool::ERASE)) {
                            if (prev_mouse_x >= 0 && prev_mouse_y >= 0 &&
                                (prev_mouse_x != mx || prev_mouse_y != my)) {
                                // Bresenham in pixel space from prev mouse to current mouse.
                                // Powered by Claude.
                                int px0 = prev_mouse_x, py0 = prev_mouse_y;
                                int px1 = mx, py1 = my;
                                int dx = std::abs(px1 - px0);
                                int dy = -std::abs(py1 - py0);
                                int sx = (px0 < px1) ? 1 : -1;
                                int sy = (py0 < py1) ? 1 : -1;
                                int err = dx + dy;
                                int last_painted_wx = -999, last_painted_wy = -999;
                                while (true) {
                                    int twx, twy;
                                    renderer.camera.screen_to_world(
                                        px0, py0, vp_x, 0, twx, twy);
                                    if (twx >= 0 && twy >= 0 &&
                                        (twx != last_painted_wx || twy != last_painted_wy)) {
                                        editor_state.cursor_wx = twx;
                                        editor_state.cursor_wy = twy;
                                        apply_tool(editor_state, scenario, undo_stack);
                                        last_painted_wx = twx;
                                        last_painted_wy = twy;
                                    }
                                    if (px0 == px1 && py0 == py1) break;
                                    int e2 = 2 * err;
                                    if (e2 >= dy) { err += dy; px0 += sx; }
                                    if (e2 <= dx) { err += dx; py0 += sy; }
                                }
                                // Restore cursor to final position.
                                // Powered by Claude.
                                renderer.camera.screen_to_world(
                                    mx, my, vp_x, 0,
                                    editor_state.cursor_wx, editor_state.cursor_wy);
                            } else {
                                apply_tool(editor_state, scenario, undo_stack);
                            }
                            prev_mouse_x = mx;
                            prev_mouse_y = my;
                        }
                    } else {
                        editor_state.cursor_wx = -1;
                        editor_state.cursor_wy = -1;
                    }

                    // Middle mouse button panning
                    if (event.motion.state & SDL_BUTTON_MMASK) {
                        renderer.camera.offset_x -= event.motion.xrel;
                        renderer.camera.offset_y -= event.motion.yrel;
                    }
                    break;
                }

                case SDL_MOUSEWHEEL: {
                    float old_zoom = renderer.camera.zoom;
                    // Zoom in 10% increments (additive) for predictable steps.
                    // Powered by Claude.
                    if (event.wheel.y > 0) {
                        renderer.camera.zoom += 0.1f;
                        if (renderer.camera.zoom > 5.0f) renderer.camera.zoom = 5.0f;
                    } else if (event.wheel.y < 0) {
                        renderer.camera.zoom -= 0.1f;
                    }
                    // Clamp minimum zoom so map width fills the viewport.
                    // Powered by Claude.
                    int zvp_w = renderer.window_width() - ToolPanel::WIDTH - PropertiesPanel::WIDTH;
                    float min_zoom = static_cast<float>(zvp_w) /
                                     (WORLD_WIDTH * renderer.camera.tile_size);
                    if (renderer.camera.zoom < min_zoom) renderer.camera.zoom = min_zoom;
                    // Adjust scroll to zoom toward mouse position.
                    // Powered by Claude.
                    int mx, my;
                    SDL_GetMouseState(&mx, &my);
                    float scale = renderer.camera.zoom / old_zoom;
                    renderer.camera.offset_x = mx + (renderer.camera.offset_x - mx) * scale;
                    renderer.camera.offset_y = my + (renderer.camera.offset_y - my) * scale;
                    break;
                }
            }
        }

        // Handle generate request from ToolPanel button.
        // Powered by Claude.
        if (editor_state.generate_requested) {
            editor_state.generate_requested = false;
            do_generate_map(editor_state, scenario, undo_stack);

            // Revalidate both planes after map generation.
            // Powered by Claude.
            editor_state.violations.clear();
            for (int p = 0; p < NUM_PLANES; ++p) {
                auto pv = SmoothingValidator::validate_plane(scenario.world, p);
                editor_state.violations.insert(
                    editor_state.violations.end(), pv.begin(), pv.end());
            }
        }

        // Handle generate wizards request from ToolPanel button.
        // Checks dirty flags; if any wizard is dirty, opens confirmation dialog.
        // Powered by Claude.
        if (editor_state.generate_wizards_requested) {
            editor_state.generate_wizards_requested = false;
            printf("[main] generate_wizards_requested received\n");

            // Check if any wizard has been manually edited.
            bool any_dirty = false;
            for (int i = 0; i < NUM_PLAYERS; ++i) {
                if (editor_state.wizard_dirty[i]) { any_dirty = true; break; }
            }

            if (any_dirty) {
                // Open confirmation dialog — generation deferred until user confirms.
                editor_state.wizard_gen_confirm_open = true;
                editor_state.wizard_gen_confirm_global = true;
            } else {
                do_generate_wizards(editor_state, scenario, undo_stack);
            }
        }

        // Handle wizard generation confirmation (from modal dialog).
        // Powered by Claude.
        if (editor_state.wizard_gen_confirmed) {
            editor_state.wizard_gen_confirmed = false;
            if (editor_state.wizard_gen_confirm_global) {
                do_generate_wizards(editor_state, scenario, undo_stack);
            } else {
                do_generate_wizard(editor_state, scenario, undo_stack,
                                   editor_state.wizard_gen_confirm_idx);
            }
        }

        // Handle .GAM export request from ToolPanel button.
        // Powered by Claude.
        if (editor_state.export_gam_requested) {
            editor_state.export_gam_requested = false;
            do_export_gam(editor_state, scenario);
        }

        // Poll for async browser file upload completion (WASM only).
        // When the user picks a file via the browser dialog, the data arrives
        // asynchronously. Process it here on the next frame.
        // Powered by Claude.
        if (platform::has_pending_file()) {
            auto buf = platform::take_pending_file();
            if (!buf.empty()) {
                // Try .mom_scenario JSON first, then .GAM binary.
                // Powered by Claude.
                if (deserialize_scenario(buf, scenario)) {
                    undo_stack.clear();
                    editor_state.deselect();
                    editor_state.violations.clear();
                    for (int p = 0; p < NUM_PLANES; ++p) {
                        auto pv = SmoothingValidator::validate_plane(
                            scenario.world, p);
                        editor_state.violations.insert(
                            editor_state.violations.end(),
                            pv.begin(), pv.end());
                    }
                    editor_state.set_status("Loaded scenario file");
                } else if (deserialize_gam(buf, scenario)) {
                    undo_stack.clear();
                    editor_state.deselect();
                    editor_state.violations.clear();
                    for (int p = 0; p < NUM_PLANES; ++p) {
                        auto pv = SmoothingValidator::validate_plane(
                            scenario.world, p);
                        editor_state.violations.insert(
                            editor_state.violations.end(),
                            pv.begin(), pv.end());
                    }
                    editor_state.set_status("Imported .GAM save file");
                } else {
                    editor_state.set_status("Failed to load file");
                }
            }
        }

        // Clamp camera
        int vp_w = renderer.window_width() - ToolPanel::WIDTH - PropertiesPanel::WIDTH;
        int vp_h = renderer.window_height() - 24; // status bar
        renderer.camera.clamp(vp_w, vp_h);

        // Render
        renderer.begin_frame();

        // Map viewport
        SDL_Rect viewport{ToolPanel::WIDTH, 0, vp_w, vp_h};
        map_renderer.render(renderer, scenario, editor_state, viewport);

        // UI panels
        tool_panel.render(renderer, editor_state, renderer.window_height());

        // Render game options panel when SETTINGS tool is active.
        // Powered by Claude.
        if (editor_state.tool == EditorTool::SETTINGS) {
            game_options_panel.render(renderer, editor_state, scenario, renderer.window_height());
        }

        // Render wizard panel when WIZARDS tool is active.
        // Powered by Claude.
        if (editor_state.tool == EditorTool::WIZARDS) {
            wizard_panel.render(renderer, editor_state, scenario, renderer.window_height());
        }

        props_panel.render(renderer, editor_state, scenario,
                           renderer.window_width(), renderer.window_height());

        // Status bar
        UIRenderer::draw_status_bar(renderer,
            renderer.window_width(), renderer.window_height(),
            editor_state.cursor_wx, editor_state.cursor_wy,
            editor_state.current_plane, renderer.camera.zoom);

        // Render status message overlay if active.
        // Powered by Claude.
        if (editor_state.status_timer > 0.0f) {
            UIRenderer::draw_status_message(renderer,
                renderer.window_width(), renderer.window_height(),
                editor_state.status_message);
        }

        renderer.end_frame();
}
