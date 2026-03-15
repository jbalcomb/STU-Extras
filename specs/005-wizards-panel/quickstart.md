# Quickstart: Wizards Panel Labels & Documentation

**Feature**: 005-wizards-panel | **Date**: 2026-03-11

## What This Feature Does

Adds text labels, current value names, and brief gameplay descriptions to all wizard configuration elements in the Wizards panel (tabs, race, portrait, spell ranks, retorts). Currently these are unlabeled colored shapes.

## File to Modify

`src/ui/WizardPanel.cpp` — the only file that needs changes.

## Implementation Pattern

Follow the same pattern established in 004-settings-panel:

1. **Add lookup tables** at the top of the file: `RACE_NAMES[14]`, `REALM_NAMES[5]`, `RETORT_NAMES[18]`, section descriptions
2. **Add layout constants**: `LABEL_H = 14`, `DESC_H = 12` (matching GameOptionsPanel)
3. **Fix PANEL_TOP**: Change from 280 to 332 (same fix as 004)
4. **For each section**, add text above the existing selector:
   - Label text (e.g., "Race: High Elves") via `renderer.draw_text()`
   - Description text in dimmer color (e.g., 160, 160, 160)
5. **Fix spell realm order**: Remove `realm_to_idx` remapping, display in struct order
6. **Reorder REALM_COLORS**: Match struct order [Nature, Sorcery, Chaos, Life, Death]
7. **Fix retort layout**: Change from auto-calculated columns to fixed 3-column grid
8. **Update handle_click()** Y-offsets to match new render() layout

## Key APIs

- `renderer.draw_text(x, y, text, r, g, b)` — renders text at screen position with RGB color
- `UIRenderer::draw_label_bar()` — section header (already used for panel title)
- `UIRenderer::draw_separator()` — horizontal divider between sections
- `wiz.is_active()` — checks if wizard has a name set
- `wiz.name` — char[20] wizard name

## Build & Verify

```bash
# Build WASM
bash build_wasm_stage.sh

# Build desktop (Windows)
cd build && cmake --build .

# Run tests (must pass — no new tests needed)
cd build && ctest --output-on-failure
```

## Acceptance Check

1. Open Wizards panel (click WIZARDS tool)
2. Verify each wizard tab shows the wizard's name or "Empty"
3. Verify Race section shows label "Race" and current race name (e.g., "Barbarians")
4. Verify Portrait section shows label "Portrait" and current index
5. Verify 5 spell rank bars show realm names (Nature, Sorcery, Chaos, Life, Death) and rank numbers
6. Verify 18 retort toggles show retort names in a 3-column grid
7. Click each element — only the intended element changes
8. Undo/redo should still work for all wizard configuration changes
