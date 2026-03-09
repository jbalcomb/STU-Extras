# Tasks: Master of Magic Scenario Editor

**Input**: Design documents from `/specs/001-mom-scenario-editor/`
**Prerequisites**: plan.md (required), spec.md (required), research.md, data-model.md, contracts/

**Tests**: Included for new data layer modules (MapGenerator, SmoothingValidator) per Constitution Principle III (Test-Verified Compatibility). UI-level tests are not included.

**Organization**: Tasks are grouped by user story to enable independent implementation and testing of each story.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which user story this task belongs to (e.g., US1, US2, US3)
- Include exact file paths in descriptions

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Create new source directories and update build configuration for new modules

- [X] T001 Create directory structure for new modules: `src/mapgen/`, `src/validation/`, `src/platform/`
- [X] T002 Add MapGenParams struct (land_proportion, roughness, continent_count, seed, mirror_planes) and Roughness enum to `src/mom_data/MomConstants.hpp` — editor-only, not part of packed structs
- [X] T003 Add SmoothingViolation struct (plane, x, y, rule, severity, message) and ViolationRule/Severity enums to `src/mom_data/MomConstants.hpp` — transient, not serialized
- [X] T004 Update `CMakeLists.txt` — add `src/mapgen/*.cpp` and `src/validation/*.cpp` to the MomData static library target (no SDL2, no platform APIs). Add `src/platform/*.cpp` to the RemomMapgenItchio executable target ONLY (contains Win32/Emscripten platform-specific code, must NOT be in MomData per Constitution Principle IV).

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Platform abstraction and Emscripten build support that multiple user stories depend on

**CRITICAL**: No WASM deployment (US7) can proceed without this phase. Save/Load (US6) also needs platform file dialogs.

- [X] T005 Implement platform file dialog abstraction in `src/platform/FileDialog.hpp` and `src/platform/FileDialog.cpp` — define `save_file()`, `load_file()`, and `download_blob()` functions with `#ifdef __EMSCRIPTEN__` compile-time branching. Desktop: call Windows `GetOpenFileName`/`GetSaveFileName`. WASM: use `EM_ASM` for browser `<input type="file">` upload and Blob URL download. All functions accept/return `std::vector<uint8_t>` byte buffers.
- [X] T006 [P] Add Emscripten conditional build configuration to `CMakeLists.txt` — detect `EMSCRIPTEN` compiler, use `-s USE_SDL=2` flag for SDL2 port, set output to `.html`+`.js`+`.wasm`, add `--preload-file` for any bundled assets, set `ALLOW_MEMORY_GROWTH=1`, and reference `src/shell.html` via `--shell-file` linker flag (shell template created in T037)

**Checkpoint**: Platform abstraction and WASM build config ready — user story implementation can now begin

---

## Phase 3: User Story 1 - Generate and Export a Scenario (Priority: P1) MVP

**Goal**: User can generate a world map from parameters and export a valid .GAM file

**Independent Test**: Generate map with defaults, export .GAM, verify file is 123,300 bytes and loads in ReMoM

### Tests for User Story 1

- [X] T007 [P] [US1] Create `tests/test_MapGenerator.cpp` — test that `MapGenerator::generate()` produces a MomWorld where: (1) terrain array contains no uninitialized values, (2) land proportion approximately matches input parameter (within 10%), (3) both planes have terrain set, (4) shore tiles are placed at land/ocean boundaries, (5) landmass IDs are computed and nonzero for land tiles. Register in `CMakeLists.txt` test target.

### Implementation for User Story 1

