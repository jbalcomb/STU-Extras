# GD Capture Points

Registry of every `[GD]` capture point, **numbered in the order they fire during a
run**. Each point is emitted on BOTH sides in byte-identical format so
`compare-gd-*.py` pairs them:

- **OG**: `src/misc/og_sort_trace.h` — landmark/stub probes read DGROUP (or EMS) and
  `fprintf(stderr, "[GD] ...")`.
- **ReMoM**: `gd_dump_*()` (INITGAME.c / MAPGEN.c / AIMOVE.c) via `LOG_DEBUG` →
  `remom_log_new.txt`.

## Naming convention

`<NNN>_<Where>_<What>_<TypeLetter>`

- **Hundreds = the capture MOMENT**, ordered by when it fires in a run.
- **Units = order/array within that moment.**
- **Trailing letter = record type**: `_P` players(s_WIZARD) · `_C` cities · `_F`
  fortresses · `_U` units · `_N` nodes · `_H` heroes · `_I` items · `_T` towers /
  terrain-specials · `_L` landmasses · `_W` world_maps · `_M` map_flags · `_E`
  square_explored · `_LR` lairs · `_TS`/`_HE` = terrain-specials / heroes (gen).
  (Letters aren't globally unique — the full point name disambiguates.)
- **New:** for the AI-turn (600) points the `<Where>` is the full function name
  (`AI_Choose_War_Landmass`), un-abbreviated; the type letter alone marks the array
  (no redundant word — `_F`, not `_Fortresses_F`). `_Squares`/`_Continents` have no
  standard letter so they use a word.

## Moments (call order)

| Series | Moment | Trigger | EXE |
|---|---|---|---|
| 000/100/200 | New Game: screen, world-gen, init | MGC landmarks + FN-EXIT traps | MAGIC.EXE |
| 300 | Load (post `Load_SAVE_GAM`) | `post_load` @ IDA 0x141A7 (on-return) | WIZARDS.EXE |
| 400 | Slot-1 Save (Stage 0 canonical) | `save_slot1_entry` @ 0x33DE0, gated `save_gam_idx==0` | WIZARDS.EXE |
| 600 | Next Turn begins: `Next_Turn_Proc` DDU#1 (600), then `Next_Turn_Calc` → AI_Choose_War_Landmass (601+, first AI player) | `j_All_Units_In_Towers` @ 0x341FC (600) · `aicwl_entry` @ 0x36A04 + `j_CONTXXX_Map` return (601+) | WIZARDS.EXE |
| 610/611 | AI_Evaluate_Continents entry/return (in-turn, every AI turn) | resident stub `j_AI_Evaluate_Continents` @ 0x369FF + far-call return | WIZARDS.EXE |
| 900 | Slot-5 Save (Stage 2 post-Next-Turn) | `save_slot5_entry` @ 0x33DE0, gated `save_gam_idx==4` | WIZARDS.EXE |

Stage-2 run order: `300s (load)` → `600s (AI turn)` → `900s (save)`. The 500 series
was moved to 900 to leave 600–800 free for in-turn capture points.

## 600 — AI_Choose_War_Landmass (in call order)

All fire once, at the FIRST AI player's call (fired-once). `_FORTRESSES`, `_CITIES`,
`_ai_continents`, `_players` are DGROUP → readable **on-entry** (`aicwl_entry` stub).
`_ai_landmass_land_squares` is in the **CONTXXX EMS frame** → captured at the
`CONTXXX_Map()` **return** (only then are the pages mapped + the far-ptrs reassigned).

| # | Point | Captures | When |
|---|---|---|---|
| 600 | `600_Delete_Dead_Units_NextTurnProc_1_U` | `_UNITS` AFTER `Next_Turn_Proc`'s **1st** `Delete_Dead_Units` — this is the **FIRST** Next-Turn capture: it runs before `Next_Turn_Calc()` (which contains 601–633), hence numbered 600 < 601 | at-return. OG: `Next_Turn_Proc__WIP` (ovr060) calls `Delete_Dead_Units` (ovr060) **intra-overlay** (CALLN, bypasses the stub) → fire at the cross-overlay landmark `j_All_Units_In_Towers` `0x341FC`, **1st hit** (ATow#1, right after DDU#1). ReMoM: `NEXTTURN.c:312`. Fire once. |
| 601 | `601_AI_Choose_War_Landmass_F` | `_FORTRESSES` | on-entry |
| 602 | `602_AI_Choose_War_Landmass_C` | `_CITIES` | on-entry |
| 603 | `603_AI_Choose_War_Landmass_Continents` | `_ai_continents` (plane×player: wx/wy/type_array) | on-entry |
| 604 | `604_AI_Choose_War_Landmass_P` | `_players` (s_WIZARD) + `player_idx` scalar | on-entry |
| 605 | `605_AI_Choose_War_Landmass_Squares` | `_ai_landmass_land_squares` heads/lists/wx/wy | CONTXXX_Map return |
| 606 | `606_AI_Choose_War_Landmass_Dock_Squares` | `_ai_landmass_dock_squares` heads/lists/wx/wy (dock/embark squares; feed `min_delta_distance`) | CONTXXX_Map return (same as 605) |
| 607 | `607_AI_Choose_War_Landmass_Weights` | `final_landmass_weights[60]` (the `Get_Weighted_Choice` input; its sum is the diverging arg) + `Continent_Values[60]` + `min_delta_distance[60]` (the `C` and `d` in `weight=C*1000/d`) + `wp`/`player_idx`/`first_hostile_player_idx` scalars | at `Get_Weighted_Choice` call |
| 608 | `608_AI_Evaluation_Map` | `g_ai_evaluation_map[2][WORLD_SIZE]` (uint16 occupation map; the occupiable gate) | `AI_Evaluation_Map` return (phase 301, between AICWL and AI_Evaluate_Continents) |
| 608 | `608_AI_Evaluation_Map_{P,U,L,C,N}` | the on-entry INPUTS `AI_Evaluation_Map` sums into the map: `_players`(P), `_UNITS`(U), `_LAIRS`(L), `_CITIES`(C), `_NODES`(N) | `AI_Evaluation_Map` on-entry (`j_AI_Evaluation_Map` stub `0x369FA`) |
| 608 | `608_AI_Evaluation_Map_Strength` | per-unit RAW `Effective_Unit_Strength(itr)` (before the `/10`), one `unit_strength[<itr>].s` record per processed (non-skipped) unit — pins WHICH unit(s) drive a 608 map value divergence | inside `AI_Evaluation_Map`'s unit loop. OG: capture `AX` at the `j_Effective_Unit_Strength` stub-return (`0x35797`) gated to a return-offset in `AI_Evaluation_Map`'s ovr162 range `[0x3AAB,0x3F14)`, indexed by `SI`(=itr) snapshotted at the call. ReMoM: `LOG_DEBUG` in the loop. Fire once (first AI player). |

606 (dock squares) mirrors 605's land-square dump for the dock arrays (DGROUP far-ptrs:
heads `9D2A`, lists `9D32`, wy `9D3A`, wx `9D42`), captured at the same CONTXXX_Map return.

