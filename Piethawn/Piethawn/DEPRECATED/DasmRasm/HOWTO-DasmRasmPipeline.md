


IDA Pro v5.5
    ¿ Options / Settings ?
    File > Produce file >
    Create MAP file...
    Create ASM file...
    Create INC file...
    Create LST file...
c:/python314/python.exe C:\STU\devel\STU-Extras\Piethawn\Piethawn\ida_dasm_export\ida55_copy_outputs.py
    From:   C:\STU_DASM\IDA55_MOM131\
    To:     C:\STU\devel\STU-Extras\Piethawn\Piethawn\in\
    MAGIC.MAP, MAGIC.ASM, MAGIC.INC, MAGIC.LST, WIZARDS.MAP, WIZARDS.ASM, WIZARDS.INC, WIZARDS.LST

c:/python314/python.exe C:\STU\devel\STU-Extras\Piethawn\Piethawn\ida_dasm_export\ida55_clean_export.py
c:/python314/python.exe C:\STU\devel\STU-Extras\Piethawn\Piethawn\ida_dasm_export\ida55_copy_outputs.py

delete stale .asm files from out directory
redo segment and procedure extractions
reconcile procedure name changes - apply function renames to .c files, MGC & WZD function todo, and Gemini translation checklist
check and update MGC & WZD function completion and Gemini translation checklist





# Dasm/Rasm Side-Project TODO Refresh

Last refreshed: 2026-04-26.

This file is the working scratchpad for turning IDA 5.5 assembly dumps into C89 candidate translations with Gemini and Antigravity. The live automation is in:

- `rasm_named_gemini.py`: run Gemini for a specific function name, whether or not it is marked `__WIP` or `__STUB`.
- `rasm_named_antigravity.py`: run Antigravity for a specific function name, whether or not it is marked `__WIP` or `__STUB`.
- `rasm_todo_gemini.py`: build a STUB/WIP queue from the external ReMoM todo file if present, or synthesize one from local dumps and `out/`.
- `rasm_random_todo_gemini.py`: pick one pending STUB/WIP item at random and run it.
- `Rasm-Gemini.cmd`: PowerShell/Gemini wrapper that performs API preflight, sends one ASM file to Gemini, and writes the generated `.c`.
- `Rasm-Antigravity.cmd`: PowerShell/Antigravity wrapper that sends one ASM file to Antigravity and writes the generated `.c`.
- `gemini_api_status.py`: quick backend reachability check used by the wrapper.

## Current Snapshot

- The hand-maintained checklist below currently has 107 checked `rasm_named_gemini.py` runs and 40 unchecked commands.
- The automated TODO queue currently reports 237 entries and 141 pending items via `rasm_todo_gemini.py list`.
- The local `out/MAGIC` and `out/WIZARDS` trees currently contain 403 `__WIP`/`__STUB` ASM inputs.
- The local `out/MAGIC` and `out/WIZARDS` trees currently contain 65 generated `__WIP`/`__STUB` C outputs.
- The active focus near the current editor state is `AI_Set_Move_Or_Goto_Target`, with generated files under `out/WIZARDS/ovr158/`.

## Normal Usage

Run one named function:

```powershell
c:/python314/python.exe .\rasm_named_gemini.py AI_Set_Move_Or_Goto_Target
```

List matches before running when a name may exist in both games or multiple segments:

```powershell
c:/python314/python.exe .\rasm_named_gemini.py AI_Set_Move_Or_Goto_Target --list-matches
```

Force regeneration if the `.c` output already exists:

```powershell
c:/python314/python.exe .\rasm_named_gemini.py City_Screen__WIP --force
```

Dry-run any named command before spending Gemini quota:

```powershell
c:/python314/python.exe .\rasm_named_gemini.py AI_UNIT_Move --dry-run
```

Run the raw ASM wrapper directly:

```powershell
.\Rasm-Gemini.cmd out\WIZARDS\ovr158\AI_Set_Move_Or_Goto_Target.asm
```

## Queue Usage

List the next TODO queue entries:

```powershell
c:/python314/python.exe .\rasm_todo_gemini.py list
```

Run the next pending queue entry:

```powershell
c:/python314/python.exe .\rasm_todo_gemini.py next
```

Run one queue item by exact STUB/WIP name:

```powershell
c:/python314/python.exe .\rasm_todo_gemini.py run AITP_CombatSpell__STUB
```

Pick a random pending item:

```powershell
c:/python314/python.exe .\rasm_random_todo_gemini.py
```

Useful filters:

```powershell
c:/python314/python.exe .\rasm_todo_gemini.py list --game WIZARDS --status WIP --limit 50
c:/python314/python.exe .\rasm_todo_gemini.py next --prefer-game WIZARDS
c:/python314/python.exe .\rasm_random_todo_gemini.py --status STUB --seed 1234
```

## Gemini Health Checks

Check API/backend status directly:

```powershell
c:/python314/python.exe .\gemini_api_status.py
```

Skip wrapper preflight for one run only when you already know the backend check is the problem:

```powershell
$env:RASM_GEMINI_SKIP_API_STATUS = '1'
.\Rasm-Gemini.cmd out\MAGIC\seg022\PageFlip_GrowOut__WIP.asm
Remove-Item Env:RASM_GEMINI_SKIP_API_STATUS
```

The project-local `.gemini/settings.json` is expected to keep `context.includeDirectoryTree=false` and `ui.errorVerbosity=low`, which prevents Gemini CLI from flooding failures with the full repo tree.
