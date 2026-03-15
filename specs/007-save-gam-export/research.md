# Research: Save .GAM File Export

**Feature**: 007-save-gam-export
**Date**: 2026-03-14

## Decision 1: Export Trigger Mechanism

**Decision**: Use an `export_gam_requested` flag in EditorState, set by the ToolPanel button click handler and consumed by the main event loop — identical to the existing `generate_requested` pattern.

**Rationale**: This is the established pattern for deferred action buttons in the codebase (`generate_requested` for map generation, `generate_wizards_requested` for wizard generation). The button sets a flag, the main loop handles it on the next frame. This keeps the ToolPanel free of business logic and avoids passing Scenario/MomGamFile dependencies into the UI layer.

**Alternatives considered**:
- Direct export from ToolPanel click handler: Rejected — would require passing Scenario reference into ToolPanel and violates the existing event-flag pattern.
- Dedicated export function called from both button and Ctrl+E: This is the plan — refactor the inline Ctrl+E code into a shared helper, called from both paths.

## Decision 2: Status Message Content

**Decision**: The export status message combines filename, entity counts, and warnings in a single line. Format: `"Exported scenario.GAM (W wizards, C cities, U units)"`. If zero wizards, append warning: `"— no active wizards"`. If smoothing violations exist, append: `"— N violations"`.

**Rationale**: The existing status bar is a single-line text display with a 3-second timer. Multiple messages would overwrite each other. A combined message gives the user all relevant information at a glance.

**Alternatives considered**:
- Separate status messages for filename and counts: Rejected — second message would overwrite the first.
- Modal dialog showing export summary: Rejected — over-engineered for a simple export action (Principle V: Simplicity First).

## Decision 3: Button Placement

**Decision**: Place the "Export .GAM" button in the ToolPanel, below the existing "Gen Wizards" button, using the same visual style (colored rectangle with text label) but with a distinct color (green tint: 60, 140, 80) to distinguish it from generation actions.

**Rationale**: The ToolPanel already houses action buttons (Generate map, Gen Wizards). Export is a global action that applies regardless of the active tool, making ToolPanel the natural location. A green color distinguishes "export/save" from "generate" (which use blue/purple).

**Alternatives considered**:
- Menu bar: No menu bar exists in the editor — would require new infrastructure.
- Keyboard-only (status quo): Ctrl+E exists but is not discoverable.

## Decision 4: Refactoring Existing Ctrl+E Code

**Decision**: Extract the inline Ctrl+E export code from main.cpp into a `do_export_gam()` helper function. Both the Ctrl+E handler and the new button handler call this shared function.

**Rationale**: The existing Ctrl+E code (approximately 8 lines: count entities, serialize, save, set status) would be duplicated if the button handler were written independently. A shared helper keeps the logic DRY and ensures both paths produce identical behavior.

**Alternatives considered**:
- Leave Ctrl+E code inline and duplicate for button: Rejected — code duplication with divergence risk.
