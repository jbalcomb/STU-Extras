# Quickstart: Master of Magic Scenario Editor

**Branch**: `001-mom-scenario-editor` | **Date**: 2026-03-08

## Prerequisites

- CMake 4.2+
- MSVC (Visual Studio 2022) or compatible C++17 compiler
- Ninja build system
- SDL2 2.32.2 at `C:/devellib/SDL2-2.32.2/`
- (Optional) Emscripten 3.1.40+ for WASM build

## Build (Desktop — Windows)

```bash
# Configure
cmake -B build -G Ninja

# Build all targets
cmake --build build

# Run tests
cd build && ctest --output-on-failure

# Or use the helper script
cmd /C build_helper.bat
```

**Build targets**:
- `MomData` — static library (data + editor layers, no SDL2)
- `RemomMapgenItchio` — SDL2 desktop executable
- `RemomTests` — GoogleTest test executable

## Build (WASM — Emscripten)

```bash
# Activate Emscripten environment
source ~/emsdk/emsdk_env.sh  # Linux/Mac
# or: C:\Users\jbalcomb\AppData\Local\emsdk\emsdk_env.bat  # Windows

# Configure with Emscripten toolchain
emcmake cmake -B build_wasm -G Ninja

# Build
cmake --build build_wasm

# Output: build_wasm/RemomMapgenItchio.html, .js, .wasm
```

## Run

```bash
# Desktop
./build/RemomMapgenItchio

# WASM (local server for testing)
cd build_wasm && python -m http.server 8080
# Open http://localhost:8080/RemomMapgenItchio.html
```

## Project Layout

```
src/
├── mom_data/        # Data layer (MomData lib) — no SDL2 dependency
├── scenario/        # Scenario container + JSON serialization
├── mapgen/          # Procedural map generation (no SDL2)
├── validation/      # Terrain smoothing rule validation (no SDL2)
├── editor/          # Editor state + undo/redo (no SDL2)
├── renderer/        # SDL2 rendering (map, UI, status bar)
├── ui/              # SDL2 UI panels (tools, properties, minimap)
├── platform/        # Platform abstraction (file dialogs)
└── main.cpp         # Entry point + event loop

tests/               # GoogleTest test files
specs/               # Feature specifications and plans
```

## Key Conventions

- **Indentation**: 4 spaces (no tabs)
- **Naming**: PascalCase classes, snake_case functions/members, UPPER_CASE enums
- **Function comments**: End with "Powered by Claude."
- **Error handling**: Return codes, no exceptions
- **Struct packing**: All game data structs use `#pragma pack(push, 2)`
- **Layer rule**: Data and Editor layers MUST NOT include SDL2 headers

## Testing

```bash
cd build && ctest --output-on-failure
```

Tests cover:
- Struct size assertions (binary compatibility)
- .GAM binary round-trip (write → read → compare)
- .mom_scenario JSON round-trip (serialize → deserialize → compare)
- Undo/redo correctness
- Map generation output validation
- Smoothing rule adjacency checks

## Workflow

1. **Generate or start blank** — Ctrl+N generates terrain, or start from blank (all grassland)
2. **Paint terrain** — Select terrain tool + swatch, click/drag on map
3. **Place entities** — Select entity tool, click to place, select to edit properties
4. **Validate** — Check smoothing warnings (yellow highlights on invalid adjacencies)
5. **Save work** — Ctrl+S saves .mom_scenario (editor format with metadata)
6. **Export** — Ctrl+E exports .GAM (game-loadable binary)
