# Tasks: Wizard Profile Generation

**Input**: Design documents from `/specs/006-wizard-profile-gen/`
**Prerequisites**: plan.md (required), spec.md (required for user stories), research.md, data-model.md, quickstart.md

**Tests**: Not explicitly requested in the feature spec. Test tasks included only for preset data verification (critical for correctness).

**Organization**: Tasks are grouped by user story to enable independent implementation and testing of each story.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which user story this task belongs to (e.g., US1, US2, US3)
- Include exact file paths in descriptions

---

## Phase 1: Setup

**Purpose**: Create the preset data table and extend EditorState with generation support fields

- [ ] T001 [P] Create WizardPresets.hpp with constexpr preset data table (14 wizards: name, spell books in spellranks order [Nature,Sorcery,Chaos,Life,Death], retort index, default race) in src/data/WizardPresets.hpp. Use research.md field mapping: preset {life,sorcery,nature,death,chaos} → spellranks {[0]=Nature,[1]=Sorcery,[2]=Chaos,[3]=Life,[4]=Death}. Include apply_preset() function that writes preset data into a Wizard struct (name, spellranks, retort booleans, capital_race, wizard_id, banner_id). No SDL2 dependency.
- [ ] T002 [P] Add session-only dirty flag and generation dialog fields to EditorState in src/editor/EditorState.hpp: `bool wizard_dirty[NUM_PLAYERS]{}`, `bool wizard_gen_confirm_open{}`, `bool wizard_gen_confirm_global{}`, `int wizard_gen_confirm_idx{0}`, `bool generate_wizards_requested{false}`

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Verify preset data correctness before building UI on top of it

**⚠️ CRITICAL**: No user story work can begin until this phase is complete

- [ ] T003 Add test file tests/test_WizardPresets.cpp to CMakeLists.txt and write tests verifying: (1) all 6 used presets match original game data from research.md table, (2) apply_preset() correctly sets wizard name, spellranks, retort booleans (exactly 1 set, others cleared), capital_race, wizard_id, and banner_id, (3) spell book field mapping is correct (preset.life → spellranks[3], etc.)
- [ ] T004 Build and run tests to verify preset data is correct: `cmake --build build && cd build && ctest --output-on-failure`

**Checkpoint**: Preset data table verified against original game — UI implementation can begin

---

## Phase 3: User Story 1 - Generate All Wizard Profiles (Priority: P1) 🎯 MVP

**Goal**: User clicks "Generate Wizards" button to populate all 6 wizard slots with preset data, with confirmation prompt for dirty wizards

**Independent Test**: Click "Generate Wizards" and verify all 6 wizard slots are populated with correct preset data (name, spell books, retort, race)

### Implementation for User Story 1

- [ ] T005 [US1] Add "Generate Wizards" button to ToolPanel in src/ui/ToolPanel.cpp: render a button (same style as existing "Generate" map button) below the map generate section. On click, set `editor_state.generate_wizards_requested = true`. Add corresponding render code with button label "Gen Wizards".
- [ ] T006 [US1] Implement global wizard generation handler in src/main.cpp: when `editor_state.generate_wizards_requested` is true, check `editor_state.wizard_dirty[]` for any dirty wizards. If none dirty: apply all 6 presets via apply_preset(), wrap in LambdaCommand for undo (capture all 6 Wizard backups + dirty flag backups), clear dirty flags, set status "Generated 6 wizard profiles". If any dirty: set `wizard_gen_confirm_open = true`, `wizard_gen_confirm_global = true` instead of applying immediately.
- [ ] T007 [US1] Implement modal confirmation dialog rendering in src/ui/WizardPanel.cpp: when `editor_state.wizard_gen_confirm_open` is true, render a semi-transparent overlay across the panel area, then a centered dialog box using UIRenderer::draw_panel() with: title "Overwrite Modified Wizards?", list of dirty wizard names (from scenario.wizards[i].name where wizard_dirty[i] is true), "Overwrite" button, "Cancel" button. Render this LAST so it appears on top.
- [ ] T008 [US1] Implement modal dialog click handling in src/ui/WizardPanel.cpp: when `wizard_gen_confirm_open` is true, consume ALL clicks. If click on "Overwrite": apply presets (global or single based on wizard_gen_confirm_global), push undo command, clear dirty flags, close dialog, set status message. If click on "Cancel" or outside buttons: close dialog, no changes.
- [ ] T009 [US1] Build and manually test global generation: verify (1) all 6 wizards populated correctly, (2) undo restores previous state, (3) status bar shows confirmation message

