# Product Requirements Document: MoM Scenario Editor

**Product:** RemomMapgenItchio — Master of Magic (1994) Scenario Editor
**Status:** Phase 1 MVP Implemented
**Distribution:** Standalone SDL2 desktop app, future itch.io web build

---

## 1. Why This Product Exists

### The Problem

Master of Magic (1994) is a beloved 4X strategy game by SimTex/MicroProse that still
has an active community on the "Master of Magic — Fans" Discord server. The ReMoM
project (c:\STU\devel\STU-Extras\ReMoM) is reassembling the original game from 8086
assembly into portable C, making the engine modifiable for the first time in 30 years.

But there is no way for community members to create, customize, or share game setups.
Every new game uses the same random generation. Players who want to demonstrate a
strategy, teach a concept, or set up a specific challenge have no tools to do so.

Compare this to Warcraft II and III, where map editors spawned massive communities.
The Warcraft III World Editor alone created DotA — an entire genre born from user
scenarios. Master of Magic's community has the passion but lacks the tooling.

### The Opportunity

The ReMoM reassembly has now documented every data structure in the game: the 60x40
world map, city buildings, unit stats, wizard spells, lair contents, and the exact
binary layout of the .GAM save file (123,300 bytes). This means a tool can write
game-save files that the engine loads directly — no game-code modification needed.

A player can design a scenario in the editor, export a .GAM file, share it on Discord,
and another player drops it into their MoM folder and plays it. Zero friction.

### Who This Is For

1. **Scenario designers** — community members who want to craft starting positions,
   challenge maps, or training exercises for other players
2. **Strategy teachers** — players who want to demonstrate optimal opening builds,
   combat tactics, or spell usage by setting up exact game states
3. **Speedrun/challenge community** — players who want standardized starting
   conditions for competitive play
4. **Modders and developers** — ReMoM contributors who need to set up test scenarios
   for debugging game logic

### Success Metric

A community member with no programming knowledge can create a custom Master of Magic
starting position, export it as a .GAM file, and have another player load and play it
in under 10 minutes.

### Non-Negotiable Requirement

**The exported .GAM file MUST load and play correctly in Master of Magic (via ReMoM).**
This is the entire reason the product exists. Every terrain tile, city, unit, wizard,
node, and lair placed in the editor must appear exactly as configured when the game
loads the save. If the .GAM file doesn't work in the game, the product has zero value.
All struct sizes, field offsets, and byte layout must be verified against the ReMoM
source and tested with round-trip loading in the actual game engine.

---

## 2. What the Product Does

The MoM Scenario Editor lets users visually edit every aspect of a Master of Magic
game's initial state — terrain, cities, units, wizards, magical nodes, monster lairs —
then export that state as a .GAM save file that loads directly in the game.

### Core User Flow

```
 [1] Open editor          [2] Paint terrain        [3] Place entities
  New blank world   --->   Mountains, forests  ---> Cities, units, nodes
  or load existing          oceans, deserts          lairs, towers

 [4] Configure wizards    [5] Save / Export         [6] Share
  Spells, retorts,  --->   .mom_scenario (JSON) --> Discord, forums,
  gold, mana, race          .GAM (binary)            itch.io browser
```

### Three Scenario Types (Current + Future)

| Type | Description | Status |
|------|-------------|--------|
| **Sandbox** | Custom map + starting positions. Player plays freely from there. | Implemented |
| **Mission** | Scripted objectives, triggers, story text, win/lose conditions. | Future |
| **Puzzle** | Focused exercises: "win this combat," "grow this city to size 5." | Future |

---

## 3. Architecture

### Why This Architecture

The editor must produce files that are **byte-identical** to what the original game
engine expects. This is non-negotiable — a single misaligned struct field means
corrupted saves. The architecture reflects this constraint:

- **Data Layer** mirrors ReMoM's packed C structs byte-for-byte
- **Data Layer has zero SDL2 dependency** so it can be tested headlessly and
  potentially compiled to WASM for web distribution
- **Editor Layer** uses the Command pattern for undo/redo, keeping mutation logic
  separated from rendering
- **Renderer Layer** wraps SDL2 and can be swapped for Emscripten canvas later

### Layer Diagram

