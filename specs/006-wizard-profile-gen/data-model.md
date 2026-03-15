# Data Model: Wizard Profile Generation

**Feature**: 006-wizard-profile-gen
**Date**: 2026-03-13

## Entities

### WizardPreset (new, compile-time constant)

A static constexpr array of preset wizard configurations. Not a packed struct — this is editor-only data used to populate Wizard structs.

| Field       | Type        | Description                                              |
|-------------|-------------|----------------------------------------------------------|
| name        | const char* | Wizard name (up to 19 chars for LEN_WIZARD_NAME)         |
| life        | int16_t     | Life spell book count                                    |
| sorcery     | int16_t     | Sorcery spell book count                                 |
| nature      | int16_t     | Nature spell book count                                  |
| death       | int16_t     | Death spell book count                                   |
| chaos       | int16_t     | Chaos spell book count                                   |
| retort      | int         | Retort index (maps to one of 18 boolean fields, or -1 for none) |
| race        | uint8_t     | Default race ID (RACE_HIGH_MAN for Arcanus, RACE_DARK_ELF for Myrran) |

**Count**: 14 presets (indices 0–13). Only 0–5 used for 6-slot generation.

**Validation rules**:
- Total spell books per wizard: must be > 0 (except "Custom" which is excluded)
- Exactly 0 or 1 retort per wizard
- Race must be plane-appropriate (Myrran retort → Myrran race)

### Wizard (existing, MomEntities.hpp — NOT MODIFIED)

No changes to the packed Wizard struct. Generation writes into existing fields:

| Target Field      | Source               | Notes                                                |
|-------------------|----------------------|------------------------------------------------------|
| name[20]          | preset.name          | snprintf with LEN_WIZARD_NAME limit                  |
| wizard_id         | slot index           | Portrait index (0–5)                                 |
| banner_id         | slot index           | Banner color matches slot                            |
| capital_race      | preset.race          | Default race for the wizard                          |
| spellranks[0]     | preset.nature        | Nature book count                                    |
| spellranks[1]     | preset.sorcery       | Sorcery book count                                   |
| spellranks[2]     | preset.chaos         | Chaos book count                                     |
| spellranks[3]     | preset.life          | Life book count                                      |
| spellranks[4]     | preset.death         | Death book count                                     |
| (retort booleans) | preset.retort        | Clear all 18 booleans, then set the one matching retort |

### EditorState (existing, EditorState.hpp — EXTENDED)

New session-only fields added:

| Field                     | Type                    | Default | Description                                       |
|---------------------------|-------------------------|---------|---------------------------------------------------|
| wizard_dirty[NUM_PLAYERS] | bool[6]                 | {false} | Per-wizard dirty flag, set on manual edit          |
| wizard_gen_confirm_open   | bool                    | false   | Modal confirmation dialog visibility flag          |
| wizard_gen_confirm_global | bool                    | false   | true = global gen, false = single-wizard gen       |
| wizard_gen_confirm_idx    | int                     | 0       | Wizard index for single-wizard gen (when !global)  |

## State Transitions

### Dirty Flag Lifecycle

```
Empty/New → [fortress placed] → Auto-stub (dirty=false)
                                     ↓
                              [user edits field] → Dirty (dirty=true)
                                     ↓
                              [generate applied] → Generated (dirty=false)
                                     ↓
                              [user edits field] → Dirty (dirty=true)
                                     ↓
                              [save/load cycle]  → Clean (dirty=false, all flags reset)
```

### Generation Flow

```
User clicks "Generate" →
  Check dirty flags for affected wizard(s) →
    Any dirty? → Show modal confirmation dialog
                   ├── "Overwrite" → Apply presets, clear dirty flags, push undo
                   └── "Cancel"    → No-op
    None dirty? → Apply presets directly, push undo
```

## Relationships

- WizardPreset[i] → Wizard slot [i] (1:1 mapping by index, slots 0–5)
- EditorState.wizard_dirty[i] tracks Wizard slot [i]
- EditorState.wizard_gen_confirm_open gates the modal overlay
- Undo command captures full Wizard struct + dirty flag state for rollback
