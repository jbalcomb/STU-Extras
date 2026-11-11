


_world_maps
_map_square_flags
movement_mode_cost_maps

_map_square_flags
...city locations don't matchup
...missing roads, maybe because missing neutral cities

movement_mode_cost_maps should just be a copy of terrain_stats_table
...with roads update to 1 or 0



River corruption — River_Terrain() runs at MAPGEN.c:381-382 but writes terrain types, not flags. The only MSF_CORRUPTION clear is in Terrain.c:28 (Square_Purify), which isn't part of world-gen.


As a different part of the overall process,
...the big issue with getting the behavior and data to matchup is the PRNG
So, we have compare-rng-streams.py to track and compare between OG-MoM and ReMoM.

Invokable via the Agent tool with subagent_type: "mom-matchup".

1.  /home/jbalcomb/STU_DBWD/dosbox-capture/prng-port/baselines/run-hemom-stage0.py
    /home/jbalcomb/STU_DBWD/dosbox-capture/prng-port/baselines/run-remom-stage0.py
2.  /home/jbalcomb/STU_DBWD/dosbox-capture/prng-port/baselines/extract-seed-stage0.py
3.  /home/jbalcomb/STU_DBWD/dosbox-capture/prng-port/baselines/run-remom-stage0.py
4.  /home/jbalcomb/STU_DBWD/dosbox-capture/prng-port/baselines/postprocess-stage0.py
5.  /home/jbalcomb/STU_DBWD/dosbox-capture/prng-port/baselines/compare-stage0.py
Optional:
cat /tmp/stage-stage0-newgame.log
ls -la /home/jbalcomb/STU/devel/ReMoM/build/bin/Debug/ReMoMber
ls -la /home/jbalcomb/STU/devel/ReMoM/out/build/clang-debug/bin/Debug/ReMoMber


/home/jbalcomb/STU_DBWD/dosbox-capture/prng-port/baselines/analyze-cities-stage0.py
 — reads both saves, walks _CITIES in creation order (not byte-offset order), and reports:
/home/jbalcomb/STU_DBWD/dosbox-capture/prng-port/baselines/stage0-newgame/analyze-cities.txt


/home/jbalcomb/STU/devel/STU-Extras/STU-DOSBox/ReMoM-CallTrace-PRD.md

anchors.py -- anchor table for Stage 0 OG-vs-ReMoM matchup.
compare-anchors.py

/home/jbalcomb/STU_DBWD/dosbox-capture/prng-port/baselines/anchors.py
/home/jbalcomb/STU_DBWD/dosbox-capture/prng-port/baselines/compare-anchors.py
/home/jbalcomb/STU_DBWD/dosbox-capture/prng-port/baselines/compare-anchors.py --og /tmp/magic_exe-calls.log --remom /tmp/stage-stage0-newgame.log


Each anchor names a point both sides cross during a `--newgame` run.
The comparator (compare-anchors-stage0.py) reports the rng_call
counter on each side at each anchor; the delta tells you in which
interval the divergence lives.