- [X] T008 [P] [US1] Implement simplex noise generator in `src/mapgen/SimplexNoise.hpp` and `src/mapgen/SimplexNoise.cpp` — provide `noise2d(x, y)` returning float in [-1,1] and `octave_noise2d(x, y, octaves, persistence)` for fractal Brownian motion. Pure C++17, no dependencies. Header-only permutation table. Function comments end with "Powered by Claude."
- [X] T009 [US1] Implement `MapGenerator` class in `src/mapgen/MapGenerator.hpp` and `src/mapgen/MapGenerator.cpp` — constructor takes `MapGenParams`. `generate(MomWorld& world)` method fills both planes: (1) use simplex noise to create height map, (2) threshold at `land_proportion` to separate ocean from land, (3) use continent_count to seed Voronoi-based landmass grouping, (4) assign biomes (grassland, forest, mountain, desert, swamp, tundra, hill) based on height/moisture noise layers, (5) call `place_shores()` to add shore tiles at ocean/land boundaries, (6) call `place_rivers()` to add river tiles from high elevation to coast, (7) compute landmass IDs, (8) set all tiles explored. Account for horizontal wrapping (x=0 neighbors x=59). Roughness parameter controls octave count (SMOOTH=2, MEDIUM=4, ROUGH=6).
- [X] T010 [P] [US1] Add MapGenParams JSON serialization to `src/scenario/ScenarioIO.cpp` — in `write_scenario()`, add `"map_gen_params"` JSON object with land_proportion, roughness (as string), continent_count, seed, mirror_planes. In `read_scenario()`, parse same keys with defaults (land_proportion=0.4, roughness=MEDIUM, continent_count=3, seed=0, mirror_planes=false). Add MapGenParams field to Scenario struct in `src/scenario/Scenario.hpp`.
- [X] T011 [US1] Add map generation UI panel to `src/ui/ToolPanel.cpp` — add a "Generate" section below tool buttons with: (1) land proportion slider rendered as a colored bar (click to cycle 20/40/60/80%), (2) roughness selector (3 colored blocks for SMOOTH/MEDIUM/ROUGH, click to cycle), (3) continent count selector (click to cycle 1-6, display as number of dots), (4) "Generate Map" button (colored rectangle). Update `ToolPanel::handle_click()` to detect clicks on these controls and return a flag indicating generate was requested.
- [X] T012 [US1] Wire map generation into main event loop in `src/main.cpp` — add Ctrl+N keyboard shortcut and handle ToolPanel generate button: (1) create MapGenerator with current MapGenParams from EditorState, (2) wrap `generate()` call in an UndoStack command that captures before/after world state, (3) call generate, (4) trigger map re-render. Add MapGenParams field to EditorState in `src/editor/EditorState.hpp`.
- [X] T013 [US1] Verify .GAM export works with generated maps in `src/main.cpp` — ensure existing Ctrl+E export (hardcoded `scenario.GAM` filename) correctly writes the generated world data. Add assertion that exported file is exactly 123,300 bytes. File dialog wiring is deferred to T032 (US6) when platform abstraction is available.

**Checkpoint**: User can generate a map with parameters, view it, and export a valid .GAM file. Core MVP complete.

---

## Phase 4: User Story 2 - Configure Game Options and Wizard Setup (Priority: P2)

**Goal**: User can set difficulty, magic, land size, wizard count, and configure each wizard's race, retorts, and spells

**Independent Test**: Set difficulty to Impossible, configure 4 wizards with specific races, export .GAM, load in ReMoM, verify settings match

### Implementation for User Story 2

- [X] T014 [P] [US2] Add game options panel to `src/ui/ToolPanel.cpp` or create new `src/ui/GameOptionsPanel.hpp` and `src/ui/GameOptionsPanel.cpp` — render a panel (toggled via a "Settings" tool button) showing: (1) Difficulty selector (5 colored blocks: green=Intro through red=Impossible, click to cycle), (2) Magic level selector (click to cycle values), (3) Land size selector (click to cycle), (4) Wizard count selector (1-6, click to cycle). Each control updates `scenario.game_data` directly through an undo command. Add `render()` and `handle_click()` methods.
- [X] T015 [P] [US2] Add wizard configuration panel in `src/ui/WizardPanel.hpp` and `src/ui/WizardPanel.cpp` — render a panel (toggled via a "Wizards" tool button) showing tabs for Wizard 0-5. Each tab displays: (1) name field (20-char max, click cycles preset names or shows colored placeholder), (2) race selector (14 races, click cycles through, show race color), (3) portrait index selector (click to cycle), (4) spell rank selectors (5 realms — Life, Death, Chaos, Nature, Sorcery — each rendered as a colored bar, click to cycle rank 0-4, maps to `spell_ranks[5]` in Wizard struct), (5) retort toggles (18 retorts, rendered as small colored squares — click to toggle), (6) active/inactive toggle (clear name to deactivate). All changes wrapped in undo commands. Register new files in `CMakeLists.txt`.
- [X] T016 [US2] Add SETTINGS and WIZARDS tools to EditorState tool enum in `src/editor/EditorState.hpp` — add `TOOL_SETTINGS` and `TOOL_WIZARDS` enum values. Update ToolPanel to show these as additional tool buttons. Update main.cpp to render GameOptionsPanel when TOOL_SETTINGS is active and WizardPanel when TOOL_WIZARDS is active, suppressing the normal tool palette.
- [X] T017 [US2] Update `GameData.Total_Wizards` and `GameData.Total_Cities`/`Total_Units` counts on export in `src/main.cpp` — before calling `MomGamFile::write()`, iterate entity arrays and set accurate counts in `scenario.game_data` so the game engine knows how many active entities exist.

