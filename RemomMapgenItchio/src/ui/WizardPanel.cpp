// Wizard setup panel rendering and interaction.
// Powered by Claude.

#include "ui/WizardPanel.hpp"
#include "renderer/UIRenderer.hpp"
#include "scenario/Scenario.hpp"
#include <algorithm>
#include <cstring>
#include <cstdio>

namespace mom {

// Layout constants for the wizard panel.
// Powered by Claude.
static constexpr int PANEL_X = 0;
static constexpr int PANEL_TOP = 280; // below the tool buttons
static constexpr int PAD = 4;
static constexpr int BLOCK_H = 20;
static constexpr int INNER_X = 8;

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

// Spell realm colors: Life, Death, Chaos, Nature, Sorcery.
// Powered by Claude.
static constexpr uint8_t REALM_COLORS[][3] = {
    {240, 240, 200}, // Life (light gold)
    { 60,  40,  80}, // Death (dark purple)
    {200,  60,  40}, // Chaos (red)
    { 60, 160,  60}, // Nature (green)
    { 60, 100, 200}  // Sorcery (blue)
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
    // Panel background overlay below tool buttons.
    // Powered by Claude.
    int panel_h = window_h - PANEL_TOP;
    UIRenderer::draw_panel(renderer, PANEL_X, PANEL_TOP, WIDTH, panel_h);

    int y = PANEL_TOP + 8;
    int btn_w = WIDTH - 16;

    // Section label bar for wizard panel.
    // Powered by Claude.
    UIRenderer::draw_label_bar(renderer, INNER_X, y, btn_w, 180, 100, 200);
    y += 16 + PAD;

    // Wizard tabs -- 6 colored boxes at the top.
    // Powered by Claude.
    {
        int tab_w = (btn_w - 10) / 6;
        int tab_h = 18;
        for (int i = 0; i < NUM_PLAYERS; ++i) {
            int tx = INNER_X + i * (tab_w + 2);
            bool sel = (state.wizard_tab == i);
            renderer.draw_rect(tx, y, tab_w, tab_h,
                               WIZARD_COLORS[i][0], WIZARD_COLORS[i][1], WIZARD_COLORS[i][2]);
            if (sel) {
                renderer.draw_rect_outline(tx, y, tab_w, tab_h, 255, 255, 255);
            }
            // Dim inactive wizards (those without a name set).
            // Powered by Claude.
            if (!scenario.wizards[i].is_active() && !sel) {
                renderer.draw_rect(tx + 2, y + 2, tab_w - 4, tab_h - 4, 30, 30, 30);
            }
        }
        y += tab_h + PAD * 2;
    }

    UIRenderer::draw_separator(renderer, INNER_X, y, btn_w);
    y += PAD * 2;

    // Clamp wizard tab index.
    // Powered by Claude.
    int wiz_idx = std::clamp(state.wizard_tab, 0, NUM_PLAYERS - 1);
    auto& wiz = scenario.wizards[wiz_idx];

    // (1) Race selector -- colored bar, click cycles 0-13.
    // Powered by Claude.
    {
        int race_val = static_cast<int>(wiz.capital_race) % RACE_COUNT;
        renderer.draw_rect(INNER_X, y, btn_w, BLOCK_H,
                           RACE_COLORS[race_val][0], RACE_COLORS[race_val][1],
                           RACE_COLORS[race_val][2]);
        renderer.draw_rect_outline(INNER_X, y, btn_w, BLOCK_H, 120, 120, 120);
        y += BLOCK_H + PAD;
    }

    // (2) Portrait selector -- colored square showing wizard_id, click cycles.
    // Powered by Claude.
    {
        int portrait = wiz.wizard_id % 15; // 14 wizard portraits + custom
        int shade = 80 + portrait * 10;
        if (shade > 240) shade = 240;
        renderer.draw_rect(INNER_X, y, BLOCK_H, BLOCK_H,
                           static_cast<uint8_t>(shade),
                           static_cast<uint8_t>(shade / 2),
                           static_cast<uint8_t>(shade / 3));
        renderer.draw_rect_outline(INNER_X, y, BLOCK_H, BLOCK_H, 120, 120, 120);
        y += BLOCK_H + PAD * 2;
    }

    // (3) Spell rank selectors -- 5 colored bars for the 5 realms.
    // spellranks order: Nature(0), Sorcery(1), Chaos(2), Life(3), Death(4)
    // Display order:    Life, Death, Chaos, Nature, Sorcery
    // Powered by Claude.
    {
        // Map display order to spellranks array index.
        // Powered by Claude.
        int realm_to_idx[] = {3, 4, 2, 0, 1}; // Life, Death, Chaos, Nature, Sorcery
        int bar_h = 14;
        for (int r = 0; r < 5; ++r) {
            int idx = realm_to_idx[r];
            int rank = wiz.spellranks[idx];
            if (rank < 0) rank = 0;
            if (rank > 4) rank = 4;
            int fill_w = static_cast<int>(btn_w * rank / 4.0f);
            renderer.draw_rect(INNER_X, y, btn_w, bar_h, 30, 30, 40);
            if (fill_w > 0) {
                renderer.draw_rect(INNER_X, y, fill_w, bar_h,
                                   REALM_COLORS[r][0], REALM_COLORS[r][1],
                                   REALM_COLORS[r][2]);
            }
            renderer.draw_rect_outline(INNER_X, y, btn_w, bar_h, 80, 80, 80);
            y += bar_h + 2;
        }
        y += PAD;
    }

    // (4) Retort toggles -- 18 small colored squares.
    // Powered by Claude.
    {
        int sq_sz = 16;
        int cols = (btn_w) / (sq_sz + 2);
        if (cols < 1) cols = 1;
        int col = 0;
        int sx = INNER_X;

        // Get retort values as array for iteration.
        // Powered by Claude.
        int8_t* retorts[] = {
            &wiz.alchemy, &wiz.warlord, &wiz.chaos_mastery,
            &wiz.nature_mastery, &wiz.sorcery_mastery, &wiz.infernal_power,
            &wiz.divine_power, &wiz.sage_master, &wiz.channeler,
            &wiz.myrran, &wiz.archmage, &wiz.mana_focusing,
            &wiz.node_mastery, &wiz.famous, &wiz.runemaster,
            &wiz.conjurer, &wiz.charismatic, &wiz.artificer
        };

        for (int i = 0; i < 18; ++i) {
            bool on = (*retorts[i] != 0);
            if (on) {
                renderer.draw_rect(sx, y, sq_sz, sq_sz,
                                   RETORT_COLORS[i][0], RETORT_COLORS[i][1],
                                   RETORT_COLORS[i][2]);
            } else {
                renderer.draw_rect(sx, y, sq_sz, sq_sz, 50, 50, 55);
            }
            renderer.draw_rect_outline(sx, y, sq_sz, sq_sz, 80, 80, 80);
            sx += sq_sz + 2;
            if (++col >= cols) {
                col = 0;
                sx = INNER_X;
                y += sq_sz + 2;
            }
        }
    }
}

bool WizardPanel::handle_click(int mx, int my, EditorState& state,
                                Scenario& scenario, UndoStack& undo) {
    if (mx < 0 || mx >= WIDTH) return false;
    if (my < PANEL_TOP) return false;

    int y = PANEL_TOP + 8;
    int btn_w = WIDTH - 16;

    // Skip section label.
    // Powered by Claude.
    y += 16 + PAD;

    // Wizard tabs -- click to select.
    // Powered by Claude.
    {
        int tab_w = (btn_w - 10) / 6;
        int tab_h = 18;
        if (my >= y && my < y + tab_h) {
            for (int i = 0; i < NUM_PLAYERS; ++i) {
                int tx = INNER_X + i * (tab_w + 2);
                if (mx >= tx && mx < tx + tab_w) {
                    state.wizard_tab = i;
                    // Ensure wizard is initialized with a name if empty.
                    // Powered by Claude.
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
                    }
                    return true;
                }
            }
        }
        y += tab_h + PAD * 2;
    }

