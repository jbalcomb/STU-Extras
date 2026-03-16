# ReMoM Terrain Type System Reference

A complete reference for the terrain type system used in Master of Magic (1994) and ReMoM. This document covers all 762 terrain entries defined in `TerrType.h`, how autotiling works, and what modders need to know to correctly generate or modify `.GAM` save files.

## Overview

Each map square's terrain is stored as a 16-bit value (`uint16_t`) in the `.GAM` file. The world is 60 squares wide x 40 squares tall x 2 planes (Arcanus = 0, Myrror = 1), for a total of 4,800 squares per plane.

There are 762 total terrain entries (`TerType_Count = 0x2FA`). Most of these are **autotile variants** — different visual representations of the same base terrain type, selected based on what neighbors surround the square.

**The game does NOT auto-fix terrain variants on save load.** Autotiling only runs during new game generation (`Simex_Autotiling`, `Desert_Autotile`, `Shuffle_Terrains`). If you write a `.GAM` file with incorrect terrain values, the game will display them as-is, producing visual glitches.

## Base Terrain Types

The editor uses 12 base terrain types internally:

| ID | Name       | Base Game Value | Notes |
|----|------------|-----------------|-------|
| 0  | Ocean      | `0x00`          | Open water |
| 1  | Shore      | `0x02`-`0xA1`  | Ocean square adjacent to land (158 variants) |
| 2  | Grassland  | `0xA2`          | 4 cosmetic variants |
| 3  | Forest     | `0xA3`          | 3 cosmetic variants |
| 4  | Mountain   | `0xA4`          | 16 connectivity variants |
| 5  | Desert     | `0xA5`          | 160 adjacency variants |
| 6  | Swamp      | `0xA6`          | 3 cosmetic variants |
| 7  | Tundra     | `0xA7`          | 160 adjacency variants |
| 8  | Hill       | `0xAB`          | 17 connectivity variants |
| 9  | River      | `0xB9`          | ~32 connectivity variants |
| 10 | Volcano    | `0xB3`          | Single variant |
| 11 | Lake       | `0x12`          | 5 variants (base + 4 directional) |

## Autotiling Pipeline

During new game generation, terrain variants are assigned in this order:

1. **`Simex_Autotiling()`** — Assigns shore, mountain, hill, tundra adjacency variants
2. **River generation** — Places rivers with NESW connectivity
3. **`Desert_Autotile()`** — Assigns desert adjacency variants
4. **`Shuffle_Terrains()`** — Assigns random cosmetic variants for grassland, forest, swamp

This pipeline runs once at map generation. It does **not** run when loading a saved game.

---

## 8-Bit Adjacency System (Shore, Tundra, Desert)

Shore, tundra, and desert all use the same 8-bit adjacency bitmask system with 8 neighbor directions:

```
Bit 7: NW (North-West)     Bit 6: N (North)
Bit 5: NE (North-East)     Bit 4: E (East)
Bit 3: SE (South-East)     Bit 2: S (South)
Bit 1: SW (South-West)     Bit 0: W (West)
```

The meaning of "flagged" differs by terrain type:

| Terrain | Bit = 1 means | Bit = 0 means |
|---------|--------------|--------------|
| Shore   | Neighbor is land (not ocean/shore/lake) | Neighbor is water |
| Tundra  | Neighbor is NOT tundra | Neighbor IS tundra |
| Desert  | Neighbor is NOT desert | Neighbor IS desert |

### Diagonal Forcing Rule

When both cardinal directions flanking a diagonal are flagged, the diagonal is forced ON:

- N + W both set → force NW on
- N + E both set → force NE on
- E + S both set → force SE on
- S + W both set → force SW on

This is a one-way rule. Diagonal-only patterns (e.g., land only at SE with no cardinal land neighbors) are valid and produce correct shore variants.

### Diagonal Fallback Masking

If the bitmask lookup returns 0 (no valid variant for that pattern), try clearing "orphan" diagonals — diagonals where neither flanking cardinal is set:

- Clear NW if neither N nor W is set
- Clear NE if neither N nor E is set
- Clear SE if neither S nor E is set
- Clear SW if neither S nor W is set

