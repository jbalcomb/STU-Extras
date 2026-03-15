# Data Model: Wizards Panel Labels & Documentation

**Feature**: 005-wizards-panel | **Date**: 2026-03-11

## Existing Entities (No Changes)

This feature adds no new data entities. All wizard configuration is stored in the existing `Wizard` struct (`MomEntities.hpp`) which remains unchanged.

### Wizard Struct Fields Used

| Field | Type | Values | Display Name |
|-------|------|--------|-------------|
| `name` | `char[20]` | ASCII string | Wizard name or "Empty" if `name[0] == '\0'` |
| `wizard_id` | `uint8_t` | 0-14 | Portrait index |
| `capital_race` | `uint8_t` | 0-13 | Race name from lookup table |
| `banner_id` | `uint8_t` | 0-5 | Banner color (used for tab colors) |
| `spellranks[0]` | `int16_t` | 0-4 | "Nature" rank |
| `spellranks[1]` | `int16_t` | 0-4 | "Sorcery" rank |
| `spellranks[2]` | `int16_t` | 0-4 | "Chaos" rank |
| `spellranks[3]` | `int16_t` | 0-4 | "Life" rank |
| `spellranks[4]` | `int16_t` | 0-4 | "Death" rank |
| `alchemy` | `int8_t` | 0 or 1 | "Alchemy" toggle |
| `warlord` | `int8_t` | 0 or 1 | "Warlord" toggle |
| `chaos_mastery` | `int8_t` | 0 or 1 | "Chaos Mastery" toggle |
| `nature_mastery` | `int8_t` | 0 or 1 | "Nature Mastery" toggle |
| `sorcery_mastery` | `int8_t` | 0 or 1 | "Sorcery Mastery" toggle |
| `infernal_power` | `int8_t` | 0 or 1 | "Infernal Power" toggle |
| `divine_power` | `int8_t` | 0 or 1 | "Divine Power" toggle |
| `sage_master` | `int8_t` | 0 or 1 | "Sage Master" toggle |
| `channeler` | `int8_t` | 0 or 1 | "Channeler" toggle |
| `myrran` | `int8_t` | 0 or 1 | "Myrran" toggle |
| `archmage` | `int8_t` | 0 or 1 | "Archmage" toggle |
| `mana_focusing` | `int8_t` | 0 or 1 | "Mana Focusing" toggle |
| `node_mastery` | `int8_t` | 0 or 1 | "Node Mastery" toggle |
| `famous` | `int8_t` | 0 or 1 | "Famous" toggle |
| `runemaster` | `int8_t` | 0 or 1 | "Runemaster" toggle |
| `conjurer` | `int8_t` | 0 or 1 | "Conjurer" toggle |
| `charismatic` | `int8_t` | 0 or 1 | "Charismatic" toggle |
| `artificer` | `int8_t` | 0 or 1 | "Artificer" toggle |

### Value-to-Name Mappings (New, Presentation Layer Only)

```
RACE_NAMES[14] = ["Barbarians", "Beastmen", "Dark Elves", "Draconians",
                   "Dwarves", "Gnolls", "Halflings", "High Elves",
                   "High Men", "Klackons", "Lizardmen", "Nomads",
                   "Orcs", "Trolls"]

REALM_NAMES[5] = ["Nature", "Sorcery", "Chaos", "Life", "Death"]
                  (struct order, matching spellranks[0..4])

RETORT_NAMES[18] = ["Alchemy", "Warlord", "Chaos Mastery", "Nature Mastery",
                     "Sorcery Mastery", "Infernal Power", "Divine Power",
                     "Sage Master", "Channeler", "Myrran", "Archmage",
                     "Mana Focusing", "Node Mastery", "Famous", "Runemaster",
                     "Conjurer", "Charismatic", "Artificer"]

SECTION_DESCRIPTIONS:
  Race     → "Starting city population and available units"
  Spells   → "Spell books per realm (0-4)"
  Retorts  → "Special wizard abilities and bonuses"
```

These mappings exist only as `const char*` arrays in `WizardPanel.cpp`. No struct, class, or file changes to the data layer.

## Relationships

- `Wizard` is an array of 6 in `Scenario` (accessed via `scenario.wizards[i]`)
- `wizard_tab` in `EditorState` tracks the currently selected wizard index (0-5)
- Settings are read during `render()` and modified during `handle_click()` via `UndoStack`
- No new relationships introduced
