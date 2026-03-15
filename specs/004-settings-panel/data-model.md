# Data Model: Settings Panel Labels & Documentation

**Feature**: 004-settings-panel | **Date**: 2026-03-11

## Existing Entities (No Changes)

This feature adds no new data entities. All settings are stored in the existing `GameData` struct (`MomEntities.hpp`) which remains unchanged.

### GameData Fields Used

| Field | Type | Values | Display Name |
|-------|------|--------|-------------|
| `Difficulty` | `uint16_t` | 0-4 | "Intro", "Easy", "Normal", "Hard", "Impossible" |
| `Magic` | `uint16_t` | 0-3 | "Weak", "Normal", "Powerful", "Overwhelming" |
| `Land_Size` | `uint16_t` | 0-2 | "Small", "Medium", "Large" |
| `Total_Wizards` | `uint16_t` | 1-6 | "N Wizard(s)" |

### Value-to-Name Mappings (New, Presentation Layer Only)

```
DIFFICULTY_NAMES = ["Intro", "Easy", "Normal", "Hard", "Impossible"]
MAGIC_NAMES = ["Weak", "Normal", "Powerful", "Overwhelming"]
LAND_SIZE_NAMES = ["Small", "Medium", "Large"]

SETTING_DESCRIPTIONS:
  Difficulty  → "AI aggressiveness and bonuses"
  Magic       → "Magical node power and spell frequency"
  Land Size   → "Land-to-ocean ratio on the map"
  Wizards     → "Total players (1 human + AI opponents)"
```

These mappings exist only as `const char*` arrays in `GameOptionsPanel.cpp`. No struct, class, or file changes to the data layer.

## Relationships

- `GameData` is a member of `Scenario` (accessed via `scenario.game_data`)
- Settings are read during `render()` and modified during `handle_click()` via `UndoStack`
- No new relationships introduced