607 OG trigger: `Get_Weighted_Choice` resident entry (WZD seg001 IDA `0x14596`), gated by
the return offset in AICWL's overlay range `[0x3F14,0x46A2)`; `BP` is still AICWL's frame
there, so the array (`Continent_Values_2` @ `[BP-0x10A]`) + scalars are read BP-relative.
`final_landmass_weights`, `Continent_Values`, `min_delta_distance` are flat `int16[60]`
(no struct nesting) → each logged as one comma-valued scalar; `compare-gd` diffs comma
scalars per-index like array recs. `Continent_Values` (OG `[BP-0x92]`) is retained, read
at the same trigger. `min_delta_distance` is a scalar reused per landmass, so it's
snapshotted into a shadow array: ReMoM in the dock loop; OG at AICWL `loc_F8CA9` (overlay
offset `0x43B9`), keyed by `itr_landmasses` (`[BP-0x0A]`), gated by AICWL's overlay CS.

### 610/611 — AI_Evaluate_Continents (in-turn, fires after AICWL)

`AI_Evaluate_Continents(player_idx)` is the **every-turn, non-throttled** evaluator
that rebuilds the continent status (`_ai_continents.type_array`) + stage points
(`wx`/`wy_array`). It runs each AI turn as a `NEXTTURN` phase (AIDUDES.c:302, i.e.
after AICWL at :255, before `AI_Set_Unit_Orders` at :303) — unlike the
`Random(20)==1`-gated `AI_Reevaluate_Continent`, which rarely fires. Both snapshots
are `_ai_continents` (DGROUP); 610 is **pre**-evaluation, 611 is **post**. Fire-once,
on the first AI player's call; `player_idx` is emitted so `compare-gd` can confirm
both sides captured the same invocation.