**Checkpoint**: User can configure all game options and wizard setups. Settings persist through save/load and export correctly in .GAM.

---

## Phase 5: User Story 3 - Hand-Edit the World Map (Priority: P3)

**Goal**: User can paint terrain, specials, and tile flags with undo/redo, switch planes, scroll and zoom

**Independent Test**: Paint a ring of mountains around grassland, toggle road flags, undo 3 times, verify map state

**Note**: Most of this story is already implemented. Tasks cover the gaps.

### Implementation for User Story 3

- [X] T018 [P] [US3] Add flag painting tool to `src/main.cpp` — add `TOOL_PAINT_FLAGS` to EditorState tool enum in `src/editor/EditorState.hpp`. In `apply_tool()`, implement flag toggling: click cycles through Road (0x08), Enchanted Road (0x10), Corruption (0x20), None. Wrap in undo command. Add flag swatch display to `src/ui/ToolPanel.cpp` (3 colored blocks for Road/EnchRoad/Corruption).
- [X] T019 [P] [US3] Add flag rendering overlay to `src/renderer/MapRenderer.cpp` — when a tile has MSF_ROAD flag, draw a small brown line across the tile center. When MSF_ENC_ROAD, draw a gold line. When MSF_CORRUPTION, draw a dark purple overlay on the tile. Render these after base terrain but before entity overlays.
- [X] T020 [US3] Fix horizontal map wrapping in `src/renderer/MapRenderer.cpp` — when rendering tiles near x=0 or x=59, also render the wrapped continuation so the map appears seamless. Update mouse-to-tile coordinate conversion in `src/renderer/Renderer.cpp` to handle wrap-around (clicking past x=59 maps to x=0). Ensure camera scrolling wraps smoothly.

**Checkpoint**: Full terrain/special/flag painting with undo/redo, plane switching, and seamless horizontal wrapping.

---

## Phase 6: User Story 4 - Place and Configure Entities (Priority: P4)

**Goal**: User can place all entity types, configure their properties, and respect slot limits

**Independent Test**: Place a city for Wizard 1, set race to Dwarf, add Smithy building, place nearby units, place a node, export and verify in ReMoM

### Implementation for User Story 4

