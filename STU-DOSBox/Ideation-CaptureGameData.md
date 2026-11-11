
Goal:

track game-data
periodic snapshot
run-time evaluation

structured, systematic, minimally intrusive


Process
add code to dosbox to snapshot the game data every frame?
provide a list of addresses or offsets from a dynamically allocated pointer
capture the data as the game plays

Use-Cases
Are the values in _players correct when the code reaches Init_Computer_Players_Spell_Library()?

Walking the call streams in order side-by-side:
    Iteration #1 — both sides identical (8 calls each):
    OG: 5×common, 2×uncommon, 1×rare → matches ReMoM (p=0, sbr=0): 5/2/1/0 ✓
Iteration #2 — diverges immediately:
    OG: 6×common, 4×uncommon, 4×rare, 2×very_rare = 16 calls
    ReMoM (p=0, sbr=1): 8×common, 4×uncommon, 5×rare, 2×very_rare = 19 calls
    +2 in common, +1 in rare

HP initial spell selection happens in Newgame_Screen_5/Newgame_Screen_5_Draw/Newgame_Screen_5_Draw_Spells.

...depends on data from Newgame_Screen_5 - _players[0].spellranks[sbr]


Newgame_Control();
    ...Human Player is set up in Newgame_Screen_
    |-> Init_New_Game();
        ...
        |-> Init_Computer_Players();
            |-> Init_Computer_Players_Wizard_Profile();
            HERE: examine _players[].spellranks[]
            |-> Init_Computer_Players_Spell_Library();

    |-> Initialize_Events();
    |-> Init_Runtime();
        |-> Draw_Building_The_Worlds(100);
        |-> Initialize_Items();
        |-> Init_Heroes();
        |-> Init_Players();
        |-> Init_CP_Strategy();
        |-> Init_Magic_Personalities_Objectives();
        |-> Init_Summoning_Circle_And_Spell_Of_Mastery();
        |-> Initialize_Messages();
        for(itr_players = 0; itr_players < _num_players; itr_players++)
        {
            _players[itr_players].capital_race = _CITIES[itr_players].race;
        }
        |-> Init_Diplomatic_Relations();


Claude said:
    LEN_WIZARD_NAME isn't a #define I can see — but I don't actually need the constants: the /* 0xNN */ comments are authoritative, so I can size every field (and derive array lengths) from the offset deltas, using the constants only as an optional cross-check. That's also more in line with "the struct comments are the source of truth." Let me switch the generator to delta-based sizing.
But, Claude is a fucking idiot, so this statement is fundamentally incorrect.
All the data structures are 2-byte aligned, so any field can have padding bytes, which is different than field length/size.
If you need a constant used in a definition, ask for it, and make sure it is noted in the structures doxygen comment.


Questions:

Does it have to be per CPU?
i.e., in /home/jbalcomb/STU/developp/dosbox-0.74-3/src/cpu/core_normal.cpp

How to recognize function calls in overlayed code segments?
...table of in-use/no-in-use stubs?
...should just track current overlays based on any/every/each call to a stub?

How to handle data segment addresses?
How to handle code segment addresses?
How to handle dynamically allocated memory?
How to handle data structures?

Short answer
_players[].spellranks[] can be captured exactly the way the rest of the matchup pipeline captures OG state: a probe in STU-DOSBox that fires at a chosen code point and dumps guest memory to the log. The wrinkle that shapes how you do it: Init_Computer_Players_Wizard_Profile lives in MAGIC.EXE (the setup launcher), as overlay code — not in WIZARDS.EXE, and not at a fixed code address. So the capture point and the memory address both depend on which executable is live.
3. The infrastructure already exists. The per-instruction hook is og_sort_trace_check(), called from core_normal.cpp:143; probes live in og_sort_trace.h; guest reads use real_readw(seg, off) from mem.h; and the overlay resolver in og_overlay_probe.PRD.md already detects which executable is live (og_detect_active_exe) and converts a runtime CS:IP to (overlay, function, offset). There's also a config-driven periodic dumper, capture_state.cpp, driven by the [capture] section.
