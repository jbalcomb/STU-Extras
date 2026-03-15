# Tasks: Wizards Panel Labels & Documentation

**Input**: Design documents from `/specs/005-wizards-panel/`
**Prerequisites**: plan.md (required), spec.md (required for user stories), research.md, data-model.md

**Tests**: No new tests required. Existing tests must continue passing.

**Organization**: Tasks are grouped by user story to enable independent implementation and testing of each story.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which user story this task belongs to (e.g., US1, US2, US3)
- Include exact file paths in descriptions

---

## Phase 1: Setup

**Purpose**: Fix panel position and verify build.

- [x] T001 Fix `PANEL_TOP` from 280 to 332 in `src/ui/WizardPanel.cpp` to prevent obscuring tool buttons (matching 004-settings-panel fix)
- [x] T002 Verify build succeeds with `bash build_wasm_stage.sh`

**Checkpoint**: Build succeeds, panel no longer overlaps tool buttons.

---

## Phase 2: Foundational (Value Name Lookup Tables & Layout Constants)

**Purpose**: Define the static name arrays, description strings, and layout constants that all user stories depend on.

- [x] T003 Add static `const char*` array `RACE_NAMES[14]` at the top of `src/ui/WizardPanel.cpp`: Barbarians, Beastmen, Dark Elves, Draconians, Dwarves, Gnolls, Halflings, High Elves, High Men, Klackons, Lizardmen, Nomads, Orcs, Trolls
- [x] T004 Add static `const char*` array `REALM_NAMES[5]` in `src/ui/WizardPanel.cpp` in struct order: Nature, Sorcery, Chaos, Life, Death
- [x] T005 Add static `const char*` array `RETORT_NAMES[18]` in `src/ui/WizardPanel.cpp`: Alchemy, Warlord, Chaos Mastery, Nature Mastery, Sorcery Mastery, Infernal Power, Divine Power, Sage Master, Channeler, Myrran, Archmage, Mana Focusing, Node Mastery, Famous, Runemaster, Conjurer, Charismatic, Artificer
- [x] T006 Add static `const char*` description strings in `src/ui/WizardPanel.cpp`: Race = "Starting city population and available units", Spells = "Spell books per realm (0-4)", Retorts = "Special wizard abilities and bonuses"
- [x] T007 Add layout constants `LABEL_H = 14` and `DESC_H = 12` in `src/ui/WizardPanel.cpp` (matching GameOptionsPanel)
- [x] T008 Reorder `REALM_COLORS` array in `src/ui/WizardPanel.cpp` from [Life, Death, Chaos, Nature, Sorcery] to struct order [Nature, Sorcery, Chaos, Life, Death]

**Checkpoint**: New constants and arrays compile. No behavior change yet.

---

## Phase 3: User Story 1 - Wizard Tabs Are Labeled (Priority: P1)

**Goal**: Each of the 6 wizard tabs displays the wizard's name or "Empty" if unconfigured.

**Independent Test**: Open Wizards panel, verify all 6 tabs show wizard names or "Empty". Click tabs and verify selection updates.

### Implementation for User Story 1

- [x] T009 [US1] Update wizard tab rendering in `render()` in `src/ui/WizardPanel.cpp` to draw each wizard's name (from `wiz.name`) or "Empty" (if `!is_active()`) as text inside or below each tab using `renderer.draw_text()`
- [x] T010 [US1] Build and verify with `bash build_wasm_stage.sh` — wizard tabs show names

**Checkpoint**: All 6 wizard tabs show names or "Empty". Tab selection still works.

---

## Phase 4: User Story 2 - Race Selector Is Labeled (Priority: P1)

**Goal**: Race selector displays "Race" label, current race name, and description.

**Independent Test**: Open Wizards panel, verify Race section shows label, race name, and description. Click to cycle races and verify name updates.

### Implementation for User Story 2

- [x] T011 [US2] Update race selector in `render()` in `src/ui/WizardPanel.cpp` to draw "Race: {name}" label above the race bar using `renderer.draw_text()` and `RACE_NAMES[]` lookup
- [x] T012 [US2] Add description text "Starting city population and available units" below the Race label in `render()` in `src/ui/WizardPanel.cpp`, rendered in dimmer color (160, 160, 160)
- [x] T013 [US2] Update `handle_click()` Y-offsets in `src/ui/WizardPanel.cpp` to account for added Race label and description line heights

**Checkpoint**: Race section shows label + race name + description. Click target still aligned.

---

## Phase 5: User Story 3 - Portrait Selector Is Labeled (Priority: P2)

**Goal**: Portrait selector displays "Portrait" label and current portrait index.

**Independent Test**: Open Wizards panel, verify Portrait section shows label and index number.

### Implementation for User Story 3

- [x] T014 [US3] Update portrait selector in `render()` in `src/ui/WizardPanel.cpp` to draw "Portrait: {index}" label above the portrait square using `renderer.draw_text()`
- [x] T015 [US3] Update `handle_click()` Y-offsets in `src/ui/WizardPanel.cpp` to account for added Portrait label line height

**Checkpoint**: Portrait section shows label + index. Click target still aligned.

---

## Phase 6: User Story 4 - Spell Rank Bars Are Labeled (Priority: P1)

**Goal**: Each spell rank bar displays its realm name and current rank value. Bars display in struct order.

**Independent Test**: Open Wizards panel, verify 5 bars show realm names (Nature, Sorcery, Chaos, Life, Death) and rank numbers. Click to cycle ranks.

### Implementation for User Story 4

