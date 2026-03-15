# Implementation Plan: Wizard Profile Generation

**Branch**: `006-wizard-profile-gen` | **Date**: 2026-03-13 | **Spec**: [spec.md](spec.md)
**Input**: Feature specification from `/specs/006-wizard-profile-gen/spec.md`

## Summary

Add wizard profile generation to the scenario editor, allowing users to populate wizard slots with preset data from Master of Magic's 14 original wizards. Includes global "Generate Wizards" button (ToolPanel), per-wizard "Generate" button (WizardPanel), dirty-flag tracking for confirmation prompts, modal overlay dialog for overwrite confirmation, and full undo support.

## Technical Context

**Language/Version**: C++17 (`CMAKE_CXX_STANDARD 17`)
**Primary Dependencies**: SDL2 2.32.2 (presentation), nlohmann/json 3.11.3 (serialization), GoogleTest 1.17.0 (testing)
**Storage**: Binary .GAM files (123,300 bytes), JSON .mom_scenario files
**Testing**: GoogleTest via CTest (`ctest --output-on-failure`)
**Target Platform**: Windows (primary), Emscripten/WASM (future)
**Project Type**: Desktop application (SDL2 scenario editor)
**Performance Goals**: Instant generation (< 1ms for 6 wizard structs)
**Constraints**: No changes to packed Wizard struct layout; no SDL2 dependency in data/editor layers
**Scale/Scope**: 6 wizard slots, 14 presets, fixed sizes

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

| Principle | Status | Notes |
|-----------|--------|-------|
| I. Binary Fidelity | PASS | No changes to packed struct layouts. Wizard struct remains 1224 bytes. Generation writes into existing fields only. |
| II. Layer Separation | PASS | Preset data table lives in data layer (no SDL2). Dirty flags in editor layer (EditorState). Modal dialog + buttons in presentation layer (WizardPanel, ToolPanel). |
| III. Test-Verified Compatibility | PASS | No data format changes. Existing .GAM and JSON round-trip tests remain valid. New tests verify preset data correctness. |
| IV. WASM-Ready Data Layer | PASS | Preset table is constexpr C++17 data, no platform dependencies. Generation logic uses only standard C++. |
| V. Simplicity First | PASS | Uses existing UI patterns (button clicks, dropdown overlay). No new abstractions or frameworks. |

**Gate result**: ALL PASS — proceed to implementation.

## Project Structure

### Documentation (this feature)

```text
specs/006-wizard-profile-gen/
├── spec.md              # Feature specification
├── plan.md              # This file
├── research.md          # Phase 0: spell book mapping, preset data, race assignment
├── data-model.md        # Phase 1: entity definitions and state transitions
├── quickstart.md        # Phase 1: integration scenarios
├── checklists/
│   └── requirements.md  # Spec quality checklist
└── tasks.md             # Phase 2 output (created by /speckit.tasks)
```

### Source Code (repository root)

```text
src/
├── data/
│   ├── MomEntities.hpp      # Wizard struct (UNCHANGED)
│   ├── MomConstants.hpp     # Race/retort constants (UNCHANGED)
│   └── WizardPresets.hpp    # NEW: constexpr preset data table
├── editor/
│   └── EditorState.hpp      # EXTENDED: wizard_dirty[], confirm dialog flags
├── ui/
│   ├── ToolPanel.cpp        # EXTENDED: "Generate Wizards" button
│   ├── WizardPanel.cpp      # EXTENDED: per-wizard "Generate" button, modal dialog, dirty tracking
│   └── WizardPanel.hpp      # EXTENDED: confirm dialog state
├── renderer/
│   └── UIRenderer.cpp       # EXTENDED: modal dialog rendering helpers (if needed)
└── main.cpp                 # EXTENDED: generation request handling in event loop

tests/
└── test_WizardPresets.cpp   # NEW: verify preset data matches original game
```

**Structure Decision**: Single project, extending existing files. One new header (WizardPresets.hpp) for preset data, one new test file. No new directories or build targets needed.

## Complexity Tracking

> No constitution violations — this section is intentionally empty.
