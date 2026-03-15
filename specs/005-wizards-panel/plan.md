# Implementation Plan: Wizards Panel Labels & Documentation

**Branch**: `005-wizards-panel` | **Date**: 2026-03-11 | **Spec**: [spec.md](spec.md)
**Input**: Feature specification from `/specs/005-wizards-panel/spec.md`

## Summary

Add text labels, current value names, and brief gameplay descriptions to all wizard configuration elements in the Wizards panel (`WizardPanel`). The panel currently renders unlabeled colored shapes for wizard tabs, race, portrait, spell ranks, and retorts. This feature uses the existing `Renderer::draw_text()` method (from 003-font-rendering) to overlay text on the existing selectors. No data layer changes required — purely presentation layer work in `WizardPanel.cpp`.

Additionally: fix PANEL_TOP from 280 to 332 (matching 004), remove spell realm display order remapping (display in struct order), and change retort layout from auto-calculated columns to a fixed 3-column grid.

## Technical Context

**Language/Version**: C++17 (`CMAKE_CXX_STANDARD 17`)
**Primary Dependencies**: SDL2 2.32.2 (presentation), SDL_ttf (font rendering via `Renderer::draw_text()`)
**Storage**: N/A (no data layer changes; Wizard struct unchanged)
**Testing**: GoogleTest 1.17.0, `ctest --output-on-failure`
**Target Platform**: Windows (primary), Emscripten/WASM (web distribution)
**Project Type**: Desktop application (SDL2 scenario editor)
**Performance Goals**: 60 fps (text rendering uses texture cache — no per-frame penalty)
**Constraints**: Panel width 200px, font size 12pt Cousine-Regular.ttf, panel starts at y=332
**Scale/Scope**: Single file change (`WizardPanel.cpp`), ~150 lines of modification

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

| Principle | Status | Notes |
|-----------|--------|-------|
| I. Binary Fidelity | PASS | No struct changes. Wizard layout unchanged. |
| II. Layer Separation | PASS | Changes only in presentation layer (`src/ui/WizardPanel.cpp`). No data/editor layer changes. |
| III. Test-Verified Compatibility | PASS | No data format changes, so no new round-trip tests needed. Existing tests must continue passing. |
| IV. WASM-Ready Data Layer | PASS | No data layer changes. |
| V. Simplicity First | PASS | Adding labels/descriptions to existing UI elements is the simplest approach. No new abstractions. Reuses proven pattern from 004-settings-panel. |

## Project Structure

### Documentation (this feature)

```text
specs/005-wizards-panel/
├── spec.md              # Feature specification
├── plan.md              # This file
├── research.md          # Phase 0 output
├── data-model.md        # Phase 1 output
├── quickstart.md        # Integration scenarios
├── checklists/
│   └── requirements.md  # Spec validation checklist
└── tasks.md             # Phase 2 output (created by /speckit.tasks)
```

### Source Code (files to modify)

```text
src/
└── ui/
    └── WizardPanel.cpp   # PRIMARY: Add labels, values, descriptions, fix layout
    └── WizardPanel.hpp   # UNCHANGED (no API changes)
```

**Structure Decision**: Single-file modification in existing presentation layer. No new files, directories, or abstractions needed.

## Key Implementation Details

### 1. Lookup Tables (top of file)

```
RACE_NAMES[14]: Barbarians through Trolls (same values as spec FR-3)
REALM_NAMES[5]: Nature, Sorcery, Chaos, Life, Death (struct order per clarification)
RETORT_NAMES[18]: Alchemy through Artificer (matching existing retort pointer order)
Section descriptions: 3 static const char* strings
```

### 2. Spell Realm Order Change

Current code uses `realm_to_idx[] = {3, 4, 2, 0, 1}` to display Life, Death, Chaos, Nature, Sorcery. Per clarification, remove this remapping and iterate `r = 0..4` directly (Nature, Sorcery, Chaos, Life, Death).

`REALM_COLORS` array must be reordered from [Life, Death, Chaos, Nature, Sorcery] to [Nature, Sorcery, Chaos, Life, Death].

### 3. Retort Grid Layout

Change from auto-calculated `cols = (btn_w) / (sq_sz + 2)` (~10 columns) to fixed `cols = 3`. Each cell in the 3-column grid needs a toggle indicator and the retort name rendered as text. At 200px panel width with 16px margins, each column gets ~56px — enough for abbreviated names but may need smaller text or truncation for "Sorcery Mastery" (15 chars).

### 4. PANEL_TOP Fix

Change from 280 to 332 to match 004-settings-panel fix.

## Complexity Tracking

No constitution violations. No complexity justifications needed.