- [x] T016 [US4] Remove `realm_to_idx[]` remapping array from `render()` in `src/ui/WizardPanel.cpp` — iterate `r = 0..4` directly to display in struct order
- [x] T017 [US4] Add "Spell books per realm (0-4)" description text above the spell rank bars in `render()` in `src/ui/WizardPanel.cpp`
- [x] T018 [US4] Add realm name label and rank number text to each spell rank bar in `render()` in `src/ui/WizardPanel.cpp` using `REALM_NAMES[r]` and `std::to_string(rank)`
- [x] T019 [US4] Remove `realm_to_idx[]` remapping from `handle_click()` in `src/ui/WizardPanel.cpp` and update Y-offsets to account for added description and label text

**Checkpoint**: Spell rank bars show realm names in struct order with rank numbers. Click targets aligned.

---

## Phase 7: User Story 5 - Retort Toggles Are Labeled (Priority: P2)

**Goal**: Retorts display in 3-column grid with retort names and active/inactive indicators. Section has header and description.

**Independent Test**: Open Wizards panel, verify 18 retorts in 3-column grid show names. Click toggles and verify state changes.

### Implementation for User Story 5

- [x] T020 [US5] Add "Retorts" section header and "Special wizard abilities and bonuses" description text above the retort grid in `render()` in `src/ui/WizardPanel.cpp`
- [x] T021 [US5] Change retort rendering in `render()` in `src/ui/WizardPanel.cpp` from auto-calculated columns to fixed 3-column grid (6 rows x 3 columns) with `RETORT_NAMES[i]` text labels on each toggle
- [x] T022 [US5] Update retort click handling in `handle_click()` in `src/ui/WizardPanel.cpp` to use fixed 3-column grid layout and updated Y-offsets matching render()

**Checkpoint**: Retorts display in 3x6 grid with names. Section header visible. Click targets aligned.

---

## Phase 8: User Story 6 - Click Targets Match Visual Layout (Priority: P1)

**Goal**: All click regions align precisely with rendered elements after all text additions.

**Independent Test**: Click each element — only the intended element changes. No off-by-one or misaligned hit zones.

### Implementation for User Story 6

- [x] T023 [US6] Final audit of `render()` and `handle_click()` Y-tracking in `src/ui/WizardPanel.cpp` — verify both methods use identical Y-advancement logic for all labels, descriptions, selectors, and separators
- [x] T024 [US6] Build and manually test all click targets with `bash build_wasm_stage.sh`

**Checkpoint**: Clicking any element changes only that element. No misaligned hit zones.

---

## Phase 9: Polish & Cross-Cutting Concerns

- [x] T025 Run existing test suite with `cd build && ctest --output-on-failure` to verify no regressions
- [x] T026 Build desktop version with `cd build && cmake --build .` and verify Wizards panel renders correctly
- [x] T027 Run quickstart.md validation steps (all 8 acceptance checks)

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies
- **Foundational (Phase 2)**: Depends on Phase 1
- **User Story 1 (Phase 3)**: Depends on Phase 2
- **User Stories 2-5 (Phases 4-7)**: Depend on Phase 2 (can be done in any order but modify same file Y-offsets sequentially)
- **User Story 6 (Phase 8)**: Depends on Phases 3-7 (final click alignment audit after all text additions)
- **Polish (Phase 9)**: Depends on all phases complete

### User Story Dependencies

- **US1 (P1)**: Wizard tab labels — standalone
- **US2 (P1)**: Race labels — adds Y-offset changes
- **US3 (P2)**: Portrait labels — adds Y-offset changes
- **US4 (P1)**: Spell rank labels + realm order fix — adds Y-offset changes
- **US5 (P2)**: Retort labels + grid layout change — adds Y-offset changes
- **US6 (P1)**: Click alignment audit — must be done last

### Parallel Opportunities

- T003, T004, T005 can run in parallel (different arrays, same file top section)
- US2-US5 modify the same Y-offset tracking, so sequential execution is safer
- T025 and T026 (test + desktop build) can run in parallel

---

## Implementation Strategy

### MVP First (User Story 1 + 2 + 4 Only)

1. Complete Phase 1: Setup (T001-T002)
2. Complete Phase 2: Foundational (T003-T008)
3. Complete Phase 3: User Story 1 — wizard tab names (T009-T010)
4. Complete Phase 4: User Story 2 — race labels (T011-T013)
5. Complete Phase 6: User Story 4 — spell rank labels (T016-T019)
6. **STOP and VALIDATE**: P1 stories done, most impactful labels visible
7. Build and deploy — wizards panel is already usable

### Incremental Delivery

1. Setup + Foundational → Name arrays and constants ready
2. User Story 1 → Wizard tab names visible
3. User Stories 2 + 4 → Race and spell rank labels → **MVP!**
4. User Stories 3 + 5 → Portrait and retort labels
5. User Story 6 → Final click alignment audit
6. Polish → Run tests, verify both build targets

### Practical Note

Since all changes are in a single file (`src/ui/WizardPanel.cpp`), the most efficient approach is to implement all user stories in a single pass through the file, adding labels + values + descriptions together for each section. The phased breakdown above is for tracking and verification — the actual implementation naturally combines US1-US5 per section.

---

## Notes

- All tasks modify `src/ui/WizardPanel.cpp` — single file feature
- No new files, structs, or data layer changes
- Existing undo/redo behavior preserved (no click handler logic changes beyond Y-offsets and grid layout)
- Function comments must end with "Powered by Claude." per project convention
- Spell realm order changes from display-remapped to struct order (research decision 1)
- Retort grid changes from auto-calculated ~10 columns to fixed 3 columns (research decision 3)