```
+-----------------------------------------------------------------------+
|                     PRESENTATION LAYER (SDL2)                         |
|  Renderer.hpp/cpp    MapRenderer.hpp/cpp    UIRenderer.hpp/cpp        |
|  ToolPanel.hpp/cpp   PropertiesPanel.hpp/cpp   Minimap.hpp/cpp        |
|                                                                       |
|  - SDL2 window management        - Terrain color rendering            |
|  - Camera (scroll + zoom)        - Entity icon rendering              |
|  - Screen<->World coordinate     - Tool palette UI                    |
|    transforms                    - Property inspector UI              |
+--------------------------------------|--------------------------------+
                                       | uses
+--------------------------------------|--------------------------------+
|                       EDITOR LAYER (no SDL2)                          |
|  EditorState.hpp/cpp               UndoStack.hpp/cpp                  |
|                                                                       |
|  - Current tool selection          - Command pattern (execute/undo)   |
|  - Cursor tracking                 - 50-level undo depth              |
|  - Entity selection state          - Redo stack with auto-clear       |
|  - Plane toggle (Arcanus/Myrror)                                      |
+--------------------------------------|--------------------------------+
                                       | mutates
+--------------------------------------|--------------------------------+
|                        DATA LAYER (pure C++)                          |
|  MomConstants.hpp    MomWorld.hpp/cpp     MomEntities.hpp/cpp          |
|  Scenario.hpp/cpp    ScenarioIO.hpp/cpp   MomGamFile.hpp/cpp          |
|                                                                       |
|  - Binary-compatible structs       - .mom_scenario JSON format        |
|  - 60x40x2 world grid             - .GAM binary read/write           |
|  - City/Unit/Wizard/Node/Lair     - base64 terrain encoding          |
|  - #pragma pack(2) alignment      - nlohmann/json serialization       |
+-----------------------------------------------------------------------+
```

### File Layout

```
RemomMapgenItchio/
  CMakeLists.txt                        # CMake 4.2+, C++17
  src/
    main.cpp                            # SDL2 entry + event loop (363 lines)
    mom_data/
      MomConstants.hpp                  # Game constants and enums
      MomWorld.hpp / .cpp               # 60x40x2 terrain grid
      MomEntities.hpp / .cpp            # Packed structs (City, Unit, Wizard, ...)
      MomGamFile.hpp / .cpp             # Binary .GAM reader/writer
    scenario/
      Scenario.hpp / .cpp               # Top-level container
      ScenarioIO.hpp / .cpp             # JSON serializer
    editor/
      EditorState.hpp / .cpp            # Tool/cursor/selection state
      UndoStack.hpp / .cpp              # Command-pattern undo/redo
    renderer/
      Renderer.hpp / .cpp               # SDL2 window + camera + draw primitives
      MapRenderer.hpp / .cpp            # Terrain + entity rendering
      UIRenderer.hpp / .cpp             # Panels, buttons, status bar
    ui/
      ToolPanel.hpp / .cpp              # Left-side tool palette
      PropertiesPanel.hpp / .cpp        # Right-side property inspector
      Minimap.hpp / .cpp                # Dual-plane minimap
  tests/
    test_MomGamFile.cpp                 # 15 tests: struct sizes, GAM round-trip
    test_Scenario.cpp                   # 13 tests: JSON round-trip
    test_UndoStack.cpp                  # 9 tests: undo/redo correctness
```

### Build Targets

| Target | Type | Dependencies | Purpose |
|--------|------|-------------|---------|
| `MomData` | Static library | nlohmann/json | Data + editor logic (no SDL2) |
| `RemomMapgenItchio` | Win32 executable | MomData, SDL2 | The editor app |
| `RemomTests` | Test executable | MomData, GoogleTest | Automated tests |

### Dependencies

| Dependency | Version | Source | Why |
|-----------|---------|--------|-----|
| SDL2 | 2.32.2 | `C:\devellib\SDL2-2.32.2` (local) | Cross-platform window/input/rendering |
| nlohmann/json | 3.11.3 | FetchContent (GitHub) | JSON serialization for .mom_scenario |
| GoogleTest | 1.17.0 | FetchContent (GitHub) | Unit testing framework |

---

## 4. The World Model

### Why These Specific Sizes

Every number below comes from the original Master of Magic executable. The editor
must use these exact sizes because the .GAM file format is a fixed-size binary dump
of the game's memory arrays. There is no header, no length prefix, no versioning —
just raw bytes at hard-coded offsets.

