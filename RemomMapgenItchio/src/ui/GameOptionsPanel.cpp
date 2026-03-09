// Game options panel rendering and interaction.
// Powered by Claude.

#include "ui/GameOptionsPanel.hpp"
#include "renderer/UIRenderer.hpp"
#include "scenario/Scenario.hpp"

namespace mom {

// Layout constants for the game options panel.
// Powered by Claude.
static constexpr int PANEL_X = 0;
static constexpr int PANEL_TOP = 280; // below the tool buttons
static constexpr int PAD = 4;
static constexpr int BLOCK_H = 20;
static constexpr int INNER_X = 8;

void GameOptionsPanel::render(Renderer& renderer, EditorState& /*state*/,
                               Scenario& scenario, int window_h) {
    // Panel background overlay below tool buttons.
    // Powered by Claude.
    int panel_h = window_h - PANEL_TOP;
    UIRenderer::draw_panel(renderer, PANEL_X, PANEL_TOP, WIDTH, panel_h);

    int y = PANEL_TOP + 8;
    int btn_w = WIDTH - 16;

    // Section label bar for game options.
    // Powered by Claude.
    UIRenderer::draw_label_bar(renderer, INNER_X, y, btn_w, 200, 180, 60);
    y += 16 + PAD;

    UIRenderer::draw_separator(renderer, INNER_X, y, btn_w);
    y += PAD * 2;

    // (1) Difficulty selector -- 5 colored blocks (green to red).
    // Powered by Claude.
    {
        int block_w = (btn_w - 8) / 5;
        uint8_t diff_colors[][3] = {
            { 80, 200,  80}, // Intro - green
            {140, 200,  80}, // Easy - yellow-green
            {200, 200,  80}, // Normal - yellow
            {200, 140,  80}, // Hard - orange
            {200,  80,  80}  // Impossible - red
        };
        for (int i = 0; i < 5; ++i) {
            int bx = INNER_X + i * (block_w + 2);
            bool sel = (scenario.game_data.Difficulty == static_cast<uint16_t>(i));
            renderer.draw_rect(bx, y, block_w, BLOCK_H,
                               diff_colors[i][0], diff_colors[i][1], diff_colors[i][2]);
            if (sel) {
                renderer.draw_rect_outline(bx, y, block_w, BLOCK_H, 255, 255, 255);
            }
        }
        y += BLOCK_H + PAD * 2;
    }

    // (2) Magic level -- single colored bar, click cycles 0-3.
    // Powered by Claude.
    {
        uint16_t magic = scenario.game_data.Magic;
        int fill_w = static_cast<int>(btn_w * (magic + 1) / 4.0f);
        renderer.draw_rect(INNER_X, y, btn_w, BLOCK_H, 40, 40, 60);
        renderer.draw_rect(INNER_X, y, fill_w, BLOCK_H, 100, 80, 200);
        renderer.draw_rect_outline(INNER_X, y, btn_w, BLOCK_H, 120, 120, 120);
        y += BLOCK_H + PAD * 2;
    }

    // (3) Land size -- 3 colored blocks, click cycles 0-2.
    // Powered by Claude.
    {
        int block_w = (btn_w - 4) / 3;
        uint8_t land_colors[][3] = {
            { 80, 140, 200}, // Small - blue
            {120, 180, 120}, // Medium - green
            {180, 140,  80}  // Large - brown
        };
        for (int i = 0; i < 3; ++i) {
            int bx = INNER_X + i * (block_w + 2);
            bool sel = (scenario.game_data.Land_Size == static_cast<uint16_t>(i));
            renderer.draw_rect(bx, y, block_w, BLOCK_H,
                               land_colors[i][0], land_colors[i][1], land_colors[i][2]);
            if (sel) {
                renderer.draw_rect_outline(bx, y, block_w, BLOCK_H, 255, 255, 255);
            }
        }
        y += BLOCK_H + PAD * 2;
    }

    // (4) Wizard count -- dots 1-6, click cycles.
    // Powered by Claude.
    {
        int wiz_count = scenario.game_data.Total_Wizards;
        int dot_r = 8;
        for (int i = 0; i < 6; ++i) {
            int dx = INNER_X + i * (dot_r * 2 + 6) + dot_r;
            int dy = y + BLOCK_H / 2;
            bool active = (i < wiz_count);
            if (active) {
                renderer.draw_rect(dx - dot_r, dy - dot_r, dot_r * 2, dot_r * 2,
                                   180, 100, 200);
            } else {
                renderer.draw_rect(dx - dot_r, dy - dot_r, dot_r * 2, dot_r * 2,
                                   60, 60, 60);
            }
        }
        y += BLOCK_H + PAD;
    }
}

bool GameOptionsPanel::handle_click(int mx, int my, EditorState& /*state*/,
                                     Scenario& scenario, UndoStack& undo) {
    if (mx < 0 || mx >= WIDTH) return false;
    if (my < PANEL_TOP) return false;

    int y = PANEL_TOP + 8;
    int btn_w = WIDTH - 16;

    // Skip section label.
    // Powered by Claude.
    y += 16 + PAD;
    y += PAD * 2; // separator

    // (1) Difficulty selector -- click cycles through 5 values.
    // Powered by Claude.
    {
        if (my >= y && my < y + BLOCK_H) {
            uint16_t old_val = scenario.game_data.Difficulty;
            uint16_t new_val = (old_val + 1) % 5;
            auto cmd = std::make_unique<LambdaCommand>(
                [&scenario, new_val]() { scenario.game_data.Difficulty = new_val; },
                [&scenario, old_val]() { scenario.game_data.Difficulty = old_val; }
            );
            undo.execute(std::move(cmd));
            return true;
        }
        y += BLOCK_H + PAD * 2;
    }

    // (2) Magic level -- click cycles 0-3.
    // Powered by Claude.
    {
        if (my >= y && my < y + BLOCK_H && mx >= INNER_X && mx < INNER_X + btn_w) {
            uint16_t old_val = scenario.game_data.Magic;
            uint16_t new_val = (old_val + 1) % 4;
            auto cmd = std::make_unique<LambdaCommand>(
                [&scenario, new_val]() { scenario.game_data.Magic = new_val; },
                [&scenario, old_val]() { scenario.game_data.Magic = old_val; }
            );
            undo.execute(std::move(cmd));
            return true;
        }
        y += BLOCK_H + PAD * 2;
    }

    // (3) Land size -- click cycles 0-2.
    // Powered by Claude.
    {
        if (my >= y && my < y + BLOCK_H) {
            uint16_t old_val = scenario.game_data.Land_Size;
            uint16_t new_val = (old_val + 1) % 3;
            auto cmd = std::make_unique<LambdaCommand>(
                [&scenario, new_val]() { scenario.game_data.Land_Size = new_val; },
                [&scenario, old_val]() { scenario.game_data.Land_Size = old_val; }
            );
            undo.execute(std::move(cmd));
            return true;
        }
        y += BLOCK_H + PAD * 2;
    }

    // (4) Wizard count -- click cycles 1-6.
    // Powered by Claude.
    {
        if (my >= y && my < y + BLOCK_H) {
            uint16_t old_val = scenario.game_data.Total_Wizards;
            uint16_t new_val = (old_val % 6) + 1;
            auto cmd = std::make_unique<LambdaCommand>(
                [&scenario, new_val]() { scenario.game_data.Total_Wizards = new_val; },
                [&scenario, old_val]() { scenario.game_data.Total_Wizards = old_val; }
            );
            undo.execute(std::move(cmd));
            return true;
        }
        y += BLOCK_H + PAD;
    }

    return false;
}

} // namespace mom