- [X] T021 [P] [US4] Implement tower placement tool handler in `src/main.cpp` `apply_tool()` — find first inactive tower slot (where active=0), set x/y/owner/active, wrap in undo command. Display slot limit message if all 6 slots are active. Follow existing PLACE_NODE pattern.
- [X] T022 [P] [US4] Implement fortress placement tool handler in `src/main.cpp` `apply_tool()` — find first inactive fortress slot, set x/y/plane/active, wrap in undo command. Display slot limit message if all 6 slots are active.
- [X] T023 [P] [US4] Implement lair placement tool handler in `src/main.cpp` `apply_tool()` — find first inactive lair slot (check if all fields are zero), set x/y/plane/type (default type=1), wrap in undo command. Display slot limit message if all 102 slots are active.
- [X] T024 [US4] Convert PropertiesPanel from read-only to read-write in `src/ui/PropertiesPanel.cpp` — for each displayed property: (1) make the value area clickable, (2) for enum fields (race, difficulty, node type), click cycles to next enum value, (3) for numeric fields (population, gold, power), click increments by step (hold shift for larger step), (4) for building flags, click toggles individual building. All property changes MUST be wrapped in undo commands via `UndoStack::execute()`. Update `handle_click()` to detect property clicks and return the modified entity.
- [X] T025 [US4] Add slot limit feedback rendering in `src/renderer/UIRenderer.cpp` — when an entity placement is rejected due to slot limits, display a colored warning bar at the bottom of the screen for 3 seconds showing "Maximum [entity type] reached ([count]/[limit])". Add a timer mechanism in EditorState to track active warning messages.
- [X] T026 [US4] Add tower, fortress, and lair overlay rendering to `src/renderer/MapRenderer.cpp` — towers: white diamond shape, fortresses: colored star matching wizard's banner, lairs: dark red triangle. Update entity rendering section to handle these entity types alongside existing city/unit/node rendering.

**Checkpoint**: All entity types can be placed, configured, and rendered. Slot limits enforced with user feedback.

---

## Phase 7: User Story 5 - Terrain Smoothing Rule Validation (Priority: P5)

**Goal**: Editor highlights tiles that violate terrain adjacency rules with visual warnings

**Independent Test**: Place a shore tile surrounded by grassland (no adjacent ocean), verify yellow highlight appears. Fix by adding adjacent ocean, verify highlight disappears.

### Tests for User Story 5

- [X] T027 [P] [US5] Create `tests/test_SmoothingValidator.cpp` — test all validation rules from data-model.md V-009 through V-012: (1) shore tile with no adjacent ocean/lake returns SHORE_NO_WATER violation, (2) shore tile with adjacent ocean returns no violation, (3) isolated river tile returns RIVER_ISOLATED violation, (4) river adjacent to another river returns no violation, (5) volcano with no adjacent mountain/hill returns VOLCANO_ISOLATED violation, (6) tundra adjacent to desert returns TUNDRA_DESERT violation, (7) wrap-around check: shore at x=0 with ocean at x=59 returns no violation, (8) validate full 60x40 plane returns violations list. Register in `CMakeLists.txt` test target.

### Implementation for User Story 5

- [X] T028 [US5] Implement SmoothingValidator class in `src/validation/SmoothingValidator.hpp` and `src/validation/SmoothingValidator.cpp` — provide `validate_tile(world, plane, x, y)` returning `std::vector<SmoothingViolation>` and `validate_plane(world, plane)` returning violations for all tiles. Implement rules: (1) Shore must have at least one cardinal neighbor (N/S/E/W) that is Ocean or Lake — account for x-axis wrapping (x=0 neighbors x=59), y-axis does NOT wrap, (2) River must have at least one cardinal neighbor that is River, Shore, Ocean, or Lake, (3) Volcano should have at least one cardinal neighbor that is Mountain or Hill, (4) Tundra should not have any cardinal neighbor that is Desert. Use `MomWorld::get_terrain()` for neighbor lookups which already handles bounds checking. Function comments end with "Powered by Claude."
- [X] T029 [US5] Integrate validation into editor in `src/main.cpp` — after any terrain edit (paint terrain, generate map), call `SmoothingValidator::validate_tile()` for the edited tile and its 4 cardinal neighbors. Store active violations in a `std::vector<SmoothingViolation>` on EditorState. On map generation, call `validate_plane()` for both planes. Add violations field to `src/editor/EditorState.hpp`.
- [X] T030 [US5] Render smoothing violation overlays in `src/renderer/MapRenderer.cpp` — for each violation in EditorState's violation list: draw a yellow semi-transparent border around the tile (2px inset from tile edge). If the user presses 'V' key (toggle), show all violations highlighted simultaneously. Add 'V' keybinding to `src/main.cpp` to toggle a `show_all_violations` flag in EditorState.

**Checkpoint**: Smoothing rule violations are highlighted in real-time. Warnings are advisory — export still proceeds.

---

## Phase 8: User Story 6 - Save and Load Native Scenario Format (Priority: P6)