### World Grid

```
+-- Arcanus (Plane 0) --+    +--- Myrror (Plane 1) ---+
|                        |    |                         |
|    60 tiles wide       |    |    60 tiles wide        |
|    40 tiles high       |    |    40 tiles high        |
|    = 2,400 tiles       |    |    = 2,400 tiles        |
|                        |    |                         |
+------------------------+    +-------------------------+

Per tile:
  terrain:   uint16_t    (2 bytes)  -- tile type index
  specials:  int8_t      (1 byte)   -- mineral deposit / feature
  flags:     uint8_t     (1 byte)   -- roads, corruption
  landmass:  uint8_t     (1 byte)   -- continent ID
  explored:  uint8_t     (1 byte)   -- per-corner visibility bits
```

### Entity Arrays

| Entity | Struct Size | Array Size | Total Bytes | Key Fields |
|--------|------------|------------|-------------|------------|
| City | 114 bytes | 100 | 11,400 | name[14], race, x/y/plane, owner, population, buildings[36], enchantments[25] |
| Unit | 32 bytes | 1,009 | 32,288 | x/y/plane, owner, type, level, XP, enchantments(u32) |
| Wizard | 1,224 bytes | 6 | 7,344 | name[20], spellranks[5], 18 retorts, spells[240], gold, mana |
| Node | 48 bytes | 30 | 1,440 | x/y/plane, type(Nature/Sorcery/Chaos), power, aura |
| Lair | 24 bytes | 102 | 2,448 | x/y/plane, type, 2 guard groups, loot(gold/mana), items |
| Tower | 4 bytes | 6 | 24 | x/y, owner |
| Fortress | 4 bytes | 6 | 24 | x/y/plane, active |
| Item | 50 bytes | 138 | 6,900 | (opaque data in Phase 1) |

All structs use `#pragma pack(push, 2)` for 8086-compatible alignment.
Static assertions verify sizes at compile time.

### Terrain Types (Editor)

The original game uses 762 tile indices (encoding edge transitions between terrain
types). The editor simplifies this to 12 base terrain types. On export, an auto-tiling
pass would compute proper transition tiles (not yet implemented — current export writes
the base type index directly).

| Base Terrain | Index | Editor Color (RGB) |
|-------------|-------|--------------------|
| Ocean | 0 | 26, 82, 118 |
| Shore | 1 | 93, 173, 226 |
| Grassland | 2 | 39, 174, 96 |
| Forest | 3 | 30, 132, 73 |
| Mountain | 4 | 125, 102, 8 |
| Desert | 5 | 244, 208, 63 |
| Swamp | 6 | 108, 122, 0 |
| Tundra | 7 | 213, 216, 220 |
| Hill | 8 | 160, 82, 45 |
| River | 9 | 41, 128, 185 |
| Volcano | 10 | 180, 30, 30 |
| Lake | 11 | 52, 152, 219 |

### Terrain Specials (Mineral Deposits & Features)

| Special | Value | Editor Color |
|---------|-------|-------------|
| None | 0 | — |
| Iron | 1 | Gray |
| Coal | 2 | Black |
| Silver | 3 | White |
| Gold | 4 | Gold |
| Gems | 5 | Magenta |
| Mithril | 6 | Cyan |
| Adamantium | 7 | Purple |
| Quork Crystals | 8 | Pink |
| Crysx Crystals | 9 | Light Cyan |
| Wild Game | 64 | Brown |
| Nightshade | 128 | Dark Purple |

### Races

14 playable races: Barbarian, Beastman, Dark Elf, Draconian, Dwarf, Gnoll,
Halfling, High Elf, High Man, Klackon, Lizardman, Nomad, Orc, Troll.

### Player Banner Colors

| Player | Color | RGB |
|--------|-------|-----|
| 0 | Green | 0, 128, 0 |
| 1 | Blue | 0, 0, 200 |
| 2 | Red | 200, 0, 0 |
| 3 | Purple | 180, 0, 180 |
| 4 | Yellow | 200, 200, 0 |
| 5 | Dark | 30, 30, 30 |

---

## 5. User Interface

### Why This Layout

