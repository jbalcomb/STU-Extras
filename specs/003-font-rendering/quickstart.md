# Quickstart: UI Font Rendering

**Branch**: `003-font-rendering` | **Date**: 2026-03-09

## Prerequisites

- SDL2_ttf 2.22.0 at `C:/devellib/SDL2_ttf-2.22.0/`
- Font file: `assets/Cousine-Regular.ttf` (copy from `C:/devellib/imgui/misc/fonts/Cousine-Regular.ttf`)
- Existing build environment (CMake 4.2+, MSVC, Ninja, SDL2)

## Build (Desktop)

```bash
cd build && cmake --build .
```

SDL2_ttf.dll must be in the build output directory alongside the executable.

## Build (WASM)

```bash
# From build_wasm directory
emcmake cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

Emscripten uses `-sUSE_SDL_TTF=2` (port) and `--embed-file` for the font.

## Verify

1. Launch the editor
2. Confirm tool buttons show text labels ("Select", "Terrain", etc.)
3. Hover over the map — status bar should show "X: N Y: N | Arcanus | Zoom: 1.0x"
4. Select a terrain swatch — label above palette should show the terrain name
5. If font file is missing, editor should still launch with colored-bar fallback

## Key Files

| File | Role |
|------|------|
| `src/renderer/Renderer.hpp/cpp` | Font loading, `draw_text()`, text cache |
| `src/renderer/UIRenderer.hpp/cpp` | Text-based UI helpers |
| `src/ui/ToolPanel.cpp` | Tool button labels |
| `src/ui/PropertiesPanel.cpp` | Property labels |
| `CMakeLists.txt` | SDL_ttf linking |
| `assets/Cousine-Regular.ttf` | Bundled font file |
