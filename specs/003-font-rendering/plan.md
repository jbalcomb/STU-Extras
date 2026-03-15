# Implementation Plan: UI Font Rendering

**Branch**: `003-font-rendering` | **Date**: 2026-03-09 | **Spec**: [spec.md](spec.md)

## Summary

Add SDL_ttf-based text rendering to the scenario editor, replacing placeholder colored bars with readable text labels across all UI panels. Uses Cousine-Regular.ttf (43KB, Apache 2.0) with a texture-caching strategy for performance. All changes are in the Presentation layer — no Data or Editor layer modifications.

## Technical Context

**Language/Version**: C++17
**Primary Dependencies**: SDL2 2.32.2, SDL2_ttf 2.22.0 (new), nlohmann/json 3.11.3
**Storage**: N/A (no persistent data changes)
**Testing**: GoogleTest 1.17.0 (existing tests unaffected)
**Target Platform**: Windows (MSVC) + Emscripten/WASM
**Project Type**: Desktop app (SDL2)
**Performance Goals**: 60fps with text rendering (no frame budget regression)
**Constraints**: Font file must embed in WASM binary; text cache must avoid per-frame allocation
**Scale/Scope**: ~6 files modified, 1 font asset added, 0 new source files

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

| Principle | Status | Notes |
|-----------|--------|-------|
| I. Binary Fidelity | PASS | No data format changes. No packed structs modified. |
| II. Layer Separation | PASS | All changes in Presentation layer. Font is a Renderer resource. MomData library untouched. |
| III. Test-Verified Compatibility | PASS | No data format changes → no new compatibility tests needed. Existing tests unaffected. |
| IV. WASM-Ready Data Layer | PASS | MomData has no SDL_ttf dependency. Font loading is in Renderer (Presentation). |
| V. Simplicity First | PASS | Direct SDL_ttf usage, no abstraction layers. Simple string→texture cache. |

**Post-design re-check**: All principles still pass. Font rendering is entirely contained in the Presentation layer.

## Project Structure

### Documentation (this feature)

```text
specs/003-font-rendering/
├── spec.md              # Feature specification
├── plan.md              # This file
├── research.md          # Phase 0: font choice, SDL_ttf integration, caching strategy
├── data-model.md        # Phase 1: runtime entities (no persistent data changes)
├── quickstart.md        # Phase 1: build and verify instructions
└── checklists/
    └── requirements.md  # Spec quality checklist
```

### Source Code (files modified)

```text
RemomMapgenItchio/
├── assets/
│   └── Cousine-Regular.ttf    # NEW: bundled font file (43KB, Apache 2.0)
├── CMakeLists.txt             # MODIFIED: SDL_ttf linking for native + Emscripten
├── src/
│   ├── renderer/
│   │   ├── Renderer.hpp       # MODIFIED: add font_, font_available_, draw_text(), text cache
│   │   ├── Renderer.cpp       # MODIFIED: TTF_Init, font loading, draw_text(), cache mgmt
│   │   ├── UIRenderer.hpp     # MODIFIED: add draw_label() text helper, update draw_status_bar signature
│   │   └── UIRenderer.cpp     # MODIFIED: implement text-based helpers
│   └── ui/
│       ├── ToolPanel.cpp      # MODIFIED: replace draw_label_bar with text labels
│       └── PropertiesPanel.cpp # MODIFIED: replace colored bars with text labels
```

**Structure Decision**: No new source files. Text rendering is added to the existing Renderer class (font is a rendering resource, like the SDL_Renderer itself). UIRenderer gets text-based helper methods alongside existing ones.

## Implementation Approach

### Step 1: CMake + Font Asset

- Copy `Cousine-Regular.ttf` to `assets/`
- Add SDL_ttf to CMakeLists.txt:
  - Native: include dirs, link lib, copy DLL
  - Emscripten: `-sUSE_SDL_TTF=2` compile+link, `--embed-file assets/Cousine-Regular.ttf@/assets/Cousine-Regular.ttf`

### Step 2: Renderer Text Support

- Add to `Renderer.hpp`: `TTF_Font* font_`, `bool font_available_`, `draw_text()` method, text texture cache
- Add to `Renderer.cpp`:
  - `TTF_Init()` in `init()`, `TTF_Quit()` in `shutdown()`
  - Font loading with platform-aware path (`assets/` native, `/assets/` Emscripten)
  - `draw_text(int x, int y, const std::string& text, uint8_t r, uint8_t g, uint8_t b)`:
    - Check `font_available_`, return silently if false
    - Lookup text in cache → if miss, render with `TTF_RenderText_Blended()`, create texture, cache
    - Query texture size, blit to renderer at (x, y)
  - Cache cleanup in `shutdown()`

### Step 3: UIRenderer Text Helpers

- Add `draw_label()` — renders text at position with default light gray color
- Add `draw_button_with_label()` — draws button background + centered text label
- Update `draw_status_bar()` — replace colored blocks with text (coordinates, plane, zoom)
- Update `draw_status_message()` — render the message string as text

### Step 4: ToolPanel Labels (P1)

- Replace `draw_label_bar()` calls with `draw_text()` for each tool button
- Add string mapping: `EditorTool → label` (e.g., `PAINT_TERRAIN → "Terrain"`)
- Add selected swatch name label above terrain/special/flag palette grids

### Step 5: PropertiesPanel Labels (P3)

- Replace colored indicator bars with text labels for:
  - Terrain type name (from `BaseTerrain` enum)
  - Special resource name
  - City race name, population value
  - Unit type, level
  - Node type name
  - Metadata section: "Title:", "Author:", "Description:" prefixes
- Add terrain/race/unit name lookup functions (string arrays indexed by enum value)

### Step 6: Map Generation Labels (P4)

- "Land: XX%" text on the land proportion bar
- "Smooth" / "Normal" / "Rough" text on roughness blocks
- "Continents: N" text for continent count
- "Generate" text on the generate button