Each entry:
    id        short slug used in the comparator output
    og_fn     function name as it appears in OG's call log
              (after the `seg__:` or `ovr__:` prefix).
              When OG's name differs from ReMoM's, list it here
              verbatim and fix when convenient.
    remom_fn  function name as it appears in ReMoM C source
    file      ReMoM source file containing the function (basename
              is what's matched; path noise is ignored)
    line      definition line in ReMoM source (used for matching
              [RNG-CALL] at=FILE:LINE within the function body, and
              for cross-referencing compare-rng-streams.py output)
    instance  which occurrence to anchor at, default 1.  Use >1
              when a function is entered multiple times and the
              later one is the meaningful anchor.

Order matters only for the report layout; the comparator scans each side independently.

ANCHORS = [
    {
        "id":       "init_new_game",
        "og_fn":    "Init_New_Game",
        "remom_fn": "Init_New_Game",
        "file":     "MAPGEN.c",
        "line":     261,
        "instance": 1,
    }
    {
        "id":       "save_save_gam",
        "og_fn":    "Save_SAVE_GAM",
        "remom_fn": "Save_SAVE_GAM",
        "file":     "LOADSAVE.c",
        "line":     40,
        "instance": 1,
    },
]


Scalars summary — _num_players, _cities, and explicitly calls out the off-by-1 direction (<< ReMoM placed more cities (+1)).
Phase 1 — Home Cities — slots 0..(_num_players-1), labeled with the producer function Generate_Home_Cities so you know where to look. Each city is side-by-side OG vs ReMoM with every s_CITY field named (name, race decoded to name, wx/wy/wp, owner_idx, population, Prod_Accu, gold_units, etc.), << marker on mismatched lines.
Phase 2 — Neutral Cities — slots _num_players..(_cities-1), labeled Generate_Neutral_Cities, same field-by-field format. This is where the +1 will surface.
Phase 3 — Tail — slots _cities..99, expected to be zero on both sides; flags any write-past-_cities anomaly.
Cascade section at the end — explicit list of downstream regions that depend on _CITIES (movement_mode_cost_maps via Make_Road, _map_square_flags's MSF_OWNED/ROAD/EROAD, world-gen terrain flattening at city tiles) so you know not to chase those until _CITIES matches.
Writes to 
jbalcomb@iMustAi:~/STU_DBWD/dosbox-capture/prng-port$ head /tmp/stage-stage0-newgame.log
[RNG-CALL] call=1  n=14  before=0x000305FD  after=0xF6000182  result=14  at=/home/jbalcomb/STU/devel/ReMoM/MoM/src/INITGAME.c:801
...this is INITGAME.c Init_Computer_Players_Wizard_Profile()

Randomize_Book_Heights
2099 NewGame.c




# Stage 0 'New Game' Comparisson Process

## Process Overview

### Stage 0 - New Game

1. Generate OG-MoM Stage 0 - 'New Game' outputs
    * SAVE1.GAM


## Process Excution

### Stage 0 - New Game

1. run /home/jbalcomb/STU_DBWD/dosbox-capture/prng-port/baselines/launch-stage0.sh

2. run /home/jbalcomb/STU_DBWD/dosbox-capture/prng-port/baselines/extract-seed-stage0.py
    runs extract-seed-stage0.py
    produces seed.txt
    populates Stage_0.ini

3. /home/jbalcomb/STU_DBWD/dosbox-capture/prng-port/baselines/run-remom-stage0.py
    * uses /home/jbalcomb/STU_DBWD/dosbox-capture/prng-port/baselines/stage0-newgame/scenario.hms

4. /home/jbalcomb/STU_DBWD/dosbox-capture/prng-port/baselines/compare-stage0.py
    == stage compare: stage0-newgame ==
    OG       : /home/jbalcomb/STU_DBWD/dosbox-capture/prng-port/baselines/stage0-newgame/canonical-save1.GAM
    ReMoM    : /home/jbalcomb/STU/devel/ReMoM/assets/SAVE1.GAM
    FAIL -- 24223 bytes differ
    full diff written to: /home/jbalcomb/STU_DBWD/dosbox-capture/prng-port/baselines/stage0-newgame/diff-bytes.txt

5. /home/jbalcomb/STU_DBWD/dosbox-capture/prng-port/baselines/compare-rng-streams.py stage0-newgame

compare-stage0.py (writes diff-bytes.txt)
diff-structured-stage0.py (writes diff-structured.txt)
    The only Stage-0-specific things in diff-structured-stage0.py are the paths:
        Constant	Value	Stage-specific
        OG_CANONICAL	stage0-newgame/canonical-save1.GAM	yes
        REMOM_OUTPUT	ReMoM/assets/SAVE1.GAM	yes (Stage 1 → SAVE3.GAM, Stage 2 → SAVE5.GAM)
        OUT_PATH	stage0-newgame/diff-structured.txt	yes
        Everything else (the byte-walk, the grouping, the field-tally, the report format) would work on any pair of SAVE.GAM files.
        So it could become diff-structured.py <stage> with the same stage-dispatch table pattern compare-rng-streams.py already uses, or stay as three sibling scripts mirroring compare-stage{0,1,2}. Up to you which fits better.

Claude's most recent delusion:
    Stage 0 OG-side + ReMoM sequence:
        ./launch-stage0.sh        # play OG, save slot 1, quit
        ./postprocess-stage0.py   # snapshot MoM131/SAVE1.GAM into the project
        ./extract-seed-stage0.py  # pull OG seed into Stage_0.ini
        ./run-remom-stage0.py     # run HeMoM, produces assets/SAVE9.GAM
        ./compare-stage0.py       # byte diff vs OG canonical
    Files
        * run-remom-stage0.py — runs HeMoM only. Reads seed from Stage_0.ini, clears stale SAVE9.GAM, invokes HeMoM --newgame Stage_0.ini --seed <S> with the 60s timeout, verifies output landed. Forwards any extra argv to HeMoM.
        * compare-stage0.py — pure compare. Reads OG canonical + ReMoM's SAVE9.GAM, byte-diffs, writes diff-bytes.txt, prints PASS/FAIL + top 20 + clusters.
        * launch-stage0.sh — "next steps" lines updated to show all four follow-on scripts.
    Verified end-to-end against the live ReMoM build: run produced assets/SAVE9.GAM, compare reported 24,226 bytes differ and wrote diff-bytes.txt. Same number as before the split.



##### Appendix A: Files

/home/jbalcomb/STU_DBWD/dosbox-capture/prng-port/baselines/BaseLine.INI
/home/jbalcomb/STU_DBWD/dosbox-capture/prng-port/baselines/BaseLine.RECIPE.md

What is /home/jbalcomb/STU_DBWD/dosbox-capture/prng-port/baselines/compare-rng-streams.py relative to the process progression for Stage 0, 1, and 2?
    It's the diagnostic layer below the byte diff — used after one of the compare-stage{0,1,2} scripts reports a non-zero byte diff to identify where in the RNG call stream the ReMoM run first diverged from OG.
    The stage chain produces a save → byte diff:
    launch-stage{0,1,2} + compare-stage{0,1,2} → "N bytes differ"
    compare-rng-streams.py <stage> consumes the logs those steps already produce and answers "WHY does it differ" by aligning OG's and ReMoM's Random() calls call-by-call:
    Per-stage inputs it reads	Produced by
    {stageN}-run.log (DOSBox trace, [WATCH] events)	launch-stage{N}.sh
    /tmp/{magic,wizards}_exe-calls.log (per-EXE call log)	DOSBox probe during launch
    /tmp/stage-{stage}.log (HeMoM stderr, [RNG-CALL] lines)	run-remom-stage0.py / compare-stage{1,2}.sh
    Stage routing baked in: stage 0 uses MAGIC.EXE (+ magic_exe-calls.log); stages 1/2 use WIZARDS.EXE (+ wizards_exe-calls.log).
    Output it prints: first seed-value divergence, side-by-side per-function call runs, per-call-site histograms, ReMoM-only call sites in the post-OG tail. Take args --context, --top, --runs-rows.
    So in the stage progression: it's not part of the chain. It's the tool you reach for after compare-stage{0,1,2} says "FAIL — N bytes differ" and you want to know which RNG call number the divergence starts at.
    Usage:
        ./compare-rng-streams.py stage0-newgame
        ./compare-rng-streams.py stage1-wizards-load
        ./compare-rng-streams.py stage2-next-turn

/home/jbalcomb/STU_DBWD/dosbox-capture/prng-port/baselines/lib_save_layout.py
/home/jbalcomb/STU_DBWD/dosbox-capture/prng-port/baselines/diff-structured-stage0.py


/home/jbalcomb/STU_DBWD/dosbox-capture/prng-port/baselines/stage0-newgame/Stage_0.ini
    says
        # Source of truth: ../BaseLine.INI plus the Books/Retorts choices
        # from ../BaseLine.RECIPE.md.  If you change OG's choices, update
        # both this file AND BaseLine.INI / BaseLine.RECIPE.md.

***Stage 0 - 'New Game'***

/home/jbalcomb/STU_DBWD/dosbox-capture/prng-port/baselines/stage0-newgame/Stage_0.ini
    * INI file for ReMoM's Stage 0

/home/jbalcomb/STU_DBWD/dosbox-capture/prng-port/baselines/extract-seed-stage0.py
    * get random seed from OG-MoM Stage 0 and populate it for ReMoM
    * Built extract-seed-stage0.py and dry-ran it against the existing stage0-run.log.

seed.txt
    * Claude doesn't know why it added this, maybe imagined it could be used downstream

/home/jbalcomb/STU_DBWD/dosbox-capture/prng-port/baselines/stage0-newgame/canonical-save1.GAM

/home/jbalcomb/STU/devel/ReMoM/assets/SAVE9.GAM 

HMS="$STAGE_DIR/scenario.hms"
INI="$STAGE_DIR/Stage_0.ini"

/home/jbalcomb/STU_DBWD/dosbox-capture/prng-port/baselines/compare-stage0.py

/home/jbalcomb/STU_DBWD/dosbox-capture/prng-port/baselines/stage0-newgame/scenario.hms
    * name home city
    * save to save slot 1 with save name

***Stage 1 - 'Load Game'***

/home/jbalcomb/STU_DBWD/dosbox-capture/prng-port/baselines/stage1-wizards-load/scenario.hms

***Stage 2 - 'Next Turn'***

/home/jbalcomb/STU_DBWD/dosbox-capture/prng-port/baselines/stage2-next-turn/RECIPE.md
