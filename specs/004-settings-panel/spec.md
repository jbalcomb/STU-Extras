# Feature Specification: Settings Panel Labels & Documentation

**Feature Branch**: `004-settings-panel`
**Created**: 2026-03-10
**Status**: Draft
**Input**: Add text labels and gameplay documentation to the Settings (Game Options) panel. The panel currently renders 4 game options as unlabeled colored shapes. Each setting needs a text label, the current selected value name, and a brief description of what the setting does in Master of Magic gameplay.

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Each Setting Has a Label and Current Value (Priority: P1)

A scenario designer opens the Settings panel and sees four clearly labeled sections: "Difficulty", "Magic", "Land Size", and "Wizards". Each section shows the setting name above the selector, and the currently selected value is displayed as text (e.g., "Normal", "Powerful", "Medium", "4 Wizards"). The designer can immediately understand what each setting controls and what value is selected.

**Why this priority**: Without labels, the colored shapes are meaningless to anyone unfamiliar with the original game's options screen. This is the minimum viable improvement.

**Acceptance Scenarios**:

1. **Given** the SETTINGS tool is active, **When** the panel renders, **Then** each of the 4 settings displays a text label naming the setting.
2. **Given** the SETTINGS tool is active, **When** the panel renders, **Then** each setting displays its currently selected value as readable text.
3. **Given** the user clicks a setting to change it, **When** the value cycles, **Then** the displayed value name updates immediately.

---

### User Story 2 - Difficulty Setting Is Documented (Priority: P2)

The Difficulty selector shows 5 labeled options: Intro, Easy, Normal, Hard, Impossible. A brief description explains that difficulty affects AI aggressiveness, AI resource bonuses, and the overall challenge level. The currently selected difficulty is visually highlighted and named.

**Why this priority**: Difficulty is the most impactful gameplay setting and the most commonly changed.

**Acceptance Scenarios**:

1. **Given** the panel is visible, **When** the user views the Difficulty section, **Then** they see the label "Difficulty" and the current value name (e.g., "Normal").
2. **Given** the user clicks a difficulty option, **When** the value changes, **Then** the selected option is highlighted and the value name updates.
3. **Given** any difficulty value 0-4 is set, **When** rendered, **Then** the correct name is displayed: Intro (0), Easy (1), Normal (2), Hard (3), Impossible (4).

---

### User Story 3 - Magic Level Setting Is Documented (Priority: P2)

The Magic selector shows 4 levels with descriptive names: Weak, Normal, Powerful, Overwhelming. A brief description explains that magic level affects the frequency and power of magical nodes, spell availability, and the prevalence of magical events in the game world.

**Why this priority**: Magic level significantly changes game balance and is not self-explanatory from the original colored bar.

**Acceptance Scenarios**:

1. **Given** the panel is visible, **When** the user views the Magic section, **Then** they see the label "Magic" and the current value name.
2. **Given** magic level is 0, **When** rendered, **Then** "Weak" is displayed. Level 1 shows "Normal", 2 shows "Powerful", 3 shows "Overwhelming".

---

### User Story 4 - Land Size Setting Is Documented (Priority: P2)

The Land Size selector shows 3 options: Small, Medium, Large. A brief description explains that land size controls the ratio of land to ocean on the generated map, affecting how spread out or compact the playable territory is.

**Why this priority**: Land size is a map generation parameter that directly affects the scenario being created.

**Acceptance Scenarios**:

1. **Given** the panel is visible, **When** the user views the Land Size section, **Then** they see the label "Land Size" and the current value name (Small, Medium, or Large).
2. **Given** any land size value 0-2 is set, **When** rendered, **Then** the correct name is displayed: Small (0), Medium (1), Large (2).

---

### User Story 5 - Wizard Count Setting Is Documented (Priority: P2)

The Wizard Count selector shows values 1 through 6 as clickable dots. A brief description explains that this sets the total number of wizards in the scenario (1 human player plus 0-5 AI opponents). The current count is displayed as text.

**Why this priority**: Wizard count determines the number of entities that need to be configured in the Wizards panel and is a fundamental scenario parameter.

**Acceptance Scenarios**:

1. **Given** the panel is visible, **When** the user views the Wizards section, **Then** they see the label "Wizards" and the current count as text (e.g., "4 Wizards").
2. **Given** wizard count is 1, **When** rendered, **Then** "1 Wizard" is displayed (singular).
3. **Given** the user clicks to change wizard count, **When** the value cycles 1-6, **Then** the text updates to match.

---

