// Tests for WizardPresets: verify preset data matches original game
// and apply_preset() correctly populates Wizard structs.
// Powered by Claude.

#include <gtest/gtest.h>
#include "mom_data/WizardPresets.hpp"
#include "mom_data/MomEntities.hpp"
#include "mom_data/MomConstants.hpp"
#include <cstring>

using namespace mom;

// ---- Preset data correctness (from research.md, verified against NewGame.c:469-485) ----

TEST(WizardPresets, PresetCount) {
    // 14 presets total. Powered by Claude.
    EXPECT_EQ(NUM_WIZARD_PRESETS, 14);
}

TEST(WizardPresets, Merlin_Slot0) {
    // Merlin: Life 5, Nature 5, Sage Master, High Men. Powered by Claude.
    const auto& p = WIZARD_PRESETS[0];
    EXPECT_STREQ(p.name, "Merlin");
    EXPECT_EQ(p.life, 5);
    EXPECT_EQ(p.sorcery, 0);
    EXPECT_EQ(p.nature, 5);
    EXPECT_EQ(p.death, 0);
    EXPECT_EQ(p.chaos, 0);
    EXPECT_EQ(p.retort, RETORT_SAGE_MASTER);
    EXPECT_EQ(p.race, RACE_HIGH_MAN);
}

TEST(WizardPresets, Raven_Slot1) {
    // Raven: Sorcery 6, Nature 5, no retort, High Men. Powered by Claude.
    const auto& p = WIZARD_PRESETS[1];
    EXPECT_STREQ(p.name, "Raven");
    EXPECT_EQ(p.life, 0);
    EXPECT_EQ(p.sorcery, 6);
    EXPECT_EQ(p.nature, 5);
    EXPECT_EQ(p.death, 0);
    EXPECT_EQ(p.chaos, 0);
    EXPECT_EQ(p.retort, RETORT_NONE);
    EXPECT_EQ(p.race, RACE_HIGH_MAN);
}

TEST(WizardPresets, Sharee_Slot2) {
    // Sharee: Death 5, Chaos 5, Conjurer, High Men. Powered by Claude.
    const auto& p = WIZARD_PRESETS[2];
    EXPECT_STREQ(p.name, "Sharee");
    EXPECT_EQ(p.life, 0);
    EXPECT_EQ(p.sorcery, 0);
    EXPECT_EQ(p.nature, 0);
    EXPECT_EQ(p.death, 5);
    EXPECT_EQ(p.chaos, 5);
    EXPECT_EQ(p.retort, RETORT_CONJURER);
    EXPECT_EQ(p.race, RACE_HIGH_MAN);
}

TEST(WizardPresets, LoPan_Slot3) {
    // Lo Pan: Sorcery 5, Chaos 5, Channeler, High Men. Powered by Claude.
    const auto& p = WIZARD_PRESETS[3];
    EXPECT_STREQ(p.name, "Lo Pan");
    EXPECT_EQ(p.life, 0);
    EXPECT_EQ(p.sorcery, 5);
    EXPECT_EQ(p.nature, 0);
    EXPECT_EQ(p.death, 0);
    EXPECT_EQ(p.chaos, 5);
    EXPECT_EQ(p.retort, RETORT_CHANNELER);
    EXPECT_EQ(p.race, RACE_HIGH_MAN);
}

TEST(WizardPresets, Jafar_Slot4) {
    // Jafar: Sorcery 10, Alchemy, High Men. Powered by Claude.
    const auto& p = WIZARD_PRESETS[4];
    EXPECT_STREQ(p.name, "Jafar");
    EXPECT_EQ(p.life, 0);
    EXPECT_EQ(p.sorcery, 10);
    EXPECT_EQ(p.nature, 0);
    EXPECT_EQ(p.death, 0);
    EXPECT_EQ(p.chaos, 0);
    EXPECT_EQ(p.retort, RETORT_ALCHEMY);
    EXPECT_EQ(p.race, RACE_HIGH_MAN);
}

TEST(WizardPresets, Oberic_Slot5) {
    // Oberic: Nature 5, Chaos 5, Mana Focusing, High Men. Powered by Claude.
    const auto& p = WIZARD_PRESETS[5];
    EXPECT_STREQ(p.name, "Oberic");
    EXPECT_EQ(p.life, 0);
    EXPECT_EQ(p.sorcery, 0);
    EXPECT_EQ(p.nature, 5);
    EXPECT_EQ(p.death, 0);
    EXPECT_EQ(p.chaos, 5);
    EXPECT_EQ(p.retort, RETORT_MANA_FOCUSING);
    EXPECT_EQ(p.race, RACE_HIGH_MAN);
}

