# Tasks: UI Font Rendering

**Input**: Design documents from `/specs/003-font-rendering/`
**Prerequisites**: plan.md (required), spec.md (required), research.md, data-model.md, quickstart.md

**Tests**: Not requested in spec — test tasks omitted. Existing tests must continue to pass.

**Organization**: Tasks grouped by user story for independent implementation and testing.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which user story this task belongs to (e.g., US1, US2, US3, US4)
- Include exact file paths in descriptions

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Add SDL_ttf dependency and font asset to the project

- [ ] T001 Copy Cousine-Regular.ttf from C:/devellib/imgui/misc/fonts/ to assets/Cousine-Regular.ttf
- [ ] T002 Add SDL_ttf linking to CMakeLists.txt — native: include dirs from C:/devellib/SDL2_ttf-2.22.0/, link SDL2_ttf.lib; Emscripten: add -sUSE_SDL_TTF=2 compile+link and --embed-file assets/Cousine-Regular.ttf@/assets/Cousine-Regular.ttf
- [ ] T003 Add install rule for SDL2_ttf.dll to CMakeLists.txt for native builds (copy to build output)

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Add text rendering capability to Renderer — MUST complete before ANY user story

**CRITICAL**: No user story work can begin until this phase is complete

- [ ] T004 Add SDL_ttf includes, font handle (TTF_Font*), font_available_ flag, and text texture cache (std::unordered_map<std::string, SDL_Texture*>) to src/renderer/Renderer.hpp
- [ ] T005 Implement TTF_Init() in Renderer::init(), TTF_OpenFont() with platform-aware path (assets/ native, /assets/ Emscripten), fallback flag on failure in src/renderer/Renderer.cpp
- [ ] T006 Implement draw_text(int x, int y, const std::string& text, uint8_t r, uint8_t g, uint8_t b) in src/renderer/Renderer.cpp — cache lookup, TTF_RenderText_Blended on miss, SDL_CreateTextureFromSurface, SDL_RenderCopy
- [ ] T007 Add TTF_CloseFont() and texture cache cleanup to Renderer::shutdown() in src/renderer/Renderer.cpp
- [ ] T008 Add draw_label() and draw_button_with_label() text helper methods to src/renderer/UIRenderer.hpp and src/renderer/UIRenderer.cpp

**Checkpoint**: Renderer can render text. Build desktop to verify font loads and draw_text works.

---

## Phase 3: User Story 1 - Tool Buttons Have Readable Labels (Priority: P1) MVP

**Goal**: All 13 tool buttons display readable text labels replacing colored bars

**Independent Test**: Launch editor, verify every tool button shows its name ("Select", "Terrain", "Special", etc.)

### Implementation for User Story 1

- [ ] T009 [US1] Add tool name string mapping (EditorTool -> const char*) in src/ui/ToolPanel.cpp — "Select", "Terrain", "Special", "Flags", "City", "Unit", "Node", "Lair", "Tower", "Fortress", "Erase", "Settings", "Wizards"
- [ ] T010 [US1] Replace draw_label_bar() calls with draw_text() for each tool button in ToolPanel::render() in src/ui/ToolPanel.cpp
- [ ] T011 [US1] Add selected swatch name label above terrain palette grid in ToolPanel::render() in src/ui/ToolPanel.cpp — show name of currently selected BaseTerrain
- [ ] T012 [US1] Add selected swatch name label above special palette grid in ToolPanel::render() in src/ui/ToolPanel.cpp — show name of currently selected TerrainSpecial
- [ ] T013 [US1] Add selected swatch name label above flag palette in ToolPanel::render() in src/ui/ToolPanel.cpp — show "Road", "Ench. Road", or "Corruption"

**Checkpoint**: All tool buttons show text. Swatch palettes show selected name. Desktop build verified.

---

## Phase 4: User Story 2 - Status Bar Shows Text (Priority: P2)

**Goal**: Status bar displays coordinates, plane name, and zoom level as readable text

**Independent Test**: Hover over map tiles, switch planes, zoom in/out — verify status bar text updates

### Implementation for User Story 2

- [ ] T014 [US2] Rewrite UIRenderer::draw_status_bar() in src/renderer/UIRenderer.cpp — replace colored blocks with draw_text() calls for "X: N Y: N", "Arcanus"/"Myrror", and "Zoom: N.Nx"
- [ ] T015 [US2] Update UIRenderer::draw_status_bar() signature in src/renderer/UIRenderer.hpp if needed to accept Renderer& for draw_text access
- [ ] T016 [US2] Rewrite UIRenderer::draw_status_message() in src/renderer/UIRenderer.cpp — render the message string as text instead of orange bar

**Checkpoint**: Status bar shows readable coordinates, plane, and zoom. Status messages display as text.

---

## Phase 5: User Story 3 - Properties Panel Shows Labels (Priority: P3)

**Goal**: Properties panel displays labeled terrain types, entity properties, and metadata as text

**Independent Test**: Select tiles and entities, verify property names and values display as text

### Implementation for User Story 3

