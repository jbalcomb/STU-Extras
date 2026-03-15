# Tasks: Save .GAM File Export

**Input**: Design documents from `/specs/007-save-gam-export/`
**Prerequisites**: plan.md (required), spec.md (required), research.md, data-model.md, quickstart.md

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which user story this task belongs to (e.g., US1, US2, US3)
- Include exact file paths in descriptions

---

## Phase 1: Setup

**Purpose**: Add the editor state flag needed for button-to-main-loop communication

- [x] T001 Add `export_gam_requested` flag to EditorState in `src/editor/EditorState.hpp`

---

## Phase 2: User Story 1 - Export via UI Button (Priority: P1) MVP

**Goal**: Add a visible "Export .GAM" button to the tool panel that triggers .GAM file export via file dialog (desktop) or browser download (WASM).

**Independent Test**: Click the "Export .GAM" button → file save dialog opens (desktop) or browser download triggers (WASM) → resulting file is exactly 123,300 bytes.

### Implementation for User Story 1

- [x] T002 [P] [US1] Add "Export .GAM" button rendering to `render()` in `src/ui/ToolPanel.cpp` — green-tinted button (60, 140, 80) below the "Gen Wizards" button, labeled "Export .GAM"
- [x] T003 [P] [US1] Add "Export .GAM" button click handler to `handle_click()` in `src/ui/ToolPanel.cpp` — sets `state.export_gam_requested = true`, matching layout math from T002
- [x] T004 [US1] Extract existing Ctrl+E export code (lines 675-692) into a `do_export_gam()` helper function in `src/main.cpp` — counts entities, serializes, calls `platform::save_file()`, sets status message
- [x] T005 [US1] Add `export_gam_requested` handler in main loop in `src/main.cpp` — reads flag, resets to false, calls `do_export_gam()`
- [x] T006 [US1] Update existing Ctrl+E handler to call `do_export_gam()` instead of inline code in `src/main.cpp`
- [x] T007 [US1] Build and manually test: click button triggers export, Ctrl+E still works, .GAM file is 123,300 bytes

**Checkpoint**: Export .GAM button works. Both button and Ctrl+E produce identical behavior.

---

## Phase 3: User Story 2 - Export Validation Feedback (Priority: P2)

**Goal**: Enhance the status bar message to show warnings when the scenario has potential issues (zero wizards, smoothing violations).

**Independent Test**: Export an empty scenario → status message includes "no active wizards" warning. Export with smoothing violations → status message notes violation count.

### Implementation for User Story 2

- [x] T008 [US2] Enhance `do_export_gam()` in `src/main.cpp` to append warning if zero active wizards: `" — no active wizards"`
- [x] T009 [US2] Enhance `do_export_gam()` in `src/main.cpp` to append smoothing violation count if `editor_state.violations` is non-empty: `" — N violations"`
- [x] T010 [US2] Build and manually test: export empty scenario shows wizard warning, export with violations shows count

**Checkpoint**: Status messages now include actionable warnings alongside the export confirmation.

---

## Phase 4: User Story 3 - Entity Count Summary (Priority: P3)

**Goal**: Include entity counts (wizards, cities, units) in the export status message.

**Independent Test**: Populate scenario with 3 wizards, 5 cities, 20 units → export → status message shows "(3 wizards, 5 cities, 20 units)".

### Implementation for User Story 3

- [x] T011 [US3] Enhance `do_export_gam()` in `src/main.cpp` to include entity counts in status message: `"Exported scenario.GAM (W wizards, C cities, U units)"`
- [x] T012 [US3] Build and manually test: verify entity counts in status message match scenario state for various configurations (empty, partial, full)

**Checkpoint**: Export status message now shows full entity summary with counts.

---

## Phase 5: Polish & Cross-Cutting Concerns

**Purpose**: Final validation across all scenarios

- [x] T013 Build full project and run all existing tests (`ctest --output-on-failure`)
- [x] T014 Run quickstart.md validation: all 4 integration scenarios
- [x] T015 Build WASM target and verify export triggers browser download

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies — T001 first
- **US1 (Phase 2)**: Depends on T001 (flag exists in EditorState)
- **US2 (Phase 3)**: Depends on T004 (do_export_gam helper exists)
- **US3 (Phase 4)**: Depends on T004 (do_export_gam helper exists)
- **Polish (Phase 5)**: Depends on all user stories complete

### User Story Dependencies

- **US1 (P1)**: Independent — can start after T001
- **US2 (P2)**: Depends on US1 T004 (shared helper function must exist)
- **US3 (P3)**: Depends on US1 T004 (shared helper function must exist)

### Within User Story 1

- T002 and T003 can run in parallel (render vs click handler, same file but different functions)
- T004 must complete before T005 and T006 (helper must exist before callers)
- T005 and T006 can run in parallel after T004 (different code paths)
- T007 must be last (manual verification)

### Parallel Opportunities

```
After T001:
  T002 ──┐
  T003 ──┤── (parallel, different functions in ToolPanel.cpp)
         │
After T004:
  T005 ──┐
  T006 ──┤── (parallel, button handler vs keyboard handler)
```

---

## Implementation Strategy

### MVP First (User Story 1 Only)

1. Complete T001 (Setup)
2. Complete T002-T007 (US1: Export button)
3. **STOP and VALIDATE**: Button works, Ctrl+E works, .GAM is valid
4. Ship it — users can now export via visible UI

### Incremental Delivery

1. T001 → Setup done
2. T002-T007 → US1 done → Export button works (MVP)
3. T008-T010 → US2 done → Warnings appear in status message
4. T011-T012 → US3 done → Entity counts in status message
5. T013-T015 → Polish → Full validation including WASM

---

## Notes

- This feature modifies only 3 existing files — no new source files needed
- The existing `serialize_gam()` and `platform::save_file()` are reused without modification
- No undo wrapping needed — export is read-only (entity count updates are transient header fixups)
- The `do_export_gam()` helper consolidates duplicate logic between button and Ctrl+E paths