**Checkpoint**: User Story 1 fully functional — users can batch-generate all wizards with undo support

---

## Phase 4: User Story 2 - Generate Single Wizard Profile (Priority: P2)

**Goal**: User clicks "Generate" on an individual wizard tab to regenerate just that wizard's preset

**Independent Test**: Open a specific wizard tab, click "Generate", verify only that wizard's data changes while others remain untouched

### Implementation for User Story 2

- [ ] T010 [US2] Add per-wizard "Generate" button to WizardPanel in src/ui/WizardPanel.cpp: render a small "Gen" button in the wizard tab header area (next to the wizard name or at the top of the tab content). On click, check wizard_dirty[wiz_idx]. If not dirty: apply preset immediately via apply_preset(), push undo command, clear dirty flag, set status. If dirty: set `wizard_gen_confirm_open = true`, `wizard_gen_confirm_global = false`, `wizard_gen_confirm_idx = wiz_idx`.
- [ ] T011 [US2] Update modal dialog handler (T008) in src/ui/WizardPanel.cpp to handle single-wizard case: when `wizard_gen_confirm_global == false`, the "Overwrite" action applies preset only to `wizard_gen_confirm_idx`, undo command captures only that one wizard's backup + dirty flag.
- [ ] T012 [US2] Build and manually test per-wizard generation: verify (1) only target wizard changes, (2) other wizards unchanged, (3) undo works, (4) confirmation only appears for dirty wizards

**Checkpoint**: Users can generate individual wizards independently

---

## Phase 5: User Story 3 - Edit After Generation (Priority: P3)

**Goal**: Dirty flag tracking marks wizards as manually edited when users modify any field after generation

**Independent Test**: Generate a wizard, modify any field, verify the wizard is marked dirty and subsequent generation shows confirmation

### Implementation for User Story 3

- [ ] T013 [US3] Add dirty flag tracking to all wizard edit operations in src/ui/WizardPanel.cpp: in the click handler, wherever a wizard field is modified (name edit, spell book increment/decrement, retort toggle, race change), set `editor_state.wizard_dirty[wiz_idx] = true`. Also update the undo command for each edit to capture and restore the dirty flag state.
- [ ] T014 [US3] Add dirty flag tracking to fortress-wizard auto-stub in src/main.cpp: when PLACE_FORTRESS creates a wizard stub, ensure `wizard_dirty[idx]` remains false (auto-stubs are not manually edited). The undo command for fortress placement must also capture and restore the dirty flag.
- [ ] T015 [US3] Build and manually test dirty flag flow: verify (1) editing a generated wizard marks it dirty, (2) generating again shows confirmation for that wizard, (3) undoing an edit restores the dirty flag, (4) auto-stubs from fortress placement are not dirty

**Checkpoint**: All user stories independently functional — dirty flag tracking complete

---

## Phase 6: Spell Picks Budget Enforcement

**Goal**: Enforce the original game's 11-pick budget: spell books + retort costs must not exceed 11 picks. Display remaining picks and prevent over-allocation.

**Independent Test**: Set a wizard to 10 spell books across realms → only 1 pick remains → can only toggle a 1-cost retort. Toggle Myrran (3 picks) → verify it's blocked. Remove some books → Myrran becomes available.

### Implementation for Spell Picks Budget