OG trigger: called cross-overlay from `AI_Next_Turn` (ovr145) → it has a resident far
stub `j_AI_Evaluate_Continents @ 0x369FF` that fires in the per-instruction probe. At
the stub, DS = WZD DGROUP; `player_idx` is the far-call arg at `[SS:SP+4]`; 611 is armed
at the far-call return (`[SS:SP]`/`[SS:SP+2]`) into `AI_Next_Turn`.

| # | Point | Captures | When |
|---|---|---|---|
| 610 | `610_AI_Evaluate_Continents_Entry_Continents` | `_ai_continents` + `player_idx` scalar | on-entry (pre) |
| 611 | `611_AI_Evaluate_Continents_Return_Continents` | `_ai_continents` (post-rebuild) | at-return (post) |
| 612 | `612_AI_Set_Unit_Orders_Entry_U` | `_UNITS` BEFORE `AI_Set_Unit_Orders` (per-AI-player order/movement setter: `dst_wx`/`dst_wy`/`Status`/`moves` via `AI_Move_Out_Boats` + stack-expedition) + `player_idx` scalar | on-entry. OG: resident far stub `j_AI_Set_Unit_Orders` `0x36710` (ovr158, cross-overlay from `AI_Next_Turn`); ReMoM: `AIMOVE.c` entry. Fire once (first AI player). |
| 613 | `613_AI_Set_Unit_Orders_Return_U` | `_UNITS` AFTER `AI_Set_Unit_Orders` (post-`EMMDATAH_Map()`, matching where OG returns to `AI_Next_Turn`) — pairs with 612 to isolate this call's effect on `_UNITS` (the 911 movement divergence) | at-return |
| 614 | `614_AI_Execute_Orders_Entry_U` | `_UNITS` BEFORE `AI_Execute_Orders` (per-AI-player order EXECUTION: dispatches each unit by `Status` → move/road/meld/settle/ferry; where `dst`/`Status` get applied) + `player_idx` scalar | on-entry. OG: resident far stub `j_AI_Execute_Orders` `0x35010` (ovr100 `AI_MoveUnits`, `__WIP` name reconciled in `.fwv`; cross-overlay from `AI_Next_Turn`); ReMoM: `SETTLE.c` entry. Fire once (first AI player). |
| 615 | `615_AI_Execute_Orders_Return_U` | `_UNITS` AFTER `AI_Execute_Orders` — pairs with 614 to isolate this call's effect on `_UNITS` (the 911 movement divergence). ReMoM dumps on BOTH exit paths (Time-Stop early return + normal exit) via a shared fire-once gate, matching OG's far-return which fires regardless of internal path | at-return |
| 616 | `616_Make_Raiders_Return_U` | `_UNITS` AFTER `Make_Raiders` (NPC raider unit generation) | at-return. OG: resident far stub `j_Make_Raiders` `0x36A7A` (ovr164, cross-overlay from `AI_Next_Turn`); ReMoM: dumped at the call site in `AIDUDES.c` (mirrors OG's far-return; handles the function's many early-return paths). Fire once. |
| 617 | `617_Make_Monsters_Return_U` | `_UNITS` AFTER `Make_Monsters` (NPC monster unit generation) — pairs with 616 to see which NPC-generation pass creates the divergent late unit records (e.g. `_UNITS[48..50]`) | at-return. OG: resident far stub `j_Make_Monsters` `0x36A75`; ReMoM: call site in `AIDUDES.c`. Fire once. |
| 618 | `618_AI_Next_Turn_Return_U` | `_UNITS` AFTER `AI_Next_Turn` (the whole AI turn) — brackets the AI turn's net effect on units; localizes whether the divergent neutral units (`_UNITS[48..50]`, owner 5 / type 104 in OG, absent in ReMoM) are created inside the AI turn (after 617) or in a later `Next_Turn_Calc` phase | at-return. **OG: can't use a far-return match** — `AI_Next_Turn` (ovr145) evicts its caller `Next_Turn_Calc` (ovr119), which reloads at a different segment, so the captured return seg goes stale (ovr119 seen at 9 segs). Instead arm at `j_AI_Next_Turn` `0x36175`, fire at the first Next_Turn_Calc call on return — `j_Next_Turn_Process_Purify` `0x342E6` (called once, only after AI_Next_Turn). ReMoM: call site in `NEXTTURN.c`. Fire once. |
| 620 | `620_Determine_Event` | `Determine_Event`'s random-event gate inputs: `event_pressure_accumulator` (the value `Random(512)` is compared against at `EVENTS.c:301`) + global `_event_delay_count` | at `Determine_Event`'s single `Random(512)` call (`Next_Turn_Calc+0x72`). OG: arm at `j_Determine_Event` stub `0x34950`, capture at `Random` entry (seg022 `0x1D5A8`) — reads `event_pressure_accumulator` from `[BP-2]` (BP still Determine_Event's frame) and `_event_delay_count` from WZD DGROUP `0x4A32`. ReMoM: `EVENTS.c` just before the `Random(512)` gate. Fire once. |
| 621 | `621_All_Outpost_Population_Growth_Return_U` | `_UNITS` AFTER `All_Outpost_Population_Growth` (`Next_Turn_Calc+0x86`, right after `AI_Next_Turn` returns; the outpost-growth pass that was the earlier RNG-desync source) | at-return. Single call. OG *does* call it (stub `j_All_Outpost_Population_Growth` `0x36004`) but draws 0 Random (no outposts), which is why it never showed in the draw stream. It evicts ovr119, so fire at the next NTC call `j_Apply_City_Changes` `0x35F0B` (shared w/ 622/623, independent) — not the stale far-return. ReMoM: `NEXTTURN.c:736` call site (before `Apply_City_Changes()`). Fire once. |
| 622 | `622_Apply_City_Changes_Entry_C` | `_CITIES` BEFORE `Apply_City_Changes` (`Next_Turn_Calc+0x8B`; per-turn city-update: outpost graduation/failure + population growth; runs right after `All_Outpost_Population_Growth`) | on-entry. OG: resident far stub `j_Apply_City_Changes` `0x35F0B` (ovr140, called cross-overlay from `Next_Turn_Calc`); ReMoM: `NEXTTURN.c` entry. Fire once. |
| 623 | `623_City_Apply_Production_Return_U` | `_UNITS` AFTER `City_Apply_Production()` **for city 9** (ReMoM `NEXTTURN.c:3141`, gated `itr_cities == 9`) — one of the 3 cities (9/13/17) whose production `Create_Unit()`s the divergent units | fire once. OG: `City_Apply_Production` is **intra-overlay** (ovr140→ovr140 CALLN, no stub) → learn ovr140's runtime CS from `j_City_Current_Product_Cost` `0x35FEB`, then **count** returns at IP `0x1987` (called once per loop iteration, no skip → 10th return == `itr_cities 9`), gated by `appcity_armed`. Grouped under 623 (mid-`Apply_City_Changes`). |
| 624 | `624_Apply_Consecration_Return_U` | `_UNITS` AFTER `Apply_Consecration()` (ReMoM `NEXTTURN.c:3152`) — the CONSECRATION enchantment handler in the per-city loop | fire once. OG: cross-overlay stub `j_Apply_Consecration` `0x35B15`, far-return into Apply_City_Changes, gated by `appcity_armed`. **Only fires for consecrated cities — none in the current turn, so untested** (both sides skip this run). |
| 625 | `625_Volcano_Counts_Return_U` | `_UNITS` AFTER `Volcano_Counts()` (ReMoM `NEXTTURN.c:3176`), which can `Destroy_City` via eruptions | fire once. **≈ same `_UNITS` state as `626_Apply_City_Changes_Return_U`** (Volcano_Counts is Apply_City_Changes's last op — nothing runs between). OG captured at the same Apply_City_Changes-return trigger (robust; avoids the eviction risk of a Volcano_Counts far-return with no landmark after it). |
| 626 | `626_Apply_City_Changes_Return_C` + `626_Apply_City_Changes_Return_U` | `_CITIES` **and** `_UNITS` AFTER `Apply_City_Changes` (post-`Volcano_Counts()`, matching where OG returns to `Next_Turn_Calc`) — `_C` pairs with 622 to isolate this pass's effect on the city `Pop_`/`size`/`construction` fields (910); `_U` because `Apply_City_Changes` can `Destroy_City` (may delete/relocate units) | at-return (`_C`+`_U` share this number — same moment) |
| 627 | `627_Determine_Offer_Return_U` | `_UNITS` AFTER `Determine_Offer` (`Next_Turn_Calc+0x11A`, hero/item offer generation) — post-`AI_Next_Turn` bisection point | at-return. Single call. OG: plain far-return via stub `j_Determine_Offer` `0x35F1A`, gated to the Next_Turn_Calc call by return offset `0x11F` (no eviction). ReMoM: `NEXTTURN.c:764` call site (before `Set_Mouse_List`). Fire once. |
| 628 | `628_Diplomacy_Growth_Entry_P` | `_players` BEFORE `Diplomacy_Growth` (`Next_Turn_Calc+0x133`; treaty/peace/exchange modifier pass) | on-entry. OG: resident far stub `j_Diplomacy_Growth` `0x34B05` (ovr085); ReMoM: `DIPLOMAC.c` entry. Fire once. |
| 629 | `629_Diplomacy_Growth_Return_P` | `_players` AFTER `Diplomacy_Growth` — pairs with 628 to localize whether this pass introduces the 902 modifier divergence (drawing from the desynced stream) or it is already present on entry | at-return |
| 630 | `630_Delete_Dead_Units_NextTurnCalc_U` | `_UNITS` AFTER `Next_Turn_Calc`'s `Delete_Dead_Units` (`+0x1E8`, mid-turn) | at-return. OG: this one is cross-overlay → stub `j_Delete_Dead_Units` `0x34250`, gated to it (vs the load call) by far-return offset `0x1ED`; no eviction, plain far-return. ReMoM: `NEXTTURN.c:812`. Fire once. |
| 631 | `631_Do_All_Units_XP_Check_Return_U` | `_UNITS` AFTER `Do_All_Units_XP_Check` (`Next_Turn_Calc+0x20B`, runs right before `Heal_All_Units`) | at-return. Single call, no eviction (ovr119 stays). OG: plain far-return via stub `j_Do_All_Units_XP_Check` `0x35F38`. ReMoM: `NEXTTURN.c:841` call site (before `Heal_All_Units()`). Fire once. |
| 632 | `632_Heal_All_Units_Return_U` | `_UNITS` AFTER `Heal_All_Units` (end-of-turn heal pass; `Next_Turn_Calc+0x210`) | at-return. Single call. OG: plain far-return via stub `j_Heal_All_Units` `0x35F2E` — Heal_All_Units does **not** evict ovr119 (stays at its seg, verified), so no landmark needed. ReMoM: `NEXTTURN.c` call site (before `Record_History()`). Fire once. |
| 633 | `633_All_City_Calculations_Return_U` | `_UNITS` AFTER the **LATE** (end-of-turn) `All_City_Calculations` (`Next_Turn_Calc+0x278`) — the last major recalc before the turn ends | at-return. Called 3× (load + early `Next_Turn_Calc+0x23` + late `+0x278`); **late one only**. OG: stub `j_All_City_Calculations` `0x35EC5` gated to the late call by far-return offset `0x27D`; ovr119 is evicted, so fire at the next NTC call `Get_Random_Seed` (seg022 `0x1D559`, not called during ACC) — not the stale far-return. ReMoM: `NEXTTURN.c:866` call site (before `Get_Random_Seed()`). Fire once. |
| 634 | `634_Delete_Dead_Units_NextTurnProc_2_U` | `_UNITS` AFTER `Next_Turn_Proc`'s **2nd** `Delete_Dead_Units` (runs after `Next_Turn_Calc` returns — the **last** turn snapshot; XP is post-`Do_All_Units_XP_Check`) | at-return. OG: same intra-overlay landmark `j_All_Units_In_Towers` `0x341FC`. **NOT the raw 2nd hit** — All_Units_In_Towers is also called from earlier sites, so counting to 2 lands pre-`Next_Turn_Calc` (the old bug: captured `_units=48`/XP=0). Instead gate on `acc_done` (633 fired) and take the **first hit after 633** = the true post-NTC ATow#2. ReMoM: `NEXTTURN.c:333`. Fire once. |

## Full registry

### 000/100/200 — MAGIC.EXE New Game
`000_Newgame_Screen_7_P`
`100_Wizard_Profile_P` `101_Spell_Library_P` `102_Init_Landmasses_L`
`103_Generate_Landmasses_L` `104_Landmasses_W` `105_BaseTerrain_W`
`106_ClimateTerrain_W` `107_Generate_Nodes_N` `108_Rebalance_Node_Types_N`
`109_PreTowers_W` `110_Generate_Towers_T` `111_Extend_Islands_L`
`112_Extend_Islands_W` `113_Generate_Lairs_LR` `114_Generate_Home_Cities_C`
`115_Generate_Home_Cities_U` `116_Generate_Neutral_Cities_C`
`117_Generate_Neutral_Cities_U` `118_Generate_Terrain_Specials_TS`
`119_Generate_Terrain_Specials_W` `120_Generate_Roads_F`
`121_Simtex_Autotiling_W` `122_Generate_River_W` `123_River_Autotile_W`
`124_Desert_Autotile_L` `125_Desert_Autotile_W` `126_Shuffle_Terrains_W`
`127_Movement_Mode_Cost_Maps_M` `128_CRP_NEWG_CreatePathGrids_U`
`129_Init_Square_Explored_E` `130_Animate_Oceans_W`
`131_Set_Upper_Lair_Guardian_Count_LR`
`200_Initialize_Items_I` `201_Init_Heroes_P` `202_Init_Heroes_HE`
`203_Init_Players_P` `204_Init_CP_Strategy_P` `205_Init_Magic_Pers_Obj_P`
`206_Init_Summoning_Circle_P` `207_capital_race_P`
`208_Init_Diplomatic_Relations_P` `209_gold_reserve_P`

### 300 — WIZARDS.EXE Load
`301_Load_Heroes_H` `302_Load_Players_P` `303_Load_World_Maps_W`
`304_Load_Landmasses_L` `305_Load_Nodes_N` `306_Load_Fortresses_F`
`307_Load_Towers_T` `308_Load_Lairs_L` `309_Load_Items_I`
`310_Load_Cities_C` `311_Load_Units_U` `312_Load_Terrain_Specials_T`
`313_Load_Map_Flags_M`
`314_Load_Continents` — `_ai_continents`, **not** save data (no loader; derived AI
scratch state). Appended after the 13 canonical load arrays as an at-load in-memory
baseline to compare against `603` (AICWL entry) and `610` (AI_Reevaluate_Continent
entry). Same struct/format as 603/610.
`315_Build_Land_Linked_List_Squares` — the four `_ai_landmass_land_squares` arrays
(heads/lists/wx/wy) at `Build_Land_Linked_List` return. `316_Build_Dock_Linked_List_Squares`
— the four `_ai_landmass_dock_squares` arrays at `Build_Dock_Linked_List` return. Both
builders run in `Loaded_Game_Update` (after the 301–314 dumps), building the derived
CONTXXX square arrays; captured **before** the builder's `EMMDATAH_Map()` remaps CONTXXX
away (arrays built, CONTXXX still mapped). Diff 315 vs 605 (land at AICWL) / 316 vs 606
(dock at AICWL) to confirm the builder is the source of a divergence. OG trigger: arm at
the builder's resident far stub (`j_Build_Land` `0x361C0` / `j_Build_Dock` `0x361C5`),
fire at `EMMDATAH_Map` entry (seg013 `0x18361`).
`317_Loaded_Game_Update_Countdown` — `_ai_reevaluate_continents_countdown[NUM_PLAYERS]`
(the AI_Evaluate_Continents throttle) at `Loaded_Game_Update` return (after 315/316,
which fire inside it). OG: arm at resident seg001 `Loaded_Game_Update` `0x144A7`, fire at
the far-return; read the DGROUP far-ptr `0x8FAA` → `uint8_t[6]`. NOTE type mismatch: OG
stores it **uint8_t** (1-byte, unsigned); ReMoM declares **int16_t\*** — matches at load
(all 0) but a suspect for the post-turn throttle divergence.

### 400 — WIZARDS.EXE Slot-1 Save (Stage 0)
`401_Save_Heroes_H` … `413_Save_Map_Flags_M` (same 13 arrays as 300, at save-entry).

### 600 — WIZARDS.EXE AI_Choose_War_Landmass (+ AI_Evaluate_Continents)
See the tables above: AICWL 601–605, and AI_Evaluate_Continents 610/611.

### 900 — WIZARDS.EXE Slot-5 Save (Stage 2)
`901_Save_Heroes_H` … `913_Save_Map_Flags_M` (same 13 arrays as 300/400, post-Next-Turn).

## Notes
- To add an in-turn point, use the 600–800 range and keep the units digit in
  call order (renumber neighbors if inserting).

## `[GD]` line format contract (compare-gd parses these — get it wrong and the point is DROPPED)

`compare-gd-stage0.py` parses every `[GD]` line with exactly two regexes, tried in order.
A line matching **neither** is **silently skipped** — the point then never shows up, not
even as a "points only in OG/ReMoM" note (it's not in the union at all). This has bitten
us repeatedly; the two accepted forms are:

- **Array record** — `[GD] <point> <target>[<idx>].<field> = <comma,values>`
  - The `.` is optional, so a flat per-index array with no sub-field also works:
    `[GD] <point> <target>[<idx>]@<base> = ...` (e.g. `g_ai_evaluation_map[0]@0 = ...`,
    field parses as `@0`). Chunk long arrays `@base` in ≤100-value lines (ReMoM's
    `LOG_DEBUG` truncates at 1024 chars; both sides must chunk identically so lines pair).
  - compare-gd shows a per-index diff when the value contains commas.
- **Scalar** — `[GD] <point> <name> = <value>` (no `[idx]`). A comma value here also gets
  the per-index diff. Scalar-only points ARE included in the point set.

Both sides (OG `fprintf` + ReMoM `gd_dump_*` `LOG_DEBUG`) **must emit byte-identical
format** — same `<point>`, `<target>`, `[idx]`, `<field>`/`@base` — or records won't pair.

### Pitfalls that have actually dropped points
- `target[idx]@base` **without** the leading `.field` — GD_RE used to require the `.`;
  now optional. (This dropped `608_AI_Evaluation_Map` entirely.)
- A **scalar-only** point (no array records) — the point-set was once built from array
  records only, dropping it. (Fixed: sets now include scalar points.)
- Stray `[GD]` inside a **debug/trace** log line — compare-gd ingests it as a phantom
  point (e.g. `[GD] AI_Choose_War_Landmass() ...`). Don't prefix debug prints with `[GD]`.

### ALWAYS test a new point end-to-end (do NOT assume it shows)
After adding/renumbering a point, before claiming it works:
1. `grep '<NNN>_<Name>' <og-log> <remom-log>` — confirm BOTH sides emit it.
2. Run `compare-gd-stage2.py` and confirm the point is listed in the summary table.
3. If it's absent, test the line against the regexes directly:
   `python3 -c "import re; print(re.search(r'\[GD\]\s+(\S+)\s+(\w+)\[(\d+)\]\.?(.+)$', LINE))"`