    // Skip separator.
    // Powered by Claude.
    y += PAD * 2;

    int wiz_idx = std::clamp(state.wizard_tab, 0, NUM_PLAYERS - 1);

    // (1) Race selector -- click cycles 0-13.
    // Powered by Claude.
    {
        if (my >= y && my < y + BLOCK_H) {
            int idx = wiz_idx;
            uint8_t old_val = scenario.wizards[idx].capital_race;
            uint8_t new_val = (old_val + 1) % RACE_COUNT;
            auto cmd = std::make_unique<LambdaCommand>(
                [&scenario, idx, new_val]() {
                    scenario.wizards[idx].capital_race = new_val;
                },
                [&scenario, idx, old_val]() {
                    scenario.wizards[idx].capital_race = old_val;
                }
            );
            undo.execute(std::move(cmd));
            return true;
        }
        y += BLOCK_H + PAD;
    }

    // (2) Portrait selector -- click cycles 0-14.
    // Powered by Claude.
    {
        if (my >= y && my < y + BLOCK_H && mx >= INNER_X && mx < INNER_X + BLOCK_H) {
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
            return true;
        }
        y += BLOCK_H + PAD * 2;
    }

    // (3) Spell rank selectors -- 5 bars, click cycles 0-4.
    // Powered by Claude.
    {
        int realm_to_idx[] = {3, 4, 2, 0, 1}; // Life, Death, Chaos, Nature, Sorcery
        int bar_h = 14;
        for (int r = 0; r < 5; ++r) {
            if (my >= y && my < y + bar_h && mx >= INNER_X && mx < INNER_X + btn_w) {
                int idx = wiz_idx;
                int realm_idx = realm_to_idx[r];
                int16_t old_val = scenario.wizards[idx].spellranks[realm_idx];
                int16_t new_val = (old_val + 1) % 5;
                auto cmd = std::make_unique<LambdaCommand>(
                    [&scenario, idx, realm_idx, new_val]() {
                        scenario.wizards[idx].spellranks[realm_idx] = new_val;
                    },
                    [&scenario, idx, realm_idx, old_val]() {
                        scenario.wizards[idx].spellranks[realm_idx] = old_val;
                    }
                );
                undo.execute(std::move(cmd));
                return true;
            }
            y += bar_h + 2;
        }
        y += PAD;
    }