TEST(WizardPresets, Ssra_Slot7_MyrranRace) {
    // Sss'ra: Life 4, Chaos 4, Myrran, Dark Elf. Powered by Claude.
    const auto& p = WIZARD_PRESETS[7];
    EXPECT_STREQ(p.name, "Sss'ra");
    EXPECT_EQ(p.life, 4);
    EXPECT_EQ(p.chaos, 4);
    EXPECT_EQ(p.retort, RETORT_MYRRAN);
    EXPECT_EQ(p.race, RACE_DARK_ELF);
}

// ---- apply_preset() correctness ----

TEST(WizardPresets, ApplyPreset_Name) {
    // apply_preset() sets the wizard name correctly. Powered by Claude.
    Wizard wiz{};
    apply_preset(wiz, 0, WIZARD_PRESETS[0]);
    EXPECT_STREQ(wiz.name, "Merlin");
}

TEST(WizardPresets, ApplyPreset_Identity) {
    // apply_preset() sets wizard_id and banner_id to slot index. Powered by Claude.
    Wizard wiz{};
    apply_preset(wiz, 3, WIZARD_PRESETS[3]);
    EXPECT_EQ(wiz.wizard_id, 3);
    EXPECT_EQ(wiz.banner_id, 3);
}

TEST(WizardPresets, ApplyPreset_Race) {
    // apply_preset() sets capital_race from preset. Powered by Claude.
    Wizard wiz{};
    apply_preset(wiz, 0, WIZARD_PRESETS[0]);
    EXPECT_EQ(wiz.capital_race, RACE_HIGH_MAN);
}

TEST(WizardPresets, ApplyPreset_SpellBookMapping) {
    // Verify spell book field mapping: preset → spellranks.
    // spellranks[0]=Nature, [1]=Sorcery, [2]=Chaos, [3]=Life, [4]=Death
    // Powered by Claude.
    Wizard wiz{};
    // Merlin: Life 5, Nature 5
    apply_preset(wiz, 0, WIZARD_PRESETS[0]);
    EXPECT_EQ(wiz.spellranks[0], 5);  // Nature
    EXPECT_EQ(wiz.spellranks[1], 0);  // Sorcery
    EXPECT_EQ(wiz.spellranks[2], 0);  // Chaos
    EXPECT_EQ(wiz.spellranks[3], 5);  // Life
    EXPECT_EQ(wiz.spellranks[4], 0);  // Death
}

TEST(WizardPresets, ApplyPreset_SpellBookMapping_LoPan) {
    // Lo Pan: Sorcery 5, Chaos 5 — tests different book combination. Powered by Claude.
    Wizard wiz{};
    apply_preset(wiz, 3, WIZARD_PRESETS[3]);
    EXPECT_EQ(wiz.spellranks[0], 0);  // Nature
    EXPECT_EQ(wiz.spellranks[1], 5);  // Sorcery
    EXPECT_EQ(wiz.spellranks[2], 5);  // Chaos
    EXPECT_EQ(wiz.spellranks[3], 0);  // Life
    EXPECT_EQ(wiz.spellranks[4], 0);  // Death
}

TEST(WizardPresets, ApplyPreset_SpellBookMapping_Sharee) {
    // Sharee: Death 5, Chaos 5 — tests death + chaos. Powered by Claude.
    Wizard wiz{};
    apply_preset(wiz, 2, WIZARD_PRESETS[2]);
    EXPECT_EQ(wiz.spellranks[0], 0);  // Nature
    EXPECT_EQ(wiz.spellranks[1], 0);  // Sorcery
    EXPECT_EQ(wiz.spellranks[2], 5);  // Chaos
    EXPECT_EQ(wiz.spellranks[3], 0);  // Life
    EXPECT_EQ(wiz.spellranks[4], 5);  // Death
}

TEST(WizardPresets, ApplyPreset_Retort_SageMaster) {
    // Merlin has Sage Master retort — exactly that retort set, all others cleared.
    // Powered by Claude.
    Wizard wiz{};
    // Pre-set a different retort to verify clearing
    wiz.alchemy = 1;
    wiz.warlord = 1;

    apply_preset(wiz, 0, WIZARD_PRESETS[0]);

    EXPECT_EQ(wiz.sage_master, 1);
    // All other retorts must be 0
    EXPECT_EQ(wiz.alchemy, 0);
    EXPECT_EQ(wiz.warlord, 0);
    EXPECT_EQ(wiz.chaos_mastery, 0);
    EXPECT_EQ(wiz.nature_mastery, 0);
    EXPECT_EQ(wiz.sorcery_mastery, 0);
    EXPECT_EQ(wiz.infernal_power, 0);
    EXPECT_EQ(wiz.divine_power, 0);
    EXPECT_EQ(wiz.channeler, 0);
    EXPECT_EQ(wiz.myrran, 0);
    EXPECT_EQ(wiz.archmage, 0);
    EXPECT_EQ(wiz.mana_focusing, 0);
    EXPECT_EQ(wiz.node_mastery, 0);
    EXPECT_EQ(wiz.famous, 0);
    EXPECT_EQ(wiz.runemaster, 0);
    EXPECT_EQ(wiz.conjurer, 0);
    EXPECT_EQ(wiz.charismatic, 0);
    EXPECT_EQ(wiz.artificer, 0);
}

