# Tasks: Settings Panel Labels & Documentation

**Input**: Design documents from `/specs/004-settings-panel/`
**Prerequisites**: plan.md (required), spec.md (required for user stories), research.md, data-model.md

**Tests**: No new tests required. Existing tests must continue passing.

**Organization**: Tasks are grouped by user story to enable independent implementation and testing of each story.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which user story this task belongs to (e.g., US1, US2, US3)
- Include exact file paths in descriptions

---

## Phase 1: Setup

**Purpose**: No project initialization needed — modifying existing file only.

- [x] T001 Verify `Renderer::draw_text()` works by building project with `bash build_wasm_stage.sh`

**Checkpoint**: Build succeeds, existing tests pass.

---

## Phase 2: Foundational (Value Name Lookup Tables)

**Purpose**: Define the static name arrays and description strings that all user stories depend on.

- [x] T002 Add static `const char*` arrays for setting value names at the top of `src/ui/GameOptionsPanel.cpp`: `DIFFICULTY_NAMES[5]` = {"Intro", "Easy", "Normal", "Hard", "Impossible"}, `MAGIC_NAMES[4]` = {"Weak", "Normal", "Powerful", "Overwhelming"}, `LAND_SIZE_NAMES[3]` = {"Small", "Medium", "Large"}
- [x] T003 Add static `const char*` strings for setting descriptions in `src/ui/GameOptionsPanel.cpp`: Difficulty = "AI aggressiveness and bonuses", Magic = "Magical node power and spell frequency", Land Size = "Land-to-ocean ratio on the map", Wizards = "Total players (1 human + AI opponents)"
- [x] T004 Add layout constants for text line heights in `src/ui/GameOptionsPanel.cpp`: `LABEL_H` (label line height ~14px), `DESC_H` (description line height ~12px), spacing between label/value/description/selector

**Checkpoint**: New constants compile. No behavior change yet.

---

## Phase 3: User Story 1 - Each Setting Has a Label and Current Value (Priority: P1) MVP

**Goal**: Each of the 4 settings displays a text label and the currently selected value name.

**Independent Test**: Open Settings panel, verify all 4 settings show label text and current value name. Click each setting and verify value name updates.

### Implementation for User Story 1

- [x] T005 [US1] Update `render()` in `src/ui/GameOptionsPanel.cpp` to draw "Difficulty" label and current value name (e.g., "Normal") above the difficulty selector blocks using `renderer.draw_text()`
- [x] T006 [US1] Update `render()` in `src/ui/GameOptionsPanel.cpp` to draw "Magic" label and current value name (e.g., "Powerful") above the magic bar
- [x] T007 [US1] Update `render()` in `src/ui/GameOptionsPanel.cpp` to draw "Land Size" label and current value name (e.g., "Medium") above the land size blocks
- [x] T008 [US1] Update `render()` in `src/ui/GameOptionsPanel.cpp` to draw "Wizards" label and current count text (e.g., "4 Wizards", singular "1 Wizard") above the wizard dots
- [x] T009 [US1] Update `handle_click()` Y-offsets in `src/ui/GameOptionsPanel.cpp` to match the new vertical layout from `render()` — each setting section is now taller due to added label/value text lines
- [x] T010 [US1] Build and verify with `bash build_wasm_stage.sh` — labels and values visible, clicks target correct settings

**Checkpoint**: All 4 settings show labels and current values. Clicking each setting changes only that setting. Undo/redo still works.

---

## Phase 4: User Story 2 - Difficulty Setting Is Documented (Priority: P2)

**Goal**: Difficulty section shows description text explaining what the setting affects.

**Independent Test**: Open Settings panel, verify Difficulty section shows "AI aggressiveness and bonuses" description text.

### Implementation for User Story 2

- [x] T011 [US2] Add description text "AI aggressiveness and bonuses" below the Difficulty label/value in `render()` in `src/ui/GameOptionsPanel.cpp`, rendered in a dimmer color (e.g., 160,160,160)
- [x] T012 [US2] Adjust Y-offsets in both `render()` and `handle_click()` in `src/ui/GameOptionsPanel.cpp` to account for the added description line height

**Checkpoint**: Difficulty section shows label + value + description. Click targets still aligned.

---

## Phase 5: User Story 3 - Magic Level Setting Is Documented (Priority: P2)

**Goal**: Magic section shows description text.

**Independent Test**: Open Settings panel, verify Magic section shows "Magical node power and spell frequency".

### Implementation for User Story 3

- [x] T013 [US3] Add description text "Magical node power and spell frequency" below the Magic label/value in `render()` in `src/ui/GameOptionsPanel.cpp`
- [x] T014 [US3] Adjust Y-offsets in both `render()` and `handle_click()` in `src/ui/GameOptionsPanel.cpp` for added description line

**Checkpoint**: Magic section shows label + value + description.

---

## Phase 6: User Story 4 - Land Size Setting Is Documented (Priority: P2)

**Goal**: Land Size section shows description text.

