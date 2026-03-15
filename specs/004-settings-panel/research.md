# Research: Settings Panel Labels & Documentation

**Feature**: 004-settings-panel | **Date**: 2026-03-11

## No Outstanding Unknowns

All technical context was resolved from the existing codebase:

### Decision 1: Text Rendering Approach

**Decision**: Use existing `Renderer::draw_text(x, y, text, r, g, b)` method.
**Rationale**: Already integrated via 003-font-rendering feature, uses SDL_ttf with texture caching (256 entries). No new dependencies needed.
**Alternatives considered**: Direct SDL_ttf calls (rejected: would bypass caching), bitmap font (rejected: lower quality, more work).

### Decision 2: Layout Strategy

**Decision**: Add label text and value text above each existing selector, adjusting Y offsets downward.
**Rationale**: The panel has ~300px of vertical space below y=280 (assuming a typical 768px window). Current content uses ~136px. Adding 3 lines of text per setting (~14px each with spacing) adds ~168px, totaling ~304px — fits within available space.
**Alternatives considered**: Inline labels beside selectors (rejected: 200px panel too narrow for side-by-side), collapsible sections (rejected: over-engineering per Principle V).

### Decision 3: Value Name Lookup

**Decision**: Static `const char*` arrays indexed by setting value, defined locally in `GameOptionsPanel.cpp`.
**Rationale**: Simplest approach. Values are fixed constants from the original game. No localization requirement.
**Alternatives considered**: Enum-to-string utility functions (rejected: unnecessary abstraction for 4 small arrays), external config file (rejected: over-engineering).

### Decision 4: Click-Render Alignment

**Decision**: Extract shared layout computation into a helper or use identical Y-tracking logic in both `render()` and `handle_click()`.
**Rationale**: The current code already uses matching Y-tracking in both methods. Adding labels requires updating both methods identically — same pattern, just more vertical offsets.
**Alternatives considered**: Layout struct computed once and passed to both (rejected: over-engineering for 4 settings).
