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
            for (int i = 0; i < NUM_NODES; ++i) {
                if (scenario.nodes[i].power == 0 && scenario.nodes[i].wx == 0 && scenario.nodes[i].wy == 0) {
                    int idx = i;
                    auto cmd = std::make_unique<LambdaCommand>(
                        [&scenario, &state, idx, wx, wy, plane]() {
                            auto& n = scenario.nodes[idx];
                            n.wx = static_cast<int8_t>(wx);
                            n.wy = static_cast<int8_t>(wy);
                            n.wp = static_cast<int8_t>(plane);
                            n.type = state.place_node_type;
                            n.power = 5;
                        },
                        [&scenario, idx]() {
                            scenario.nodes[idx] = Node{};
                        }
                    );
                    undo.execute(std::move(cmd));
                    break;
                }
            }
            break;
        }
        case EditorTool::PLACE_TOWER: {
            // Find first inactive tower slot (wx==0 && wy==0 means empty).
            // Powered by Claude.
            for (int i = 0; i < NUM_TOWERS; ++i) {
                auto& t = scenario.towers[i];
                if (t.wx == 0 && t.wy == 0) {
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
            // Powered by Claude.
            for (int i = 0; i < NUM_FORTRESSES; ++i) {
                auto& f = scenario.fortresses[i];
                if (f.active == 0) {
                    int idx = i;
                    auto cmd = std::make_unique<LambdaCommand>(
                        [&scenario, idx, wx, wy, plane]() {
                            auto& ft = scenario.fortresses[idx];
                            ft.wx = static_cast<int8_t>(wx);
                            ft.wy = static_cast<int8_t>(wy);
                            ft.wp = static_cast<int8_t>(plane);
                            ft.active = 1;
                        },
                        [&scenario, idx]() {
                            scenario.fortresses[idx] = Fortress{};
                        }
                    );
                    undo.execute(std::move(cmd));
                    return;
                }
            }
            state.set_status("All 6 fortress slots are full");
            break;
        }
        case EditorTool::PLACE_LAIR: {
            // Find first inactive lair slot (wx==0 && wy==0 means empty).
            // Powered by Claude.
            for (int i = 0; i < NUM_LAIRS; ++i) {
                auto& l = scenario.lairs[i];
                if (l.wx == 0 && l.wy == 0) {
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
            // Erase towers at cursor.
            // Powered by Claude.
            for (int i = 0; i < NUM_TOWERS; ++i) {
                auto& t = scenario.towers[i];
                if (!(t.wx == 0 && t.wy == 0) && t.wx == wx && t.wy == wy) {
                    Tower backup = t;
                    int idx = i;
                    auto cmd = std::make_unique<LambdaCommand>(
                        [&scenario, idx]() { scenario.towers[idx] = Tower{}; },
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
                if (!(l.wx == 0 && l.wy == 0) && l.wx == wx && l.wy == wy && l.wp == plane) {
                    Lair backup = l;
                    int idx = i;
                    auto cmd = std::make_unique<LambdaCommand>(
                        [&scenario, idx]() { scenario.lairs[idx] = Lair{}; },
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
                if (!(n.wx == 0 && n.wy == 0) && n.wx == wx && n.wy == wy && n.wp == plane) {
                    state.selected_node = i;
                    return;
                }
            }
            // Select tower at cursor.
            // Powered by Claude.
            for (int i = 0; i < NUM_TOWERS; ++i) {
                auto& t = scenario.towers[i];
                if (!(t.wx == 0 && t.wy == 0) && t.wx == wx && t.wy == wy) {
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
                if (!(l.wx == 0 && l.wy == 0) && l.wx == wx && l.wy == wy && l.wp == plane) {
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
    Uint32 last_tick{0};
};

static void main_loop_iteration(void* arg);

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
#else
int main(int, char**) {
#endif
    static AppState app;

    if (!app.renderer.init("MoM Scenario Editor", 1280, 720)) {
        std::fprintf(stderr, "Failed to initialize SDL2\n");
        return 1;
    }

    // Log renderer info for WASM builds to aid diagnostics.
    // Powered by Claude.
#ifdef __EMSCRIPTEN__
    EM_ASM({
        console.log("MoM Scenario Editor loaded - WASM build");
    });
#endif

    app.scenario.clear();

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
                        // Update entity counts before export.
                        // Powered by Claude.
                        scenario.game_data.Total_Cities = static_cast<uint16_t>(scenario.count_active_cities());
                        scenario.game_data.Total_Units  = static_cast<uint16_t>(scenario.count_active_units());
                        // Count active wizards for export.
                        // Powered by Claude.
                        int wiz_count = 0;
                        for (auto& w : scenario.wizards) if (w.is_active()) ++wiz_count;
                        scenario.game_data.Total_Wizards = static_cast<uint16_t>(wiz_count > 1 ? wiz_count : 1);

                        // Export .GAM via platform file dialog.
                        // Powered by Claude.
                        auto gam_buf = serialize_gam(scenario);
                        platform::save_file("scenario.GAM",
                                            "GAM Save Files",
                                            "*.GAM",
                                            gam_buf);
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
                        // Powered by Claude.
                        if (props_panel.handle_minimap_click(
                                mx, my, renderer.camera, editor_state, scenario,
                                renderer.window_width(), renderer.window_height())) {
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
                            if (wizard_panel.handle_click(mx, my, editor_state, scenario, undo_stack)) {
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
                            apply_tool(editor_state, scenario, undo_stack);
                        }
                    }
                    break;

                case SDL_MOUSEBUTTONUP:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        mouse_down = false;
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

                        // Drag-painting for terrain, special, flags, and erase tools.
                        // Powered by Claude.
                        if (mouse_down && (editor_state.tool == EditorTool::PAINT_TERRAIN ||
                                           editor_state.tool == EditorTool::PAINT_SPECIAL ||
                                           editor_state.tool == EditorTool::PAINT_FLAGS ||
                                           editor_state.tool == EditorTool::ERASE)) {
                            apply_tool(editor_state, scenario, undo_stack);
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
                    if (event.wheel.y > 0) {
                        renderer.camera.zoom *= 1.15f;
                        if (renderer.camera.zoom > 5.0f) renderer.camera.zoom = 5.0f;
                    } else if (event.wheel.y < 0) {
                        renderer.camera.zoom /= 1.15f;
                        if (renderer.camera.zoom < 0.3f) renderer.camera.zoom = 0.3f;
                    }
                    // Adjust scroll to zoom toward mouse position
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
