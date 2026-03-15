# RemomMapgenItchio

A Master of Magic (1994) scenario editor that creates custom `.GAM` save files compatible with the [ReMoM](https://github.com/jbalcomb/ReMoM) game engine. Built with C++17 and SDL2, with an Emscripten/WASM build for browser-based editing.

**Play in browser:** https://jbalcomb.itch.io/maker-of-magic

## Features

- Visual tile-based map editor (60x40 grid, 2 planes: Arcanus and Myrror)
- Place cities, units, wizards, nodes, lairs, towers, and fortresses
- Export `.GAM` files loadable in Master of Magic / ReMoM
- Native `.mom_scenario` JSON format for editor state
- Undo/redo with 50-command depth
- Minimap navigation
- WASM build for in-browser editing

## Quick Start

### Prerequisites

- CMake 4.2+
- MSVC (Visual Studio 2022) or compatible C++17 compiler
- Ninja build system
- SDL2 2.32.2 at `C:/devellib/SDL2-2.32.2/`

nlohmann/json and GoogleTest are fetched automatically via CMake FetchContent.

### Build

```bash
cmake -B build -G Ninja
cmake --build build
```

### Run Tests

```bash
cd build && ctest --output-on-failure
```

### Build for Web (WASM)

Requires [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html):

```bash
source /path/to/emsdk/emsdk_env.sh
./build_wasm.sh
```

Or on Windows:

```batch
emsdk_env.bat
build_wasm.bat
```

### Deploy to itch.io

Requires [Butler CLI](https://itch.io/docs/butler/installing.html):

```bash
cp deploy.conf.example deploy.conf   # edit with your itch.io details
./deploy_itchio.sh                   # deploy existing WASM build
./deploy_itchio.sh --build           # build + deploy in one step
```

See [quickstart.md](../specs/002-butler-itchio-deploy/quickstart.md) for detailed setup instructions.

## Architecture

Three-layer architecture with strict downward-only dependencies:

```
Presentation (SDL2)  →  Renderer, MapRenderer, UIRenderer, Panels, Minimap
Editor (no SDL2)     →  EditorState, UndoStack (command pattern)
Data (pure C++)      →  MomConstants, MomEntities, MomWorld, MomGamFile, Scenario, ScenarioIO
```

**Build targets:**

| Target | Type | Description |
|--------|------|-------------|
| `MomData` | Static library | Data + editor layers (no SDL2 dependency) |
| `RemomMapgenItchio` | Executable | SDL2 desktop application |
| `RemomTests` | Test binary | GoogleTest suite |

## Binary Compatibility

All entity structs use `#pragma pack(push, 2)` for 8086-compatible alignment matching the original executable. Struct sizes are verified by `static_assert` and tested in the test suite.

- `.GAM` files must be exactly 123,300 bytes
- Struct layouts must match ReMoM's C definitions byte-for-byte
- Reference structs: ReMoM `src/MOX/MOM_Data.H` and `src/MOX/LOADSAVE.C`

## World Model

| Property | Value |
|----------|-------|
| Map size | 60 wide x 40 high |
| Planes | 2 (Arcanus=0, Myrror=1) |
| Cities | 100 slots |
| Units | 1,009 slots |
| Wizards | 6 slots |
| Nodes | 30 slots |
| Lairs | 102 slots |
| Towers | 6 slots |
| Fortresses | 6 slots |
| Items | 138 slots |

## File Formats

- **`.mom_scenario`** (JSON) — Editor's native format with metadata, base64-encoded terrain, and entity arrays
- **`.GAM`** (binary, 123,300 bytes fixed) — Game engine save format, loadable in Master of Magic / ReMoM

## Project Structure

```
src/
├── editor/        Editor state, undo/redo
├── mapgen/        Map generation algorithms
├── mom_data/      Game structs, world grid, serialization
├── platform/      Platform-specific code (file dialogs)
├── renderer/      SDL2 rendering
├── scenario/      Scenario I/O (JSON + binary)
├── ui/            UI panels (tools, properties, minimap)
├── validation/    Data validation
└── main.cpp       Application entry point
tests/             GoogleTest suite
```

## Contributing

### Conventions

- **Naming**: PascalCase classes, snake_case functions/members, UPPER_CASE enums/constants
- **Error handling**: Return codes, no exceptions
- **Indentation**: 4 spaces (no tabs)
- **Function comments**: End with "Powered by Claude."

### Before Committing

1. Build succeeds with zero warnings in data layer code
2. All tests pass (`ctest --output-on-failure`)
3. For `.GAM` export changes: verify the exported file loads in ReMoM

### Key Rules

- Never change entity struct layouts without verifying against ReMoM source
- Data layer (`MomData`) must have zero SDL2 dependency
- Every user-facing mutation must be wrapped in an undo command