The three-panel layout (tool palette / map / properties) follows the convention
established by tile-map editors (Tiled, RPG Maker, Warcraft III World Editor). Users
of any map editor will immediately understand the spatial layout. The minimap occupies
dead space in the properties panel and shows both planes simultaneously so users can
maintain context when editing Myrror.

### Full Application Layout

```
+--[MoM Scenario Editor]----------- [_][O][X] ---+
|   File   Edit   View   Help                     |  <- Menu bar (future)
+---------+---------------------------+-----------+
|         |                           |           |
| TOOL    |    MAP VIEWPORT           | PROPERTY  |
| PALETTE |                           | PANEL     |
|         |    Scrollable, zoomable   |           |
| 200px   |    60x40 tile grid        | 250px     |
|         |    Color-coded terrain    |           |
| [Select]|    Entity overlays        | (selected |
| [Paint ]|    Grid lines (toggle)   |  entity   |
| [Spec. ]|    Cursor highlight       |  fields)  |
| [City  ]|                           |           |
| [Unit  ]|                           |           |
| [Node  ]|                           |           |
| [Lair  ]|                           +-----------+
| [Erase ]|                           | MINIMAP   |
|         |                           | +---+---+ |
| --------+                           | |Arc|Myr| |
| Terrain |                           | |   |   | |
| palette |                           | +---+---+ |
| (12     |                           |           |
| swatches|                           |           |
+---------+---------------------------+-----------+
| Coords: (25, 18)  | Plane: Arcanus | Zoom: 1.0x|  <- Status bar
+-------------------------------------------------+
```

### Tool Palette Detail (Left Panel, 200px)

```
+---Tool Panel (200px)---+
| +----+ +----+ +------+ |
| | Se | | Te | | Spec | |   Row 1: SELECT, PAINT_TERRAIN,
| | le | | rr | | ials | |          PAINT_SPECIAL
| +----+ +----+ +------+ |
| +----+ +----+ +------+ |
| | Ci | | Un | | Node | |   Row 2: PLACE_CITY, PLACE_UNIT,
| | ty | | it | |      | |          PLACE_NODE
| +----+ +----+ +------+ |
| +----+ +----+          |
| |Lair| | Er |          |   Row 3: PLACE_LAIR, ERASE
| |    | | as |          |
| +----+ +----+          |
|=========================|   <- separator
|                         |
| Terrain Swatches:       |   Shown when PAINT_TERRAIN active
| +--+ +--+ +--+ +--+   |
| |Oc| |Sh| |Gr| |Fo|   |   Row 1: Ocean, Shore, Grass, Forest
| +--+ +--+ +--+ +--+   |
| +--+ +--+ +--+ +--+   |
| |Mt| |De| |Sw| |Tu|   |   Row 2: Mountain, Desert, Swamp, Tundra
| +--+ +--+ +--+ +--+   |
| +--+ +--+ +--+ +--+   |
| |Hi| |Ri| |Vo| |La|   |   Row 3: Hill, River, Volcano, Lake
| +--+ +--+ +--+ +--+   |
|                         |
| Special Swatches:       |   Shown when PAINT_SPECIAL active
| +--+ +--+ +--+ +--+   |
| |Fe| |Co| |Si| |Au|   |   Iron, Coal, Silver, Gold
| +--+ +--+ +--+ +--+   |
| +--+ +--+ +--+ +--+   |
| |Ge| |Mi| |Ad| |QC|   |   Gems, Mithril, Adamantium, Quork
| +--+ +--+ +--+ +--+   |
| +--+ +--+ +--+ +--+   |
| |CC| |WG| |NS| |  |   |   Crysx, Wild Game, Nightshade, None
| +--+ +--+ +--+ +--+   |
+-------------------------+
```

Each swatch is a 22x22 colored rectangle. The active swatch has a white outline.
The active tool button is highlighted with a brighter background (rgb 70,130,180).

### Map Viewport Detail (Center)

