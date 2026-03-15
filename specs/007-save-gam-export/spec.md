# Feature Specification: Save .GAM File Export

**Feature Branch**: `007-save-gam-export`
**Created**: 2026-03-14
**Status**: Draft
**Input**: User description: "Save .GAM file export — allow the user to export/save the current scenario as a binary .GAM file (123,300 bytes) compatible with Master of Magic / ReMoM"

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Export Scenario as .GAM via UI Button (Priority: P1)

As a scenario designer, I want a visible "Export .GAM" button in the editor so I can save my scenario as a playable Master of Magic save file without memorizing keyboard shortcuts.

**Why this priority**: The core feature — making .GAM export discoverable. Currently export only exists as a hidden Ctrl+E shortcut, which many users will never discover.

**Independent Test**: Can be fully tested by clicking the export button and verifying the resulting .GAM file loads in ReMoM/Master of Magic.

**Acceptance Scenarios**:

1. **Given** a scenario with at least one wizard and terrain data, **When** the user clicks the "Export .GAM" button, **Then** a file save dialog opens (desktop) or a browser download triggers (WASM) with a default filename of `scenario.GAM`.
2. **Given** a scenario with no modifications since last export, **When** the user clicks "Export .GAM", **Then** the export proceeds normally (no "unsaved changes" prompt — export is always available).
3. **Given** a completely empty scenario (no wizards, default terrain), **When** the user clicks "Export .GAM", **Then** a valid 123,300-byte .GAM file is produced (empty scenarios are valid game files).

---

### User Story 2 - Pre-Export Validation Feedback (Priority: P2)

As a scenario designer, I want to see a status message confirming export success or warning about potential issues so I know the export completed and whether the file is game-ready.

**Why this priority**: Without feedback, users don't know if the export succeeded or if their scenario has issues that might cause problems in-game.

**Independent Test**: Can be tested by exporting and observing the status bar message, then verifying the message content matches the scenario state.

**Acceptance Scenarios**:

1. **Given** a scenario with active wizards and valid terrain, **When** the user exports to .GAM, **Then** the status bar shows "Exported scenario.GAM" for 3 seconds.
2. **Given** a scenario with no active wizards, **When** the user exports to .GAM, **Then** the status bar shows a warning such as "Exported scenario.GAM (0 active wizards)" to alert the user.
3. **Given** a scenario with smoothing violations present, **When** the user exports to .GAM, **Then** the export proceeds (violations don't block export) but the status message notes the violation count.

---

### User Story 3 - Entity Count Summary Before Export (Priority: P3)

As a scenario designer, I want to see a summary of what will be exported (wizard count, city count, unit count) so I can verify my scenario is complete before sharing the .GAM file.

**Why this priority**: Provides confidence that the scenario contains what the user expects. Less critical than the export itself but valuable for catching mistakes.

**Independent Test**: Can be tested by populating various entity types, triggering export, and verifying the summary counts match the scenario state.

**Acceptance Scenarios**:

1. **Given** a scenario with 3 active wizards, 5 cities, and 20 units, **When** the user exports to .GAM, **Then** the status message includes the entity counts (e.g., "Exported: 3 wizards, 5 cities, 20 units").
2. **Given** a scenario with no entities, **When** the user exports to .GAM, **Then** the status message shows zero counts for all entity types.

---

### Edge Cases

- What happens when the user cancels the file save dialog on desktop? Export is silently cancelled, no status message.
- What happens if the browser blocks the download (WASM pop-up blocker)? The download may fail silently; this is a browser limitation outside the app's control.
- What happens with maximum entity counts (all 100 cities, all 1009 units)? The .GAM format supports these counts natively; export should succeed without issue.
- What happens if the exported file is exactly 123,300 bytes? This is always the case — the .GAM format is fixed-size by definition.

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: System MUST provide a visible "Export .GAM" button in the tool panel that triggers .GAM file export.
- **FR-002**: System MUST produce a valid 123,300-byte binary .GAM file compatible with Master of Magic and ReMoM when export is triggered.
- **FR-003**: System MUST update active entity counts (cities, units, wizards) in the game data header before writing the .GAM buffer.
- **FR-004**: System MUST use the platform file dialog (desktop) or browser download (WASM) to deliver the exported file to the user.
- **FR-005**: System MUST display a status bar message confirming successful export, including the filename.
- **FR-006**: System MUST include entity count summary (wizards, cities, units) in the export status message.
- **FR-007**: System MUST continue to support the existing Ctrl+E keyboard shortcut for .GAM export alongside the new UI button.
- **FR-008**: System MUST use a default filename of "scenario.GAM" for the export dialog/download.
- **FR-009**: System MUST NOT block export based on validation warnings (smoothing violations, missing wizards) — export always succeeds if triggered.

### Key Entities

- **Scenario**: The complete editor state including world terrain, wizards, cities, units, nodes, lairs, towers, fortresses, and items. Serialized to a fixed 123,300-byte binary buffer.
- **GameData**: Header structure containing active entity counts (Total_Cities, Total_Units) that must be updated before export.
- **.GAM File**: Fixed-size binary save file matching the original Master of Magic 8086 struct layout with 2-byte alignment packing.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: Users can export a .GAM file in under 2 clicks from the main editor view (click button, confirm save dialog).
- **SC-002**: 100% of exported .GAM files are exactly 123,300 bytes and load successfully in ReMoM.
- **SC-003**: Export status message appears within 1 second of triggering export and displays for at least 3 seconds.
- **SC-004**: Entity counts shown in the export status message match the actual active entity counts in the scenario.

## Assumptions

- The existing `serialize_gam()` function and `platform::save_file()` infrastructure are correct and do not need modification — this feature wraps them with a UI button and status feedback.
- The "Export .GAM" button will be placed in the tool panel alongside other action buttons (Generate, Gen Wizards).
- No "Save As" workflow is needed — each export is a standalone file download/save operation.
- The existing Ctrl+E shortcut code in main.cpp will be refactored to share the same export logic as the new button.
