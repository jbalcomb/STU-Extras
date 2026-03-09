# Implementation Plan: Master of Magic Scenario Editor

**Branch**: `001-mom-scenario-editor` | **Date**: 2026-03-08 | **Spec**: [spec.md](spec.md)
**Input**: Feature specification from `/specs/001-mom-scenario-editor/spec.md`

## Summary

Build a browser-hosted scenario editor for Master of Magic (1994) that generates world maps, provides visual tile editing with smoothing rule validation, configures game options/wizards/entities, and exports binary-compatible .GAM save files. The existing C++17/SDL2 desktop application (~85% MVP complete) will be extended with map generation, property editing, terrain validation, and compiled to WebAssembly via Emscripten for itch.io distribution.

## Technical Context

**Language/Version**: C++17 (`CMAKE_CXX_STANDARD 17`)
**Primary Dependencies**: SDL2 2.32.2 (presentation), nlohmann/json 3.11.3 (serialization), GoogleTest 1.17.0 (testing), Emscripten 3.1.40+ (WASM compilation)
**Storage**: File-based — .GAM (123,300-byte binary, game engine format), .mom_scenario (JSON, editor native format). No server-side storage; browser uses download/upload via file dialogs.
**Testing**: GoogleTest via CTest. 37 existing tests covering struct sizes, .GAM round-trip, JSON round-trip, undo/redo.
**Target Platform**: Desktop (Windows, MSVC) and Web (WASM via Emscripten, hosted on itch.io)
**Project Type**: Desktop/Web application (scenario editor)
**Performance Goals**: 30 FPS minimum during map editing, <10s initial load on broadband, <1s smoothing validation feedback
**Constraints**: .GAM files exactly 123,300 bytes, `#pragma pack(push, 2)` struct alignment, client-side only (no backend), horizontal map wrapping (cylindrical world)
**Scale/Scope**: Single-user editor, 60×40×2 tile grid (4,800 tiles total), fixed entity slots (100 cities, 1009 units, 30 nodes, 102 lairs, 6 wizards, 138 items)

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

### Pre-Phase 0 Gate

| Principle | Status | Evidence |
|-----------|--------|----------|
| I. Binary Fidelity | ✅ PASS | Data layer uses `#pragma pack(push, 2)`, `static_assert` on all struct sizes, .GAM round-trip tested. No struct layout changes proposed. |
| II. Layer Separation | ✅ PASS | MomData lib has zero SDL2 dependency. New features (mapgen, validation) will be in Data layer. Emscripten touches only Presentation layer build config. |
| III. Test-Verified Compatibility | ✅ PASS | 37 tests pass. New features will add tests for map generation output and smoothing validation logic. |
| IV. WASM-Ready Data Layer | ✅ PASS | MomData uses only C++17 stdlib + nlohmann/json. All I/O via std::istream/ostream. No platform-specific APIs. |
| V. Simplicity First | ✅ PASS | Plan builds incrementally on existing working code. Map generation uses standard noise algorithms. No speculative abstractions. |

**Gate result**: ALL PASS — proceed to Phase 0.

## Project Structure

### Documentation (this feature)

```text
specs/001-mom-scenario-editor/
├── plan.md              # This file
├── research.md          # Phase 0 output
├── data-model.md        # Phase 1 output
├── quickstart.md        # Phase 1 output
├── contracts/           # Phase 1 output
│   └── file-formats.md  # .GAM and .mom_scenario format contracts
└── tasks.md             # Phase 2 output (/speckit.tasks command)
```

### Source Code (repository root)