```
+--- Map Viewport (variable width) ---+
|                                      |
|  +--+--+--+--+--+--+--+--+--+--+   |
|  |Oc|Oc|Sh|Gr|Gr|Fo|Fo|Mt|Mt|De|   |  Each cell = 1 tile
|  +--+--+--+--+--+--+--+--+--+--+   |  rendered at (24 * zoom) px
|  |Oc|Sh|Gr|Gr|[CITY]|Fo|Hi|Mt|De|   |
|  +--+--+--+--+-+--+-+--+--+--+--+   |  Cities: filled square
|  |Sh|Gr|Gr|Fo|Fo|Ri|Ri|Hi|De|De|   |     in banner color
|  +--+--+--+--+--+--+--+--+--+--+   |
|  |Gr|Gr|Fo|Fo|[u]|Ri|Hi|Hi|Sw|Sw|   |  Units: small centered
|  +--+--+--+--+--+--+--+--+--+--+   |     square in banner color
|  |Gr|Fo|Fo|[N]|Fo|Gr|Hi|Sw|Sw|Oc|   |
|  +--+--+--+--+--+--+--+--+--+--+   |  Nodes: colored square
|  |Fo|Fo|Gr|Gr|Gr|Gr|Sw|Oc|Oc|Oc|   |     (green/blue/red)
|  +--+--+--+--+--+--+--+--+--+--+   |     with white outline
|                                      |
|     [cursor] = white outline         |  Specials: small dot
|     on hovered tile                  |     in top-right corner
+--------------------------------------+
```

### Status Bar Detail (Bottom, 24px)

```
+----------------------------------------------------+
| [##][##]       [======Arcanus======]     [===1.0x]  |
| ^coord  ^coord  ^plane indicator         ^zoom      |
| (green)  (red)  (green=Arc, purple=Myr)  (blue bar) |
+----------------------------------------------------+
```

Note: Phase 1 uses colored blocks as placeholders for text labels. No font rendering
is implemented yet. The coordinate and plane indicators are colored rectangles that
convey information through color and size rather than text.

---

## 6. Interactions

### Why These Controls

The controls follow conventions from Tiled, Photoshop, and other 2D editors that
the target audience (PC gamers comfortable with modding tools) will already know.
Paint-on-drag is critical for terrain editing — clicking each tile individually
would make the 2,400-tile map tedious to populate.

### Mouse

| Input | Context | Action |
|-------|---------|--------|
| Left click | Tool panel | Select tool or palette swatch |
| Left click | Map viewport | Apply current tool at tile |
| Left drag | Map (paint/erase tools) | Continuous painting |
| Middle drag | Map viewport | Pan camera |
| Mouse wheel | Map viewport | Zoom (x1.15 per tick, range 0.3x–5.0x) |
| Right click | Map viewport | Deselect (future: context menu) |

### Keyboard

| Key | Action |
|-----|--------|
| Tab | Toggle between Arcanus and Myrror planes |
| G | Toggle grid lines on/off |
| Ctrl+Z | Undo (up to 50 levels) |
| Ctrl+Y | Redo |
| Ctrl+S | Save as `scenario.mom_scenario` (JSON) |
| Ctrl+O | Open `scenario.mom_scenario` (JSON) |
| Ctrl+E | Export as `scenario.GAM` (binary, playable) |
| Arrow keys | Pan camera (40px per press) |

### Tool Behaviors

| Tool | Left Click | Left Drag | Selection |
|------|-----------|-----------|-----------|
| SELECT | Select entity at tile | — | Highlights selected entity with yellow outline |
| PAINT_TERRAIN | Set tile to current terrain type | Continuous paint | — |
| PAINT_SPECIAL | Set tile's mineral deposit | Continuous paint | — |
| PLACE_CITY | Create city at tile (first empty slot) | — | — |
| PLACE_UNIT | Create unit at tile (first empty slot) | — | — |
| PLACE_NODE | Create mana node at tile | — | — |
| PLACE_LAIR | Create lair at tile | — | — |
| ERASE | Remove entity at tile | Continuous erase | — |

All mutations are wrapped in undo commands. PAINT_TERRAIN and PAINT_SPECIAL create
one undo entry per tile changed (while dragging, each new tile is a separate command).

### New Scenario Initialization

When the editor starts, it creates a blank scenario:
- All terrain set to Grassland
- All explored flags set to 0x0F (fully explored)
- Difficulty: Normal, Magic: 1, Land Size: 1, Wizards: 2
- All entity arrays zeroed (no cities, units, nodes, etc.)

---

## 7. File Formats

### Why Two Formats