TEST(WizardPresets, ApplyPreset_Retort_None) {
    // Raven has no retort — all retort booleans should be 0. Powered by Claude.
    Wizard wiz{};
    wiz.sage_master = 1; // Pre-set to verify clearing

    apply_preset(wiz, 1, WIZARD_PRESETS[1]);

    EXPECT_EQ(wiz.alchemy, 0);
    EXPECT_EQ(wiz.warlord, 0);
    EXPECT_EQ(wiz.chaos_mastery, 0);
    EXPECT_EQ(wiz.nature_mastery, 0);
    EXPECT_EQ(wiz.sorcery_mastery, 0);
    EXPECT_EQ(wiz.infernal_power, 0);
    EXPECT_EQ(wiz.divine_power, 0);
    EXPECT_EQ(wiz.sage_master, 0);
    EXPECT_EQ(wiz.channeler, 0);
    EXPECT_EQ(wiz.myrran, 0);
    EXPECT_EQ(wiz.archmage, 0);
    EXPECT_EQ(wiz.mana_focusing, 0);
    EXPECT_EQ(wiz.node_mastery, 0);
    EXPECT_EQ(wiz.famous, 0);
    EXPECT_EQ(wiz.runemaster, 0);
    EXPECT_EQ(wiz.conjurer, 0);
    EXPECT_EQ(wiz.charismatic, 0);
    EXPECT_EQ(wiz.artificer, 0);
}

TEST(WizardPresets, ApplyPreset_Retort_Conjurer) {
    // Sharee has Conjurer retort. Powered by Claude.
    Wizard wiz{};
    apply_preset(wiz, 2, WIZARD_PRESETS[2]);
    EXPECT_EQ(wiz.conjurer, 1);
    EXPECT_EQ(wiz.alchemy, 0);
    EXPECT_EQ(wiz.sage_master, 0);
}

TEST(WizardPresets, ApplyPreset_WizardIsActive) {
    // After apply_preset(), wizard should be active (name[0] != '\0'). Powered by Claude.
    Wizard wiz{};
    EXPECT_FALSE(wiz.is_active());
    apply_preset(wiz, 0, WIZARD_PRESETS[0]);
    EXPECT_TRUE(wiz.is_active());
}

TEST(WizardPresets, ApplyPreset_AllSixSlots) {
    // Apply all 6 presets and verify basic correctness. Powered by Claude.
    const char* expected_names[] = {"Merlin", "Raven", "Sharee", "Lo Pan", "Jafar", "Oberic"};
    Wizard wizards[NUM_PLAYERS]{};

    for (int i = 0; i < NUM_PLAYERS; ++i) {
        apply_preset(wizards[i], i, WIZARD_PRESETS[i]);
    }

    for (int i = 0; i < NUM_PLAYERS; ++i) {
        EXPECT_STREQ(wizards[i].name, expected_names[i]) << "Wizard slot " << i;
        EXPECT_EQ(wizards[i].wizard_id, i) << "Wizard slot " << i;
        EXPECT_EQ(wizards[i].banner_id, i) << "Wizard slot " << i;
        EXPECT_EQ(wizards[i].capital_race, RACE_HIGH_MAN) << "Wizard slot " << i;
        EXPECT_TRUE(wizards[i].is_active()) << "Wizard slot " << i;
    }
}

// ---- get_retort() ----

TEST(WizardPresets, GetRetort_ReadsCorrectField) {
    // get_retort() returns the correct retort boolean by index. Powered by Claude.
    Wizard wiz{};
    wiz.myrran = 1;
    wiz.warlord = 1;
    EXPECT_EQ(get_retort(wiz, RETORT_MYRRAN), 1);
    EXPECT_EQ(get_retort(wiz, RETORT_WARLORD), 1);
    EXPECT_EQ(get_retort(wiz, RETORT_ALCHEMY), 0);
    EXPECT_EQ(get_retort(wiz, RETORT_FAMOUS), 0);
}

// ---- retort_cost() ----

