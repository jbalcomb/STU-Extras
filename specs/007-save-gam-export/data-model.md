# Data Model: Save .GAM File Export

**Feature**: 007-save-gam-export
**Date**: 2026-03-14

## Entities

### EditorState (existing, EditorState.hpp — EXTENDED)

One new flag added to support the button-to-main-loop communication pattern.

| Field | Type | Description |
|-------|------|-------------|
| export_gam_requested | bool | Set by ToolPanel button click, consumed by main loop to trigger .GAM export. Defaults to `false`. |

**Lifecycle**: Set to `true` on button click. Main loop reads it, resets to `false`, then performs the export. One-shot flag, never persisted.

### Scenario (existing — NOT MODIFIED)

The export reads the full Scenario struct (world, wizards, cities, units, nodes, lairs, towers, fortresses, items) and serializes it via the existing `serialize_gam()` function. No changes to the Scenario struct.

### GameData (existing, MomEntities.hpp — NOT MODIFIED)

The `Total_Cities` and `Total_Units` fields in `scenario.game_data` are updated before export by counting active entities. This is existing behavior from the Ctrl+E handler.

### .GAM File (existing format — NOT MODIFIED)

Fixed-size 123,300-byte binary file. No format changes.

## State Transitions

```
[Editor idle] --click "Export .GAM"--> [export_gam_requested = true]
    |
    v
[Main loop] --reads flag--> [export_gam_requested = false]
    |
    v
[Count active entities] --> [Update game_data counts]
    |
    v
[serialize_gam()] --> [platform::save_file()]
    |
    v
[set_status("Exported ...")]
```

No undo wrapping needed — export is a read-only operation that doesn't modify scenario state (entity count updates are transient header fixups for the binary output, not persistent state changes).