**.mom_scenario (JSON)** is the editor's native format. It's human-readable, diff-able,
and stores metadata (title, author, tags) that the game engine doesn't need but the
community does for browsing and sharing. Only active entities are serialized (a
scenario with 3 cities stores 3 city objects, not 100).

**.GAM (binary)** is the game engine's save format. It's what the player actually loads
in Master of Magic. It must be exactly 123,300 bytes with every struct at the exact
byte offset the engine expects. This is the "delivery artifact."

### .mom_scenario JSON Schema

```json
{
  "format_version": 1,
  "scenario_type": "sandbox",

  "meta": {
    "title": "The Dark Continent",
    "author": "Rjak",
    "description": "Myrror start with Death magic focus",
    "version": "1.0.0",
    "tags": ["myrror", "death-magic", "advanced"]
  },

  "game_settings": {
    "difficulty": 2,
    "magic": 1,
    "land_size": 1,
    "num_wizards": 4
  },

  "world": {
    "width": 60,
    "height": 40,
    "planes": [
      {
        "name": "Arcanus",
        "terrain": "<base64: 9600 bytes, 2400 uint16>",
        "specials": "<base64: 2400 bytes>",
        "flags": "<base64: 2400 bytes>",
        "landmasses": "<base64: 2400 bytes>"
      },
      { "name": "Myrror", "...same..." }
    ]
  },

  "wizards": [
    {
      "id": 0, "name": "Rjak", "banner": 2,
      "capital_race": 2, "gold": 1000, "mana": 500,
      "fame": 42, "tax_rate": 2,
      "spellranks": [0, 0, 0, 0, 5],
      "retorts": { "myrran": 1, "archmage": 1, "...": "..." },
      "spells_b64": "<base64: 240 bytes>"
    }
  ],

  "cities": [
    {
      "name": "Mirion", "race": 7,
      "x": 25, "y": 18, "plane": 0, "owner": 0,
      "population": 8, "size": 2,
      "buildings": [1, 0, 0, 1, "...36 values..."]
    }
  ],

  "units": [
    {
      "type": 36, "x": 25, "y": 18, "plane": 0,
      "owner": 0, "level": 2, "xp": 150,
      "hero_slot": -1, "enchantments": 0
    }
  ],

  "nodes": [
    { "x": 30, "y": 10, "plane": 0, "owner": -1, "power": 8, "type": 1 }
  ],

  "lairs": [
    {
      "x": 15, "y": 22, "plane": 0, "type": 4, "intact": 1,
      "guard1_type": 50, "guard1_count": 3,
      "guard2_type": 0, "guard2_count": 0,
      "loot_gold": 200, "loot_mana": 50
    }
  ]
}
```

### .GAM Binary Layout

| Offset | Size (bytes) | Content | Struct |
|--------|-------------|---------|--------|
| 0 | 2,520 | Hero data | 35 * 12 * 6 |
| 2,520 | 16 | Game settings | GameData |
| 2,536 | 7,344 | Wizards | 6 * Wizard(1224) |
| 9,880 | 9,600 | World terrain | 2 * 2400 * uint16 |
| 19,480 | 384 | Unknown tables | UU_TBL_1 + UU_TBL_2 |
| 19,864 | 4,800 | Landmasses | 2 * 2400 * uint8 |
| 24,664 | 1,440 | Mana nodes | 30 * Node(48) |
| 26,104 | 24 | Fortresses | 6 * Fortress(4) |
| 26,128 | 24 | Towers | 6 * Tower(4) |
| 26,152 | 2,448 | Lairs | 102 * Lair(24) |
| 28,600 | 6,900 | Items | 138 * Item(50) |
| 35,500 | 11,400 | Cities | 100 * City(114) |
| 46,900 | 32,288 | Units | 1009 * Unit(32) |
| 79,188 | 4,800 | Terrain specials | 2 * 2400 * int8 |
| 83,988 | 4,800 | Explored state | 2 * 2400 * uint8 |
| 88,788 | 28,800 | Movement costs | (zero-filled) |
| 117,588 | 100 | Events | (zero-filled) |
| 117,688 | 4,800 | Map flags | 2 * 2400 * uint8 |
| 122,488 | 2 | Grand vizier flag | |
| 122,490 | 250 | Premade items | |
| 122,740 | 560 | Hero names | 35 * char[16] |
| **Total** | **123,300** | | |

---

## 8. Editor State Model

### Why the EditorState Struct

