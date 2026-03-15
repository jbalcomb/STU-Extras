# Research: Wizards Panel Labels & Documentation

**Feature**: 005-wizards-panel | **Date**: 2026-03-11

## Decision 1: Spell Realm Display Order

**Decision**: Display spell ranks in struct order: Nature (index 0), Sorcery (1), Chaos (2), Life (3), Death (4).

**Rationale**: The current code uses a remapping array `realm_to_idx[] = {3, 4, 2, 0, 1}` to display as Life, Death, Chaos, Nature, Sorcery. Per clarification, this "New Game screen" order is not canonical. The struct order is the correct order for the editor. Removing the remapping simplifies the code.

**Alternatives Considered**:
- Game-canonical order (Life, Death, Chaos, Nature, Sorcery) — rejected; specific to New Game UI code area.
- Alphabetical — rejected; no precedent in MoM.

**Implementation Impact**: Remove `realm_to_idx` remapping array from both `render()` and `handle_click()`. Reorder `REALM_COLORS` array to match struct order: Nature, Sorcery, Chaos, Life, Death.

## Decision 2: Text Rendering Approach

**Decision**: Use existing `Renderer::draw_text()` method, same approach as 004-settings-panel.

**Rationale**: Proven pattern from 004. Font rendering via SDL_ttf with texture caching. No per-frame penalty. Cousine-Regular.ttf at 12pt is readable at 200px panel width.

**Alternatives Considered**: None — the existing approach is well-tested.

## Decision 3: Retort Grid Layout

**Decision**: 3-column grid (6 rows x 3 columns), matching the original game's retort layout.

**Rationale**: The current code auto-calculates columns as `(btn_w) / (sq_sz + 2)` which yields ~10 columns with 16px squares. This makes retorts too small and unnamed. A 3-column layout with text labels provides space for retort names (~60px per column at 200px panel width).

**Alternatives Considered**:
- Single-column list (18 rows) — too tall, would overflow panel.
- 2-column grid (9 rows) — viable but doesn't match original game.
- Auto-calculated columns (current) — unusable without labels.

**Implementation Impact**: Replace dynamic `cols` calculation with fixed `cols = 3`. Retort squares will need to be smaller or the label placed beside/inside each toggle. Each cell needs: toggle indicator + retort name text.

## Decision 4: Panel Position

**Decision**: Change `PANEL_TOP` from 280 to 332 to match the fix applied in 004-settings-panel.

**Rationale**: The tool panel has 13 tools x 24px + 8px start + separator = 332px. PANEL_TOP=280 causes the panel to obscure the Settings and Wizards tool buttons. This was identified and fixed in 004.

**Alternatives Considered**: None — 332 is the correct value.

## Decision 5: Layout Constants

**Decision**: Add `LABEL_H = 14` and `DESC_H = 12` constants matching 004-settings-panel.

**Rationale**: Consistent text height constants across panels ensures uniform appearance. These values are proven readable with Cousine-Regular.ttf at 12pt.

**Alternatives Considered**: None — reuse established pattern.