Then retry the lookup. If still 0, use the isolated variant (see below).

### Shore Placement Rule

Shores are placed on the **ocean side** of a land-ocean boundary. An ocean square becomes a shore if it has any land neighbor in 8 directions (cardinal + diagonal). This matches MoM's `NEWG_CreateShores()` function.

Shore squares are never placed on land. A land square adjacent to ocean stays as its base terrain type — the ocean square next to it becomes shore.

### Shore Variants (0x02 - 0xA1)

There are 158 valid shore entries. The full lookup table maps each 8-bit adjacency bitmask to a game value:

```
Bitmask → Game Value (hex)
0x01 → 0x13    0x02 → 0x06    0x03 → 0x0C    0x04 → 0x0B
0x06 → 0x05    0x07 → 0x2E    0x08 → 0x02    0x09 → 0x82
0x0A → 0x07    0x0B → 0x84    0x0C → 0x03    0x0E → 0x04
0x0F → 0x2F    0x10 → 0x11    0x11 → 0x4E    0x12 → 0x6D
0x13 → 0x4A    0x18 → 0x0A    0x19 → 0x4F    0x1A → 0x6C
0x1B → 0x4B    0x1C → 0x26    0x1E → 0x28    0x1F → 0x52
0x20 → 0x1B    0x21 → 0x7A    0x22 → 0x08    0x23 → 0x7C
0x24 → 0x79    0x26 → 0x76    0x27 → 0x9E    0x28 → 0x0F
0x29 → 0x7E    0x2A → 0x19    0x2B → 0x80    0x2C → 0x73
0x2E → 0x70    0x2F → 0x9F    0x30 → 0x15    0x31 → 0x50
0x32 → 0x6B    0x33 → 0x4C    0x38 → 0x10    0x39 → 0x51
0x3A → 0x6A    0x3B → 0x4D    0x3C → 0x27    0x3E → 0x29
0x3F → 0x5A    0x40 → 0x16    0x42 → 0x86    0x44 → 0x41
0x46 → 0x3F    0x48 → 0x88    0x4A → 0x87    0x4C → 0x40
0x4E → 0x3E    0x60 → 0x1C    0x62 → 0x8C    0x64 → 0x3D
0x66 → 0x3B    0x68 → 0x8E    0x6A → 0x8D    0x6C → 0x3C
0x6E → 0x3A    0x70 → 0x2A    0x72 → 0x9A    0x78 → 0x2B
0x7A → 0x9B    0x7C → 0x55    0x7E → 0x59    0x80 → 0x1F
0x81 → 0x17    0x82 → 0x09    0x83 → 0x14    0x84 → 0x77
0x86 → 0x74    0x87 → 0x30    0x88 → 0x0E    0x89 → 0x83
0x8A → 0x1A    0x8B → 0x85    0x8C → 0x71    0x8E → 0x6E
0x8F → 0x31    0x90 → 0x65    0x91 → 0x46    0x92 → 0x69
0x93 → 0x42    0x98 → 0x64    0x99 → 0x47    0x9A → 0x68
0x9B → 0x43    0x9C → 0x96    0x9E → 0x98    0x9F → 0x56
0xA0 → 0x0D    0xA1 → 0x7B    0xA2 → 0x20    0xA3 → 0x7D
0xA4 → 0x78    0xA6 → 0x75    0xA7 → 0xA0    0xA8 → 0x18
0xA9 → 0x7F    0xAA → 0x21    0xAB → 0x81    0xAC → 0x72
0xAE → 0x6F    0xAF → 0xA1    0xB0 → 0x63    0xB1 → 0x48
0xB2 → 0x67    0xB3 → 0x44    0xB8 → 0x62    0xB9 → 0x49
0xBA → 0x66    0xBB → 0x45    0xBC → 0x97    0xBE → 0x99
0xBF → 0x5E    0xC0 → 0x1E    0xC1 → 0x22    0xC2 → 0x89
0xC3 → 0x24    0xC4 → 0x39    0xC6 → 0x37    0xC7 → 0x53
0xC8 → 0x8B    0xC9 → 0x92    0xCA → 0x8A    0xCB → 0x94
0xCC → 0x38    0xCE → 0x36    0xCF → 0x5B    0xE0 → 0x1D
0xE1 → 0x23    0xE2 → 0x8F    0xE3 → 0x25    0xE4 → 0x35
0xE6 → 0x33    0xE7 → 0x57    0xE8 → 0x91    0xE9 → 0x93
0xEA → 0x90    0xEB → 0x95    0xEC → 0x34    0xEE → 0x32
0xEF → 0x5F    0xF0 → 0x2C    0xF1 → 0x54    0xF2 → 0x9C
0xF3 → 0x5C    0xF8 → 0x2D    0xF9 → 0x58    0xFA → 0x9D
0xFB → 0x60    0xFC → 0x5D    0xFE → 0x61
```

