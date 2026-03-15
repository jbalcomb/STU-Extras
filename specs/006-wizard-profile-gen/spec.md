# Feature Specification: Wizard Profile Generation

**Feature Branch**: `006-wizard-profile-gen`
**Created**: 2026-03-13
**Status**: Draft
**Input**: User description: "Generate wizard profiles based on the existing 14 preset wizards from Master of Magic, with global and per-wizard generate buttons, confirmation prompts when overwriting manually edited data."

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Generate All Wizard Profiles (Priority: P1)

A scenario designer wants to quickly populate all wizard slots with classic Master of Magic wizard profiles so they can start customizing a scenario without manually configuring each wizard from scratch. They click a "Generate Wizards" button near the world map generate button, and all empty or auto-stubbed wizard slots are filled with preset profiles based on the original game's 14 wizards (Merlin, Raven, Sharee, Lo Pan, Jafar, Oberic, Rjak, Sss'ra, Tauron, Freya, Horus, Ariel, Tlaloc, Kali).

**Why this priority**: This is the primary workflow — most users will want to batch-generate all wizards at once to get a playable scenario quickly.

**Independent Test**: Can be fully tested by clicking "Generate Wizards" and verifying all 6 wizard slots are populated with correct preset data (name, spell books, retort, race).

**Acceptance Scenarios**:

1. **Given** all 6 wizard slots are empty (no fortress placed), **When** user clicks "Generate Wizards", **Then** wizards 0–5 are populated with the first 6 presets (Merlin, Raven, Sharee, Lo Pan, Jafar, Oberic) including name, spell book distribution, retort, and race
2. **Given** some wizard slots have auto-generated stubs (from fortress placement) but no manual edits, **When** user clicks "Generate Wizards", **Then** those slots are overwritten with preset data without any confirmation prompt
3. **Given** one or more wizard slots have been manually edited by the user, **When** user clicks "Generate Wizards", **Then** a confirmation prompt appears warning that manually edited wizard data will be overwritten, listing which wizards have been modified
4. **Given** user sees the confirmation prompt, **When** user confirms, **Then** all wizard slots are overwritten with preset data; **When** user cancels, **Then** no changes are made

---

### User Story 2 - Generate Single Wizard Profile (Priority: P2)

A scenario designer wants to regenerate or generate just one specific wizard's profile — perhaps they deleted one wizard's data and want to restore it, or they want to replace one wizard with a preset while keeping others untouched. They click a "Generate" button on the individual wizard's tab in the Wizard Panel.

**Why this priority**: Per-wizard generation gives fine-grained control, but is less commonly needed than batch generation.

**Independent Test**: Can be tested by opening a specific wizard tab, clicking "Generate", and verifying only that wizard's data changes while others remain untouched.

**Acceptance Scenarios**:

1. **Given** wizard slot 2 is empty, **When** user clicks "Generate" on wizard 2's tab, **Then** wizard 2 is populated with the preset for slot 2 (Sharee: 5 Death + 5 Sorcery books, Conjurer retort)
2. **Given** wizard slot 3 has been manually edited, **When** user clicks "Generate" on wizard 3's tab, **Then** a confirmation prompt appears warning that manually edited data will be overwritten
3. **Given** wizard slot 3 has only auto-stub data (from fortress placement, unmodified), **When** user clicks "Generate" on wizard 3's tab, **Then** preset data is applied immediately without confirmation

---

### User Story 3 - Edit After Generation (Priority: P3)

After generating wizard profiles, a scenario designer wants to customize individual wizards — changing spell book distributions, swapping retorts, renaming wizards, or adjusting races. All generated data serves as a starting point, not a locked configuration.

**Why this priority**: Editing is already supported by the existing Wizard Panel; this story ensures generation doesn't break editability and that the dirty-flag tracking works correctly after generation.

**Independent Test**: Can be tested by generating a wizard, then modifying any field and verifying the change persists and the wizard is now marked as "manually edited."

**Acceptance Scenarios**:

1. **Given** wizard 0 was just generated with Merlin's preset, **When** user changes the name from "Merlin" to "Gandalf", **Then** the change is saved and wizard 0 is now marked as manually edited
2. **Given** wizard 0 is marked as manually edited, **When** user clicks "Generate Wizards" (global), **Then** confirmation prompt includes wizard 0 in the list of modified wizards
3. **Given** wizard 0 was generated and then re-generated (without manual edits in between), **When** user clicks "Generate Wizards" again, **Then** no confirmation prompt appears for wizard 0

---

### Edge Cases

- What happens when fewer than 6 fortress slots are active? Generation still populates wizard data for all 6 slots; fortresses are independent (wizard can exist without a fortress)
- What happens if the user generates wizards, undoes, then generates again? The undo system restores previous wizard state including dirty flags; re-generating works normally
- What happens with wizard slot 0 (the human player slot)? Slot 0 is treated identically — it gets the first preset (Merlin). The editor doesn't distinguish human vs AI slots during editing
- What happens when generating globally with some manually edited and some not? Only manually edited wizards trigger the confirmation prompt; unedited slots are silently overwritten

## Clarifications

### Session 2026-03-13

- Q: Should the dirty flag persist when saving/loading `.mom_scenario` files, or is it session-only? → A: Session-only — dirty flag resets on save/load; loaded wizards are treated as unmodified for generation purposes.
- Q: Which specific race should be assigned to generated wizards? → A: Each preset uses its canonical race from the original game (e.g., Merlin=High Men, Sss'ra=Dark Elves). Always start with what the original game did.
- Q: How should the confirmation prompt work in the SDL2 UI? → A: Modal overlay dialog rendered in SDL2 with "Overwrite" / "Cancel" buttons.

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: System MUST provide a "Generate Wizards" button accessible from the main editor UI, near the existing world map generate button
- **FR-002**: System MUST provide a per-wizard "Generate" button on each wizard's tab in the Wizard Panel
- **FR-003**: System MUST use the original Master of Magic preset wizard data (14 presets: Merlin, Raven, Sharee, Lo Pan, Jafar, Oberic, Rjak, Sss'ra, Tauron, Freya, Horus, Ariel, Tlaloc, Kali)
- **FR-004**: Global generation MUST assign presets to wizard slots 0–5 using the first 6 presets in order (Merlin through Oberic)
- **FR-005**: Per-wizard generation MUST assign the preset corresponding to that wizard's slot index (slot 0 = Merlin, slot 1 = Raven, ..., slot 5 = Oberic)
- **FR-006**: Generated wizards MUST be assigned their canonical race from the original game (e.g., Merlin=High Men, Sss'ra=Dark Elves). Each preset's race must match the original game's default for that wizard
- **FR-007**: System MUST display a modal overlay dialog with "Overwrite" and "Cancel" buttons before overwriting any wizard slot that has been manually edited by the user. The dialog MUST list which wizard slots have been modified
- **FR-008**: System MUST NOT display a confirmation prompt for empty slots, auto-generated stubs (from fortress placement), or unmodified generated data
- **FR-009**: All generation operations MUST support undo/redo through the existing undo system
- **FR-010**: Generated preset data MUST include: wizard name, spell book distribution (Life, Death, Chaos, Nature, Sorcery counts), one retort, and canonical race
- **FR-011**: System MUST track a per-wizard dirty flag indicating whether the user has manually modified that wizard's data since last generation or creation

### Key Entities

- **Wizard Preset**: A fixed data record representing one of the 14 original Master of Magic wizards, containing name, spell book counts (5 realms), and a single retort. Source: `_wizard_presets_table` in the original game code
- **Wizard Profile**: The editable wizard data in a scenario slot (existing Wizard struct in MomEntities.hpp). Generation copies preset data into this profile
- **Dirty Flag**: A per-wizard session-only boolean indicating whether the user has manually modified wizard data since the last generation or auto-stub creation. Resets on save/load (loaded wizards are treated as unmodified). Used to determine whether confirmation prompts are needed

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: Users can populate all 6 wizard slots with preset data in a single action (one click + optional confirmation)
- **SC-002**: Users can populate any individual wizard slot with preset data in a single action (one click + optional confirmation)
- **SC-003**: Manually edited wizard data is never silently overwritten — confirmation is always required
- **SC-004**: All generation operations are fully reversible via undo
- **SC-005**: Generated wizard profiles match the original Master of Magic preset data (name, spell books, retort) for the corresponding slot