**Goal**: User can save work-in-progress to .mom_scenario and load it later, preserving all data including editor metadata

**Independent Test**: Create scenario with edits, save, reload, verify all terrain/entities/options/metadata match

**Note**: JSON serialization already exists. Tasks cover metadata editing and platform-aware file dialogs.

### Implementation for User Story 6

- [X] T031 [P] [US6] Add metadata editing to PropertiesPanel in `src/ui/PropertiesPanel.cpp` — when no entity is selected, show scenario metadata: title (click to cycle preset names or edit), author (click to edit), description, version. All changes update `scenario.metadata` fields through undo commands. Show metadata section at top of properties panel always visible.
- [X] T032 [US6] Wire all file operations through platform file dialogs in `src/main.cpp` — replace hardcoded filenames for both `scenario.mom_scenario` (Ctrl+S/Ctrl+O) and `scenario.GAM` (Ctrl+E) with `FileDialog::save_file()` and `FileDialog::load_file()` calls. On save: serialize to JSON via `ScenarioIO::write_scenario()`, pass bytes to FileDialog. On load: receive bytes from FileDialog, deserialize via `ScenarioIO::read_scenario()`, update EditorState (clear undo stack, reset camera, revalidate smoothing). On export: write .GAM via `MomGamFile::write()`, pass bytes to FileDialog for download.
- [X] T033 [US6] Add MapGenParams round-trip test to `tests/test_Scenario.cpp` — verify that MapGenParams values survive JSON serialization and deserialization: set non-default values (land_proportion=0.7, roughness=ROUGH, continent_count=5, seed=42, mirror_planes=true), write to JSON, read back, assert all fields match.

**Checkpoint**: Full save/load workflow with metadata editing. Round-trip preserves 100% of data.

---

## Phase 9: User Story 7 - Share Scenarios with the Community (Priority: P7)

**Goal**: Application runs in browser via WASM on itch.io, enabling anyone to create and download .GAM files without installation

**Independent Test**: Build WASM target, serve locally, open in Chrome/Firefox/Edge/Safari, generate map, export .GAM, verify downloaded file is valid

### Implementation for User Story 7

- [X] T034 [US7] Adapt main event loop for Emscripten in `src/main.cpp` — wrap the main loop body in a function `main_loop_iteration()`. In `main()`, use `#ifdef __EMSCRIPTEN__` to call `emscripten_set_main_loop(main_loop_iteration, 0, 1)` instead of the `while(!quit)` loop. Include `<emscripten.h>` conditionally. Ensure SDL_PollEvent is called once per iteration (not blocking).
- [X] T035 [US7] Implement browser file download in `src/platform/FileDialog.cpp` — in the `__EMSCRIPTEN__` branch of `save_file()` and `download_blob()`: use `EM_ASM` to create a JavaScript Blob from the byte buffer, generate a Blob URL, create a temporary `<a>` element with `download` attribute set to the filename, programmatically click it, and revoke the URL. Support both `.GAM` and `.mom_scenario` file extensions.
- [X] T036 [US7] Implement browser file upload in `src/platform/FileDialog.cpp` — in the `__EMSCRIPTEN__` branch of `load_file()`: use `EM_ASM` to create a temporary `<input type="file">` element with `accept=".mom_scenario,.GAM"`, attach an `onchange` handler that reads the file via `FileReader`, and passes bytes back to C++ via `emscripten_async_call` or a callback function pointer. Store loaded bytes in a buffer accessible from the main loop.
- [X] T037 [US7] Create HTML shell template at `src/shell.html` — custom Emscripten shell page with: (1) centered `<canvas>` element for SDL2 rendering, (2) minimal CSS for dark background matching editor aesthetic, (3) loading progress bar, (4) WASM feature detection with fallback message for unsupported browsers ("This application requires WebAssembly. Please use a modern browser."), (5) itch.io-friendly viewport meta tag. Reference in CMakeLists.txt via `--shell-file` linker flag.
- [X] T038 [US7] Create WASM build script at `build_wasm.sh` (and `build_wasm.bat` for Windows) — activate Emscripten SDK, run `emcmake cmake -B build_wasm -G Ninja`, run `cmake --build build_wasm`, report output file sizes. Add instructions for local testing via `python -m http.server`.
- [X] T039 [US7] Test WASM build compiles and runs — build with Emscripten toolchain, serve `build_wasm/` directory locally, verify app loads in Chrome and Firefox, verify map generation works, verify .GAM download triggers browser save dialog. Fix any compilation issues (missing headers, platform-specific code paths, SDL2 Emscripten port differences).

