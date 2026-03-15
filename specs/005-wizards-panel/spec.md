# Feature Specification: Wizards Panel Labels & Documentation

**Feature Branch**: `005-wizards-panel`
**Created**: 2026-03-11
**Status**: Draft
**Input**: Add text labels and documentation to the Wizards panel. The panel currently renders wizard configuration as unlabeled colored shapes. Each element needs a text label, the current selected value name, and a description of what it does in Master of Magic gameplay.

## Clarifications

### Session 2026-03-11

- Q: Should spell realms display in game-canonical order (Life, Death, Chaos, Nature, Sorcery) or struct order (Nature, Sorcery, Chaos, Life, Death)? → A: Struct order [Nature, Sorcery, Chaos, Life, Death]. The alternative order is specific to MoM's New Game code area and not canonical.
- Q: How should the 18 retort toggles be laid out? → A: 3-column grid (6 rows x 3 columns), matching the original game's retort layout for familiarity.

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Wizard Tabs Are Labeled (Priority: P1)

A scenario designer opens the Wizards panel and sees 6 wizard tabs at the top. Each tab displays the wizard's name (e.g., "Merlin", "Raven") or "Empty" if unconfigured. The currently selected tab is visually highlighted. The designer can immediately tell which wizard slot they're editing and which slots are populated.

**Why this priority**: Without labels, the colored boxes give no indication of which wizard is which or whether a slot is configured. This is the minimum viable improvement.

**Acceptance Scenarios**:

1. **Given** the WIZARDS tool is active, **When** the panel renders, **Then** each of the 6 wizard tabs displays the wizard's name or "Empty" if inactive.
2. **Given** the user clicks a wizard tab, **When** the tab changes, **Then** the selected tab is highlighted and the panel updates to show that wizard's configuration.
3. **Given** a wizard has a custom name, **When** rendered, **Then** the tab shows that custom name (not just "Wizard 1").

---

### User Story 2 - Race Selector Is Labeled (Priority: P1)

The Race selector shows the currently selected race name (e.g., "High Elves", "Draconians") and a label "Race". A brief description explains that race determines the wizard's starting city population type and available units.

**Why this priority**: Race is one of the most impactful wizard configuration choices and is completely opaque as a colored bar.

**Acceptance Scenarios**:

1. **Given** the panel shows a wizard's configuration, **When** the user views the Race section, **Then** they see the label "Race" and the current race name.
2. **Given** the user clicks the race selector, **When** the value cycles through 14 races, **Then** the displayed race name updates immediately.
3. **Given** any race value 0-13, **When** rendered, **Then** the correct race name is displayed: Barbarians (0), Beastmen (1), Dark Elves (2), Draconians (3), Dwarves (4), Gnolls (5), Halflings (6), High Elves (7), High Men (8), Klackons (9), Lizardmen (10), Nomads (11), Orcs (12), Trolls (13).

---

### User Story 3 - Portrait Selector Is Labeled (Priority: P2)

The Portrait selector shows the label "Portrait" and a numeric identifier or wizard name for the current portrait. A brief description explains that the portrait determines the wizard's visual identity in the game.

**Why this priority**: Portrait is a cosmetic setting with less gameplay impact than race or spells, but is still confusing as an unlabeled colored square.

**Acceptance Scenarios**:

1. **Given** the panel shows a wizard's configuration, **When** the user views the Portrait section, **Then** they see the label "Portrait" and the current portrait number (0-14).
2. **Given** the user clicks the portrait selector, **When** the value cycles, **Then** the displayed portrait number updates.

---

### User Story 4 - Spell Rank Bars Are Labeled (Priority: P1)

The 5 spell rank bars are labeled with their realm names: Life, Death, Chaos, Nature, Sorcery. Each bar shows the current rank value (0-4) as text. A brief description explains that spell ranks determine the wizard's starting spell books and access to higher-tier spells in each realm.

**Why this priority**: Spell ranks are a core wizard configuration element. The 5 unlabeled colored bars are indistinguishable without realm names, and rank values are meaningless without numbers.

**Acceptance Scenarios**:

1. **Given** the panel shows spell rank bars, **When** rendered, **Then** each bar displays its realm name in struct order: "Nature", "Sorcery", "Chaos", "Life", "Death".
2. **Given** any spell rank value 0-4, **When** rendered, **Then** the current rank number is displayed next to the bar.
3. **Given** the user clicks a spell rank bar, **When** the rank cycles 0-4, **Then** the displayed rank number updates.

---

### User Story 5 - Retort Toggles Are Labeled (Priority: P2)

The 18 retort toggles are displayed in a 3-column grid (6 rows x 3 columns), matching the original game's layout. Each toggle is labeled with its retort name (e.g., "Alchemy", "Warlord", "Myrran", "Archmage") and shows whether it is active or inactive. A section header "Retorts" explains that retorts are special abilities that provide unique bonuses.

**Why this priority**: Retorts are important for experienced players but there are 18 of them. Labels transform an unusable grid of colored squares into a readable checklist.

**Acceptance Scenarios**:

1. **Given** the panel shows retort toggles, **When** rendered, **Then** each toggle displays its retort name.
2. **Given** a retort is active, **When** rendered, **Then** it is visually distinguishable from inactive retorts (e.g., brighter color, checkmark, or highlight).
3. **Given** the user clicks a retort toggle, **When** the state changes, **Then** the visual indicator updates immediately.

---

### User Story 6 - Click Targets Match Visual Layout (Priority: P1)

When the user clicks on any element in the Wizards panel, the click is handled by the correct element. Adding labels and text changes the vertical layout, so click regions must be updated to match.

**Why this priority**: Misaligned click targets make the panel unusable.