Bitmask `0xFF` (all 8 neighbors flagged) has no entry — this pattern should not occur for shores because a shore square surrounded entirely by land would just be land.

### Tundra Variants (0x25A - 0x2F9)

Tundra uses the same adjacency system as shore. To get a tundra game value:

```
tundra_value = shore_value + 0x258
```

For a tundra square where all 8 neighbors are also tundra (bitmask `0x00`), use the base value `0xA7` (`tTundra`).

For an isolated tundra square (all neighbors are non-tundra, bitmask `0xFF`), use `0x26A` (`_1Tundra`).

### Desert Variants (0x124 - 0x1C3)

Desert uses the same adjacency system as shore. To get a desert game value:

```
desert_value = shore_value + 0x122
```

For a desert square where all 8 neighbors are also desert (bitmask `0x00`), use the base value `0xA5` (`tDesert`).

For an isolated desert square (all neighbors are non-desert, bitmask `0xFF`), use `0x134` (`_1Desert`).

---

## 4-Bit Cardinal System (Mountain, Hill, River)

Mountain, hill, and river use a simpler 4-bit system with only cardinal directions:

```
Bit 3: N (North)
Bit 2: E (East)
Bit 1: S (South)
Bit 0: W (West)
```

### Mountain Variants (0xA4, 0x103 - 0x112)

A bit is set when the neighbor IS a mountain. 16 connectivity patterns:

| NESW | Binary | Game Value | Description |
|------|--------|------------|-------------|
| ---- | 0000   | `0xA4`     | Isolated mountain |
| ---W | 0001   | `0x107`    | Mountain to west |
| --S- | 0010   | `0x103`    | Mountain to south |
| --SW | 0011   | `0x10C`    | Mountain to south and west |
| -E-- | 0100   | `0x104`    | Mountain to east |
| -E-W | 0101   | `0x106`    | Mountain to east and west |
| -ES- | 0110   | `0x10A`    | Mountain to east and south |
| -ESW | 0111   | `0x10B`    | Mountain to east, south, and west |
| N--- | 1000   | `0x109`    | Mountain to north |
| N--W | 1001   | `0x112`    | Mountain to north and west |
| N-S- | 1010   | `0x108`    | Mountain to north and south |
| N-SW | 1011   | `0x10F`    | Mountain to north, south, and west |
| NE-- | 1100   | `0x110`    | Mountain to north and east |
| NE-W | 1101   | `0x111`    | Mountain to north, east, and west |
| NES- | 1110   | `0x10D`    | Mountain to north, east, and south |
| NESW | 1111   | `0x105`    | Mountain on all four sides |

### Hill Variants (0xAB, 0x113 - 0x122)

A bit is set when the neighbor IS a hill. Same 16 connectivity patterns:

| NESW | Binary | Game Value | Description |
|------|--------|------------|-------------|
| ---- | 0000   | `0xAB`     | Isolated hill |
| ---W | 0001   | `0x117`    | Hill to west |
| --S- | 0010   | `0x113`    | Hill to south |
| --SW | 0011   | `0x11C`    | Hill to south and west |
| -E-- | 0100   | `0x114`    | Hill to east |
| -E-W | 0101   | `0x116`    | Hill to east and west |
| -ES- | 0110   | `0x11A`    | Hill to east and south |
| -ESW | 0111   | `0x11B`    | Hill to east, south, and west |
| N--- | 1000   | `0x119`    | Hill to north |
| N--W | 1001   | `0x122`    | Hill to north and west |
| N-S- | 1010   | `0x118`    | Hill to north and south |
| N-SW | 1011   | `0x11F`    | Hill to north, south, and west |
| NE-- | 1100   | `0x120`    | Hill to north and east |
| NE-W | 1101   | `0x121`    | Hill to north, east, and west |
| NES- | 1110   | `0x11D`    | Hill to north, east, and south |
| NESW | 1111   | `0x115`    | Hill on all four sides |

