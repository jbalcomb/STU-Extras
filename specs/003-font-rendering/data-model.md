# Data Model: UI Font Rendering

**Branch**: `003-font-rendering` | **Date**: 2026-03-09

## Overview

This feature adds no new persistent data entities. All font-related state is runtime-only in the Presentation layer, consistent with Constitution Principle II (Layer Separation).

## Runtime Entities (Presentation Layer Only)

### Font Resource

Held by `Renderer`. Loaded at init, destroyed at shutdown.

- `TTF_Font* font_` — loaded TrueType font handle (12px)
- `bool font_available_` — false if font failed to load (enables fallback)

### Text Cache

Held by `Renderer`. Maps rendered strings to GPU textures.

- Key: `std::string` (the text content)
- Value: `SDL_Texture*` (the rendered text texture)
- Lifecycle: Created on first render of a string, destroyed when cache is cleared or Renderer shuts down
- Invalidation: Dynamic text (coordinates, zoom) re-renders only when value changes

## No Data Layer Changes

- No changes to `MomData` library
- No changes to `.GAM` binary format
- No changes to `.mom_scenario` JSON format
- No new entity structs or packed data
- All existing tests remain unchanged