```text
src/
├── mom_data/              # Data layer — MomData static lib (no SDL2)
│   ├── MomConstants.hpp   # Enums, dimensions, terrain types
│   ├── MomWorld.hpp/.cpp  # 60×40×2 tile grid with accessors
│   ├── MomEntities.hpp/.cpp  # Packed entity structs (City, Unit, Wizard, etc.)
│   └── MomGamFile.hpp/.cpp   # Binary .GAM read/write (123,300 bytes)
├── scenario/              # Data layer — part of MomData lib
│   ├── Scenario.hpp/.cpp     # Top-level container (world + entities + options)
│   └── ScenarioIO.hpp/.cpp   # JSON .mom_scenario serialization
├── mapgen/                # NEW — Data layer (part of MomData lib, no SDL2)
│   ├── MapGenerator.hpp/.cpp    # Procedural terrain generation
│   └── TerrainSmoother.hpp/.cpp # Autotile transition computation
├── validation/            # NEW — Data layer (part of MomData lib, no SDL2)
│   └── SmoothingValidator.hpp/.cpp  # Adjacency rule checking
├── editor/                # Editor layer (no SDL2)
│   ├── EditorState.hpp/.cpp  # Tool selection, cursor, editing defaults
│   └── UndoStack.hpp/.cpp    # Command pattern undo/redo (50-level depth)
├── renderer/              # Presentation layer (SDL2)
│   ├── Renderer.hpp/.cpp     # SDL2 window, camera, draw primitives
│   ├── MapRenderer.hpp/.cpp  # Tile-by-tile terrain + entity rendering
│   └── UIRenderer.hpp/.cpp   # Status bar, text rendering
├── ui/                    # Presentation layer (SDL2)
│   ├── ToolPanel.hpp/.cpp       # Tool buttons, terrain/special swatches
│   ├── PropertiesPanel.hpp/.cpp # Entity properties (currently read-only → read-write)
│   └── Minimap.hpp/.cpp         # World overview with viewport indicator
├── platform/              # NEW — Platform abstraction
│   └── FileDialog.hpp/.cpp      # Native file dialogs (desktop) vs download/upload (WASM)
└── main.cpp               # Event loop, input handling, tool application

tests/
├── test_MomGamFile.cpp    # Struct sizes, .GAM round-trip
├── test_Scenario.cpp      # JSON round-trip, entity state
├── test_UndoStack.cpp     # Undo/redo correctness
├── test_MapGenerator.cpp  # NEW — Map generation output validation
└── test_SmoothingValidator.cpp  # NEW — Adjacency rule correctness
```

**Structure Decision**: Extends existing three-layer architecture. New modules (`mapgen/`, `validation/`, `platform/`) placed in Data layer (no SDL2) to maintain WASM compatibility. Platform abstraction isolates desktop vs. browser file I/O differences.

### Post-Phase 1 Gate (Design Review)

| Principle | Status | Evidence |
|-----------|--------|----------|
| I. Binary Fidelity | ✅ PASS | data-model.md documents all existing packed structs as "DO NOT MODIFY". New entities (MapGenParams, SmoothingViolation) are editor-only — not written to .GAM. No struct layout changes in the plan. |
| II. Layer Separation | ✅ PASS | New modules placed correctly: `mapgen/` and `validation/` in Data layer (no SDL2), `platform/` abstracts I/O. PropertiesPanel editing stays in Presentation. No upward dependencies introduced. |
| III. Test-Verified Compatibility | ✅ PASS | Plan adds `test_MapGenerator.cpp` and `test_SmoothingValidator.cpp`. Existing 37 tests unchanged. data-model.md includes validation rules table (V-001 through V-014). |
| IV. WASM-Ready Data Layer | ✅ PASS | All new Data layer modules use only C++17 stdlib. Platform abstraction (`FileDialog.hpp`) uses compile-time branching (`#ifdef __EMSCRIPTEN__`). No platform-specific code in MomData. |
| V. Simplicity First | ✅ PASS | Map generation uses 3 user parameters (land proportion, roughness, continents). Smoothing validation checks 5 adjacency rules (not full 762-tile autotile). Property editing uses simple click-to-cycle, not full text input widgets. |

**Gate result**: ALL PASS — design is constitution-compliant. Proceed to `/speckit.tasks` for task generation.

## Complexity Tracking

No constitution violations detected. No complexity justification needed.