### River Variants (0xB9 - 0xC8, 0xE9 - 0x102, 0x1D4 - 0x1D8)

A bit is set when the neighbor connects to the river. Rivers connect to: other rivers, shores, oceans, and lakes.

| NESW | Binary | Game Value | Description |
|------|--------|------------|-------------|
| ---- | 0000   | `0xB9`     | Isolated river (displays as S-only) |
| ---W | 0001   | `0xBA`     | River flowing west |
| --S- | 0010   | `0xB9`     | River flowing south |
| --SW | 0011   | `0xBE`     | River corner: south to west |
| -E-- | 0100   | `0xBC`     | River flowing east |
| -E-W | 0101   | `0xF0`     | River flowing east-west |
| -ES- | 0110   | `0xBF`     | River corner: east to south |
| -ESW | 0111   | `0xF7`     | River T-junction: east, south, west |
| N--- | 1000   | `0xBB`     | River flowing north |
| N--W | 1001   | `0xC0`     | River corner: north to west |
| N-S- | 1010   | `0xED`     | River flowing north-south |
| N-SW | 1011   | `0xFF`     | River T-junction: north, south, west |
| NE-- | 1100   | `0xBD`     | River corner: north to east |
| NE-W | 1101   | `0xF3`     | River T-junction: north, east, west |
| NES- | 1110   | `0xFB`     | River T-junction: north, east, south |
| NESW | 1111   | `0x1D4`    | River crossroads: all four directions |

---

## Lake System (Landlocked Ocean Conversion)

In Master of Magic, ocean squares should never appear landlocked inside a landmass. The game handles this through a multi-step process during map generation:

### Step 1: Ocean Autotiling (Simex_Autotiling)

When `Simex_Autotiling()` processes ocean squares, it builds the same 8-bit adjacency bitmask used for shores. If all 8 neighbors are land (mask = 255), the TERRTYPE.LBX lookup returns `_1Lake` (`0x12`) instead of a shore variant. This converts landlocked ocean into a lake.

### Step 2: Lake Normalization (River_Terrain)

After river generation, `River_Terrain()` post-processes every lake square based on how many rivers flow into it from cardinal directions:

| River Inflows | Result | Game Value |
|---------------|--------|------------|
| 0 (no rivers) | Lake dries up → desert | `0x134` (`_1Desert`) |
| N only | Directional lake (river from north) | `0xC6` (`_1LakeRiv_N`) |
| E only | Directional lake (river from east) | `0xC7` (`_1LakeRiv_E`) |
| S only | Directional lake (river from south) | `0xC8` (`_1LakeRiv_S`) |
| W only | Directional lake (river from west) | `0xC5` (`_1LakeRiv_W`) |
| 2+ rivers | Keep one inflow, convert excess river neighbors to grassland, backtrack and re-process |

The multiple-inflow case is destructive: ReMoM converts neighboring river squares to grassland until only a single river feeds the lake, then backtracks the iteration to re-process affected squares.

### Lake Entries

| Game Value | Name | Description |
|------------|------|-------------|
| `0x12` | `_1Lake` | Base lake (used during autotiling, normalized before save) |
| `0xC5` | `_1LakeRiv_W` / `tt_Lake1` | Lake with river inflow from west |
| `0xC6` | `_1LakeRiv_N` / `tt_Lake2` | Lake with river inflow from north |
| `0xC7` | `_1LakeRiv_E` / `tt_Lake3` | Lake with river inflow from east |
| `0xC8` | `_1LakeRiv_S` / `tt_Lake4` | Lake with river inflow from south |

### Key Rule

