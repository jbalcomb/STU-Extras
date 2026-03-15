# Data Model: Master of Magic Scenario Editor

**Branch**: `001-mom-scenario-editor` | **Date**: 2026-03-08

## Overview

The data model is constrained by binary compatibility with the Master of Magic .GAM save format. All entity structs use 2-byte packing (`#pragma pack(push, 2)`) and fixed-size arrays. The model below documents the existing struct layouts (which MUST NOT change) and the new structures needed for map generation, validation, and editor state.

## Existing Entities (Binary-Compatible — DO NOT MODIFY)

### GameData (16 bytes)

Global scenario settings matching .GAM file header.

| Field | Type | Size | Description |
|-------|------|------|-------------|
| Current_Unit | int16_t | 2 | Currently active unit index |
| Turn | int16_t | 2 | Current game turn (starts at 1) |
| Total_Units | int16_t | 2 | Count of active units |
| Total_Cities | int16_t | 2 | Count of active cities |
| Difficulty | int16_t | 2 | 0=Intro, 1=Easy, 2=Normal, 3=Hard, 4=Impossible |
| Magic | uint16_t | 2 | Magic intensity level |
| Land_Size | uint16_t | 2 | Land size parameter |
| Total_Wizards | int16_t | 2 | Number of active wizards (1-6) |

### Wizard (1,224 bytes per slot, 6 slots)

Player or AI wizard configuration.

| Field | Type | Size | Description |
|-------|------|------|-------------|
| name | char[20] | 20 | Wizard name (active if name[0] != '\0') |
| race | int16_t | 2 | Race index (0-13, see Race enum) |
| portrait | int16_t | 2 | Portrait index |
| spell_ranks | int16_t[5] | 10 | Spell mastery per realm |
| retorts | int16_t[18] | 36 | Retort flags |
| spells | uint8_t[240] | 240 | Known spells bitmask |
| gold | int16_t | 2 | Starting gold |
| mana | int16_t | 2 | Starting mana |
| ... | ... | ~910 | Diplomacy, astrology, misc state |

### City (114 bytes per slot, 100 slots)

Settlement with buildings and population.

| Field | Type | Size | Description |
|-------|------|------|-------------|
| name | char[14] | 14 | City name |
| race | int16_t | 2 | Population race (0-13) |
| plane | int8_t | 1 | 0=Arcanus, 1=Myrror |
| x | int8_t | 1 | Map x coordinate (0-59) |
| y | int8_t | 1 | Map y coordinate (0-39) |
| owner_idx | int8_t | 1 | Owning wizard index (-1 = inactive) |
| size | int16_t | 2 | City size class |
| population | int16_t | 2 | Population count (×1000) |
| buildings | uint8_t[36] | 36 | Building presence flags |
| enchantments | int16_t[25] | 50 | City enchantment flags |
| roads | uint8_t | 1 | Road connections bitmask |
| ... | ... | ~4 | Misc fields |

### Unit (32 bytes per slot, 1,009 slots)

Military unit or hero.

| Field | Type | Size | Description |
|-------|------|------|-------------|
| plane | int8_t | 1 | 0=Arcanus, 1=Myrror |
| x | int8_t | 1 | Map x coordinate |
| y | int8_t | 1 | Map y coordinate |
| owner_idx | int8_t | 1 | Owning wizard index (-1 = inactive) |
| type | int16_t | 2 | Unit type ID (< 35 = hero) |
| level | int16_t | 2 | Experience level |
| xp | int16_t | 2 | Experience points |
| hero_slot | int8_t | 1 | Hero item slot assignment |
| ... | ... | ~21 | Movement, combat, enchantment state |

### Node (48 bytes per slot, 30 slots)

Magic power node.

| Field | Type | Size | Description |
|-------|------|------|-------------|
| plane | int8_t | 1 | 0=Arcanus, 1=Myrror |
| x | int8_t | 1 | Map x coordinate |
| y | int8_t | 1 | Map y coordinate |
| owner | int8_t | 1 | Controlling wizard (-1 = unowned) |
| type | int16_t | 2 | 0=Sorcery, 1=Nature, 2=Chaos |
| power | int16_t | 2 | Mana output |
| aura_coords | int16_t[40] | 80 | 20 tiles of node aura (x,y pairs) |

### Lair (24 bytes per slot, 102 slots)

Monster lair or ruin.

| Field | Type | Size | Description |
|-------|------|------|-------------|
| plane | int8_t | 1 | Plane |
| x | int8_t | 1 | Map x |
| y | int8_t | 1 | Map y |
| type | int8_t | 1 | Lair type |
| guards_1 | int16_t[3] | 6 | First guard group |
| guards_2 | int16_t[3] | 6 | Second guard group |
| gold | int16_t | 2 | Gold reward |
| mana | int16_t | 2 | Mana reward |
| item_1 | int16_t | 2 | First item reward index |
| item_2 | int16_t | 2 | Second item reward index |

### Tower (4 bytes per slot, 6 slots)

Plane-crossing tower.

| Field | Type | Size | Description |
|-------|------|------|-------------|
| x | int8_t | 1 | Map x |
| y | int8_t | 1 | Map y |
| owner | int8_t | 1 | Controlling wizard |
| active | int8_t | 1 | Active flag |

### Fortress (4 bytes per slot, 6 slots)

Wizard's seat of power.

| Field | Type | Size | Description |
|-------|------|------|-------------|
| x | int8_t | 1 | Map x |
| y | int8_t | 1 | Map y |
| plane | int8_t | 1 | Plane |
| active | int8_t | 1 | Active flag |

### Item (50 bytes per slot, 138 slots)