**Checkpoint**: Application compiles to WASM, runs in browser, supports file download/upload. Ready for itch.io upload.

---

## Phase 10: Polish & Cross-Cutting Concerns

**Purpose**: Improvements that affect multiple user stories and edge case handling

- [X] T040 [P] Implement standalone minimap component in `src/ui/Minimap.hpp` and `src/ui/Minimap.cpp` — extract minimap rendering from PropertiesPanel into its own class. Show both planes side-by-side in compressed view (60x40 → ~120x40px). Draw a white rectangle indicating current viewport position. Click on minimap jumps camera to that location. Register in `CMakeLists.txt`.
- [X] T041 [P] Add WASM compatibility detection at startup in `src/main.cpp` — on desktop: no check needed. On WASM (shell.html): add JavaScript check for `WebAssembly` global, show fallback message if absent. In C++: on startup, log SDL2 renderer info to console for debugging.
- [X] T042 [P] Handle edge case: loading external .GAM files in `src/main.cpp` — add Ctrl+I "Import .GAM" shortcut that calls `FileDialog::load_file()` with .GAM filter, reads via `MomGamFile::read()`, and populates the scenario. Show a warning status message "Imported game save — some data may not be editable" since mid-game saves contain state the editor does not understand.
- [X] T043 Clamp extreme map generation parameters in `src/mapgen/MapGenerator.cpp` — ensure land_proportion is clamped to [0.05, 0.95] so maps always have some ocean and some land. Clamp continent_count to [1, min(6, land_tile_count/100)] so tiny landmasses are not forced. Log a warning to status if parameters were clamped.
- [X] T044 Verify all existing tests still pass after changes — run `cd build && cmake --build . && ctest --output-on-failure` and fix any regressions. Ensure new test files (`test_MapGenerator.cpp`, `test_SmoothingValidator.cpp`) are registered and passing.
- [X] T045 Run quickstart.md validation — follow the steps in `specs/001-mom-scenario-editor/quickstart.md` end-to-end on a clean build. Verify desktop build, WASM build (if Emscripten available), and the described workflow (generate → paint → place → validate → save → export) works as documented.

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies — can start immediately
- **Foundational (Phase 2)**: Depends on Setup (Phase 1) completion — BLOCKS US6 (save/load file dialogs) and US7 (WASM build)
- **User Stories (Phase 3-9)**: All depend on Setup (Phase 1). US6 and US7 additionally depend on Foundational (Phase 2)
- **Polish (Phase 10)**: Depends on all desired user stories being complete

### User Story Dependencies

- **US1 (P1 — Generate & Export)**: Depends on Phase 1 only. No other story dependencies. **This is the MVP.**
- **US2 (P2 — Game Options)**: Depends on Phase 1 only. Independent of US1.
- **US3 (P3 — Hand-Edit Map)**: Depends on Phase 1 only. Independent — extends already-working functionality.
- **US4 (P4 — Place & Configure Entities)**: Depends on Phase 1 only. Independent — extends already-working entity placement.
- **US5 (P5 — Smoothing Validation)**: Depends on Phase 1 only. Independent — works on any map state.
- **US6 (P6 — Save/Load)**: Depends on Phase 1 AND Phase 2 (platform file dialogs). Extends existing serialization.
- **US7 (P7 — Community Sharing)**: Depends on Phase 1 AND Phase 2 (Emscripten build config + platform file I/O). All other stories should be complete first for full-featured web build.

### Within Each User Story

- Tests first (if included), then implementation
- Data layer changes before presentation layer changes
- Core logic before UI wiring
- Undo command wrapping for all mutations

### Parallel Opportunities

