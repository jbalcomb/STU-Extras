# Implementation Plan: Save .GAM File Export

**Branch**: `007-save-gam-export` | **Date**: 2026-03-14 | **Spec**: [spec.md](spec.md)
**Input**: Feature specification from `/specs/007-save-gam-export/spec.md`

## Summary

Add a visible "Export .GAM" button to the tool panel so users can export the current scenario as a playable Master of Magic save file without relying on the hidden Ctrl+E keyboard shortcut. The export uses the existing `serialize_gam()` and `platform::save_file()` infrastructure. Additionally, the status bar message is enhanced to show entity count summary (wizards, cities, units) and warnings (e.g., zero wizards, smoothing violations) after export.

## Technical Context

**Language/Version**: C++17 (`CMAKE_CXX_STANDARD 17`)
**Primary Dependencies**: SDL2 2.32.2 (presentation), nlohmann/json 3.11.3 (serialization), GoogleTest 1.17.0 (testing)
**Storage**: Binary .GAM files (123,300 bytes), JSON .mom_scenario files
**Testing**: GoogleTest via CTest (`ctest --output-on-failure`)
**Target Platform**: Windows (primary), Emscripten/WASM (web distribution)
**Project Type**: Desktop application (SDL2 scenario editor)
**Performance Goals**: Instant export (serialize_gam is O(1) memcpy of fixed-size buffer)
**Constraints**: No changes to packed struct layouts; no SDL2 dependency in data/editor layers; existing Ctrl+E shortcut must continue to work
**Scale/Scope**: Single button, single handler, status message enhancement

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

| Principle | Status | Notes |
|-----------|--------|-------|
| I. Binary Fidelity | PASS | No changes to packed struct layouts or .GAM serialization. Uses existing `serialize_gam()` unchanged. |
| II. Layer Separation | PASS | Button in presentation layer (ToolPanel). Export logic already in main.cpp event loop. Entity counting uses data layer functions. No layer violations. |
| III. Test-Verified Compatibility | PASS | No data format changes. Existing .GAM and JSON round-trip tests remain valid. No new serialization code to test. |
| IV. WASM-Ready Data Layer | PASS | No data layer changes. WASM export path already works via `platform::download_blob()`. |
| V. Simplicity First | PASS | Adds one button + one status message. Reuses existing infrastructure entirely. No new abstractions. |

**Gate result**: ALL PASS — proceed to implementation.

## Project Structure

### Documentation (this feature)

```text
specs/007-save-gam-export/
├── spec.md              # Feature specification
├── plan.md              # This file
├── research.md          # Phase 0: existing export flow analysis
├── data-model.md        # Phase 1: entity definitions (minimal — no new entities)
├── quickstart.md        # Phase 1: integration scenarios
├── checklists/
│   └── requirements.md  # Spec quality checklist
└── tasks.md             # Phase 2 output (created by /speckit.tasks)
```

### Source Code (repository root)

```text
src/
├── ui/
│   └── ToolPanel.cpp        # EXTENDED: "Export .GAM" button render + click handler
├── editor/
│   └── EditorState.hpp      # EXTENDED: export_gam_requested flag
└── main.cpp                 # EXTENDED: export handler (refactored from Ctrl+E inline code)

tests/
└── (no new test files — existing round-trip tests cover .GAM serialization)
```

**Structure Decision**: Minimal changes to 3 existing files. No new source files needed. The existing `serialize_gam()` and `platform::save_file()` are reused without modification.

## Complexity Tracking

> No constitution violations — this section is intentionally empty.
