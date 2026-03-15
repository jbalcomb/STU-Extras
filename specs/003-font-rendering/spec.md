# Feature Specification: UI Font Rendering

**Feature Branch**: `003-font-rendering`
**Created**: 2026-03-09
**Status**: Draft
**Input**: User description: "Add SDL_ttf font rendering to the scenario editor UI. Replace all placeholder colored bars/blocks with readable text labels on tool buttons, property panel sections, status bar indicators, and map generation controls. Must work on both desktop (MSVC/SDL2) and WASM (Emscripten) builds."

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Tool Buttons Have Readable Labels (Priority: P1)

A user opens the scenario editor and sees the left-side tool panel with clearly labeled buttons: "Select", "Terrain", "Special", "Flags", "City", "Unit", "Node", "Lair", "Tower", "Fortress", "Erase", "Settings", and "Wizards". Each button displays its name as text so the user knows what tool they are selecting without relying on color-coded bars.

**Why this priority**: Tool identification is the most fundamental UX need. Without labels, users cannot discover or distinguish tools at all.

**Independent Test**: Can be fully tested by launching the editor and visually confirming every tool button displays its name. Delivers immediate usability for all users.

**Acceptance Scenarios**:

1. **Given** the editor is launched, **When** the user views the tool panel, **Then** all 13 tool buttons display readable text labels.
2. **Given** a tool button is selected, **When** the user views the panel, **Then** the selected button's label remains readable against its highlighted background.
3. **Given** the editor is running in a browser (WASM build), **When** the user views the tool panel, **Then** text labels render identically to the desktop build.

---

### User Story 2 - Status Bar Shows Coordinates, Plane, and Zoom (Priority: P2)

A user working in the editor sees a status bar at the bottom displaying the cursor's tile coordinates (e.g., "X: 30 Y: 25"), the current plane name ("Arcanus" or "Myrror"), and the current zoom level (e.g., "Zoom: 1.5x"). This replaces the colored block placeholders with meaningful numeric and textual information.

**Why this priority**: Coordinate and zoom feedback is essential for precise map editing. Users currently cannot read any of this information.

**Independent Test**: Can be tested by hovering over different map tiles, switching planes, and zooming in/out, then confirming the status bar text updates correctly.

**Acceptance Scenarios**:

1. **Given** the cursor is over a valid map tile, **When** the user views the status bar, **Then** tile coordinates are displayed as readable text.
2. **Given** the user switches planes, **When** the status bar updates, **Then** the plane name ("Arcanus" or "Myrror") is displayed as text.
3. **Given** the user changes zoom level, **When** the status bar updates, **Then** the zoom multiplier is displayed as text (e.g., "1.0x", "2.0x").
4. **Given** the cursor is off the map, **When** the user views the status bar, **Then** no coordinates are shown.

---

### User Story 3 - Properties Panel Shows Labeled Information (Priority: P3)

A user selects a tile or entity and sees the properties panel on the right with labeled fields: terrain type name, special resource name, flag indicators, and entity properties (city race, population, unit type, node type, etc.) displayed as readable text instead of colored bars.

**Why this priority**: Property editing requires knowing what values are being shown and changed. Colored bars give no semantic information.

**Independent Test**: Can be tested by selecting different tiles and entities, then confirming each property displays its name and value as readable text.

**Acceptance Scenarios**:

1. **Given** a tile is under the cursor, **When** the properties panel renders, **Then** the terrain type name (e.g., "Grassland", "Ocean") is displayed as text.
2. **Given** a city is selected, **When** the properties panel renders, **Then** the race name and population value are displayed as text labels.
3. **Given** no entity is selected, **When** the properties panel renders, **Then** the scenario metadata section shows "Title", "Author", and "Description" as labeled text.

---

### User Story 4 - Map Generation Controls Have Labels (Priority: P4)

A user sees the map generation section of the tool panel with labeled controls: "Land: 40%", roughness level labels ("Smooth", "Normal", "Rough"), continent count, and a "Generate" button with readable text.

**Why this priority**: Map generation is a key workflow but less frequently used than basic tool selection. Labels make the controls self-documenting.

**Independent Test**: Can be tested by viewing the map generation section and confirming all controls display descriptive text labels.

**Acceptance Scenarios**:

1. **Given** the map generation controls are visible, **When** the user views the land proportion bar, **Then** the current percentage is displayed as text (e.g., "Land: 40%").
2. **Given** the roughness selector is visible, **When** the user views it, **Then** each option is labeled ("Smooth", "Normal", "Rough").
3. **Given** the generate button is visible, **When** the user views it, **Then** it displays the text "Generate".

---

### Edge Cases

- What happens when the window is very small and text labels would overflow button boundaries? Text should be clipped to the button area.
- What happens when the font file cannot be loaded at startup? The editor should fall back to the existing colored-bar rendering so the application remains usable.
- What happens when status message text is very long? It should be truncated or clipped to fit the available display area.

## Clarifications

### Session 2026-03-09

- Q: Do existing color indicators stay alongside new text labels, or get replaced? → A: Keep color indicators for swatches (terrain/special/flag palettes), text-only for buttons and status bar.
- Q: Do terrain/special/flag swatch palettes get individual text labels? → A: Show the name of the currently selected swatch as a label near the palette grid (not on each individual swatch).
- Q: What font size target for UI text? → A: 12px — fits in 20px buttons with padding, readable at normal viewing distance.

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: System MUST load a TrueType font at startup for use in all UI text rendering.
- **FR-002**: System MUST render text-only labels on all 13 tool panel buttons (replacing the colored bars).
- **FR-003**: System MUST render cursor coordinates, plane name, and zoom level as text in the status bar (replacing the colored block placeholders).
- **FR-004**: System MUST render property labels and values as text in the properties panel (terrain type, entity properties, metadata fields).
- **FR-010**: System MUST display the name of the currently selected swatch as a text label near each palette grid (terrain, special, flag) rather than labeling individual swatches.
- **FR-005**: System MUST render labels on map generation controls (land percentage, roughness names, "Generate" button).
- **FR-006**: System MUST render status notification messages as readable text (replacing the orange bar placeholder).
- **FR-007**: System MUST work on both desktop (native SDL2) and browser (Emscripten/WASM) builds without platform-specific font rendering code in UI components.
- **FR-008**: System MUST fall back to the existing colored-bar rendering if the font fails to load, so the application remains usable.
- **FR-009**: Text MUST be readable against both the default button background and the selected/highlighted button background.
- **FR-011**: UI text MUST render at 12px size, fitting comfortably within the 20px button height and 24px status bar height.

### Key Entities

- **Font**: A loaded TrueType font resource with a specific point size, used to render text to the screen.
- **Text Label**: A string rendered at a screen position with a specified color, associated with a UI element (button, status indicator, property field).

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: All 13 tool buttons display readable text labels that a user can identify without prior knowledge of the color coding.
- **SC-002**: Status bar displays tile coordinates, plane name, and zoom level as text that updates in real time as the user interacts.
- **SC-003**: Properties panel displays terrain type names, entity property names and values, and metadata field labels as readable text.
- **SC-004**: Map generation controls display percentage, roughness names, and "Generate" as readable text.
- **SC-005**: The editor builds and runs correctly on both desktop (MSVC) and browser (Emscripten WASM) without font-related errors.
- **SC-006**: If the font file is missing or corrupt, the editor launches and remains functional using fallback rendering.