**Acceptance Scenarios**:

1. **Given** the user clicks on a wizard tab, **When** processed, **Then** only the tab selection changes (not race or spells).
2. **Given** the user clicks on the race selector, **When** processed, **Then** only the race value changes.
3. **Given** the user clicks on a spell rank bar, **When** processed, **Then** only that realm's rank changes (not adjacent realms).
4. **Given** the user clicks on a retort toggle, **When** processed, **Then** only that retort toggles (not adjacent retorts).

---

### Edge Cases

- What happens when wizard name is empty (new/unconfigured wizard)? Tab shows "Empty".
- What happens when wizard name is longer than the tab width? Truncate or clip to available width.
- What happens when Total_Wizards is less than 6? Tabs beyond the count should still be clickable (to allow adding wizards) but marked as inactive/empty.

## Functional Requirements *(mandatory)*

### FR-1: Wizard Tab Labels
Each of the 6 wizard tabs must display the wizard's name or "Empty" if `name[0] == '\0'`.

### FR-2: Race Label and Value Display
The race selector must display the label "Race" and the currently selected race name from the 14 Master of Magic races.

### FR-3: Race Names
Race values 0-13 must map to: Barbarians, Beastmen, Dark Elves, Draconians, Dwarves, Gnolls, Halflings, High Elves, High Men, Klackons, Lizardmen, Nomads, Orcs, Trolls.

### FR-4: Race Description
The race section must include a brief description: "Starting city population and available units".

### FR-5: Portrait Label
The portrait selector must display the label "Portrait" and the current portrait index (0-14).

### FR-6: Spell Rank Labels
Each of the 5 spell rank bars must display its realm name in struct order: Nature, Sorcery, Chaos, Life, Death (matching spellranks[0..4]). The current rank (0-4) must be displayed as text.

### FR-7: Spell Rank Description
The spell ranks section must include a brief description: "Spell books per realm (0-4)".

### FR-8: Retort Labels and Layout
The 18 retort toggles must be displayed in a 3-column grid (6 rows x 3 columns), matching the original game's layout. Each toggle must display its retort name: Alchemy, Warlord, Chaos Mastery, Nature Mastery, Sorcery Mastery, Infernal Power, Divine Power, Sage Master, Channeler, Myrran, Archmage, Mana Focusing, Node Mastery, Famous, Runemaster, Conjurer, Charismatic, Artificer.

### FR-9: Retort Section Header
The retorts section must include a header "Retorts" and brief description: "Special wizard abilities and bonuses".

### FR-10: Click-Render Alignment
The `handle_click()` Y-coordinate tracking must match the `render()` layout exactly after all label additions.

### FR-11: Undo Support Preserved
All value changes must continue to go through the UndoStack.

### Key Entities

- **Wizard**: Player entity with wizard_id (portrait 0-14), name (20 chars), capital_race (0-13), banner_id, spellranks[5] (Life/Death/Chaos/Nature/Sorcery, each 0-4), and 18 retort flags (each 0 or 1). Size: 1224 bytes.
- **Race**: One of 14 fantasy races from Master of Magic. Determines starting city type and available military units.
- **Spell Realm**: One of 5 magic realms (Life, Death, Chaos, Nature, Sorcery). Ranks 0-4 determine spell book count and access to higher-tier spells.
- **Retort**: One of 18 special abilities providing unique bonuses (e.g., Myrran allows starting on the shadow plane, Archmage reduces spell casting cost).

## Success Criteria *(mandatory)*

### Measurable Outcomes

1. A new user can identify all wizard configuration elements and their current values without prior knowledge of Master of Magic.
2. Each wizard tab displays the wizard's name or "Empty" for unconfigured slots.
3. Race, portrait, spell ranks, and retorts all display human-readable labels and current values.
4. Clicking any element changes only that element's value.
5. All settings render correctly in both desktop and WASM builds.
6. Undo/redo continues to work for all wizard configuration changes.

---

## Scope & Boundaries *(mandatory)*

### In Scope
- Text labels for all wizard configuration elements
- Current value display as named text (race names, realm names, retort names, rank numbers)
- Brief descriptions for each section
- Click handling alignment with new layout
- Desktop and WASM rendering

### Out of Scope
- Wizard name editing (text input field) — may be added later
- Wizard portrait preview images — requires sprite assets
- Spell book visualization — requires additional UI beyond rank numbers
- Retort dependency validation (e.g., requiring certain spell ranks for certain retorts)
- Adding new wizard configuration options beyond existing struct fields
- Changes to the Wizard struct or .GAM binary format

---

## Assumptions *(mandatory)*

1. The existing `Renderer::draw_text()` method provides sufficient text rendering capability (confirmed from 003-font-rendering).
2. The Cousine-Regular.ttf font at 12pt provides readable text at the panel width of 200px.
3. The panel has sufficient vertical space to accommodate labels for all sections. The retort grid uses a 3-column layout (6 rows x 3 columns) to keep vertical space manageable.
4. The 14 race names match the original Master of Magic game terminology.
5. The spellranks array order in the Wizard struct is [Nature, Sorcery, Chaos, Life, Death] (matching ReMoM source). The UI displays in this same struct order — the "Life, Death, Chaos, Nature, Sorcery" order seen in MoM's New Game screen is specific to that code area and not canonical.
6. The 18 retort field names in the struct map directly to their in-game names.

---

## Dependencies *(optional)*

- **003-font-rendering**: SDL_ttf integration and `draw_text()` must be functional (completed).
- **004-settings-panel**: Established the pattern for adding labels/descriptions to unlabeled UI panels.
- **Wizard struct**: Must remain binary-compatible with .GAM format (no struct changes needed).
