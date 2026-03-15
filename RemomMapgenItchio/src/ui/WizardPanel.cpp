// Wizard setup panel rendering and interaction.
// Takes over the full map area when the WIZARDS tool is active.
// Powered by Claude.

#include "ui/WizardPanel.hpp"
#include "renderer/UIRenderer.hpp"
#include "scenario/Scenario.hpp"
#include "mom_data/WizardPresets.hpp"
#include "ui/ToolPanel.hpp"
#include "ui/PropertiesPanel.hpp"
#include <algorithm>
#include <cstring>
#include <cstdio>
#include <string>

namespace mom {

// Layout constants for the wizard panel.
// Powered by Claude.
static constexpr int PAD = 4;
static constexpr int BLOCK_H = 20;
static constexpr int LABEL_H = 14;  // height for label/value text line
static constexpr int DESC_H = 12;   // height for description text line
static constexpr int COL_GAP = 16;  // gap between left and right columns

// Race name lookup table for the 14 Master of Magic races.
// Powered by Claude.
static const char* RACE_NAMES[] = {
    "Barbarians", "Beastmen", "Dark Elves", "Draconians",
    "Dwarves", "Gnolls", "Halflings", "High Elves",
    "High Men", "Klackons", "Lizardmen", "Nomads",
    "Orcs", "Trolls"
};

// Spell realm names in struct order (spellranks[0..4]).
// Powered by Claude.
static const char* REALM_NAMES[] = {
    "Nature", "Sorcery", "Chaos", "Life", "Death"
};

// Retort name lookup table for the 18 retort toggles.
// Powered by Claude.
static const char* RETORT_NAMES[] = {
    "Alchemy", "Warlord", "Chaos Mastery", "Nature Mastery",
    "Sorcery Mastery", "Infernal Power", "Divine Power", "Sage Master",
    "Channeler", "Myrran", "Archmage", "Mana Focusing",
    "Node Mastery", "Famous", "Runemaster", "Conjurer",
    "Charismatic", "Artificer"
};

// Section description strings.
// Powered by Claude.
static const char* RACE_DESC = "Starting city population and available units";
static const char* SPELLS_DESC = "Spell books per realm (0-11)";
static const char* RETORTS_DESC = "Special wizard abilities and bonuses";

// Banner colors for wizard tabs (matches the game's 6 wizard banners).
// Powered by Claude.
static constexpr uint8_t WIZARD_COLORS[][3] = {
    { 80, 120, 200}, // Blue
    {200, 200,  80}, // Yellow
    { 80, 200,  80}, // Green
    {200, 200, 200}, // White
    {200,  80,  80}, // Red
    {160,  80, 200}  // Purple
};

// Race colors for the race selector display.
// Powered by Claude.
static constexpr uint8_t RACE_COLORS[][3] = {
    {180, 100,  60}, // Barbarian
    {140, 100,  60}, // Beastman
    {100,  60, 140}, // Dark Elf
    { 60, 140, 100}, // Draconian
    {160, 140, 100}, // Dwarf
    {120, 100,  60}, // Gnoll
    {100, 180, 100}, // Halfling
    {160, 200, 240}, // High Elf
    {200, 180, 140}, // High Man
    { 60, 120,  60}, // Klackon
    { 80, 140,  80}, // Lizardman
    {200, 180, 100}, // Nomad
    {100, 120,  60}, // Orc
    { 80, 140, 120}  // Troll
};

// Spell realm colors in struct order: Nature, Sorcery, Chaos, Life, Death.
// Powered by Claude.
static constexpr uint8_t REALM_COLORS[][3] = {
    { 60, 160,  60}, // Nature (green)
    { 60, 100, 200}, // Sorcery (blue)
    {200,  60,  40}, // Chaos (red)
    {240, 240, 200}, // Life (light gold)
    { 60,  40,  80}  // Death (dark purple)
};

// Retort colors for the 18 retort toggle squares.
// Powered by Claude.
static constexpr uint8_t RETORT_COLORS[][3] = {
    {200, 200, 100}, // Alchemy
    {200, 100,  60}, // Warlord
    {200,  60,  40}, // Chaos Mastery
    { 60, 160,  60}, // Nature Mastery
    { 60, 100, 200}, // Sorcery Mastery
    {120,  40,  60}, // Infernal Power
    {240, 240, 200}, // Divine Power
    {140, 180, 200}, // Sage Master
    {100, 200, 200}, // Channeler
    {100,  60, 140}, // Myrran
    {180, 140, 240}, // Archmage
    {120, 100, 200}, // Mana Focusing
    {160, 200, 160}, // Node Mastery
    {240, 200, 100}, // Famous
    {180, 120,  80}, // Runemaster
    {100, 160, 140}, // Conjurer
    {240, 180, 100}, // Charismatic
    {160, 140, 100}  // Artificer
};

void WizardPanel::render(Renderer& renderer, EditorState& state,
                          Scenario& scenario, int window_h) {
    // Compute map area bounds.
    // Powered by Claude.
    int panel_x = ToolPanel::WIDTH;
    int panel_w = renderer.window_width() - ToolPanel::WIDTH - PropertiesPanel::WIDTH;
    printf("[WizardPanel] render called: panel_x=%d panel_w=%d window_h=%d (build 2026-03-12a)\n",
           panel_x, panel_w, window_h);
    if (panel_w < 100) {
        printf("[WizardPanel] panel_w < 100, skipping render\n");
        return;
    }

    // Fill the entire map area with panel background.
    // Powered by Claude.
    UIRenderer::draw_panel(renderer, panel_x, 0, panel_w, window_h);

    // DEBUG: visible build stamp so we know this is the latest code.
    // Powered by Claude.
    renderer.draw_text(panel_x + 10, window_h - 20,
                       "BUILD 2026-03-12a map-area wizard panel", 255, 80, 80);

    int margin = 12;
    int inner_x = panel_x + margin;
    int content_w = panel_w - margin * 2;

    // Two-column layout: left for wizard config, right for retorts.
    // Powered by Claude.
    int left_w = content_w / 2 - COL_GAP / 2;
    int right_x = inner_x + left_w + COL_GAP;
    int right_w = content_w - left_w - COL_GAP;

    int y = margin;

    // Section label bar spanning full width.
    // Powered by Claude.
    UIRenderer::draw_label_bar(renderer, inner_x, y, content_w, 180, 100, 200);
    y += 16 + PAD;

    // Wizard tabs -- 6 colored boxes with name labels.
    // Powered by Claude.
    {
        int tab_w = (content_w - 10) / 6;
        int tab_h = 24;
        for (int i = 0; i < NUM_PLAYERS; ++i) {
            int tx = inner_x + i * (tab_w + 2);
            bool sel = (state.wizard_tab == i);
            renderer.draw_rect(tx, y, tab_w, tab_h,
                               WIZARD_COLORS[i][0], WIZARD_COLORS[i][1], WIZARD_COLORS[i][2]);
            if (sel) {
                renderer.draw_rect_outline(tx, y, tab_w, tab_h, 255, 255, 255);
            }
            if (!scenario.wizards[i].is_active() && !sel) {
                renderer.draw_rect(tx + 2, y + 2, tab_w - 4, tab_h - 4, 30, 30, 30);
            }

            // Draw wizard name inside the tab.
            // Powered by Claude.
            bool active = scenario.wizards[i].is_active();
            const char* label = active ? scenario.wizards[i].name : "Empty";
            if (sel) {
                renderer.draw_text(tx + 4, y + 5, label, 255, 255, 255);
            } else if (active) {
                renderer.draw_text(tx + 4, y + 5, label, 180, 180, 180);
            } else {
                renderer.draw_text(tx + 4, y + 5, label, 100, 100, 100);
            }
        }
        y += tab_h + PAD;
    }

    UIRenderer::draw_separator(renderer, inner_x, y, content_w);
    y += PAD * 2;

    // Clamp wizard tab index.
    // Powered by Claude.
    int wiz_idx = std::clamp(state.wizard_tab, 0, NUM_PLAYERS - 1);
    auto& wiz = scenario.wizards[wiz_idx];

    // Fortress status indicator -- shows whether this wizard has a fortress placed.
    // Fortress index == wizard index (1:1 positional mapping).
    // Powered by Claude.
    {
        const auto& fort = scenario.fortresses[wiz_idx];
        if (fort.active != 0) {
            std::string loc = "Fortress: (" + std::to_string(fort.wx) + ", " +
                              std::to_string(fort.wy) + ") " +
                              (fort.wp == 0 ? "Arcanus" : "Myrror");
            renderer.draw_text(inner_x, y, loc, 120, 200, 120);
        } else {
            renderer.draw_text(inner_x, y,
                               "No fortress placed (use Fortress tool on map)",
                               200, 120, 80);
        }
        y += LABEL_H + PAD;
    }

    // Per-wizard "Gen" button — generates this wizard from preset data.
    // Powered by Claude.
    {
        int gen_btn_w = 80;
        int gen_btn_h = 20;
        renderer.draw_rect(inner_x, y, gen_btn_w, gen_btn_h, 100, 50, 160);
        renderer.draw_rect_outline(inner_x, y, gen_btn_w, gen_btn_h, 130, 130, 140);
        renderer.draw_text(inner_x + 8, y + 3, "Gen Wizard", 220, 220, 220);
        y += gen_btn_h + PAD;
    }

    // Save y for both columns to start at the same height.
    // Powered by Claude.
    int col_start_y = y;

    // ========== LEFT COLUMN: Race, Portrait, Spell Ranks ==========

    // (1) Race selector -- "Race:" label with inline dropdown for the race name.
    // Powered by Claude.
    {
        int race_val = static_cast<int>(wiz.capital_race) % RACE_COUNT;
        const char* race_name = (race_val < 14) ? RACE_NAMES[race_val] : "Unknown";

        // Draw "Race:" as static label text.
        // Powered by Claude.
        renderer.draw_text(inner_x, y + 3, "Race:", 200, 200, 200);
        int label_offset = 48; // pixels after "Race:" for the dropdown

        // Draw the race name as a dropdown button.
        // Powered by Claude.
        int dd_x = inner_x + label_offset;
        int dd_w = left_w - label_offset;
        renderer.draw_rect(dd_x, y, dd_w, BLOCK_H,
                           RACE_COLORS[race_val][0] / 3, RACE_COLORS[race_val][1] / 3,
                           RACE_COLORS[race_val][2] / 3);
        renderer.draw_rect_outline(dd_x, y, dd_w, BLOCK_H, 120, 120, 120);
        renderer.draw_text(dd_x + 4, y + 3, race_name,
                           RACE_COLORS[race_val][0], RACE_COLORS[race_val][1],
                           RACE_COLORS[race_val][2]);
        renderer.draw_text(dd_x + dd_w - 12, y + 3,
                           race_dropdown_open_ ? "-" : "+", 200, 200, 200);

        race_dropdown_y_ = y + BLOCK_H + 1;
        y += BLOCK_H + PAD;

        renderer.draw_text(inner_x, y, RACE_DESC, 160, 160, 160);
        y += DESC_H + PAD * 2;
    }

    // (2) Portrait selector -- label, colored square showing wizard_id, click cycles.
    // Powered by Claude.
    {
        int portrait = wiz.wizard_id % 15;
        std::string label_text = std::string("Portrait: ") + std::to_string(portrait);
        renderer.draw_text(inner_x, y, label_text);
        y += LABEL_H + PAD;

        int shade = 80 + portrait * 10;
        if (shade > 240) shade = 240;
        renderer.draw_rect(inner_x, y, BLOCK_H, BLOCK_H,
                           static_cast<uint8_t>(shade),
                           static_cast<uint8_t>(shade / 2),
                           static_cast<uint8_t>(shade / 3));
        renderer.draw_rect_outline(inner_x, y, BLOCK_H, BLOCK_H, 120, 120, 120);
        y += BLOCK_H + PAD * 2;
    }

    // (3) Spell rank selectors -- 5 colored bars in struct order.
    // Powered by Claude.
    {
        renderer.draw_text(inner_x, y, SPELLS_DESC, 160, 160, 160);
        y += DESC_H + PAD;

        int bar_h = 14;
        for (int r = 0; r < 5; ++r) {
            int rank = wiz.spellranks[r];
            if (rank < 0) rank = 0;
            if (rank > 11) rank = 11;

            std::string realm_label = std::string(REALM_NAMES[r]) + ": " + std::to_string(rank);
            renderer.draw_text(inner_x, y, realm_label);
            y += LABEL_H;

            int fill_w = static_cast<int>(left_w * rank / 11.0f);
            renderer.draw_rect(inner_x, y, left_w, bar_h, 30, 30, 40);
            if (fill_w > 0) {
                renderer.draw_rect(inner_x, y, fill_w, bar_h,
                                   REALM_COLORS[r][0], REALM_COLORS[r][1],
                                   REALM_COLORS[r][2]);
            }
            renderer.draw_rect_outline(inner_x, y, left_w, bar_h, 80, 80, 80);
            y += bar_h + 2;
        }

        // Picks budget counter — shows remaining spell picks out of 11.
        // Powered by Claude.
        {
            int remaining = calc_picks_remaining(wiz);
            char picks_buf[32];
            std::snprintf(picks_buf, sizeof(picks_buf), "Picks: %d/11", 11 - remaining);
            uint8_t pr, pg, pb;
            if (remaining > 0)      { pr = 80;  pg = 200; pb = 80;  }  // green
            else if (remaining == 0) { pr = 200; pg = 200; pb = 80;  }  // yellow
            else                     { pr = 240; pg = 80;  pb = 80;  }  // red (over budget)
            std::snprintf(picks_buf, sizeof(picks_buf), "Picks: %d/11  (%d remaining)",
                          11 - remaining, remaining);
            renderer.draw_text(inner_x, y, picks_buf, pr, pg, pb);
            y += LABEL_H + PAD;
        }
    }

    // ========== RIGHT COLUMN: Retorts ==========

    // (4) Retort toggles -- 3-column grid matching original game layout.
    // Column order from NewGame.c: Col1 (general), Col2 (misc), Col3 (realm mastery).
    // Powered by Claude.
    {
        int ry = col_start_y;

        renderer.draw_text(right_x, ry, "Retorts");
        ry += LABEL_H;
        renderer.draw_text(right_x, ry, RETORTS_DESC, 160, 160, 160);
        ry += DESC_H + PAD;

        int row_h = LABEL_H + 4;

        // Original game 3-column layout (indices into RETORT_NAMES/retorts arrays).
        // Powered by Claude.
        static constexpr int RETORT_GRID[6][3] = {
            { 0,  7,  2}, // Alchemy,    Sage Master,  Chaos Mastery
            { 1,  9,  3}, // Warlord,    Myrran,       Nature Mastery
            { 8,  6,  4}, // Channeler,  Divine Power,  Sorcery Mastery
            {10, 13,  5}, // Archmage,   Famous,       Infernal Power
            {17, 14, 11}, // Artificer,  Runemaster,   Mana Focusing
            {15, 16, 12}  // Conjurer,   Charismatic,  Node Mastery
        };

        int8_t* retorts[] = {
            &wiz.alchemy, &wiz.warlord, &wiz.chaos_mastery,
            &wiz.nature_mastery, &wiz.sorcery_mastery, &wiz.infernal_power,
            &wiz.divine_power, &wiz.sage_master, &wiz.channeler,
            &wiz.myrran, &wiz.archmage, &wiz.mana_focusing,
            &wiz.node_mastery, &wiz.famous, &wiz.runemaster,
            &wiz.conjurer, &wiz.charismatic, &wiz.artificer
        };

        int col_w = right_w / 3;

        for (int row = 0; row < 6; ++row) {
            for (int col = 0; col < 3; ++col) {
                int ri = RETORT_GRID[row][col];
                bool on = (*retorts[ri] != 0);
                int cx = right_x + col * col_w;

                // Draw toggle indicator and retort name.
                // Powered by Claude.
                if (on) {
                    renderer.draw_rect(cx, ry + 1, col_w - 2, row_h - 2,
                                       RETORT_COLORS[ri][0] / 4, RETORT_COLORS[ri][1] / 4,
                                       RETORT_COLORS[ri][2] / 4);
                    renderer.draw_text(cx + 2, ry + 1, RETORT_NAMES[ri],
                                       RETORT_COLORS[ri][0], RETORT_COLORS[ri][1],
                                       RETORT_COLORS[ri][2]);
                } else {
                    renderer.draw_text(cx + 2, ry + 1, RETORT_NAMES[ri], 80, 80, 80);
                }
            }
            ry += row_h;
        }
    }

    // Race dropdown overlay -- drawn last so it appears on top of other elements.
    // Powered by Claude.
    if (race_dropdown_open_) {
        int race_val = static_cast<int>(wiz.capital_race) % RACE_COUNT;
        int item_h = LABEL_H + 2;
        int list_h = RACE_COUNT * item_h;
        int dy = race_dropdown_y_;

        renderer.draw_rect(inner_x, dy, left_w, list_h, 20, 20, 30);
        renderer.draw_rect_outline(inner_x, dy, left_w, list_h, 140, 140, 140);

        for (int i = 0; i < RACE_COUNT; ++i) {
            int iy = dy + i * item_h;
            if (i == race_val) {
                renderer.draw_rect(inner_x + 1, iy, left_w - 2, item_h,
                                   RACE_COLORS[i][0] / 3, RACE_COLORS[i][1] / 3,
                                   RACE_COLORS[i][2] / 3);
            }
            renderer.draw_text(inner_x + 4, iy + 1, RACE_NAMES[i],
                               RACE_COLORS[i][0], RACE_COLORS[i][1],
                               RACE_COLORS[i][2]);
        }
    }

    // Modal confirmation dialog for wizard generation overwrite.
    // Drawn last so it appears on top of all other elements.
    // Powered by Claude.
    if (state.wizard_gen_confirm_open) {
        int dlg_w = 300;
        int dlg_h = 120;
        int dlg_x = panel_x + (panel_w - dlg_w) / 2;
        int dlg_y = window_h / 3;

        // Semi-transparent backdrop.
        renderer.draw_rect(panel_x, 0, panel_w, window_h, 0, 0, 0);

        // Dialog box.
        renderer.draw_rect(dlg_x, dlg_y, dlg_w, dlg_h, 30, 20, 40);
        renderer.draw_rect_outline(dlg_x, dlg_y, dlg_w, dlg_h, 180, 140, 220);

        // Title.
        renderer.draw_text(dlg_x + 10, dlg_y + 10,
                           "Overwrite wizard data?", 240, 200, 255);

        // Description.
        if (state.wizard_gen_confirm_global) {
            renderer.draw_text(dlg_x + 10, dlg_y + 35,
                               "Modified wizards will be overwritten", 180, 180, 180);
            renderer.draw_text(dlg_x + 10, dlg_y + 52,
                               "with preset data.", 180, 180, 180);
        } else {
            char buf[64];
            std::snprintf(buf, sizeof(buf), "Wizard %d has been modified.",
                          state.wizard_gen_confirm_idx);
            renderer.draw_text(dlg_x + 10, dlg_y + 35, buf, 180, 180, 180);
            renderer.draw_text(dlg_x + 10, dlg_y + 52,
                               "Overwrite with preset data?", 180, 180, 180);
        }

        // Buttons: Overwrite and Cancel.
        int btn_w = 100;
        int btn_h = 24;
        int btn_y = dlg_y + dlg_h - btn_h - 10;

        // Overwrite button.
        int ow_x = dlg_x + dlg_w / 2 - btn_w - 10;
        renderer.draw_rect(ow_x, btn_y, btn_w, btn_h, 140, 60, 60);
        renderer.draw_rect_outline(ow_x, btn_y, btn_w, btn_h, 200, 100, 100);
        renderer.draw_text(ow_x + 14, btn_y + 5, "Overwrite", 240, 240, 240);

        // Cancel button.
        int cn_x = dlg_x + dlg_w / 2 + 10;
        renderer.draw_rect(cn_x, btn_y, btn_w, btn_h, 60, 60, 80);
        renderer.draw_rect_outline(cn_x, btn_y, btn_w, btn_h, 120, 120, 140);
        renderer.draw_text(cn_x + 22, btn_y + 5, "Cancel", 240, 240, 240);
    }
}

bool WizardPanel::handle_click(int mx, int my, EditorState& state,
                                Scenario& scenario, UndoStack& undo, int window_w, int window_h) {
    // Compute map area bounds -- must match render().
    // Powered by Claude.
    int panel_x = ToolPanel::WIDTH;
    int panel_w = window_w - ToolPanel::WIDTH - PropertiesPanel::WIDTH;
    printf("[WizardPanel] handle_click mx=%d my=%d panel_x=%d panel_w=%d (build 2026-03-12a)\n",
           mx, my, panel_x, panel_w);

    if (mx < panel_x || mx >= panel_x + panel_w) return false;
    if (my < 0) return false;

    int margin = 12;
    int inner_x = panel_x + margin;
    int content_w = panel_w - margin * 2;
    int left_w = content_w / 2 - COL_GAP / 2;
    int right_x = inner_x + left_w + COL_GAP;
    int right_w = content_w - left_w - COL_GAP;

    // Modal confirmation dialog — handle clicks first (drawn on top).
    // Powered by Claude.
    if (state.wizard_gen_confirm_open) {
        int dlg_w = 300;
        int dlg_h = 120;
        int dlg_x = panel_x + (panel_w - dlg_w) / 2;
        int dlg_y = window_h / 3;
        int btn_w = 100;
        int btn_h = 24;
        int btn_y = dlg_y + dlg_h - btn_h - 10;

        int ow_x = dlg_x + dlg_w / 2 - btn_w - 10;
        int cn_x = dlg_x + dlg_w / 2 + 10;

        if (my >= btn_y && my < btn_y + btn_h) {
            if (mx >= ow_x && mx < ow_x + btn_w) {
                // Overwrite confirmed.
                state.wizard_gen_confirm_open = false;
                state.wizard_gen_confirmed = true;
                return true;
            }
            if (mx >= cn_x && mx < cn_x + btn_w) {
                // Cancel.
                state.wizard_gen_confirm_open = false;
                return true;
            }
        }
        // Click anywhere else inside dialog area closes it.
        state.wizard_gen_confirm_open = false;
        return true;
    }

    int y = margin;

    // Skip section label.
    // Powered by Claude.
    y += 16 + PAD;

    // Wizard tabs -- click to select.
    // Powered by Claude.
    {
        int tab_w = (content_w - 10) / 6;
        int tab_h = 24;
        if (my >= y && my < y + tab_h) {
            for (int i = 0; i < NUM_PLAYERS; ++i) {
                int tx = inner_x + i * (tab_w + 2);
                if (mx >= tx && mx < tx + tab_w) {
                    state.wizard_tab = i;
                    if (!scenario.wizards[i].is_active()) {
                        int idx = i;
                        auto cmd = std::make_unique<LambdaCommand>(
                            [&scenario, idx]() {
                                std::snprintf(scenario.wizards[idx].name,
                                              LEN_WIZARD_NAME, "Wizard %d", idx);
                                scenario.wizards[idx].wizard_id =
                                    static_cast<uint8_t>(idx);
                                scenario.wizards[idx].banner_id =
                                    static_cast<uint8_t>(idx);
                            },
                            [&scenario, idx]() {
                                scenario.wizards[idx] = Wizard{};
                            }
                        );
                        undo.execute(std::move(cmd));
                        state.wizard_dirty[idx] = true;
                    }
                    return true;
                }
            }
        }
        y += tab_h + PAD;
    }

    // Skip separator.
    // Powered by Claude.
    y += PAD * 2;

    int wiz_idx = std::clamp(state.wizard_tab, 0, NUM_PLAYERS - 1);

    // Skip fortress status line (matches render layout).
    // Powered by Claude.
    y += LABEL_H + PAD;

    // Per-wizard "Gen" button — click to generate this wizard from preset.
    // Powered by Claude.
    {
        int gen_btn_w = 80;
        int gen_btn_h = 20;
        if (my >= y && my < y + gen_btn_h && mx >= inner_x && mx < inner_x + gen_btn_w) {
            if (state.wizard_dirty[wiz_idx]) {
                // Dirty — open confirmation dialog for single wizard.
                state.wizard_gen_confirm_open = true;
                state.wizard_gen_confirm_global = false;
                state.wizard_gen_confirm_idx = wiz_idx;
            } else {
                // Not dirty — generate immediately via main loop.
                state.wizard_gen_confirm_global = false;
                state.wizard_gen_confirm_idx = wiz_idx;
                state.wizard_gen_confirmed = true;
            }
            return true;
        }
        y += gen_btn_h + PAD;
    }

    int col_start_y = y;

    // Race dropdown overlay -- handle clicks first since it draws on top.
    // Powered by Claude.
    if (race_dropdown_open_) {
        int item_h = LABEL_H + 2;
        int list_h = RACE_COUNT * item_h;
        int dy = race_dropdown_y_;
        if (my >= dy && my < dy + list_h && mx >= inner_x && mx < inner_x + left_w) {
            int selected = (my - dy) / item_h;
            if (selected >= 0 && selected < RACE_COUNT) {
                int idx = wiz_idx;
                uint8_t old_val = scenario.wizards[idx].capital_race;
                uint8_t new_val = static_cast<uint8_t>(selected);
                if (new_val != old_val) {
                    auto cmd = std::make_unique<LambdaCommand>(
                        [&scenario, idx, new_val]() {
                            scenario.wizards[idx].capital_race = new_val;
                        },
                        [&scenario, idx, old_val]() {
                            scenario.wizards[idx].capital_race = old_val;
                        }
                    );
                    undo.execute(std::move(cmd));
                    state.wizard_dirty[idx] = true;
                }
                race_dropdown_open_ = false;
                return true;
            }
        }
        race_dropdown_open_ = false;
        return true;
    }

    // ========== LEFT COLUMN clicks ==========
    if (mx >= inner_x && mx < inner_x + left_w) {

        // (1) Race selector -- click the race name dropdown to open list.
        // Powered by Claude.
        {
            int label_offset = 48;
            int dd_x = inner_x + label_offset;
            int dd_w = left_w - label_offset;
            if (my >= y && my < y + BLOCK_H && mx >= dd_x && mx < dd_x + dd_w) {
                race_dropdown_open_ = true;
                return true;
            }
            y += BLOCK_H + PAD;
            y += DESC_H + PAD * 2;  // skip description
        }

        // (2) Portrait selector -- click cycles portrait index.
        // Powered by Claude.
        {
            y += LABEL_H + PAD;   // skip "Portrait: {index}" label
            if (my >= y && my < y + BLOCK_H && mx >= inner_x && mx < inner_x + BLOCK_H) {
                int idx = wiz_idx;
                uint8_t old_val = scenario.wizards[idx].wizard_id;
                uint8_t new_val = (old_val + 1) % 15;
                auto cmd = std::make_unique<LambdaCommand>(
                    [&scenario, idx, new_val]() {
                        scenario.wizards[idx].wizard_id = new_val;
                    },
                    [&scenario, idx, old_val]() {
                        scenario.wizards[idx].wizard_id = old_val;
                    }
                );
                undo.execute(std::move(cmd));
                state.wizard_dirty[idx] = true;
                return true;
            }
            y += BLOCK_H + PAD * 2;
        }

        // (3) Spell rank selectors -- 5 bars in struct order, click cycles 0-11.
        // Enforces 11-pick budget: increment blocked when no picks remain.
        // Powered by Claude.
        {
            y += DESC_H + PAD;    // skip description
            int bar_h = 14;
            for (int r = 0; r < 5; ++r) {
                y += LABEL_H;     // skip realm name label
                if (my >= y && my < y + bar_h && mx >= inner_x && mx < inner_x + left_w) {
                    int idx = wiz_idx;
                    int realm_idx = r;
                    int16_t old_val = scenario.wizards[idx].spellranks[realm_idx];
                    int used = calc_picks_used(scenario.wizards[idx]);
                    int remaining = calc_picks_remaining(scenario.wizards[idx]);
                    printf("[WizardPanel] spell click realm=%d old_val=%d used=%d remaining=%d\n",
                           realm_idx, old_val, used, remaining);
                    int16_t new_val;
                    if (old_val >= 11) {
                        new_val = 0;  // wrap back to 0
                    } else if (remaining <= 0) {
                        new_val = 0;  // no picks remaining, wrap to 0
                        state.set_status("No picks remaining");
                    } else {
                        new_val = old_val + 1;
                    }
                    printf("[WizardPanel] spell click new_val=%d\n", new_val);
                    if (new_val != old_val) {
                        auto cmd = std::make_unique<LambdaCommand>(
                            [&scenario, idx, realm_idx, new_val]() {
                                scenario.wizards[idx].spellranks[realm_idx] = new_val;
                            },
                            [&scenario, idx, realm_idx, old_val]() {
                                scenario.wizards[idx].spellranks[realm_idx] = old_val;
                            }
                        );
                        undo.execute(std::move(cmd));
                        state.wizard_dirty[idx] = true;
                    }
                    return true;
                }
                y += bar_h + 2;
            }
        }
    }

    // ========== RIGHT COLUMN clicks: Retorts (3-column grid) ==========
    // Powered by Claude.
    if (mx >= right_x && mx < right_x + right_w) {
        int ry = col_start_y;
        ry += LABEL_H;         // skip "Retorts" header
        ry += DESC_H + PAD;    // skip description

        int row_h = LABEL_H + 4;
        int col_w = right_w / 3;

        // Same grid as render(). Powered by Claude.
        static constexpr int RETORT_GRID[6][3] = {
            { 0,  7,  2}, // Alchemy,    Sage Master,  Chaos Mastery
            { 1,  9,  3}, // Warlord,    Myrran,       Nature Mastery
            { 8,  6,  4}, // Channeler,  Divine Power,  Sorcery Mastery
            {10, 13,  5}, // Archmage,   Famous,       Infernal Power
            {17, 14, 11}, // Artificer,  Runemaster,   Mana Focusing
            {15, 16, 12}  // Conjurer,   Charismatic,  Node Mastery
        };

        for (int row = 0; row < 6; ++row) {
            if (my >= ry && my < ry + row_h) {
                int col = (mx - right_x) / col_w;
                if (col < 0) col = 0;
                if (col > 2) col = 2;
                int retort_idx = RETORT_GRID[row][col];
                int idx = wiz_idx;
                auto& wiz = scenario.wizards[idx];
                int8_t* retorts[] = {
                    &wiz.alchemy, &wiz.warlord, &wiz.chaos_mastery,
                    &wiz.nature_mastery, &wiz.sorcery_mastery, &wiz.infernal_power,
                    &wiz.divine_power, &wiz.sage_master, &wiz.channeler,
                    &wiz.myrran, &wiz.archmage, &wiz.mana_focusing,
                    &wiz.node_mastery, &wiz.famous, &wiz.runemaster,
                    &wiz.conjurer, &wiz.charismatic, &wiz.artificer
                };
                int8_t old_val = *retorts[retort_idx];
                int8_t new_val = (old_val != 0) ? 0 : 1;

                // Enforce picks budget and Runemaster prerequisite on enable.
                // Disabling a retort is always allowed.
                // Powered by Claude.
                if (new_val == 1) {
                    int cost = retort_cost(retort_idx);
                    if (calc_picks_remaining(wiz) < cost) {
                        state.set_status("Not enough picks for this retort");
                        return true;
                    }
                    // Runemaster requires 2+ books in 3+ realms.
                    if (retort_idx == RETORT_RUNEMASTER) {
                        int realms_with_2 = 0;
                        for (int i = 0; i < 5; ++i)
                            if (wiz.spellranks[i] >= 2) ++realms_with_2;
                        if (realms_with_2 < 3) {
                            state.set_status("Runemaster requires 2+ books in 3+ realms");
                            return true;
                        }
                    }
                }

                auto cmd = std::make_unique<LambdaCommand>(
                    [&scenario, idx, retort_idx, new_val]() {
                        int8_t* r[] = {
                            &scenario.wizards[idx].alchemy,
                            &scenario.wizards[idx].warlord,
                            &scenario.wizards[idx].chaos_mastery,
                            &scenario.wizards[idx].nature_mastery,
                            &scenario.wizards[idx].sorcery_mastery,
                            &scenario.wizards[idx].infernal_power,
                            &scenario.wizards[idx].divine_power,
                            &scenario.wizards[idx].sage_master,
                            &scenario.wizards[idx].channeler,
                            &scenario.wizards[idx].myrran,
                            &scenario.wizards[idx].archmage,
                            &scenario.wizards[idx].mana_focusing,
                            &scenario.wizards[idx].node_mastery,
                            &scenario.wizards[idx].famous,
                            &scenario.wizards[idx].runemaster,
                            &scenario.wizards[idx].conjurer,
                            &scenario.wizards[idx].charismatic,
                            &scenario.wizards[idx].artificer
                        };
                        *r[retort_idx] = new_val;
                    },
                    [&scenario, idx, retort_idx, old_val]() {
                        int8_t* r[] = {
                            &scenario.wizards[idx].alchemy,
                            &scenario.wizards[idx].warlord,
                            &scenario.wizards[idx].chaos_mastery,
                            &scenario.wizards[idx].nature_mastery,
                            &scenario.wizards[idx].sorcery_mastery,
                            &scenario.wizards[idx].infernal_power,
                            &scenario.wizards[idx].divine_power,
                            &scenario.wizards[idx].sage_master,
                            &scenario.wizards[idx].channeler,
                            &scenario.wizards[idx].myrran,
                            &scenario.wizards[idx].archmage,
                            &scenario.wizards[idx].mana_focusing,
                            &scenario.wizards[idx].node_mastery,
                            &scenario.wizards[idx].famous,
                            &scenario.wizards[idx].runemaster,
                            &scenario.wizards[idx].conjurer,
                            &scenario.wizards[idx].charismatic,
                            &scenario.wizards[idx].artificer
                        };
                        *r[retort_idx] = old_val;
                    }
                );
                undo.execute(std::move(cmd));
                state.wizard_dirty[idx] = true;
                return true;
            }
            ry += row_h;
        }
    }

    // Click was inside the panel area but didn't hit any element -- consume it
    // so it doesn't pass through to the map.
    // Powered by Claude.
    return true;
}

} // namespace mom
