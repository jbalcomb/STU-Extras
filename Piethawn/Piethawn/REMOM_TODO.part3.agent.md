# Agent task: tag ReMoM's AIL sound-library stand-ins with their ids (REMOM_TODO.md, Part 3)

## Goal

ReMoM (`/c/STU/devel/ReMoM`) is a C reconstruction of Master of Magic v1.31 (WIZARDS.EXE and MAGIC.EXE). Each
reconstructed function has an id comment above it that names the original procedure. Both programs link Miles' Audio
Interface Library (AIL 2.x) as one assembly module, which IDA calls the segment `_AIL_`.

Each item in **Part 3** of `/c/STU/devel/STU-Extras/Piethawn/Piethawn/REMOM_TODO.md` is an AIL procedure without
a ReMoM id comment. ReMoM does not reconstruct AIL. It replaces it with a platform sound layer, and
`/c/STU/devel/ReMoM/MoX/src/AIL.c` holds stand-ins for the AIL entry points (`AIL_startup`, `AIL_register_driver`,
...). Your job: put each AIL procedure's id comments above its stand-in, and report the procedures that have none.

## Ground rules

- Read and follow `/c/STU/devel/ReMoM/AGENTS.md`. It overrides anything here that conflicts with it. The main rules
  are: never delete code or comments; mark your changes `/* CLAUDE */`.
- **Work in place** in `/c/STU/devel/ReMoM` on the current branch (`bq-base`), which has a lot of the user's
  uncommitted work. **Do not commit, stash, reset, switch branches, or reformat anything.** Add only id comments.
- Don't run this at the same time as another `REMOM_TODO.part*.agent.md` task: they share the working tree.
- Never launch IDA. In the Piethawn repository, run only the `--report-only` command below.

## Ids for AIL

| Program | `_AIL_` | `SOUND` |
|---|---|---|
| WIZARDS.EXE | segment 37, written `// WZD s37pNN` | segment 38, `// WZD s38pNN` |
| MAGIC.EXE | segment 35, written `// MGC s35pNN` | segment 36, `// MGC s36pNN` |

ReMoM already uses `SOUND` ids; see `MoX/src/SOUND.c`. Both programs link the same AIL, so an item usually shows a
WZD id and an MGC id. Stack both id comments above the one stand-in, WZD first.

## How to read an item

```
- [ ] **_AIL_startup** — `WZD s037p12` 2FC1:06CD · `MGC s035p12` 2E35:06CF — mom131 `AIL_startup` `src/ail/AIL.ASM:824`
```

The bold name is the IDA name. AIL's API names (`AIL_startup`) are Miles' published names, and ReMoM's stand-ins
already use them, so keep ReMoM's names. The AIL internals (`read_CMOS`, `RTC_set_freq`, ...) have no ReMoM counterpart.

## Steps

1. For each item, find its stand-in in `MoX/src/AIL.c` (or `AIL.h`, `MOX_SND.c`) with a word-boundary grep. Strip
   the leading underscore of the IDA name.
2. If there is one, add the id comments directly above the definition. Don't change the function.
   ```c
   // WZD s37p12
   // MGC s35p12
   // drake178: _AIL_startup()
   ```
3. If there is none (an internal helper, or an entry point ReMoM never calls), **don't write one**. List it in the
   hand-back. Adding stubs for unused library internals is the user's call.
4. Build: `cmake --build --preset MSVC-debug` in `/c/STU/devel/ReMoM`.
5. Run the check command. The tagged items must be gone from Part 3.

## About mom131 (BinAlgo's source)

`/c/STU/devel/mom131-source-map/src/ail/AIL.ASM` is the AIL source in procedure order. Use it to match IDA's
procedures to the AIL API names. Don't copy code from it into ReMoM.

## Check command

From Git Bash:

```bash
python3 -B /c/STU/devel/STU-Extras/Piethawn/Piethawn/run_mom131_name_sync.py --report-only
```

This regenerates `REMOM_TODO.md`; finished items drop out, so don't tick boxes. Items you tagged will show up under
"Rename plan" where ReMoM's name differs from the mom131 name; that is normal. A HALT message, or an `ambiguous` or
`multiple-functions` count, means an id is on the wrong function; fix it.

## Hand back

- The `--report-only` summary line, before and after.
- Tagged: `id(s) → ReMoM function (file:line link)`.
- Not tagged, no stand-in: the ids and IDA names.
