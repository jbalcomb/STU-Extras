# Quickstart: Wizard Profile Generation

**Feature**: 006-wizard-profile-gen
**Date**: 2026-03-13

## Integration Scenario 1: Generate All Wizards (Happy Path)

1. Launch editor, no wizards configured
2. Click "Generate Wizards" button (ToolPanel, near existing map Generate button)
3. All 6 wizard slots are populated:
   - Slot 0: Merlin (Life 5, Nature 5, Sage Master, High Men)
   - Slot 1: Raven (Sorcery 6, Nature 5, no retort, High Men)
   - Slot 2: Sharee (Death 5, Chaos 5, Conjurer, High Men)
   - Slot 3: Lo Pan (Sorcery 5, Chaos 5, Channeler, High Men)
   - Slot 4: Jafar (Sorcery 10, Alchemy, High Men)
   - Slot 5: Oberic (Nature 5, Chaos 5, Mana Focusing, High Men)
4. Status bar shows "Generated 6 wizard profiles"
5. Switch to Wizard tool → all wizard tabs show populated data
6. Undo → all wizards revert to empty

## Integration Scenario 2: Generate with Dirty Wizards

1. Place a fortress → wizard 0 auto-stubbed as "Wizard 0"
2. Switch to Wizard tool, select wizard 0 tab
3. Change name from "Wizard 0" to "Custom Mage" → wizard 0 is now dirty
4. Click "Generate Wizards" button
5. Modal dialog appears: "Overwrite modified wizards? Wizard 0: Custom Mage"
6. Click "Overwrite" → all 6 wizards generated, wizard 0 becomes "Merlin"
7. Dirty flags cleared for all wizards
8. Undo → wizard 0 is "Custom Mage" again, dirty flag restored

## Integration Scenario 3: Generate Single Wizard

1. Generate all wizards (scenario 1)
2. Change wizard 2's name to "Dark Sorceress" → wizard 2 is dirty
3. Open wizard 3 tab in Wizard Panel
4. Click "Generate" button on wizard 3's tab
5. Wizard 3 is regenerated as Lo Pan (no confirmation — not dirty)
6. Open wizard 2 tab, click "Generate"
7. Modal dialog: "Overwrite modified wizard? Wizard 2: Dark Sorceress"
8. Click "Cancel" → wizard 2 unchanged
9. Click "Generate" again, then "Overwrite" → wizard 2 becomes "Sharee"

## Integration Scenario 4: Edit After Generation

1. Generate all wizards
2. Open wizard 0 tab, change spell books from Life 5 / Nature 5 to Life 10
3. Wizard 0 is now dirty
4. Click "Generate Wizards" → confirmation shows wizard 0 modified
5. Cancel → wizard 0 keeps Life 10 customization
6. Change wizard 1 name to "Corvus" → wizard 1 is dirty
7. Click "Generate Wizards" → confirmation shows wizards 0 and 1 modified

## Verification Checks

- [ ] Generated wizard data matches preset table (spell books, retort, race, name)
- [ ] Dirty flag set on any manual wizard edit (name, books, retort, race)
- [ ] Dirty flag cleared on generation
- [ ] Dirty flag cleared on load (session-only)
- [ ] Modal dialog appears only when dirty wizards exist in scope
- [ ] Modal dialog lists all affected dirty wizard names
- [ ] Cancel on modal leaves all data unchanged
- [ ] Overwrite on modal applies presets and clears dirty flags
- [ ] Undo restores wizard data AND dirty flag state
- [ ] Per-wizard generate only affects the target slot
- [ ] Global generate affects all 6 slots
