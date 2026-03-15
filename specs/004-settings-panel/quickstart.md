# Quickstart: Settings Panel Labels & Documentation

**Feature**: 004-settings-panel | **Date**: 2026-03-11

## What This Feature Does

Adds text labels, current value names, and brief gameplay descriptions to the 4 game settings in the Settings panel (Difficulty, Magic, Land Size, Wizards). Currently these are unlabeled colored shapes.

## File to Modify

`src/ui/GameOptionsPanel.cpp` — the only file that needs changes.

## Implementation Pattern

For each of the 4 settings, add three lines of text above the existing selector:

1. **Label** (e.g., "Difficulty") — rendered with `draw_text()` at default light gray
2. **Current value** (e.g., "Normal") — rendered beside or below the label
3. **Description** (e.g., "AI aggressiveness and bonuses") — rendered in a dimmer color

Then adjust Y-offsets in both `render()` and `handle_click()` to account for the added text height.

## Key APIs

- `renderer.draw_text(x, y, text, r, g, b)` — renders text at screen position with RGB color
- `UIRenderer::draw_label_bar()` — section header (already used for panel title)
- `UIRenderer::draw_separator()` — horizontal divider between sections

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

1. Open Settings panel (click SETTINGS tool)
2. Verify each setting shows: label, current value name, description
3. Click each setting — value name should update
4. Click Difficulty, then click Magic — only the intended setting changes
5. Undo/redo should still work for all settings
