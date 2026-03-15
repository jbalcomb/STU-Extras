# Quickstart: Save .GAM File Export

**Feature**: 007-save-gam-export
**Date**: 2026-03-14

## Integration Scenario 1: Export via Button (Happy Path)

1. Launch editor, create some terrain
2. Generate wizards (click "Gen Wizards" button)
3. Click "Export .GAM" button in the tool panel
4. Desktop: Save dialog opens with default filename "scenario.GAM" — choose location and save
5. WASM: Browser triggers download of "scenario.GAM"
6. Status bar shows: "Exported scenario.GAM (6 wizards, 0 cities, 0 units)"
7. Verify: open the .GAM file in ReMoM — terrain and wizards appear correctly

## Integration Scenario 2: Export via Ctrl+E Shortcut

1. Launch editor, make some edits
2. Press Ctrl+E
3. Same export behavior as clicking the button (file dialog / download + status message)
4. Verify: both paths produce identical .GAM output

## Integration Scenario 3: Export Empty Scenario

1. Launch editor fresh (no edits)
2. Click "Export .GAM"
3. File dialog / download triggers
4. Status bar shows: "Exported scenario.GAM (0 wizards, 0 cities, 0 units) — no active wizards"
5. Verify: .GAM file is exactly 123,300 bytes

## Integration Scenario 4: Export with Smoothing Violations

1. Launch editor, paint some terrain with smoothing violations (e.g., shore tile with no adjacent ocean)
2. Click "Export .GAM"
3. Export proceeds (not blocked by violations)
4. Status bar shows: "Exported scenario.GAM (0 wizards, 0 cities, 0 units) — 3 violations"
5. Verify: .GAM file is valid despite violations
