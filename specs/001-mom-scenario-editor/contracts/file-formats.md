# File Format Contracts: Master of Magic Scenario Editor

**Branch**: `001-mom-scenario-editor` | **Date**: 2026-03-08

## Contract 1: .GAM Binary Save Format

The .GAM format is the game engine's native save file. This format is NOT under our control — it is defined by Master of Magic (1994) and the ReMoM reimplementation. Our editor MUST produce files that conform exactly.

### Invariants

- File size: exactly **123,300 bytes** (no headers, no padding, no EOF marker)
- Byte order: little-endian (x86)
- Struct alignment: 2-byte packed (`#pragma pack(push, 2)`)
- All arrays are fixed-size and contiguous

### Layout

| Offset | Size (bytes) | Content |
|--------|-------------|---------|
| 0x0000 | 16 | GameData struct |
| 0x0010 | 7,344 | Wizard[6] × 1,224 bytes |
| 0x1CC4 | 11,400 | City[100] × 114 bytes |
| 0x4A1C | 32,288 | Unit[1,009] × 32 bytes |
| 0xC8FC | 1,440 | Node[30] × 48 bytes |
| 0xCE9C | 2,448 | Lair[102] × 24 bytes |
| 0xD82C | 24 | Tower[6] × 4 bytes |
| 0xD844 | 24 | Fortress[6] × 4 bytes |
| 0xD85C | 6,900 | Item[138] × 50 bytes |
| 0xF350 | 9,600 | terrain[2][2,400] × uint16_t |
| 0x1B850 | 4,800 | specials[2][2,400] × int8_t |
| 0x1CAD0 | 4,800 | flags[2][2,400] × uint8_t |
| 0x1DD50 | 4,800 | landmasses[2][2,400] × uint8_t |
| 0x1EFD0 | 4,800 | explored[2][2,400] × uint8_t |

*Note: Exact offsets are derived from MomGamFile.cpp write sequence. The reference source is ReMoM's `src/MOX/LOADSAVE.C`.*

### Verification

- Every struct size is verified at compile time via `static_assert`
- Round-trip test: write scenario → read back → compare all fields
- File size assertion after write

---

## Contract 2: .mom_scenario JSON Format

The .mom_scenario format is the editor's native format. This format IS under our control and can evolve with the editor.

### Structure

```json
{
    "metadata": {
        "title": "string",
        "author": "string",
        "description": "string",
        "version": "string (semver)",
        "tags": ["string"],
        "scenario_type": "SANDBOX | MISSION | PUZZLE",
        "format_version": "string (semver)",
        "created_at": "ISO 8601",
        "modified_at": "ISO 8601"
    },
    "game_data": {
        "difficulty": 0-4,
        "magic": "uint16",
        "land_size": "uint16",
        "total_wizards": 1-6
    },
    "map_gen_params": {
        "land_proportion": 0.0-1.0,
        "roughness": "SMOOTH | MEDIUM | ROUGH",
        "continent_count": 1-6,
        "seed": "uint32",
        "mirror_planes": "bool"
    },
    "world": {
        "arcanus": {
            "terrain": "base64-encoded uint16_t[2400]",
            "specials": "base64-encoded int8_t[2400]",
            "flags": "base64-encoded uint8_t[2400]"
        },
        "myrror": {
            "terrain": "base64-encoded uint16_t[2400]",
            "specials": "base64-encoded int8_t[2400]",
            "flags": "base64-encoded uint8_t[2400]"
        }
    },
    "wizards": [
        {
            "slot": 0-5,
            "name": "string",
            "race": 0-13,
            "portrait": "int16",
            "retorts": [0-17],
            "gold": "int16",
            "mana": "int16"
        }
    ],
    "cities": [
        {
            "slot": 0-99,
            "name": "string",
            "race": 0-13,
            "x": 0-59,
            "y": 0-39,
            "plane": 0-1,
            "owner": 0-5,
            "population": "int16",
            "buildings": [0-35]
        }
    ],
    "units": [
        {
            "slot": 0-1008,
            "type": "int16",
            "x": 0-59,
            "y": 0-39,
            "plane": 0-1,
            "owner": 0-5,
            "level": "int16"
        }
    ],
    "nodes": [
        {
            "slot": 0-29,
            "type": 0-2,
            "x": 0-59,
            "y": 0-39,
            "plane": 0-1,
            "power": "int16"
        }
    ],
    "lairs": [
        {
            "slot": 0-101,
            "type": "int8",
            "x": 0-59,
            "y": 0-39,
            "plane": 0-1
        }
    ],
    "towers": [
        {
            "slot": 0-5,
            "x": 0-59,
            "y": 0-39
        }
    ],
    "fortresses": [
        {
            "slot": 0-5,
            "x": 0-59,
            "y": 0-39,
            "plane": 0-1
        }
    ]
}
```

### Conventions

- **Sparse serialization**: Only active entities are included in arrays. Inactive slots (owner_idx = -1 or name[0] = '\0') are omitted.
- **Base64 encoding**: Terrain arrays are base64-encoded binary for compact representation of 2,400-element arrays.
- **Forward compatibility**: Unknown JSON keys are silently ignored on load. Missing optional keys use defaults.
- **Backward compatibility**: `format_version` field tracks schema changes. Older formats are upgraded on load.

### Verification

- Round-trip test: serialize → deserialize → compare all fields
- Active entity counts must match after round-trip
- Base64 decode length must equal expected array size

---

## Contract 3: User Interaction Model

The editor presents a single-window interface with these interaction regions:

### Input Regions

| Region | Location | Purpose |
|--------|----------|---------|
| Map viewport | Center | Displays terrain tiles, entities; receives paint/place/select clicks |
| Tool panel | Left edge | Tool selection buttons, terrain/special swatches |
| Properties panel | Right edge | Selected entity properties (read-write), minimap |
| Status bar | Bottom | Current coordinates, plane, zoom level, active tool |

### Input Bindings

| Input | Action |
|-------|--------|
| Left click (map) | Apply current tool at tile |
| Left drag (map) | Continuous paint/erase |
| Middle drag (map) | Pan camera |
| Mouse wheel | Zoom toward cursor |
| Left click (tool panel) | Select tool or swatch |
| Left click (properties) | Edit selected property |
| Tab | Toggle Arcanus/Myrror |
| G | Toggle grid overlay |
| Ctrl+Z | Undo |
| Ctrl+Y | Redo |
| Ctrl+S | Save .mom_scenario |
| Ctrl+O | Load .mom_scenario |
| Ctrl+E | Export .GAM |
| Ctrl+N | Generate new map |
| Arrow keys | Pan camera |

### File Operations (Platform-Dependent)

| Operation | Desktop | Browser (WASM) |
|-----------|---------|----------------|
| Save | Native "Save As" dialog | Browser file download |
| Load | Native "Open" dialog | Browser file upload (`<input type="file">`) |
| Export .GAM | Native "Save As" dialog | Browser file download |