**There should never be a bare ocean square (`0x00`) surrounded entirely by land in a valid `.GAM` file.** Any such square should be either a lake (with river inflow) or converted to desert (if no river feeds it).

---

## Cosmetic Variants (Random, No Autotiling)

These terrain types have multiple visual variants that are assigned randomly by `Shuffle_Terrains()` during map generation. They have no adjacency logic — each variant is purely cosmetic.

### Grassland (4 variants)

| Game Value | Name |
|------------|------|
| `0xA2`     | tGrasslands (base) |
| `0xAC`     | tGrass2 |
| `0xAD`     | tGrass3 |
| `0xAE`     | tGrass4 |

Assigned by `Random(4)` — each variant has equal probability.

### Forest (3 variants)

| Game Value | Name |
|------------|------|
| `0xA3`     | tForest (base) |
| `0xAF`     | tForest2 |
| `0xB0`     | tForest3 |

Assigned by `Random(3)`.

### Swamp (3 variants)

| Game Value | Name |
|------------|------|
| `0xA6`     | tSwamp (base) |
| `0xB1`     | tSwamp2 |
| `0xB2`     | tSwamp3 |

Assigned by `Random(3)`.

### Known Bug: Tundra Shuffle

`Shuffle_Terrains()` also shuffles tundra squares, but this runs **after** `Simex_Autotiling()` has already assigned correct tundra adjacency variants. The shuffle overwrites them with random base tundra values, breaking the autotiling. This is a bug in the original game.

---

## Special Terrain Entries

### Ocean

| Game Value | Name | Notes |
|------------|------|-------|
| `0x00`     | tOcean | Standard ocean |
| `0x01`     | BugGrass | Displays as grassland; a leftover bug entry |
| `0x259`    | Animated ocean | Used for the animated water effect |

### Nodes (Magic Power Sources)

| Game Value | Name |
|------------|------|
| `0xA8`     | tSorcery_Node |
| `0xA9`     | tNature_Node |
| `0xAA`     | tChaos_Node |

Nodes display as grassland terrain with a special overlay. They are functionally treated as grassland for terrain purposes.

### Volcano

| Game Value | Name | Notes |
|------------|------|-------|
| `0xB3`     | tVolcano | Single variant, no autotiling |

---

## Isolated and Surrounded Variants

Some terrain types have special entries for fully isolated squares (no same-type neighbors) and fully surrounded squares (all neighbors are same type):

| Terrain  | Isolated (no same-type neighbors) | Surrounded (all same-type neighbors) |
|----------|-----------------------------------|--------------------------------------|
| Shore    | N/A (shores are always near land)  | N/A |
| Desert   | `0x134` (`_1Desert`)               | `0xA5` (base, bitmask `0x00`) |
| Tundra   | `0x26A` (`_1Tundra`)               | `0xA7` (base, bitmask `0x00`) |
| Mountain | `0xA4` (base, NESW `0000`)         | `0x105` (NESW `1111`) |
| Hill     | `0xAB` (base, NESW `0000`)         | `0x115` (NESW `1111`) |
| River    | `0xB9` (displays as S-only)        | `0x1D4` (crossroads) |

---

## Game Value Ranges Summary

This table shows how the full `0x000`-`0x2FA` range is allocated:

| Range | Count | Terrain Type | System |
|-------|-------|-------------|--------|
| `0x00`-`0x01` | 2 | Ocean | Fixed |
| `0x02`-`0xA1` | 160 | Shore (primary set) | 8-bit adjacency |
| `0xA2` | 1 | Grassland (base) | Cosmetic |
| `0xA3` | 1 | Forest (base) | Cosmetic |
| `0xA4` | 1 | Mountain (isolated) | 4-bit cardinal |
| `0xA5` | 1 | Desert (surrounded) | 8-bit adjacency |
| `0xA6` | 1 | Swamp (base) | Cosmetic |
| `0xA7` | 1 | Tundra (surrounded) | 8-bit adjacency |
| `0xA8`-`0xAA` | 3 | Nodes | Fixed |
| `0xAB` | 1 | Hill (isolated) | 4-bit cardinal |
| `0xAC`-`0xAE` | 3 | Grassland variants | Cosmetic |
| `0xAF`-`0xB0` | 2 | Forest variants | Cosmetic |
| `0xB1`-`0xB2` | 2 | Swamp variants | Cosmetic |
| `0xB3` | 1 | Volcano | Fixed |
| `0xB4`-`0xB8` | 5 | (unused/reserved) | — |
| `0xB9`-`0xC4` | 12 | River (primary set) | 4-bit cardinal |
| `0xC5`-`0xC8` | 4 | Lake directional variants | River inflow |
| `0xC9`-`0xE8` | 32 | Shore (extended set) | 8-bit adjacency |
| `0xE9`-`0x102` | 26 | River (extended set) | 4-bit cardinal |
| `0x103`-`0x112` | 16 | Mountain variants | 4-bit cardinal |
| `0x113`-`0x122` | 16 | Hill variants | 4-bit cardinal |
| `0x123` | 1 | Hill (extra) | 4-bit cardinal |
| `0x124`-`0x1C3` | 160 | Desert variants | 8-bit adjacency |
| `0x1C4`-`0x1D3` | 16 | Shore (tertiary set) | 8-bit adjacency |
| `0x1D4`-`0x1D8` | 5 | River (extended) | 4-bit cardinal |
| `0x1D9`-`0x258` | 128 | Shore (quaternary set) | 8-bit adjacency |
| `0x259` | 1 | Animated ocean | Fixed |
| `0x25A`-`0x2F9` | 160 | Tundra variants | 8-bit adjacency |

**Total: 762 entries (`TerType_Count = 0x2FA`)**

---

## Practical Guide for Modders

### Writing a .GAM File

When writing terrain to a `.GAM` file, you must compute the correct game value for each square based on its neighbors. The steps are:

1. **Place shores first.** Scan all ocean squares. Any ocean square with at least one land neighbor (8-way check) becomes a shore. Shores go on the ocean side, not the land side.

2. **Compute 8-bit adjacency** for shore, tundra, and desert squares:
   - Build the 8-bit bitmask from neighbors
   - Apply diagonal forcing (both flanking cardinals set → force diagonal)
   - Look up the game value from the shore table
   - For tundra: add `0x258` to the shore value
   - For desert: add `0x122` to the shore value
   - If lookup returns 0, apply diagonal fallback masking and retry
   - If still 0, use the isolated variant (`_1Desert` or `_1Tundra`) or the base value

3. **Compute 4-bit connectivity** for mountain, hill, and river squares:
   - Build the 4-bit NESW bitmask
   - Look up the game value from the appropriate table

4. **Assign cosmetic variants** for grassland, forest, and swamp:
   - Pick a random variant from the available set for each square

### Reading a .GAM File

When reading terrain from a `.GAM` file back to base terrain types, use these game value ranges:

```
0x00, 0x01           → Ocean
0x02-0xA1            → Shore
0x12                 → Lake (check before shore range)
0xA2, 0xAC-0xAE      → Grassland
0xA3, 0xAF-0xB0      → Forest
0xA4, 0x103-0x112    → Mountain
0xA5, 0x124-0x1C3    → Desert
0xA6, 0xB1-0xB2      → Swamp
0xA7, 0x25A-0x2F9    → Tundra
0xA8-0xAA            → Node (treat as Grassland)
0xAB, 0x113-0x123    → Hill
0xB3                 → Volcano
0xB9-0xC4, 0xE9-0x102, 0x1D4-0x1D8 → River
0xC5-0xC8            → Lake (directional, with river inflow)
0xC9-0xE8, 0x1C4-0x258 → Shore (extended ranges)
```

### Key Gotchas

1. **Lake value `0x12` falls inside the shore range** (`0x02`-`0xA1`). Check for lake first.
2. **BugGrass at `0x01`** looks like grassland but is technically ocean. Original game bug.
3. **Tundra shuffle bug** means loaded games may have incorrect tundra autotiling from the original map generator.
4. **The game does not re-run autotiling on load.** Your exported values must be correct.
5. **`.GAM` files are exactly 123,300 bytes.** No more, no less.
6. **All struct fields use 2-byte alignment** (`#pragma pack(push, 2)`) matching the original 8086 executable.