All UI, rendering, and input handling code needs to agree on what tool is active,
where the cursor is, and what's selected. Rather than scattering this across
callbacks, a single `EditorState` struct is the single source of truth that gets
passed by reference to every subsystem.

### EditorState Fields

```
EditorState
  .tool              EditorTool    Currently active tool (enum of 10 values)
  .paint_terrain     BaseTerrain   Terrain type to paint (0-11)
  .paint_special     TerrainSpecial  Mineral deposit to paint
  .place_owner       int8_t        Owner player for new entities (0-5)
  .place_unit_type   uint8_t       Unit type index for PLACE_UNIT (default: 35)
  .place_race        int8_t        Race for new cities (default: HIGH_MAN)
  .place_node_type   int8_t        Node type: Nature/Sorcery/Chaos
  .current_plane     int           0=Arcanus, 1=Myrror
  .cursor_wx/wy      int           Current cursor world position (-1 = off map)
  .selected_city     int           Selected city array index (-1 = none)
  .selected_unit     int           Selected unit array index
  .selected_node     int           Selected node array index
  .selected_lair     int           Selected lair array index
  .show_grid         bool          Whether grid lines are visible
```

### EditorTool Enum

```
SELECT          -- Click entities to select; shows properties in right panel
PAINT_TERRAIN   -- Click/drag to set terrain type
PAINT_SPECIAL   -- Click/drag to set mineral deposits
PLACE_CITY      -- Click to create a city
PLACE_UNIT      -- Click to create a unit
PLACE_NODE      -- Click to create a mana node
PLACE_LAIR      -- Click to create a lair
PLACE_TOWER     -- Click to create a tower of wizardry
PLACE_FORTRESS  -- Click to create a fortress
ERASE           -- Click/drag to remove entities
```

---

## 9. Undo System

### Why Command Pattern

Every edit must be reversible because terrain painting at the wrong zoom level or
accidentally placing a city is trivially easy. The command pattern captures both the
"do" and "undo" as closures at the moment the action occurs, so undo is always exact
— no need to snapshot the entire 2,400-tile world.

### Specification

- Max depth: 50 undo entries
- When depth exceeded: oldest command silently discarded
- On new command after undo: redo history cleared
- Implementation: `LambdaCommand` stores two `std::function<void()>` closures
- Thread safety: none needed (single-threaded SDL event loop)

---

## 10. Camera System

### Specification

```
Camera
  .offset_x/y     float    Scroll position in pixels
  .zoom            float    Scale factor (range: 0.3x to 5.0x)
  .tile_size       int      Base tile width = 24px (height = 24 * 0.9 = ~22px)

Rendered tile size = tile_size * zoom
  At 1.0x zoom: 24x22 pixels per tile
  At 5.0x zoom: 120x108 pixels per tile
  At 0.3x zoom: 7x6 pixels per tile (minimap-like)

Zoom step: multiply/divide by 1.15 per mouse wheel tick
Zoom toward mouse: offset adjusted so point under cursor stays fixed
Scroll clamp: cannot scroll past map edges
```

---

## 11. Test Suite

### Why These Tests

Binary compatibility is the highest-risk aspect of this project. If the City struct
is 113 bytes instead of 114, every field after it shifts and the game crashes.
Round-trip tests (write then read, compare) catch alignment and serialization bugs
that code review alone cannot.

### Test Coverage

**test_MomGamFile.cpp** (15 tests)
- Struct size assertions: City=114, Unit=32, Wizard=1224, Node=48, Lair=24,
  Tower=4, Fortress=4, GameData=16, Item=50
- MomWorld: get/set terrain, specials, flags; bounds checking; clear; explored
- Entity state: is_active() for City, Unit, Wizard; is_hero() for Unit
- Scenario: clear, count_active_cities, count_active_units
- .GAM round-trip: empty scenario, with terrain, with entities
- Error cases: nonexistent file, truncated file

**test_Scenario.cpp** (13 tests)
- JSON round-trip: metadata, game settings, terrain+specials+flags
- Entity round-trip: wizards (with retorts), cities (with buildings),
  units, nodes, lairs
- Active count auto-update on load
- Compaction: only active entities serialized; loaded into first slots
- Error cases: nonexistent file, invalid JSON