- **Phase 1**: T002 and T003 can run in parallel (different additions to same file, but non-overlapping sections)
- **Phase 2**: T005 and T006 can run in parallel (different files: platform/ vs CMakeLists.txt)
- **Phase 3 (US1)**: T007, T008, T010 can run in parallel (different files). T009 depends on T008 (uses SimplexNoise)
- **Phase 4 (US2)**: T014 and T015 can run in parallel (different panel files)
- **Phase 5 (US3)**: T018 and T019 can run in parallel (main.cpp vs MapRenderer.cpp)
- **Phase 6 (US4)**: T021, T022, T023 can run in parallel (all in main.cpp but independent tool handlers). T026 is parallel with T024.
- **Phase 7 (US5)**: T027 is parallel with T028 (test vs implementation, test written first)
- **US1-US5 can run in parallel** after Phase 1 completes (different files, no cross-dependencies)

---

## Parallel Example: User Story 1

```bash
# Launch tests and independent implementation tasks together:
Task: "Create tests/test_MapGenerator.cpp with generation validation tests"
Task: "Implement SimplexNoise in src/mapgen/SimplexNoise.hpp/.cpp"
Task: "Add MapGenParams JSON serialization to src/scenario/ScenarioIO.cpp"

# After SimplexNoise is complete, MapGenerator can proceed:
Task: "Implement MapGenerator in src/mapgen/MapGenerator.hpp/.cpp"

# After MapGenerator is complete, wire into UI:
Task: "Add map generation UI panel to src/ui/ToolPanel.cpp"
Task: "Wire map generation into main event loop in src/main.cpp"
Task: "Wire .GAM export through platform file dialog in src/main.cpp"
```

## Parallel Example: User Stories 1-5 Simultaneously

```bash
# After Phase 1 (Setup) is complete, launch all independent stories:
Story 1: "MapGenerator + generation UI + export wiring"
Story 2: "GameOptionsPanel + WizardPanel + settings integration"
Story 3: "Flag painting + flag rendering + horizontal wrapping"
Story 4: "Tower/Fortress/Lair tools + PropertiesPanel read-write + slot limits"
Story 5: "SmoothingValidator + validation integration + violation overlays"
```

---

## Implementation Strategy

### MVP First (User Story 1 Only)

1. Complete Phase 1: Setup (T001-T004)
2. Complete Phase 3: User Story 1 (T007-T013)
3. **STOP and VALIDATE**: Generate a map, export .GAM, load in ReMoM
4. The user can now generate and export scenarios — core product works

### Incremental Delivery

1. Setup (Phase 1) → Foundation ready
2. US1 (Generate & Export) → **MVP! Users can create and share scenarios**
3. US2 (Game Options) → Users can customize difficulty and wizards
4. US3 (Hand-Edit Map) → Users have tile-level control (mostly exists already)
5. US4 (Place Entities) → Users can populate maps with cities, units, nodes
6. US5 (Smoothing Validation) → Quality feedback on terrain placement
7. Phase 2 (Foundational) → Platform abstraction ready
8. US6 (Save/Load) → Users can iterate on scenarios across sessions
9. US7 (Community Sharing) → App runs in browser on itch.io
10. Polish (Phase 10) → Edge cases, minimap, compatibility checks

### Solo Developer Strategy

1. Phase 1 → Phase 3 (US1) → validate MVP
2. Phase 5 (US3) → quick wins, mostly exists
3. Phase 6 (US4) → entity tools complete the editor
4. Phase 4 (US2) → game options and wizard config
5. Phase 7 (US5) → smoothing validation
6. Phase 2 → Phase 8 (US6) → Phase 9 (US7) → web deployment
7. Phase 10 → polish

---

## Notes

- [P] tasks = different files, no dependencies
- [Story] label maps task to specific user story for traceability
- Each user story is independently completable and testable
- Constitution Principle I: No packed struct modifications — all new data is editor-only
- Constitution Principle II: mapgen/ and validation/ go in Data layer (no SDL2)
- Constitution Principle V: Colored blocks before text, hardcoded defaults before configurability
- Commit after each task or logical group
- Stop at any checkpoint to validate story independently
- Total: 45 tasks across 10 phases
