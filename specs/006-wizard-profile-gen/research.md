# Research: Wizard Profile Generation

**Feature**: 006-wizard-profile-gen
**Date**: 2026-03-13

## Decision 1: Spell Book Field Mapping

**Decision**: Map preset table fields to Wizard struct spellranks using verified field ordering.

**Rationale**: The preset struct (`s_WIZARD_PRESET` in NewGame.h:58-68) has fields:
```
name[10], life, sorcery, nature, death, chaos, special
```

The Wizard struct's `spellranks[5]` in MomEntities.hpp has indices:
```
[0]=Nature, [1]=Sorcery, [2]=Chaos, [3]=Life, [4]=Death
```

Mapping from preset → spellranks:
- preset.life    → spellranks[3]
- preset.sorcery → spellranks[1]
- preset.nature  → spellranks[0]
- preset.death   → spellranks[4]
- preset.chaos   → spellranks[2]

**Alternatives considered**: None — this is a direct factual mapping from source code.

## Decision 2: Preset Data (Verified from NewGame.c:469-485)

Using verified field order `{name, life, sorcery, nature, death, chaos, retort}`:

| Slot | Name    | Life | Sorc | Nat  | Death | Chaos | Retort         |
|------|---------|------|------|------|-------|-------|----------------|
| 0    | Merlin  | 5    | 0    | 5    | 0     | 0     | Sage Master    |
| 1    | Raven   | 0    | 6    | 5    | 0     | 0     | (none)         |
| 2    | Sharee  | 0    | 0    | 0    | 5     | 5     | Conjurer       |
| 3    | Lo Pan  | 0    | 5    | 0    | 0     | 5     | Channeler      |
| 4    | Jafar   | 0    | 10   | 0    | 0     | 0     | Alchemy        |
| 5    | Oberic  | 0    | 0    | 5    | 0     | 5     | Mana Focusing  |
| 6    | Rjak    | 0    | 0    | 0    | 9     | 0     | Infernal Power |
| 7    | Sss'ra  | 4    | 0    | 0    | 0     | 4     | Myrran         |
| 8    | Tauron  | 0    | 0    | 0    | 0     | 10    | Chaos Mastery  |
| 9    | Freya   | 0    | 0    | 10   | 0     | 0     | Nature Mastery |
| 10   | Horus   | 5    | 5    | 0    | 0     | 0     | Archmage       |
| 11   | Ariel   | 10   | 0    | 0    | 0     | 0     | Charismatic    |
| 12   | Tlaloc  | 0    | 0    | 4    | 5     | 0     | Warlord        |
| 13   | Kali    | 0    | 5    | 0    | 5     | 0     | Artificer      |

Only slots 0–5 are used for the 6-wizard scenario editor.

## Decision 3: Race Assignment for Generated Wizards

**Decision**: Use High Men (RACE_HIGH_MAN = 8) as default for Arcanus wizards, Dark Elf (RACE_DARK_ELF = 2) for Myrran wizards.

**Rationale**: The original game does NOT store canonical races in the wizard preset table. Race is assigned randomly from `TBL_Arcanian_Races[9]` or `TBL_Myrran_Races[5]` during world generation (see NewGame.c:818-821). Since there is no "original game race" per wizard, we use reasonable defaults:
- High Men: the most generic/common Arcanus race, associated with human wizard archetypes
- Dark Elf: commonly associated with Sss'ra in game lore and community

For the 6 wizards used (slots 0–5), only Sss'ra (slot 7) has the Myrran retort, but slot 7 is beyond the 6-slot range. All slots 0–5 use Arcanus races, so all get High Men by default.

**Alternatives considered**:
- Random from plane-appropriate list: more faithful but less predictable, harder to test
- Wizard-specific hardcoded mapping: no basis in original game data

## Decision 4: Retort Mapping to Wizard Struct Fields

**Decision**: Map the `special` enum value from the preset to the corresponding boolean retort field in the Wizard struct.

The Wizard struct has 18 individual boolean retort fields (alchemy, warlord, chaos_mastery, etc.). The preset's `special` field is an enum `e_WIZARD_SPECIAL_ABILITY`. The mapping:

| Enum (wsa_*)       | Wizard struct field |
|--------------------|---------------------|
| wsa_Alchemy        | alchemy             |
| wsa_Warlord        | warlord             |
| wsa_Chaos_Mastery  | chaos_mastery       |
| wsa_Nature_Mastery | nature_mastery      |
| wsa_Sorcery_Mastery| sorcery_mastery     |
| wsa_Infernal_Power | infernal_power      |
| wsa_Divine_Power   | divine_power        |
| wsa_Sage_Master    | sage_master         |
| wsa_Channeller     | channeler           |
| wsa_Myrran         | myrran              |
| wsa_Archmage       | archmage            |
| wsa_Mana_Focusing  | mana_focusing       |
| wsa_Node_Mastery   | node_mastery        |
| wsa_Famous         | famous              |
| wsa_Runemaster     | runemaster          |
| wsa_Conjurer       | conjurer            |
| wsa_Charismatic    | charismatic         |
| wsa_Artificer      | artificer           |
| wsa_NONE           | (no retort set)     |

Each preset has exactly 0 or 1 retort. Clear all retort booleans first, then set the one matching the preset's special field.

## Decision 5: Dirty Flag Storage

**Decision**: Add `bool wizard_dirty[NUM_PLAYERS]{}` array to EditorState (session-only).

**Rationale**: EditorState already holds session-only UI state (wizard_tab, status_message, generate_requested). Adding a dirty flag array follows the existing pattern. The flag is set when the user modifies any wizard field via the WizardPanel click handler. It is cleared on generation and implicitly reset on save/load (EditorState is reconstructed or reset).

**Alternatives considered**:
- Store in Wizard struct: violates layer separation (data layer shouldn't track UI state)
- Store in WizardPanel: would require WizardPanel to be queried by ToolPanel, coupling UI components

## Decision 6: Modal Overlay Dialog

**Decision**: Implement a simple SDL2-rendered modal overlay using existing UIRenderer primitives.

**Rationale**: The codebase already has an overlay pattern (race dropdown in WizardPanel). A modal confirmation dialog follows the same approach:
1. A flag in EditorState (`bool wizard_gen_confirm_open{}`)
2. Render the dialog last (on top of everything) when flag is true
3. Handle clicks on the dialog first, consuming the event
4. Two buttons: "Overwrite" and "Cancel"
5. List modified wizard names in the dialog body

**Alternatives considered**:
- SDL_ShowSimpleMessageBox: native OS dialog, but less control over appearance and doesn't list wizard names
- Two-click confirm: simpler but less clear to users
