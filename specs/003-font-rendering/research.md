# Research: UI Font Rendering

**Branch**: `003-font-rendering` | **Date**: 2026-03-09

## R1: Font File Selection

**Decision**: Use Cousine-Regular.ttf (Google Fonts, Apache 2.0 license)

**Rationale**: Available locally at `C:/devellib/imgui/misc/fonts/Cousine-Regular.ttf`. Monospace font, 43KB — small enough for WASM embedding without significant download impact. Apache 2.0 license is fully permissive for redistribution. Monospace is ideal for UI labels where consistent character width aids alignment.

**Alternatives considered**:
- Cantarell (sans-serif, larger, not monospace)
- Inconsolata (good but larger file)
- System fonts (not available in WASM builds)
- Bitmap font (custom rendering code, no SDL_ttf needed but more work, worse quality)

## R2: SDL_ttf Integration with Emscripten

**Decision**: Use Emscripten's SDL_ttf port (`-sUSE_SDL_TTF=2`) and embed font via `--embed-file`

**Rationale**: Emscripten provides SDL2_ttf as a built-in port, matching the native SDL2_ttf 2.x API. The font file is embedded into the WASM binary using `--embed-file assets/Cousine-Regular.ttf@/assets/Cousine-Regular.ttf`, making it available via the Emscripten virtual filesystem at `/assets/Cousine-Regular.ttf`. This avoids async network fetches and works offline.

**Native build**: Link against `C:/devellib/SDL2_ttf-2.22.0/lib/x64/SDL2_ttf.lib` and include from `C:/devellib/SDL2_ttf-2.22.0/include/`. Copy `SDL2_ttf.dll` to the build output directory.

**Alternatives considered**:
- Preload font via `--preload-file` (works but creates a separate .data file, more deploy complexity)
- Fetch font via XHR at runtime (requires async handling, fails offline)

## R3: Text Rendering Strategy

**Decision**: Render text to SDL_Texture via `TTF_RenderText_Blended()`, cache textures by string content

**Rationale**: `TTF_RenderText_Blended()` produces anti-aliased text with alpha blending — best visual quality. The returned SDL_Surface is converted to SDL_Texture once and cached. For static labels (tool button names), the texture is created once at init or first use and reused indefinitely. For dynamic text (coordinates, zoom), the texture is re-rendered only when the displayed value changes.

**Caching approach**: Store textures in an `std::unordered_map<std::string, SDL_Texture*>` keyed by the rendered string. Clear the cache when the font is destroyed. For dynamic text, compare the current string to the last rendered string — only re-render on change.

**Alternatives considered**:
- Render text every frame without caching (poor performance — TTF_RenderText allocates a surface each call)
- Pre-render all possible strings at startup (wastes memory for strings never displayed)
- Use SDL_ttf's `TTF_RenderText_Solid()` (faster but no anti-aliasing, looks jagged)

## R4: Font Loading and Fallback

**Decision**: Load font in `Renderer::init()`. Store a `bool font_available_` flag. All `draw_text()` calls check the flag and silently skip if false, preserving existing colored-bar rendering as fallback.

**Rationale**: The font is a presentation-layer resource loaded alongside the SDL window and renderer. If `TTF_OpenFont()` fails, the application logs a warning but continues. UI components call `draw_text()` unconditionally — the Renderer handles the fallback internally. This avoids `#ifdef HAS_FONT` guards throughout UI code.

**Font path resolution**:
- Native: `assets/Cousine-Regular.ttf` relative to executable, or absolute path
- Emscripten: `/assets/Cousine-Regular.ttf` (embedded in virtual filesystem)

## R5: Text Color for Readability

**Decision**: Use light gray (#E0E0E0) as the default text color for all UI labels. This provides sufficient contrast against the dark button backgrounds (rgb 50,50,55 unselected; rgb 70,130,180 selected) and dark panel backgrounds (rgb 45,45,50).

**Rationale**: Light text on dark backgrounds matches the existing editor aesthetic. A single default color keeps the implementation simple. Specific UI elements (status bar, properties) may use slightly different colors for emphasis but the default covers most cases.