### User Story 6 - Click Targets Match Visual Layout (Priority: P1)

When the user clicks on any setting in the panel, the click is handled by the correct setting. The clickable regions align precisely with the rendered labels and selectors — no off-by-one errors or misaligned hit zones.

**Why this priority**: Broken click handling makes the entire panel unusable. This was a known issue class in prior UI work (font rendering feature).

**Acceptance Scenarios**:

1. **Given** the user clicks on the Difficulty row, **When** the click is processed, **Then** the Difficulty value changes (not Magic or another setting).
2. **Given** the user clicks on any of the 4 settings, **When** the click is processed, **Then** only the intended setting changes.
3. **Given** the panel has been resized or reflowed due to added labels, **When** the user clicks, **Then** click Y-offsets match the rendered layout.

---

## Functional Requirements *(mandatory)*

### FR-1: Setting Labels
Each of the 4 settings must display a text label above or beside its selector: "Difficulty", "Magic", "Land Size", "Wizards".

### FR-2: Current Value Display
Each setting must display the currently selected value as readable text:
- Difficulty: "Intro", "Easy", "Normal", "Hard", "Impossible"
- Magic: "Weak", "Normal", "Powerful", "Overwhelming"
- Land Size: "Small", "Medium", "Large"
- Wizards: "N Wizard(s)" where N is 1-6

### FR-3: Setting Descriptions
Each setting section must include a brief one-line description of what the setting affects:
- Difficulty: "AI aggressiveness and bonuses"
- Magic: "Magical node power and spell frequency"
- Land Size: "Land-to-ocean ratio on the map"
- Wizards: "Total players (1 human + AI opponents)"

### FR-4: Click-Render Alignment
The handle_click() Y-coordinate tracking must match the render() layout exactly. Any changes to vertical spacing, label heights, or separator gaps must be reflected in both render and click handling.

### FR-7: Tool Button Toggle
Clicking the SETTINGS tool button while the Settings panel is already open must close the panel (deselect back to SELECT). This applies to all tool buttons — clicking the active tool deselects it. This provides consistent UX: tools with panels (Settings, Wizards) and tools without panels (terrain brushes) all behave the same way.

### FR-5: Undo Support Preserved
All value changes must continue to go through the UndoStack (existing behavior must not regress).

### FR-6: Value Persistence
Changed settings must persist when saving to .GAM or .mom_scenario format (existing behavior must not regress).

---

## Success Criteria *(mandatory)*

1. A new user can identify all 4 settings and their current values without prior knowledge of Master of Magic.
2. Each setting displays a human-readable name and brief explanation of its gameplay effect.
3. Clicking any setting changes only that setting's value.
4. All settings render correctly in both desktop and WASM builds.
5. Undo/redo continues to work for all setting changes.

---

## Scope & Boundaries *(mandatory)*

### In Scope
- Text labels for all 4 existing settings
- Current value display as named text
- Brief gameplay descriptions for each setting
- Click handling alignment with new layout
- Desktop and WASM rendering

### Out of Scope
- Adding new game options beyond the existing 4
- Tooltip hover behavior (may be added later)
- Keyboard navigation for settings
- Direct numeric input (settings continue to cycle on click)
- Changes to the GameData struct or .GAM binary format

---

## Assumptions *(mandatory)*

1. The existing `Renderer::draw_text()` method provides sufficient text rendering capability (confirmed working from 003-font-rendering).
2. The Cousine-Regular.ttf font at 12pt provides readable text at the panel width of 200px.
3. The panel has sufficient vertical space below y=280 to accommodate labels, descriptions, and selectors without scrolling.
4. The magic level names (Weak/Normal/Powerful/Overwhelming) match the original Master of Magic game terminology.
5. The existing click-cycles-through-values interaction pattern is acceptable; no dropdowns or radio buttons are needed.

---

## Dependencies *(optional)*

- **003-font-rendering**: SDL_ttf integration and `draw_text()` must be functional (completed).
- **GameData struct**: Must remain binary-compatible with .GAM format (no struct changes needed for this feature).

---

## Key Entities *(optional)*

| Entity | Description | Values |
|--------|-------------|--------|
| Difficulty | AI challenge level | 0=Intro, 1=Easy, 2=Normal, 3=Hard, 4=Impossible |
| Magic | Magical power level | 0=Weak, 1=Normal, 2=Powerful, 3=Overwhelming |
| Land_Size | Map land ratio | 0=Small, 1=Medium, 2=Large |
| Total_Wizards | Player count | 1-6 (1 human + 0-5 AI) |
