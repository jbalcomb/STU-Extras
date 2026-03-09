# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

RemomMapgenItchio is a Master of Magic (1994) scenario editor — an SDL2 desktop application that creates custom .GAM save files compatible with the ReMoM game engine. Written in C++17.

## Build & Test Commands

```bash
# Build (from repo root)
cd build && cmake --build .

# Run tests
cd build && ctest --output-on-failure

# Build + test together (Windows)
cmd /C build_helper.bat
```

Build requires CMake 4.2+, MSVC (VS 2022), Ninja, and SDL2 at `C:/devellib/SDL2-2.32.2/`. nlohmann/json and GoogleTest are fetched automatically via CMake FetchContent.

## Architecture

Three-layer architecture with strict dependency direction (no upward references):

```
Presentation (SDL2)     → Renderer, MapRenderer, UIRenderer, ToolPanel, PropertiesPanel, Minimap
Editor (no SDL2)        → EditorState, UndoStack (command pattern, 50-command depth)
Data (pure C++)         → MomConstants, MomEntities, MomWorld, MomGamFile, Scenario, ScenarioIO
```

**Build targets**: `MomData` (static lib, data+editor layers), `RemomMapgenItchio` (SDL2 executable), `RemomTests` (GoogleTest).

The `MomData` library has no SDL2 dependency — all data/editor logic is testable without graphics.

## Critical: Binary Compatibility

All entity structs in `MomEntities.hpp` use `#pragma pack(push, 2)` for 2-byte alignment matching the original 8086 executable. Struct sizes are verified by `static_assert` and tested in `test_MomGamFile.cpp`.

- `.GAM` files must be exactly 123,300 bytes
- Struct layouts must match ReMoM's C definitions byte-for-byte
- Reference structs live in ReMoM at `src/MOX/MOM_Data.H` and `src/MOX/LOADSAVE.C`

**Never change entity struct layouts without verifying against the ReMoM source.**

## File Formats

- `.mom_scenario` (JSON): editor's native format with metadata, base64-encoded terrain, and entity arrays
- `.GAM` (binary, 123,300 bytes fixed): game engine save format, loadable in Master of Magic / ReMoM

## World Model

60 tiles wide x 40 tiles high x 2 planes (Arcanus=0, Myrror=1). Fixed entity slot counts: 100 cities, 1009 units, 6 wizards, 30 nodes, 102 lairs, 6 towers, 6 fortresses, 138 items.

## Conventions

- **Naming**: PascalCase classes, snake_case functions/members, UPPER_CASE enums/constants
- **Error handling**: return codes, no exceptions; out-of-bounds reads return 0, writes are silently ignored
- **Indentation**: 4 spaces (no tabs)
- **Function comments**: end with "Powered by Claude."

## Test Structure

Three test files (~37 tests) covering struct sizes, .GAM round-trip, JSON serialization round-trip, entity state checks, and undo/redo correctness.