**test_UndoStack.cpp** (9 tests)
- Basic flow: execute, undo, redo
- Multi-step sequences
- Redo cleared on new command
- Max depth eviction (50 commands)
- Complex interleaved sequence
- Edge cases: undo/redo on empty stack

---

## 12. Critical Reference Files

These files in the ReMoM project (c:\STU\devel\STU-Extras\ReMoM) are the
authoritative source for binary layout. Any struct size or offset discrepancy
means corrupted .GAM files.

| File | What It Defines |
|------|----------------|
| `src/MOX/MOM_Data.H` | All struct definitions with byte offsets and packing |
| `src/MOX/LOADSAVE.C` | Exact .GAM file layout (read/write order and offsets) |
| `src/MOM_DEF.H` | World dimensions, entity limits, all game constants |
| `src/Terrain.H` | Terrain special enum values |
| `src/RACETYPE.H` | Race enum (14 playable races) |
| `src/UNITTYPE.H` | Unit type enum (198 types, 0-34 are heroes) |
| `src/sdl2_MOM.C` | SDL2 initialization pattern |
| `SAVETEST.GAM` | Reference save file for round-trip testing (123,300 bytes) |

---

## 13. Current Limitations

| Limitation | Impact | Resolution |
|-----------|--------|------------|
| No text rendering | UI uses colored blocks instead of labels | Add SDL2_ttf or bitmap font in Phase 2 |
| No file dialog | Save/load uses hardcoded filenames | Add native file dialog (nfd or tinyfiledialogs) |
| Terrain mapping is heuristic | Low byte of uint16 mapped to base type; may not match original tile indices perfectly | Implement proper terrain table lookup from ReMoM's terrain data |
| No auto-tiling on export | **BLOCKER for distribution.** Exported .GAM uses base type index, not the edge-transition tile index the game expects. Game may render terrain incorrectly or crash. | Must build terrain transition resolver before any .GAM files are shared with players. This requires mapping the 12 editor base types to the ~762 tile indices using ReMoM's terrain tables. |
| No entity property editing | Properties panel exists but editing fields is not interactive | Wire up text input and dropdown widgets |
| No validation | Can place cities on ocean, units out of bounds | Add validation pass before export |
| 50-command undo limit | Large painting sessions may lose early undo history | Consider compressing consecutive paint commands |
| Single hardcoded filename | Always saves to "scenario.mom_scenario" / "scenario.GAM" | Add file dialog |

---

## 14. Future Phases (Roadmap)

### Phase 2: Polish & itch.io

- Text rendering (bitmap font or SDL2_ttf)
- Native file open/save dialogs
- Full wizard configuration UI (all 240 spells, 18 retorts)
- City building editor (all 36 buildings, enchantments)
- Unit enchantment editing
- Input validation (no water cities, bounds checks)
- Emscripten/WASM build target for itch.io web play
- Optional LBX sprite loading from user's MoM installation folder

### Phase 3: Missions & Community Browser

- Objective editor (win/lose conditions: defeat wizards, control N cities, etc.)
- Story text editor (briefing, debriefing, narration events)
- Turn-based trigger editor (spawn units, show messages at turn N)
- REST API backend for scenario upload/download/search/rate
- Built-in browser UI with scenario cards (thumbnail + title + rating)

### Phase 4: Training Puzzles

- Combat grid editor (21x22 tactical grid)
- Puzzle scenario type with par turns and hints
- Advanced trigger scripting

### Phase 5: Hardening & Launch

- Cross-platform testing
- Performance optimization
- Documentation and tutorials
- itch.io page and community launch

---

## 15. How to Verify the Build

```bash
# Configure
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug

# Build all targets
cmake --build build

# Run tests
cd build && ctest --output-on-failure

# Run the editor
./build/RemomMapgenItchio

# Manual verification (ALL steps required):
# 1. Paint terrain on both planes (Tab to switch)
# 2. Place a city and unit
# 3. Ctrl+S to save .mom_scenario, Ctrl+O to reload — verify data preserved
# 4. Ctrl+E to export .GAM — verify file is exactly 123,300 bytes
# 5. Load exported .GAM in ReMoM — game MUST load and play correctly
#    (terrain, cities, units, nodes must appear as placed in the editor)
#    This is the product's core deliverable — if the .GAM doesn't load,
#    nothing else matters.
```