    // (4) Retort toggles -- 18 small squares, click toggles.
    // Powered by Claude.
    {
        int sq_sz = 16;
        int cols = (btn_w) / (sq_sz + 2);
        if (cols < 1) cols = 1;

        auto& wiz = scenario.wizards[wiz_idx];
        int8_t* retorts[] = {
            &wiz.alchemy, &wiz.warlord, &wiz.chaos_mastery,
            &wiz.nature_mastery, &wiz.sorcery_mastery, &wiz.infernal_power,
            &wiz.divine_power, &wiz.sage_master, &wiz.channeler,
            &wiz.myrran, &wiz.archmage, &wiz.mana_focusing,
            &wiz.node_mastery, &wiz.famous, &wiz.runemaster,
            &wiz.conjurer, &wiz.charismatic, &wiz.artificer
        };

        for (int i = 0; i < 18; ++i) {
            int col = i % cols;
            int row = i / cols;
            int sx = INNER_X + col * (sq_sz + 2);
            int sy = y + row * (sq_sz + 2);
            if (mx >= sx && mx < sx + sq_sz && my >= sy && my < sy + sq_sz) {
                int idx = wiz_idx;
                int retort_idx = i;
                int8_t old_val = *retorts[i];
                int8_t new_val = (old_val != 0) ? 0 : 1;
                // Use offset from wizard start to identify the retort field.
                // Powered by Claude.
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
                return true;
            }
        }
    }

    return false;
}

} // namespace mom
