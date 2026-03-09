# Research: Master of Magic Scenario Editor

**Branch**: `001-mom-scenario-editor` | **Date**: 2026-03-08

## R-001: Procedural Map Generation Approach

**Decision**: Use simplex noise-based terrain generation with island/continent shaping masks.

**Rationale**: The existing codebase has no map generation. The original MoM uses `NEWG_RandomizeTiles()`, `NEWG_CreateShores()`, and `NEWG_SetRiverTiles()` (found in ReMoM's `MAPGEN.C`), but these are stub functions in the decompilation. Simplex noise is the standard approach for tile-based strategy game map generation — it produces natural-looking landmasses with controllable parameters (land proportion, roughness, continent count) and is well-documented. The algorithm runs entirely in CPU with no dependencies, fitting the WASM-ready data layer requirement.

**Alternatives considered**:
- Diamond-square algorithm: Produces artifacts at power-of-2 boundaries; 60×40 grid is not power-of-2.
- Voronoi-based biome placement: More complex than needed for 12 terrain types; better suited for games with climate simulation.
- Perlin noise: Simplex is the improved successor — fewer directional artifacts, faster in higher dimensions.

## R-002: Terrain Smoothing / Autotile Rules

**Decision**: Implement a two-phase approach — (1) a validation layer that checks tile adjacency and highlights violations, and (2) an optional autotile pass on export that computes MoM's 762 transition tile indices.

**Rationale**: ReMoM's `TerrType.H` defines 762 distinct terrain tile types using 4-bit binary adjacency encoding in their names (e.g., `_Shore00001100` encodes which cardinal neighbors are land vs. ocean). The key terrain types with adjacency rules are:
- **Shore** (~350 variants): Must be adjacent to ocean on at least one side. The 4-bit pattern encodes NESW neighbors.
- **River** (~90 variants): Must connect to other river tiles or shore/ocean. Isolated rivers are invalid.
- **Mountain** (~16 variants): Edge transitions based on adjacent mountain tiles.
- **Hill** (~17 variants): Edge transitions based on adjacent hill tiles.
- **Desert** (~160 variants): Edge transitions for desert-to-other boundaries.
- **Tundra** (~160 variants): Edge transitions for tundra boundaries.

The validation layer (P5 in spec) checks base terrain adjacency rules without computing the full 762-tile mapping. The autotile export pass writes the correct tile index so the game renders smooth transitions.

**Alternatives considered**:
- Autotile-only (no validation): Users would not see problems until loading in-game. Poor UX.
- Full 762-tile validation in editor: Too complex for real-time feedback. Validating base terrain adjacency is sufficient to catch the errors users care about.

## R-003: Emscripten / WASM Build Configuration

**Decision**: Add conditional CMake configuration that detects the Emscripten toolchain and builds with SDL2's Emscripten port, outputting `.html` + `.js` + `.wasm` for itch.io upload.

**Rationale**: Emscripten 3.1.40+ is installed at `C:\Users\jbalcomb\AppData\Local\emsdk`. SDL2 has first-class Emscripten support via `-s USE_SDL=2` compiler flag (no separate SDL2 download needed). The MomData library has no platform dependencies, so it compiles directly. The main differences for WASM:
- `emscripten_set_main_loop()` replaces the blocking event loop in `main.cpp`
- File I/O uses Emscripten's virtual filesystem + browser download/upload APIs
- No native file dialogs — use HTML `<input type="file">` for upload, `emscripten_browser_async_call` + Blob URL for download

**Alternatives considered**:
- Separate WASM-specific codebase: Violates DRY; high maintenance burden. Conditional compilation is standard practice.
- WebGL-only rendering (skip SDL2): Requires rewriting the entire presentation layer. SDL2's Emscripten port handles this transparently.

## R-004: Browser File I/O Strategy

**Decision**: Use platform abstraction layer (`FileDialog.hpp`) with compile-time branching: native file dialogs on desktop, browser download/upload APIs on WASM.

**Rationale**: The current codebase uses hardcoded filenames (`scenario.mom_scenario`, `scenario.GAM`). For the browser build, there is no filesystem — files must be downloaded to the user's device via Blob URLs and uploaded via `<input type="file">`. The abstraction uses `std::istream`/`std::ostream` so the data layer remains platform-agnostic (Constitution Principle IV). On desktop, this can use native OS file dialogs (Windows `GetOpenFileName` / `GetSaveFileName`). On WASM, it uses Emscripten's `EM_ASM` to trigger browser file operations.

**Alternatives considered**:
- Emscripten IDBFS (IndexedDB filesystem): Persists files in browser storage but is opaque to users — they cannot share files. Download/upload is still needed.
- Drag-and-drop only: Limited UX; not all browsers handle drop consistently. Standard file input is more reliable.

## R-005: Property Editing UI

**Decision**: Extend the existing `PropertiesPanel` from read-only display to read-write editing using clickable fields with dropdown menus for enums (race, difficulty, node type) and direct text input for numeric values (population, gold, coordinates).

**Rationale**: The current PropertiesPanel renders entity properties as colored blocks (no text rendering). Adding property editing requires: (1) text rendering via SDL_ttf or bitmap font, (2) clickable fields that cycle through enum values or accept numeric input, (3) undo/redo wrapping for property changes. This aligns with Constitution Principle V (Simplicity First) — colored-block dropdowns before full text editing.

**Alternatives considered**:
- ImGui integration: Provides immediate-mode UI widgets but adds a heavyweight dependency. Overkill for the limited property editing needed.
- HTML overlay (WASM only): Would provide rich form controls but breaks the single-codebase goal and doesn't work on desktop.

## R-006: Map Generation Parameters

**Decision**: Expose three user-configurable parameters for map generation: Land Proportion (0-100%), Terrain Roughness (smooth/medium/rough), and Continent Count (1-6). These map directly to simplex noise parameters (threshold, octave count, seed-based continent mask).

**Rationale**: The original MoM "New Game" offers only a "Land Size" option (small/medium/large). The spec calls for user-configurable parameters. Three parameters provide meaningful control without overwhelming users — each has a clear visual effect. Internal implementation maps these to noise algorithm parameters:
- Land Proportion → sea level threshold
- Terrain Roughness → noise octave count and amplitude
- Continent Count → number of seed points for Voronoi-based landmass grouping

**Alternatives considered**:
- Single "Land Size" slider (matching original game): Too limited for a scenario editor.
- 10+ parameters (noise frequency, lacunarity, etc.): Overwhelming for non-technical users. Advanced users can hand-edit tiles anyway.

## R-007: Terrain Adjacency Validation Rules

**Decision**: Implement the following base terrain adjacency rules for smoothing validation (derived from ReMoM's `TerrType.H` adjacency encoding):

1. **Shore**: MUST have at least one cardinal neighbor (N/S/E/W) that is Ocean or Lake. Shore without adjacent water is invalid.
2. **River**: MUST have at least one cardinal neighbor that is River, Shore, Ocean, or Lake. Isolated rivers are invalid. River tiles should form connected paths.
3. **Volcano**: SHOULD be adjacent to Mountain or Hill. Isolated volcanoes are flagged as warnings (not errors — the game handles them, but they look odd).
4. **Tundra**: SHOULD NOT be adjacent to Desert. This combination does not occur naturally and produces jarring visual transitions.
5. **All terrain**: Horizontal map wrapping must be checked — tile at x=0 is adjacent to tile at x=59.

These rules produce warnings (yellow highlight), not errors. All tile arrangements are legal in the .GAM format; validation helps users produce visually coherent maps.

**Rationale**: The full 762-tile autotile system is too complex for real-time validation. These 5 rules catch the most common and visually obvious problems. They are derived from the terrain type naming patterns in `TerrType.H` where adjacency bits determine which transition tile to render.

**Alternatives considered**:
- Full 762-tile transition validation: Too granular — most transition tiles render acceptably even with imperfect adjacency. Only the base terrain mismatches produce visible artifacts.
- No validation (defer to autotile export): Users would not discover problems until loading in-game.