**Independent Test**: Open Settings panel, verify Land Size section shows "Land-to-ocean ratio on the map".

### Implementation for User Story 4

- [x] T015 [US4] Add description text "Land-to-ocean ratio on the map" below the Land Size label/value in `render()` in `src/ui/GameOptionsPanel.cpp`
- [x] T016 [US4] Adjust Y-offsets in both `render()` and `handle_click()` in `src/ui/GameOptionsPanel.cpp` for added description line

**Checkpoint**: Land Size section shows label + value + description.

---

## Phase 7: User Story 5 - Wizard Count Setting Is Documented (Priority: P2)

**Goal**: Wizards section shows description text.

**Independent Test**: Open Settings panel, verify Wizards section shows "Total players (1 human + AI opponents)".

### Implementation for User Story 5

- [x] T017 [US5] Add description text "Total players (1 human + AI opponents)" below the Wizards label/value in `render()` in `src/ui/GameOptionsPanel.cpp`
- [x] T018 [US5] Adjust Y-offsets in both `render()` and `handle_click()` in `src/ui/GameOptionsPanel.cpp` for added description line

**Checkpoint**: Wizards section shows label + value + description.

---

## Phase 8: User Story 6 - Click Targets Match Visual Layout (Priority: P1)

**Goal**: All click regions align precisely with rendered labels and selectors after all text additions.

**Independent Test**: Click each of the 4 settings — only the intended setting changes. No off-by-one or misaligned hit zones.

### Implementation for User Story 6

- [x] T019 [US6] Final audit of `render()` and `handle_click()` Y-tracking in `src/ui/GameOptionsPanel.cpp` — verify both methods use identical Y-advancement logic for all labels, descriptions, selectors, and separators
- [x] T020 [US6] Build and manually test all 4 click targets with `bash build_wasm_stage.sh`

**Checkpoint**: Clicking any setting changes only that setting. No misaligned hit zones.

---

## Phase 9: Polish & Cross-Cutting Concerns

- [x] T021 Run existing test suite with `cd build && ctest --output-on-failure` to verify no regressions (67/67 passed)
- [ ] T022 Build desktop version with `cd build && cmake --build .` and verify Settings panel renders correctly (BLOCKED: pre-existing MSVC cstdint include path issue unrelated to this feature)
- [ ] T023 Run quickstart.md validation steps (all 5 acceptance checks) (requires manual testing in browser)

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies
- **Foundational (Phase 2)**: Depends on Phase 1
- **User Story 1 (Phase 3)**: Depends on Phase 2 — **MVP delivery point**
- **User Stories 2-5 (Phases 4-7)**: Depend on Phase 3 (US1 establishes the label/value layout pattern; descriptions extend it)
- **User Story 6 (Phase 8)**: Depends on Phases 4-7 (final click alignment audit after all text additions)
- **Polish (Phase 9)**: Depends on all phases complete

### User Story Dependencies

- **US1 (P1)**: Labels + values — standalone, establishes layout pattern
- **US2-US5 (P2)**: Descriptions — each adds one description line. Can be done in any order but all modify the same file's Y-offsets sequentially
- **US6 (P1)**: Click alignment audit — must be done last after all layout changes

### Parallel Opportunities

- T002 and T003 can run in parallel (different arrays, same file top section)
- US2-US5 description tasks could theoretically run in parallel but all modify the same Y-offset tracking in `GameOptionsPanel.cpp`, so sequential execution is safer
- T021 and T022 (test + desktop build) can run in parallel

---

## Parallel Example: User Story 1

```bash
# All label tasks modify different sections of render(), but share Y-tracking.
# Execute sequentially within the story for correctness:
T005 → T006 → T007 → T008 → T009 → T010
```

---

## Implementation Strategy

### MVP First (User Story 1 Only)

1. Complete Phase 1: Setup (T001)
2. Complete Phase 2: Foundational (T002-T004)
3. Complete Phase 3: User Story 1 (T005-T010)
4. **STOP and VALIDATE**: All 4 settings show labels and values, clicks work correctly
5. Build and deploy — settings panel is already usable

### Incremental Delivery

1. Setup + Foundational → Name arrays and constants ready
2. User Story 1 → Labels + values visible → **MVP!**
3. User Stories 2-5 → Add descriptions one at a time
4. User Story 6 → Final click alignment audit
5. Polish → Run tests, verify both build targets

### Practical Note

Since all changes are in a single file (`src/ui/GameOptionsPanel.cpp`), the most efficient approach is to implement all user stories in a single pass through the file, adding labels + values + descriptions together for each setting section. The phased breakdown above is for tracking and verification — the actual implementation naturally combines US1-US5 per setting.

---

## Notes

- All tasks modify `src/ui/GameOptionsPanel.cpp` — single file feature
- No new files, structs, or data layer changes
- Existing undo/redo behavior preserved (no click handler logic changes, only Y-offsets)
- Function comments must end with "Powered by Claude." per project convention
