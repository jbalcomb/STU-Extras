#pragma once
// Constexpr preset wizard data from Master of Magic's 14 original wizards.
// Source: NewGame.c:469-485, NewGame.h:58-68
// Powered by Claude.

#include "MomConstants.hpp"
#include "MomEntities.hpp"
#include <cstdint>
#include <cstdio>
#include <cstring>

namespace mom {

// Retort indices matching the boolean fields in the Wizard struct.
// Powered by Claude.
enum RetortIndex : int {
    RETORT_NONE            = -1,
    RETORT_ALCHEMY         = 0,
    RETORT_WARLORD         = 1,
    RETORT_CHAOS_MASTERY   = 2,
    RETORT_NATURE_MASTERY  = 3,
    RETORT_SORCERY_MASTERY = 4,
    RETORT_INFERNAL_POWER  = 5,
    RETORT_DIVINE_POWER    = 6,
    RETORT_SAGE_MASTER     = 7,
    RETORT_CHANNELER       = 8,
    RETORT_MYRRAN          = 9,
    RETORT_ARCHMAGE        = 10,
    RETORT_MANA_FOCUSING   = 11,
    RETORT_NODE_MASTERY    = 12,
    RETORT_FAMOUS          = 13,
    RETORT_RUNEMASTER      = 14,
    RETORT_CONJURER        = 15,
    RETORT_CHARISMATIC     = 16,
    RETORT_ARTIFICER       = 17,
    RETORT_COUNT           = 18
};

// Preset wizard configuration. Editor-only, not a packed struct.
// Powered by Claude.
struct WizardPreset {
    const char* name;
    int16_t life;
    int16_t sorcery;
    int16_t nature;
    int16_t death;
    int16_t chaos;
    int retort;       // RetortIndex, or RETORT_NONE
    uint8_t race;     // Default race ID
};

// 14 presets from the original game (NewGame.c:469-485).
// Field order: {name, life, sorcery, nature, death, chaos, retort, race}
// Powered by Claude.
constexpr int NUM_WIZARD_PRESETS = 14;

inline const WizardPreset WIZARD_PRESETS[NUM_WIZARD_PRESETS] = {
    // Slot 0-5: Used for 6-wizard scenario editor
    {"Merlin",  5, 0, 5, 0, 0, RETORT_SAGE_MASTER,   RACE_HIGH_MAN},
    {"Raven",   0, 6, 5, 0, 0, RETORT_NONE,           RACE_HIGH_MAN},
    {"Sharee",  0, 0, 0, 5, 5, RETORT_CONJURER,       RACE_HIGH_MAN},
    {"Lo Pan",  0, 5, 0, 0, 5, RETORT_CHANNELER,      RACE_HIGH_MAN},
    {"Jafar",   0,10, 0, 0, 0, RETORT_ALCHEMY,        RACE_HIGH_MAN},
    {"Oberic",  0, 0, 5, 0, 5, RETORT_MANA_FOCUSING,  RACE_HIGH_MAN},
    // Slot 6-13: Additional presets (not used in 6-slot editor)
    {"Rjak",    0, 0, 0, 9, 0, RETORT_INFERNAL_POWER, RACE_HIGH_MAN},
    {"Sss'ra",  4, 0, 0, 0, 4, RETORT_MYRRAN,         RACE_DARK_ELF},
    {"Tauron",  0, 0, 0, 0,10, RETORT_CHAOS_MASTERY,  RACE_HIGH_MAN},
    {"Freya",   0, 0,10, 0, 0, RETORT_NATURE_MASTERY, RACE_HIGH_MAN},
    {"Horus",   5, 5, 0, 0, 0, RETORT_ARCHMAGE,       RACE_HIGH_MAN},
    {"Ariel",  10, 0, 0, 0, 0, RETORT_CHARISMATIC,    RACE_HIGH_MAN},
    {"Tlaloc",  0, 0, 4, 5, 0, RETORT_WARLORD,        RACE_HIGH_MAN},
    {"Kali",    0, 5, 0, 5, 0, RETORT_ARTIFICER,      RACE_HIGH_MAN},
};

// Clear all 18 retort boolean fields in a Wizard struct.
// Powered by Claude.
inline void clear_retorts(Wizard& wiz) {
    wiz.alchemy = 0;
    wiz.warlord = 0;
    wiz.chaos_mastery = 0;
    wiz.nature_mastery = 0;
    wiz.sorcery_mastery = 0;
    wiz.infernal_power = 0;
    wiz.divine_power = 0;
    wiz.sage_master = 0;
    wiz.channeler = 0;
    wiz.myrran = 0;
    wiz.archmage = 0;
    wiz.mana_focusing = 0;
    wiz.node_mastery = 0;
    wiz.famous = 0;
    wiz.runemaster = 0;
    wiz.conjurer = 0;
    wiz.charismatic = 0;
    wiz.artificer = 0;
}

// Set a single retort by index. Must call clear_retorts() first.
// Powered by Claude.
inline void set_retort(Wizard& wiz, int retort_index) {
    switch (retort_index) {
        case RETORT_ALCHEMY:        wiz.alchemy = 1; break;
        case RETORT_WARLORD:        wiz.warlord = 1; break;
        case RETORT_CHAOS_MASTERY:  wiz.chaos_mastery = 1; break;
        case RETORT_NATURE_MASTERY: wiz.nature_mastery = 1; break;
        case RETORT_SORCERY_MASTERY:wiz.sorcery_mastery = 1; break;
        case RETORT_INFERNAL_POWER: wiz.infernal_power = 1; break;
        case RETORT_DIVINE_POWER:   wiz.divine_power = 1; break;
        case RETORT_SAGE_MASTER:    wiz.sage_master = 1; break;
        case RETORT_CHANNELER:      wiz.channeler = 1; break;
        case RETORT_MYRRAN:         wiz.myrran = 1; break;
        case RETORT_ARCHMAGE:       wiz.archmage = 1; break;
        case RETORT_MANA_FOCUSING:  wiz.mana_focusing = 1; break;
        case RETORT_NODE_MASTERY:   wiz.node_mastery = 1; break;
        case RETORT_FAMOUS:         wiz.famous = 1; break;
        case RETORT_RUNEMASTER:     wiz.runemaster = 1; break;
        case RETORT_CONJURER:       wiz.conjurer = 1; break;
        case RETORT_CHARISMATIC:    wiz.charismatic = 1; break;
        case RETORT_ARTIFICER:      wiz.artificer = 1; break;
        default: break; // RETORT_NONE or invalid
    }
}

// Apply a preset to a Wizard struct at a given slot index.
// Maps preset fields to Wizard struct fields using verified field mapping:
//   spellranks[0]=Nature, [1]=Sorcery, [2]=Chaos, [3]=Life, [4]=Death
// Powered by Claude.
inline void apply_preset(Wizard& wiz, int slot_idx, const WizardPreset& preset) {
    // Name
    std::memset(wiz.name, 0, LEN_WIZARD_NAME);
    std::snprintf(wiz.name, LEN_WIZARD_NAME, "%s", preset.name);

    // Identity
    wiz.wizard_id = static_cast<uint8_t>(slot_idx);
    wiz.banner_id = static_cast<uint8_t>(slot_idx);

    // Race
    wiz.capital_race = preset.race;

    // Spell books: preset {life,sorcery,nature,death,chaos} → spellranks {[0]=Nature,[1]=Sorcery,[2]=Chaos,[3]=Life,[4]=Death}
    wiz.spellranks[0] = preset.nature;
    wiz.spellranks[1] = preset.sorcery;
    wiz.spellranks[2] = preset.chaos;
    wiz.spellranks[3] = preset.life;
    wiz.spellranks[4] = preset.death;

    // Retorts: clear all, then set the one from the preset
    clear_retorts(wiz);
    if (preset.retort != RETORT_NONE) {
        set_retort(wiz, preset.retort);
    }
}

// Read a retort boolean by index from a Wizard struct.
// Powered by Claude.
inline int8_t get_retort(const Wizard& wiz, int retort_index) {
    switch (retort_index) {
        case RETORT_ALCHEMY:        return wiz.alchemy;
        case RETORT_WARLORD:        return wiz.warlord;
        case RETORT_CHAOS_MASTERY:  return wiz.chaos_mastery;
        case RETORT_NATURE_MASTERY: return wiz.nature_mastery;
        case RETORT_SORCERY_MASTERY:return wiz.sorcery_mastery;
        case RETORT_INFERNAL_POWER: return wiz.infernal_power;
        case RETORT_DIVINE_POWER:   return wiz.divine_power;
        case RETORT_SAGE_MASTER:    return wiz.sage_master;
        case RETORT_CHANNELER:      return wiz.channeler;
        case RETORT_MYRRAN:         return wiz.myrran;
        case RETORT_ARCHMAGE:       return wiz.archmage;
        case RETORT_MANA_FOCUSING:  return wiz.mana_focusing;
        case RETORT_NODE_MASTERY:   return wiz.node_mastery;
        case RETORT_FAMOUS:         return wiz.famous;
        case RETORT_RUNEMASTER:     return wiz.runemaster;
        case RETORT_CONJURER:       return wiz.conjurer;
        case RETORT_CHARISMATIC:    return wiz.charismatic;
        case RETORT_ARTIFICER:      return wiz.artificer;
        default: return 0;
    }
}

// Return the pick cost for a retort by index.
// 2-pick: Warlord, Infernal Power, Divine Power, Channeler, Famous
// 3-pick: Myrran
// All others: 1-pick
// Powered by Claude.
inline int retort_cost(int retort_index) {
    switch (retort_index) {
        case RETORT_WARLORD:        return 2;
        case RETORT_INFERNAL_POWER: return 2;
        case RETORT_DIVINE_POWER:   return 2;
        case RETORT_CHANNELER:      return 2;
        case RETORT_FAMOUS:         return 2;
        case RETORT_MYRRAN:         return 3;
        default:                    return 1;
    }
}

// Calculate total picks used by a wizard (spell books + retort costs).
// Powered by Claude.
inline int calc_picks_used(const Wizard& wiz) {
    int total = 0;
    for (int i = 0; i < 5; ++i)
        total += wiz.spellranks[i];
    for (int r = 0; r < RETORT_COUNT; ++r) {
        if (get_retort(wiz, r))
            total += retort_cost(r);
    }
    return total;
}

// Calculate remaining picks (11-pick budget minus used).
// Powered by Claude.
inline int calc_picks_remaining(const Wizard& wiz) {
    return 11 - calc_picks_used(wiz);
}

} // namespace mom