- [ ] T017 [US3] Add terrain type name lookup (BaseTerrain -> const char*) in src/ui/PropertiesPanel.cpp — "Ocean", "Grassland", "Forest", etc.
- [ ] T018 [P] [US3] Add race name lookup (race index -> const char*) in src/ui/PropertiesPanel.cpp — "Barbarian", "Beastmen", "Dark Elf", etc.
- [ ] T019 [US3] Replace colored bars with text labels for tile info section in PropertiesPanel::render() in src/ui/PropertiesPanel.cpp — "Terrain: Grassland", "Special: Gold", "Flags: Road"
- [ ] T020 [US3] Replace colored bars with text labels for city properties in PropertiesPanel::render() in src/ui/PropertiesPanel.cpp — "Race: High Men", "Pop: 5", "Buildings: 3"
- [ ] T021 [P] [US3] Replace colored bars with text labels for unit, node, tower, fortress, and lair sections in PropertiesPanel::render() in src/ui/PropertiesPanel.cpp
- [ ] T022 [US3] Replace colored bars with text labels for metadata section in render_metadata_section() in src/ui/PropertiesPanel.cpp — "Title: ...", "Author: ...", "Description: ..."

**Checkpoint**: Properties panel shows all labels as text. Metadata section shows labeled fields.

---

## Phase 6: User Story 4 - Map Generation Control Labels (Priority: P4)

**Goal**: Map generation controls display percentage, roughness names, and "Generate" as text

**Independent Test**: View map generation section, verify all controls have text labels

### Implementation for User Story 4

- [ ] T023 [US4] Add "Land: XX%" text label on land proportion bar in ToolPanel::render() in src/ui/ToolPanel.cpp
- [ ] T024 [P] [US4] Add "Smooth", "Normal", "Rough" text labels on roughness selector blocks in ToolPanel::render() in src/ui/ToolPanel.cpp
- [ ] T025 [P] [US4] Add "Continents: N" text label for continent count in ToolPanel::render() in src/ui/ToolPanel.cpp
- [ ] T026 [US4] Add "Generate" text label on generate button in ToolPanel::render() in src/ui/ToolPanel.cpp

**Checkpoint**: All map generation controls have readable labels.

---

## Phase 7: Polish & Cross-Cutting Concerns

**Purpose**: Build verification across both platforms

- [ ] T027 Build desktop (MSVC) and verify all text renders correctly — run existing tests with ctest
- [ ] T028 Build WASM (Emscripten) and verify text renders in browser
- [ ] T029 Verify font fallback: rename/remove font file, confirm editor launches with colored-bar fallback
- [ ] T030 Run quickstart.md validation checklist

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies — can start immediately
- **Foundational (Phase 2)**: Depends on Setup — BLOCKS all user stories
- **User Stories (Phases 3-6)**: All depend on Foundational phase completion
  - User stories can proceed sequentially in priority order (P1 -> P2 -> P3 -> P4)
  - US3 and US4 touch different files and could run in parallel after US1
- **Polish (Phase 7)**: Depends on all user stories being complete

### User Story Dependencies

- **US1 (P1)**: Can start after Foundational (Phase 2) — no dependencies on other stories
- **US2 (P2)**: Can start after Foundational (Phase 2) — independent of US1 (different files: UIRenderer vs ToolPanel)
- **US3 (P3)**: Can start after Foundational (Phase 2) — independent (PropertiesPanel.cpp)
- **US4 (P4)**: Depends on US1 completion (same file: ToolPanel.cpp — avoids merge conflicts)

### Within Each User Story

- Name lookup arrays before render code that uses them
- Core labels before polish labels
- Story complete before moving to next priority

### Parallel Opportunities

- T002 and T003 can run in parallel (CMakeLists changes, but different sections)
- T004 and T005 are sequential (header before impl)
- T011, T012, T013 can run in parallel (different palette sections, same file but independent code blocks)
- T017 and T018 can run in parallel (different lookup arrays)
- T023, T024, T025 touch different control sections and could be parallelized
- US1, US2, and US3 modify different files and can run in parallel

---

## Parallel Example: After Foundational Phase

```text
# These three stories can run in parallel (different files):
Story US1: ToolPanel.cpp (tool button labels)
Story US2: UIRenderer.cpp (status bar text)
Story US3: PropertiesPanel.cpp (property labels)

# Then US4 follows US1 (same file: ToolPanel.cpp)
Story US4: ToolPanel.cpp (map gen labels) — after US1 complete
```

---

## Implementation Strategy

### MVP First (User Story 1 Only)

1. Complete Phase 1: Setup (font + CMake)
2. Complete Phase 2: Foundational (draw_text in Renderer)
3. Complete Phase 3: User Story 1 (tool button labels)
4. **STOP and VALIDATE**: Build desktop, verify text on buttons
5. Deploy/demo if ready

### Incremental Delivery

1. Setup + Foundational -> Text rendering works
2. Add US1 -> Tool buttons labeled -> Build + verify (MVP!)
3. Add US2 -> Status bar has text -> Build + verify
4. Add US3 -> Properties panel labeled -> Build + verify
5. Add US4 -> Map gen controls labeled -> Build + verify
6. Polish -> Both platforms verified, fallback tested

---

## Notes

- [P] tasks = different files, no dependencies
- [Story] label maps task to specific user story for traceability
- Each user story is independently completable and testable
- Commit after each phase or logical group
- Stop at any checkpoint to validate story independently
- All changes are Presentation layer only — MomData library untouched
- Existing tests (test_MomGamFile, test_Scenario, test_UndoStack, test_MapGenerator, test_SmoothingValidator) should pass unchanged
