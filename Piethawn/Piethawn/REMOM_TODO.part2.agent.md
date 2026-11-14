# Agent task: give ReMoM an id for every untagged procedure (REMOM_TODO.md, Part 2)

## Goal

ReMoM (`/c/STU/devel/ReMoM`) is a C reconstruction of Master of Magic v1.31 (WIZARDS.EXE and MAGIC.EXE). Each
reconstructed function has an id comment above it that names the original procedure, e.g. `// WZD o56p14` (WIZARDS.EXE,
IDA segment ovr056, 14th procedure) or `// MGC s25p02` (MAGIC.EXE, segment seg025, 2nd procedure).

Each item in **Part 2** of `/c/STU/devel/STU-Extras/Piethawn/Piethawn/REMOM_TODO.md` is an original procedure that
**no ReMoM id comment names**. Often the function already exists in ReMoM, untagged or under another name. Sometimes it
is missing. Your job: every item ends up as exactly one ReMoM function with its id comment(s).

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
- [ ] **Draw_File_Animation_Frame_ASM** — `WZD s025p01` 1E9F:000E · `MGC s025p01` 1E0D:000A — mom131 `Anim_Blit_EMS_Frame` `src/wizards/ANIMEMS.ASM:29`
```

| Part | Meaning |
|---|---|
| **Draw_File_Animation_Frame_ASM** | Our name, from our IDA database. **The name to use** for new code (see Naming). |
| `WZD s025p01` 1E9F:000E | The id and its IDA address (segment:offset). In ReMoM it is written without leading zeros: `// WZD s25p01`. |
| `MGC s025p01` ... | The same function in MAGIC.EXE. Two ids mean both programs link the same code, so it is one ReMoM function with both id comments. |
| mom131 `Anim_Blit_EMS_Frame` `src/...:29` | BinAlgo's name and source line for the same procedure. For orientation only; it also gives the batch. |

The disassembly is `/c/STU/devel/STU-Extras/Piethawn/Piethawn/out/WIZARDS/<segment>/<IDA name>.asm`. For an MGC-only
item, look under `out/MAGIC/...`. The segment is the id's `sNNN` → `segNNN` or `oNNN` → `ovrNNN`.

## Steps for each item

1. **Look for it in ReMoM first.** Search in this order:
   - a word-boundary grep for the bold name, then for `drake178: <name>` notes;
   - its neighbours. Find the ids before and after it in the same segment (`pNN±1`) in
     `/c/STU/devel/STU-Extras/Piethawn/Piethawn/in/name-snapshots/mom131-remom.name-map.tsv` (columns `id`,
     `remom_function`, `remom_file`, `remom_line`). The missing function usually sits between them in the same ReMoM
     file, untagged;
   - its callers and callees in the asm, and where ReMoM calls those.
2. **If it exists**, check it against the asm, then add the id comment(s) directly above the definition, WZD first when
   there are two:
   ```c
   // WZD s25p01
   // MGC s25p01
   ```
   The tag must sit directly above the signature: only comments, blank lines and preprocessor lines may come between.
   A tag with a helper or a variable between it and the function binds nothing, so move helpers above the tag. Tags
   in `.h` files count too, and every tag of one id must name the same function. If ReMoM's name differs from the bold
   name, keep ReMoM's name and list the pair in your hand-back. Don't rename existing ReMoM code.
3. **For an MGC-only item that is the same code as a WZD procedure ReMoM already has**, diff the two asm files,
   ignoring addresses. If they are identical, add an MGC id comment followed by `// === // WZD <id>  <function>()` next
   to the WZD function's comment, and write no code. If anything differs, it is a separate function named
   `<WZD name>_MGC`.
4. **If it does not exist**, write it. Put it in the ReMoM file that holds its neighbouring ids, in procedure order,
   with its id comment(s). Write it faithful to the asm, and match or add a prototype in the module's header. Hardware
   routines (port I/O; BIOS, DOS or EMS interrupts) go in the last batch as stubs: the right signature, a `/* HACK */`
   body, and a `// DEDU` note.

Work one mom131 source file per batch, with hardware routines last. Each batch runs this loop:

1. Tag or write.
2. Build: `cmake --build --preset MSVC-debug` in `/c/STU/devel/ReMoM`. Treat warnings as errors.
3. Run `--report-only`.
4. Review 1:1 against the asm before the next batch:
   - calls, arguments and their order;
   - constants and globals;
   - branches, including signed vs. unsigned compares;
   - return values;
   - nothing added.

   For code that was already there, the check is that it really is this procedure. A clean build is not a review.

## Naming rules

- New code uses the bold name, never the mom131 name.
- A placeholder bold name (`sub_XXXXX`, `fxn_oNNpNN`, `nullsub_N`, `j_...`) gets a descriptive `Proper_Snake_Case`
  name, with no `UU_` prefix. If nothing calls it, and nothing calls its overlay stub thunk, write
  `XREF: none - no caller in WIZARDS.EXE` in its comment block.
- A bold name starting with `_` or a C runtime name gets a new name that doesn't.
- 32 characters or fewer. Not a C keyword, and not a name Borland's headers use (`DIR`, `Fill`, `Line`, `Path`, ...).
- One id comment per id. Never put the same id above two functions.

## About mom131 (BinAlgo's source)

`/c/STU/devel/mom131-source-map` builds a byte-identical v1.31. **Do not copy code from it into ReMoM**: its repository
has no license. You may use it to confirm which procedure is which, or to work out what a placeholder does.

## Check command

From Git Bash:

```bash
python3 -B /c/STU/devel/STU-Extras/Piethawn/Piethawn/run_mom131_name_sync.py --report-only
```

This rescans ReMoM and regenerates `REMOM_TODO.md`; finished items drop out, so don't tick boxes. If you tagged code
whose ReMoM name differs, it appears under "Rename plan". That is expected; list it. A HALT message, an `ambiguous` or
`multiple-functions` count, or a name over 32 characters means something is wrong; fix it.

## Hand back

- The `--report-only` summary line from before you started and after each batch.
- Items tagged on existing code: `id → function (file:line link)`, and whether ReMoM's name differs from the bold name.
- MGC copies handled with `// === //`, and MGC variants written as `_MGC`.
- Items written new: `id → function (file:line link)`, with review notes and open `// DEDU` points.
- Every name you coined, as `old IDA name → new name`.
- Hardware items stubbed, and items skipped, with the reason for each.
