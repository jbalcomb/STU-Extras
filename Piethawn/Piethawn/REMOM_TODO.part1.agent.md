# Agent task: write the code for ReMoM's comment-only ids (REMOM_TODO.md, Part 1)

## Goal

ReMoM (`/c/STU/devel/ReMoM`) is a C reconstruction of Master of Magic v1.31 (WIZARDS.EXE and MAGIC.EXE). Each
reconstructed function has an id comment above it that names the original procedure, e.g. `// WZD o56p14` (WIZARDS.EXE,
IDA segment ovr056, 14th procedure) or `// MGC s25p02` (MAGIC.EXE, segment seg025, 2nd procedure).

Each item in **Part 1** of `/c/STU/devel/STU-Extras/Piethawn/Piethawn/REMOM_TODO.md` is an id comment with no
C function bound to it. The comment is there, often with notes or pasted disassembly, but the tooling found no
definition for it. Your job is to make every item end up with exactly one faithful C function.

## Ground rules

- Read and follow `/c/STU/devel/ReMoM/AGENTS.md`. It overrides anything here that conflicts with it. The main rules
  are: 100% fidelity to the disassembly; never delete faithful code or comments; mark your changes `/* CLAUDE */`; Allman
  braces; `Proper_Snake_Case` function names; `lower_snake_case` variables.
- **Work in place** in `/c/STU/devel/ReMoM` on the current branch (`bq-base`), which has a lot of the user's
  uncommitted work. **Do not commit, stash, reset, switch branches, or reformat anything.** Change only the lines your
  items need. The user reviews and commits.
- Don't run this at the same time as another `REMOM_TODO.part*.agent.md` task: they share the working tree.
- Never launch IDA. In the Piethawn repository, run only the `--report-only` command below.

## How to read an item

```
- [ ] **Map_File_Animation_Frame_ASM** — `WZD s025p02` 1E9F:01D5 · `MGC s025p02` 1E0D:01D1 — mom131 `Anim_EMS_Map` `src/wizards/ANIMEMS.ASM:255` — ReMoM comment `MoX/src/file_ani.c:461`
```

| Part | Meaning |
|---|---|
| **Map_File_Animation_Frame_ASM** | Our name, from our IDA database. **The name to use** (see Naming). |
| `WZD s025p02` 1E9F:01D5 | The id and its IDA address (segment:offset). |
| `MGC s025p02` ... | The same function in MAGIC.EXE. Two ids mean both programs link the same code, so it is one ReMoM function. |
| mom131 `Anim_EMS_Map` `src/...:255` | BinAlgo's name and source line for the same procedure. For orientation only; it also tells you which batch the item belongs to. |
| ReMoM comment `MoX/src/file_ani.c:461` | Where the id comment is. Line numbers shift as you edit; rerun `--report-only` to refresh them. |

The disassembly for each item is `/c/STU/devel/STU-Extras/Piethawn/Piethawn/out/WIZARDS/<segment>/<IDA name>.asm`.
For an MGC-only item, look under `out/MAGIC/...`. The segment is the id's `sNNN` → `segNNN` or `oNNN` → `ovrNNN`; `_AIL_`
and `SOUND` are WZD s037/s038 and MGC s035/s036. A `.c` file next to the `.asm`, if there is one, is an earlier
translation attempt. Check it against the asm before you use any of it.

## First: fix mistyped id comments

If `REMOM_TODO.md` starts with a **Fix first** list, correct those id comments (e.g. `// WZD  s14o03` → `// WZD s14p03`)
and rerun `--report-only`. A mistyped id hides an existing function, so its item can drop out of Part 1 without any new
code.

## Before writing any item: make sure the code does not already exist

An id comment is a **tag for the function signature directly below it**. Only comments, blank lines and
preprocessor lines may come between. A tag with code or data after it (helpers, variables) binds nothing, so the item
looks like "no code yet" even when the function exists further down. Areas of code are marked with BEGIN/END notes,
not with the tag. So for each item:

1. Grep ReMoM (word boundary, `*.c` and `*.h`) for the bold name. Also grep for any name in the id comment's notes
   (`// drake178: ...`, `// MoO2 ...`).
2. If a definition exists and the asm confirms it is this procedure, **don't write a second one.** Put the id
   comment(s) directly above that definition. Turn the old tag into a plain note, so it is no longer a tag:
   `// WZD o147p01` → `// /* CLAUDE */ moved: WZD o147p01 -> <file>:<line>`. If it marks an area, use
   `BEGIN:`/`END:` notes instead. Rerun `--report-only`; the item must disappear.
3. **MGC-only items whose IDA name is a WZD function's name, or the same name plus `_MGC`/`__MGC`.** Diff the two asm
   files, ignoring addresses.
   - If the code is identical, write no code. Put `// === // WZD <id>  <WZD function>()` on the line under the MGC id
     comment; the tooling reads that line as "same code".
   - If anything differs (constants, help entries, calls), it is a separate function. Write it as `<WZD name>_MGC`.
     Example: MAGIC's load screen (o050p03/04/28) is **not** the same as WIZARDS' o160p01-03.

## Order of work: one mom131 source file per batch

