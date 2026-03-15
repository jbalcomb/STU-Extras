# Implementation Plan: Settings Panel Labels & Documentation

**Branch**: `004-settings-panel` | **Date**: 2026-03-11 | **Spec**: [spec.md](spec.md)
**Input**: Feature specification from `/specs/004-settings-panel/spec.md`

## Summary

Add text labels, current value names, and brief gameplay descriptions to each of the 4 game options in the Settings panel (`GameOptionsPanel`). The panel currently renders unlabeled colored shapes. This feature uses the existing `Renderer::draw_text()` method (from 003-font-rendering) to overlay text on the existing selectors. No data layer changes required — purely presentation layer work in `GameOptionsPanel.cpp`.

## Technical Context

**Language/Version**: C++17 (`CMAKE_CXX_STANDARD 17`)
**Primary Dependencies**: SDL2 2.32.2 (presentation), SDL_ttf (font rendering via `Renderer::draw_text()`)
**Storage**: N/A (no data layer changes; GameData struct unchanged)
**Testing**: GoogleTest 1.17.0, `ctest --output-on-failure`
**Target Platform**: Windows (primary), Emscripten/WASM (web distribution)
**Project Type**: Desktop application (SDL2 scenario editor)
**Performance Goals**: 60 fps (text rendering uses texture cache — no per-frame penalty)
**Constraints**: Panel width 200px, font size 12pt Cousine-Regular.ttf, panel starts at y=280
**Scale/Scope**: Single file change (`GameOptionsPanel.cpp`), ~100 lines of modification

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

| Principle | Status | Notes |
|-----------|--------|-------|
| I. Binary Fidelity | PASS | No struct changes. GameData layout unchanged. |
| II. Layer Separation | PASS | Changes only in presentation layer (`src/ui/GameOptionsPanel.cpp`). No data/editor layer changes. |
| III. Test-Verified Compatibility | PASS | No data format changes, so no new round-trip tests needed. Existing tests must continue passing. |
| IV. WASM-Ready Data Layer | PASS | No data layer changes. |
| V. Simplicity First | PASS | Adding labels/descriptions to existing UI elements is the simplest approach. No new abstractions. |

## Project Structure

### Documentation (this feature)

```text
specs/004-settings-panel/
├── spec.md              # Feature specification
├── plan.md              # This file
├── research.md          # Phase 0 output
├── data-model.md        # Phase 1 output
├── checklists/
│   └── requirements.md  # Spec validation checklist
└── tasks.md             # Phase 2 output (created by /speckit.tasks)
```

### Source Code (files to modify)

```text
src/
└── ui/
    └── GameOptionsPanel.cpp   # PRIMARY: Add labels, values, descriptions
    └── GameOptionsPanel.hpp   # UNCHANGED (no API changes)
```

**Structure Decision**: Single-file modification in existing presentation layer. No new files, directories, or abstractions needed.

## Complexity Tracking

No constitution violations. No complexity justifications needed.