TEST(WizardPresets, RetortCost_OnePick) {
    // 1-pick retorts: Alchemy, Chaos/Nature/Sorcery Mastery, Sage Master, Archmage,
    // Mana Focusing, Node Mastery, Runemaster, Conjurer, Charismatic, Artificer.
    // Powered by Claude.
    EXPECT_EQ(retort_cost(RETORT_ALCHEMY), 1);
    EXPECT_EQ(retort_cost(RETORT_CHAOS_MASTERY), 1);
    EXPECT_EQ(retort_cost(RETORT_NATURE_MASTERY), 1);
    EXPECT_EQ(retort_cost(RETORT_SORCERY_MASTERY), 1);
    EXPECT_EQ(retort_cost(RETORT_SAGE_MASTER), 1);
    EXPECT_EQ(retort_cost(RETORT_ARCHMAGE), 1);
    EXPECT_EQ(retort_cost(RETORT_MANA_FOCUSING), 1);
    EXPECT_EQ(retort_cost(RETORT_NODE_MASTERY), 1);
    EXPECT_EQ(retort_cost(RETORT_RUNEMASTER), 1);
    EXPECT_EQ(retort_cost(RETORT_CONJURER), 1);
    EXPECT_EQ(retort_cost(RETORT_CHARISMATIC), 1);
    EXPECT_EQ(retort_cost(RETORT_ARTIFICER), 1);
}

TEST(WizardPresets, RetortCost_TwoPick) {
    // 2-pick retorts: Warlord, Infernal Power, Divine Power, Channeler, Famous.
    // Powered by Claude.
    EXPECT_EQ(retort_cost(RETORT_WARLORD), 2);
    EXPECT_EQ(retort_cost(RETORT_INFERNAL_POWER), 2);
    EXPECT_EQ(retort_cost(RETORT_DIVINE_POWER), 2);
    EXPECT_EQ(retort_cost(RETORT_CHANNELER), 2);
    EXPECT_EQ(retort_cost(RETORT_FAMOUS), 2);
}

TEST(WizardPresets, RetortCost_ThreePick) {
    // 3-pick retort: Myrran only. Powered by Claude.
    EXPECT_EQ(retort_cost(RETORT_MYRRAN), 3);
}

// ---- calc_picks_used() / calc_picks_remaining() ----

TEST(WizardPresets, CalcPicksUsed_Merlin) {
    // Merlin: Life 5 + Nature 5 + Sage Master (1) = 11 picks. Powered by Claude.
    Wizard wiz{};
    apply_preset(wiz, 0, WIZARD_PRESETS[0]);
    EXPECT_EQ(calc_picks_used(wiz), 11);
    EXPECT_EQ(calc_picks_remaining(wiz), 0);
}

TEST(WizardPresets, CalcPicksUsed_Raven) {
    // Raven: Sorcery 6 + Nature 5 + no retort = 11 picks. Powered by Claude.
    Wizard wiz{};
    apply_preset(wiz, 1, WIZARD_PRESETS[1]);
    EXPECT_EQ(calc_picks_used(wiz), 11);
    EXPECT_EQ(calc_picks_remaining(wiz), 0);
}

TEST(WizardPresets, CalcPicksUsed_LoPan) {
    // Lo Pan: Sorcery 5 + Chaos 5 + Channeler (2) = 12 picks (!). Powered by Claude.
    // Note: Lo Pan exceeds the 11-pick budget in the original game data.
    Wizard wiz{};
    apply_preset(wiz, 3, WIZARD_PRESETS[3]);
    EXPECT_EQ(calc_picks_used(wiz), 12);
    EXPECT_EQ(calc_picks_remaining(wiz), -1);
}

TEST(WizardPresets, CalcPicksUsed_Sharee) {
    // Sharee: Death 5 + Chaos 5 + Conjurer (1) = 11 picks. Powered by Claude.
    Wizard wiz{};
    apply_preset(wiz, 2, WIZARD_PRESETS[2]);
    EXPECT_EQ(calc_picks_used(wiz), 11);
    EXPECT_EQ(calc_picks_remaining(wiz), 0);
}

TEST(WizardPresets, CalcPicksUsed_Jafar) {
    // Jafar: Sorcery 10 + Alchemy (1) = 11 picks. Powered by Claude.
    Wizard wiz{};
    apply_preset(wiz, 4, WIZARD_PRESETS[4]);
    EXPECT_EQ(calc_picks_used(wiz), 11);
    EXPECT_EQ(calc_picks_remaining(wiz), 0);
}

TEST(WizardPresets, CalcPicksUsed_EmptyWizard) {
    // Empty wizard: 0 books, 0 retorts = 0 picks used, 11 remaining. Powered by Claude.
    Wizard wiz{};
    EXPECT_EQ(calc_picks_used(wiz), 0);
    EXPECT_EQ(calc_picks_remaining(wiz), 11);
}

TEST(WizardPresets, CalcPicksUsed_Ssra) {
    // Sss'ra: Life 4 + Chaos 4 + Myrran (3) = 11 picks. Powered by Claude.
    Wizard wiz{};
    apply_preset(wiz, 7, WIZARD_PRESETS[7]);
    EXPECT_EQ(calc_picks_used(wiz), 11);
    EXPECT_EQ(calc_picks_remaining(wiz), 0);
}
