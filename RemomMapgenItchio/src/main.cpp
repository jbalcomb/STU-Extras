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
#include "ui/ToolPanel.hpp"
#include "ui/PropertiesPanel.hpp"
#include <SDL.h>
#include <cstdio>
#include <string>

using namespace mom;

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
        case EditorTool::ERASE: {
            // Erase any entity at cursor position
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
            break;
        }
        case EditorTool::SELECT: {
            state.deselect();
            // Select city at cursor
            for (int i = 0; i < NUM_CITIES; ++i) {
                auto& c = scenario.cities[i];
                if (c.is_active() && c.wx == wx && c.wy == wy && c.wp == plane) {
                    state.selected_city = i;
                    return;
                }
            }
            break;
        }
        default:
            break;
    }
}

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
#else
int main(int, char**) {
#endif
    Renderer renderer;
    if (!renderer.init("MoM Scenario Editor", 1280, 720)) {
        std::fprintf(stderr, "Failed to initialize SDL2\n");
        return 1;
    }

    Scenario scenario;
    scenario.clear();

    // Fill with grassland by default
    for (int p = 0; p < NUM_PLANES; ++p) {
        for (int i = 0; i < WORLD_SIZE; ++i) {
            scenario.world.terrain[p][i] = TERRAIN_GRASSLAND;
        }
    }

    EditorState editor_state;
    UndoStack undo_stack;
    ToolPanel tool_panel;
    PropertiesPanel props_panel;
    MapRenderer map_renderer;

    bool running = true;
    bool mouse_down = false;

    while (running) {
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
                        save_scenario("scenario.mom_scenario", scenario);
                    } else if (ctrl && key.sym == SDLK_o) {
                        load_scenario("scenario.mom_scenario", scenario);
                    } else if (ctrl && key.sym == SDLK_e) {
                        scenario.game_data.Total_Cities = static_cast<uint16_t>(scenario.count_active_cities());
                        scenario.game_data.Total_Units  = static_cast<uint16_t>(scenario.count_active_units());
                        save_gam_file("scenario.GAM", scenario);
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

                        // Paint while dragging
                        if (mouse_down && (editor_state.tool == EditorTool::PAINT_TERRAIN ||
                                           editor_state.tool == EditorTool::PAINT_SPECIAL ||
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
        props_panel.render(renderer, editor_state, scenario,
                           renderer.window_width(), renderer.window_height());

        // Status bar
        UIRenderer::draw_status_bar(renderer,
            renderer.window_width(), renderer.window_height(),
            editor_state.cursor_wx, editor_state.cursor_wy,
            editor_state.current_plane, renderer.camera.zoom);

        renderer.end_frame();
    }

    renderer.shutdown();
    return 0;
}