Magical artifact — opaque data block matching .GAM layout.

### World Map (per plane)

| Array | Type | Size per plane | Description |
|-------|------|----------------|-------------|
| terrain | uint16_t[2400] | 4,800 bytes | Tile terrain index (0-761 for autotiled, 0-11 for base) |
| specials | int8_t[2400] | 2,400 bytes | Terrain special (minerals, flora) |
| flags | uint8_t[2400] | 2,400 bytes | Tile flags (road, corruption) |
| landmasses | uint8_t[2400] | 2,400 bytes | Landmass group ID |
| explored | uint8_t[2400] | 2,400 bytes | Exploration state per wizard |

**Dimensions**: 60 wide × 40 tall, indexed as `[plane][y * 60 + x]`.
**Wrapping**: x-axis wraps (x=0 neighbors x=59). y-axis does not wrap.

## New Entities (Editor-Only — Not in .GAM)

### MapGenParams

Parameters for procedural world generation. Stored in .mom_scenario JSON only.

| Field | Type | Description |
|-------|------|-------------|
| land_proportion | float | 0.0 to 1.0 — fraction of tiles that are land (default 0.4) |
| roughness | enum | SMOOTH / MEDIUM / ROUGH — controls terrain variety (default MEDIUM) |
| continent_count | int | 1-6 — number of distinct landmasses per plane (default 3) |
| seed | uint32_t | Random seed for reproducible generation (default: random) |
| mirror_planes | bool | Whether Myrror mirrors Arcanus layout (default: false) |

### SmoothingViolation

Result of terrain adjacency validation. Transient — not serialized.

| Field | Type | Description |
|-------|------|-------------|
| plane | int | 0=Arcanus, 1=Myrror |
| x | int | Tile x coordinate |
| y | int | Tile y coordinate |
| rule | enum | SHORE_NO_WATER / RIVER_ISOLATED / VOLCANO_ISOLATED / TUNDRA_DESERT / CUSTOM |
| severity | enum | WARNING / INFO |
| message | string | Human-readable description |

### ScenarioMetadata

Editor metadata preserved in .mom_scenario but not in .GAM.

| Field | Type | Description |
|-------|------|-------------|
| title | string | Scenario display name (default "Untitled Scenario") |
| author | string | Creator name |
| description | string | Scenario description/notes |
| version | string | Semantic version (default "1.0.0") |
| tags | string[] | Categorization tags |
| scenario_type | enum | SANDBOX / MISSION / PUZZLE |
| created_at | string | ISO 8601 creation timestamp |
| modified_at | string | ISO 8601 last modified timestamp |

## Entity Relationships

```
Scenario (1)
├── GameData (1) ─── difficulty, magic, land_size, wizard_count
├── ScenarioMetadata (1) ─── title, author, version (editor-only)
├── MapGenParams (1) ─── generation parameters (editor-only)
├── MomWorld (1)
│   ├── Arcanus plane (1) ─── terrain[2400], specials[2400], flags[2400]
│   └── Myrror plane (1) ─── terrain[2400], specials[2400], flags[2400]
├── Wizard[6] ─── owns → Cities, Units, Nodes, Towers, Fortresses
├── City[100] ─── located on → World tile, owned by → Wizard
├── Unit[1009] ─── located on → World tile, owned by → Wizard
├── Node[30] ─── located on → World tile, owned by → Wizard
├── Lair[102] ─── located on → World tile
├── Tower[6] ─── located on → World tile, owned by → Wizard
├── Fortress[6] ─── located on → World tile, linked to → Wizard
└── Item[138] ─── equipped by → Unit (hero)
```

## State Transitions

### Scenario Lifecycle

```
[Empty] → clear() → [Blank Scenario]
[Blank Scenario] → generate_map(params) → [Generated Scenario]
[Generated Scenario] → edit tiles/entities → [Modified Scenario]
[Modified Scenario] → save .mom_scenario → [Saved Scenario]
[Saved Scenario] → load .mom_scenario → [Modified Scenario]
[Any Scenario] → export .GAM → [file downloaded, scenario unchanged]
```

### Entity Slot Lifecycle

```
[Inactive] (owner_idx = -1 or name[0] = '\0')
    → place_entity() → [Active] (slot populated)
    → [Active] → edit_properties() → [Active] (attributes changed)
    → [Active] → erase_entity() → [Inactive] (slot zeroed)
```

## Validation Rules

| Rule ID | Entity | Constraint | Enforcement |
|---------|--------|-----------|-------------|
| V-001 | City | max 100 active slots | Hard limit — reject placement |
| V-002 | Unit | max 1,009 active slots | Hard limit — reject placement |
| V-003 | Node | max 30 active slots | Hard limit — reject placement |
| V-004 | Lair | max 102 active slots | Hard limit — reject placement |
| V-005 | Tower | max 6 active slots | Hard limit — reject placement |
| V-006 | Fortress | max 6 active slots | Hard limit — reject placement |
| V-007 | Wizard | max 6 active slots | Hard limit — reject configuration |
| V-008 | Item | max 138 active slots | Hard limit — reject creation |
| V-009 | Tile (Shore) | Must neighbor Ocean/Lake | Warning — highlight |
| V-010 | Tile (River) | Must neighbor River/Shore/Ocean/Lake | Warning — highlight |
| V-011 | Tile (Volcano) | Should neighbor Mountain/Hill | Warning — highlight |
| V-012 | Tile (Tundra) | Should not neighbor Desert | Warning — highlight |
| V-013 | .GAM export | File must be exactly 123,300 bytes | Hard assert |
| V-014 | Tile coords | x: 0-59, y: 0-39, plane: 0-1 | Bounds check — silently clamp |