- [x] T019 [P] Add retort cost lookup and picks calculation helper to `src/mom_data/WizardPresets.hpp`: `retort_cost(int retort_idx)` returns 1, 2, or 3 based on retort index (2-pick: Warlord=1, Channeler=8, Divine Power=6, Famous=13, Infernal Power=5; 3-pick: Myrran=9; all others 1-pick). Add `calc_picks_used(const Wizard& wiz)` that sums all spellranks + retort costs and `calc_picks_remaining(const Wizard& wiz)` returning `11 - calc_picks_used(wiz)`.
- [x] T020 [P] Add picks budget tests to `tests/test_WizardPresets.cpp`: verify retort_cost() returns correct values for all 18 retorts, verify calc_picks_used() for preset wizards matches expected totals, verify calc_picks_remaining() = 11 - used.
- [x] T021 Build and run tests to verify picks calculation: `cmake --build build && cd build && ctest --output-on-failure`
- [x] T022 Display remaining picks counter in WizardPanel render in `src/ui/WizardPanel.cpp`: show "Picks: N/11" below the spell rank bars (or above retorts), color green if >0, red if 0, yellow if negative (invalid state).
- [x] T023 Enforce picks limit on spell book increment in `src/ui/WizardPanel.cpp` `handle_click()`: before incrementing a spellrank, check `calc_picks_remaining(wiz) > 0`. If no picks remain, skip the increment (set status "No picks remaining").
- [x] T024 Enforce picks limit on retort toggle in `src/ui/WizardPanel.cpp` `handle_click()`: before enabling a retort, check `calc_picks_remaining(wiz) >= retort_cost(retort_idx)`. If insufficient picks, skip the toggle (set status "Not enough picks for this retort"). Disabling a retort is always allowed.
- [x] T025 Enforce Runemaster prerequisite in `src/ui/WizardPanel.cpp` `handle_click()`: Runemaster (index 14) requires 2+ books in 3+ realms. Before enabling Runemaster, count realms with spellranks >= 2. If fewer than 3, skip toggle (set status "Runemaster requires 2+ books in 3+ realms").
- [x] T026 Build and manually test picks enforcement: verify (1) picks counter displays correctly, (2) spell book clicks blocked at budget limit, (3) retort toggles blocked when cost exceeds remaining, (4) Myrran blocked when <3 remaining, (5) Runemaster blocked without 3-realm prerequisite, (6) disabling retorts always works and frees picks.

**Checkpoint**: Wizard customization enforces original game pick rules

---

## Phase 7: Polish & Cross-Cutting Concerns

**Purpose**: Final verification and cleanup

- [x] T027 Build full project and run all tests: `cmake --build build && cd build && ctest --output-on-failure`
- [ ] T028 Run quickstart.md validation scenarios 1-4 manually to verify end-to-end flows
- [x] T029 Build WASM target to verify no platform-specific code introduced: `bash build_wasm.sh`

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies — T001 and T002 can run in parallel
- **Foundational (Phase 2)**: Depends on T001 (preset data needed for tests)
- **User Story 1 (Phase 3)**: Depends on Phase 2 completion
- **User Story 2 (Phase 4)**: Depends on Phase 3 (reuses modal dialog from T007/T008)
- **User Story 3 (Phase 5)**: Can start after Phase 2, but best after Phase 3 (dirty flags need generation to be meaningful)
- **Polish (Phase 6)**: Depends on all user stories being complete

### User Story Dependencies

- **User Story 1 (P1)**: Standalone after foundational phase — delivers global generation + modal dialog
- **User Story 2 (P2)**: Depends on US1 modal dialog (T007/T008) for confirmation flow
- **User Story 3 (P3)**: Depends on US1/US2 generation functions being in place to test dirty flag integration

### Within Each User Story

- Core implementation before UI integration
- UI rendering before click handling
- Build verification after each story

### Parallel Opportunities

- **Phase 1**: T001 and T002 can run in parallel (different files)
- **Phase 3**: T005 (ToolPanel) and T007 (modal rendering) can potentially overlap
- **Phase 4**: T010 and T011 modify the same file (WizardPanel.cpp) — must be sequential

---

## Parallel Example: Phase 1 Setup

```bash
# Launch both setup tasks in parallel (different files):
Task T001: "Create WizardPresets.hpp in src/data/WizardPresets.hpp"
Task T002: "Add dirty flag fields to EditorState in src/editor/EditorState.hpp"
```

---

## Implementation Strategy

### MVP First (User Story 1 Only)

1. Complete Phase 1: Setup (T001, T002)
2. Complete Phase 2: Foundational (T003, T004)
3. Complete Phase 3: User Story 1 (T005–T009)
4. **STOP and VALIDATE**: Test global wizard generation independently
5. This delivers the primary workflow — batch-generating all wizards

### Incremental Delivery

1. Complete Setup + Foundational → Preset data verified
2. Add User Story 1 → Test global generation → Validate (MVP!)
3. Add User Story 2 → Test per-wizard generation → Validate
4. Add User Story 3 → Test dirty flag tracking → Validate
5. Polish → Full test suite + WASM build verification

---

## Notes

- [P] tasks = different files, no dependencies
- [Story] label maps task to specific user story for traceability
- Each user story should be independently completable and testable
- Commit after each task or logical group
- Stop at any checkpoint to validate story independently
- All generation operations must be wrapped in LambdaCommand for undo support
- Dirty flag state must be captured in undo commands alongside wizard data