The items are grouped by mom131 source file. Do them in this order:

1. **Game logic** (plain C that ReMoM runs). AIORDERS (AIMOVE.c), ECONOMY, ORDERS, CITY, CITYCALC, DIPLO*,
   MAPDRAW, MAPUNITS, MOVECHK, MOVESTK, NEXTTURN, SPELLTGT, STACKSEL, TREASURE, AITURN, AISPELL, WORLDGEN, ARMYLIST,
   the removed loader routines (LOADER.c), and every other `src/wizards` or `src/magic` C file not listed in 3.
2. **Portable MoX helpers.** UTIL (memory routines), TEXT, PARAGRPH, PIC, GRAPHICS, DRAW.ASM (gradients become
   plain bitmap operations), LINEPATH, PATHWIN, UI, HEAP.
3. **Hardware routines, last.** GFX.ASM, TRANSIT.C, ERROR.C, PALETTE.C (the DAC routines), DOSIO.ASM, PICEMS.ASM,
   UTIL.C (BIOS/clock), PICDEC.ASM, DOSEXIT.ASM, SOUND.C, ANIMEMS.ASM, DOSFIND.ASM, TIMER.ASM. In a mixed file, a
   function goes here if its asm does port I/O (`in`/`out`), BIOS/DOS/EMS interrupts (`int 10h/15h/1Ah/21h/67h`), or
   EMS paging. Write each as a stub: the right signature, a `/* HACK */` body that does the nearest portable thing (or
   nothing), and a `// DEDU` note that says what the original does. Copy how neighbouring functions in the same ReMoM
   file handle this. Where ReMoM already calls the C library directly (e.g. `fopen`/`fclose` for `lbx_open`/
   `lbx_close`), the stub just wraps that call.

Each batch runs this loop:

1. **Write** each function from the asm, directly below its id comment block (keep everything already there). Put any
   helper you need *above* the id comment, never between the tag and the function.
   Use the existing ReMoM types, globals and helpers; find them by grep and don't invent duplicates. Match an existing
   prototype, or add one to the module's header.
2. **Build:** `cmake --build --preset MSVC-debug` in `/c/STU/devel/ReMoM`. Treat warnings as errors.
3. **Check binding:** run the `--report-only` command below. The batch's items must be gone from Part 1.
4. **Review 1:1 against the asm** before you start the next batch, function by function:
   - every call is there, in the same order, with the same arguments;
   - every constant and every global matches;
   - every branch matches, including signed vs. unsigned compares (`jl`/`jg` vs. `jb`/`ja`), loop bounds, and
     fall-throughs;
   - return values match;
   - nothing is added that the asm doesn't have.

   Note anything you could not settle as a `// DEDU` comment in the code and in your hand-back. A clean build is not a
   review.

## Naming rules

- **Use the bold name.** Never use the mom131 name: ReMoM names are the authority, and mom131 is renamed to match them.
- If the bold name is a placeholder (`sub_XXXXX`, `fxn_oNNpNN`, `nullsub_N`, `j_...`), choose a descriptive
  `Proper_Snake_Case` name from what the code does, with no `UU_` prefix. If nothing in the disassembly calls it, and
  nothing calls its overlay stub thunk, say so in its comment block (`XREF: none - no caller in WIZARDS.EXE`). Keep
  `UU_`/`NIU_`/`IDK_`/`USELESS_` prefixes that are already in the bold name.
- If the bold name starts with `_` or is a C runtime name (e.g. `_fmemcpy`), choose a new name that doesn't.
- 32 characters or fewer (Borland C keeps 32). Not a C keyword, and not a name Borland's headers use (`DIR`, `Fill`,
  `Line`, `Path`, `exit`, `abs`, ...).
- If the item shows both a WZD id and an MGC id, write one function with both id comments stacked above it, WZD first:
  ```c
  // WZD s25p02
  // MGC s25p02
  ```

## About mom131 (BinAlgo's source)

`/c/STU/devel/mom131-source-map` builds a byte-identical v1.31. **Do not copy code from it into ReMoM**: its repository
has no license. Write from the disassembly. You may use mom131 to check your reading of the asm, or to work out what a
placeholder does.

## Check command

From Git Bash:

```bash
python3 -B /c/STU/devel/STU-Extras/Piethawn/Piethawn/run_mom131_name_sync.py --report-only
```

This rescans ReMoM and regenerates `REMOM_TODO.md`, so finished items drop out and line numbers refresh. Don't tick
boxes by hand. Any of the following means an id is bound wrong or a name breaks a rule; fix it before going on:
- a new line under "Rename plan" for a function you wrote;
- a HALT message;
- an `ambiguous` or `multiple-functions` count.

## Hand back

- The `--report-only` summary line from before you started and after each batch.
- Per batch: functions written, as clickable `file:line` links, with your review notes and open `// DEDU` points.
- Items that already had code, and what you did about each.
- MGC copies handled with `// === //`, and MGC variants written as `_MGC`.
- Every name you coined, as `old IDA name → new name`, so the IDA database can be renamed to match.
- Hardware items stubbed, and items skipped, with the reason for each.
