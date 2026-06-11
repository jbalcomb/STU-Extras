
From the top:
DOSBOX=/home/jbalcomb/STU/developp/dosbox-0.74-3/src/dosbox
MOM_DIR=/home/jbalcomb/STU_DBWD/MoM131
BASE_DIR=/home/jbalcomb/STU_DBWD/dosbox-capture/prng-port/baselines/
Stage 0 - 'New Game'
    OG-MoM
    /home/jbalcomb/STU_DBWD/dosbox-capture/prng-port/baselines/launch-stage0.sh
    /home/jbalcomb/STU_DBWD/dosbox-capture/prng-port/baselines/stage0.conf
    /home/jbalcomb/STU_DBWD/dosbox-capture/prng-port/baselines/stage0-run.log
    /tmp/magic_exe-calls.log
    ? postprocess-stage0.sh ?
    ...updates random seed for ReMoM process
    ReMoM
        python and HMS
        save? /home/jbalcomb/STU/devel/ReMoM/assets/SAVE9.GAM
        log?

--seed 0x000305FD: pulled from the [Game] section of Stage_0.ini, which was set by postprocess-stage0.sh from the first MAGIC.EXE [INSTR] label=seed_set event (the post-Init_Drivers Randomize() result).

/home/jbalcomb/STU_DBWD/dosbox-capture/prng-port/baselines/stage0-newgame/Stage_0.ini

/tmp/stage-stage0-newgame.log  ...logs [RNG-CALL]


cd /home/jbalcomb/STU/devel/ReMoM/assets
timeout 60 /home/jbalcomb/STU/devel/ReMoM/build/bin/Debug/HeMoM \
    --newgame /home/jbalcomb/STU_DBWD/dosbox-capture/prng-port/baselines/stage0-newgame/Stage_0.ini \
    --seed 0x000305FD \
    2> /tmp/stage-stage0-newgame.log


DIFF OG-MoM & ReMoM
    python?
    log?

/home/jbalcomb/STU_DBWD/dosbox-capture/prng-port/baselines/compare-stage.sh stage0-newgame 2>&1 | tail -80
ls -la /home/jbalcomb/STU/devel/ReMoM/assets/SAVE9.GAM /home/jbalcomb/STU_DBWD/dosbox-capture/prng-port/baselines/stage0-newgame/canonical-save1.GAM 2>&1

cmp -l /home/jbalcomb/STU/devel/ReMoM/assets/SAVE9.GAM /home/jbalcomb/STU_DBWD/dosbox-capture/prng-port/baselines/stage0-newgame/canonical-save1.GAM 2>/dev/null | awk '{print $1-1}' | awk '
  BEGIN { prev=-1; start=-1; cnt=0; total=0; clusters=0 }
  {
    if (start<0 || $1-prev > 0x40) {
      if (start>=0) { printf "0x%06X..0x%06X  %d bytes\n", start, prev, cnt; clusters++ }
      start=$1; cnt=0
    }
    prev=$1; cnt++; total++
  }
  END { if (start>=0) { printf "0x%06X..0x%06X  %d bytes\n", start, prev, cnt; clusters++ }
        printf "\nTOTAL: %d differing bytes in %d clusters\n", total, clusters
  }'



Standard facilities?
check data-segment
check EXE signature
capture some value at a dseg:offset

The second [FN_RETURN] is a false positive from my too-loose entry signature (55 8B EC 83 EC 04 06 B8 00 00 matches any function that allocates 4 stack bytes and zeros ES — not unique to Randomize).
...
But, what is matching functions?
But, what is (still) matching functions by bytes?

You're right on all three. Concretely:
    1. UU_Legacy_Startup is unused — so my "two Randomize calls" framing was wrong. The second [FN_RETURN] is a false positive from my too-loose entry signature (55 8B EC 83 EC 04 06 B8 00 00 matches any function that allocates 4 stack bytes and zeros ES — not unique to Randomize).
    2. Random() advances the seed — so the second value (0x00126EBF) was just an LFSR-advanced state of the first, not a fresh Randomize call. The [WATCH] line confirms this — every Random() call emits a seed change.
    3. Should capture Set_Random_Seed too — yes, that's in the disassembly right above Randomize, at seg022 offset 0x6A. Both are the canonical "seed gets set" events; everything else is Random() LFSR advance.
Tightening Randomize's signature + adding Set_Random_Seed:

Changes:

og_sort_trace.h — two table rows now:

Randomize — entry signature extended to 16 bytes through mov ax, [es:046Ch] so it's unique to Randomize, not a generic ES-zeroing prologue.
Set_Random_Seed — entry signature 9 bytes, anchored on A3 1E 43 (mov to DS:0x431E), which is specific to "function that writes random_seed."
postprocess-stage0.sh — greps both func=Randomize and func=Set_Random_Seed, reports counts for each, picks the last event as the seed to write to Stage_0.ini. That's the most recent seed-set before world-gen draws start advancing it.

Re-run ./launch-stage0.sh (chains into postprocess) and you'll see:

How many Randomize() events actually fired (should now be exactly 1 if the tightened signature is correct)
How many Set_Random_Seed() events fired (might be 0, might be N — interesting either way)
The picked seed = last fire
If Randomize() count is still > 1 with the tighter signature, that's a real second call we need to track down; if it drops to 1 and Set_Random_Seed shows 0, the false-positive theory is confirmed.

Why would it not be just matching the exact signature of bytes that sets random_seed?

Set_Random_Seed()
MGC
1CBB:007A   A3 1E 43    move [word ptr random_seed], ax
1CBB:0080   A3 20 43    move [word ptr random_seed+2], ax
WZD
1D4D:007E   A3 46 78    move [word ptr random_seed], ax
1D4D:0084   A3 48 78    move [word ptr random_seed+2], ax
Randomize()
MGC
1CBB:00C7   A3 1E 43    move [word ptr random_seed], ax
1CBB:00CD   A3 20 43    move [word ptr random_seed+2], ax
WZD
1D4D:00CB   A3 46 78    move [word ptr random_seed], ax
1D4D:00D1   A3 48 78    move [word ptr random_seed+2], ax


    uint32_t seed_before;  /* CLAUDE: captured for [RNG-CALL] trace */

    /* CLAUDE: count every call so divergence in call-count between runs is observable. */
    g_random_call_count++;

    /* CLAUDE 2026-05-27: per-call trace, gated on _cmd_line_seed != 0
     * so normal runs aren't spammy.  Format chosen to be greppable and
     * to line up column-for-column with OG MoM's captured magic.csv
     * `random_seed` transitions (the `after=` value is what the CSV
     * records).  ~55k lines per new-game run; pipe to a file. */
    if (_cmd_line_seed != 0) {
        fprintf(stderr,
            "[RNG-CALL] call=%llu  n=%d  before=0x%08X  after=0x%08X  result=%d\n",
            (unsigned long long)g_random_call_count,
            (int)n,
            (unsigned)seed_before,
            (unsigned)random_seed,
            (int)ret);
    }



# BaseLine RECIPE

As part of STU-DOSBox,
we are working through matching up the behavior and data of ReMoM to the original 'Master of Magic' (OG-MoM).
To do so, we need to start from the same position.
Here, we start with creating a new game using OG-MoM's MAGIC.EXE.
Then, we work to match the ReMoM project to that baseline.
The selections made for the 'New Game' are captured in the INI file that is used to automatically create the new game in ReMoM.
The random seed OG-MoM uses is captured and passed to ReMoM to ensure deterministic recreation of the bahavior and data.
NOTE: This is the same new game used as the baseline for all of ReMoM and STU-Extras, for tests, demos, etc.
NOTE: This baseline new/save game is carried forward through each stage of the matchup project.

Stage starts with a new game
Stage 0 ends with SAVE1.GAM
Stage 1 starts with a copy of SAVE1.GAM as its canonical save-game as SAVE2.GAM
Stage 1 ends with SAVE3.GAM
Stage 2 starts with a copy of SAVE3.GAM as its canonical save-game as SAVE4.GAM
Stage 2 ends with SAVE5.GAM

NOTE: Post-Process for Stage 0 needs to capture the random seed somewhere and update the ReMoM side
WTF /home/jbalcomb/STU_DBWD/dosbox-capture/prng-port/test_rjak.ini
random_seed needs to be in the INI?
Maybe, probably, let Stage 0 do its own, undounded thing and save to Stage_0-random_seed.txt

launch-stage0.sh
stage0.conf
post-process-stage0.sh

Chained: each launch-stage<N>.sh ends with exec "$BASE/postprocess-stage<N>.sh" — one script per stage, post-process happens automatically.



stage0.conf (stage0.conf:21-39) — capture now targets MAGIC's seed:

positive sentinel verify = ds : 0004 : Borland
negative sentinel denyif = ds : 4B85 : Scouts have spotted (excludes WIZARDS)
random_seed : ds : 431E : dword (MAGIC's seed location)
launch-stage0.sh (launch-stage0.sh:91-135) — post-process now:

Reads /tmp/og_stage0_rng.csv row 2 (pre-Randomize uninitialized) and row 3 (post-Randomize seed).
Formats row 3 as 0x-prefixed hex.
Writes it to stage0-newgame/Stage_0-random_seed.txt.
Prints both values so the user can see what was captured and what was skipped.
Dry-run against the existing /tmp/og_rng.csv (the stale WIZARDS-side capture) confirms the logic:


pre-Randomize:  0x35683568  (Borland uninit pattern — skipped)
post-Randomize: 0xCF07F     (would be MAGIC seed after the new stage0.conf is used)
Bash syntax check passes. Ready for the next Stage 0 run to produce a real MAGIC-side Stage_0-random_seed.txt.





ls -l /home/jbalcomb/STU_DBWD/dosbox-capture/prng-port/baselines/stage0-newgame/canonical-save1.GAM
balcomb@iMustAi:~/STU_DBWD/dosbox-capture/prng-port$ ls -l /home/jbalcomb/STU_DBWD/dosbox-capture/prng-port/baselines/stage0-newgame/canonical-save1.GAM
-rw-rw-r-- 1 jbalcomb jbalcomb 123300 May 31 12:59 /home/jbalcomb/STU_DBWD/dosbox-capture/prng-port/baselines/stage0-newgame/canonical-save1.GAM

-rw-rw-r-- 1 jbalcomb jbalcomb    1250 May 31 12:56 og_rng.csv

cat /tmp/og_rng.csv



stage1-wizards-load/og-save1.GAM
stage1-wizards-load/canonical-save3.GAM

stage2-next-turn/og-save1.GAM
stage2-next-turn/canonical-save5.GAM



Now: ./compare-stage.sh stage1-wizards-load


run:
/home/jbalcomb/STU_DBWD/dosbox-capture/prng-port/baselines/launch-baseline.sh
...runs dosbox -conf baseline.conf (which mounts MoM131 and starts MAGIC.EXE)
...on exit, snapshots MoM131/SAVE9.GAM → canonical-save9.GAM and MoM131/SAVE1.GAM → stage1-wizards-load/og-save1.GAM

Heads-up: it currently snapshots og-save1.GAM only into stage1-wizards-load/. If you intend to refresh Stage 2's og-save1.GAM from this same run, the script doesn't do that — you'd need to copy it manually or run refresh-stage.sh stage2-next-turn after.

Game Options:
   Difficulty Normal
   Opponents  Four
   Land Size  Large
   Magic      Normal
Select Wizard
   Custom
Select Picture
   Rjak
Wizard's Name
   Rjak
Skills and Spells
   Artificer
   Life    2
   Death   0
   Chaos   2
   Nature  2
   Sorcery 4
Life Spells
   Heroism (default)
Chaos Spells
   Fire Bolt (default)
Nature Spells
   War Bears (default)
Sorcery Spells
   Phantom Warriors (default) 
   Nagas (default)
   Psionic Blast (default)
Select Race
   Orc
Select Banner
   Yellow
* Building the Worlds...
Name Starting City
   RandomCity
Save Slot 1
   PrngStage0
type G for Game button, click save slot 1, delete save game name, type "PrngBaseLine", click Save, type G for Game button, click Quit, type Q for Quit to DOS
type exit, hit enter; script completes by capturing the new baseline save-game

baseline-run.log





overall project goal:
prove that ReMoM produces the same behavior and data as OG-MoM
pieces & parts:
create a STU-Extras sub-project, STU-DOSBox, that is add-in code for dosbox to track function calls and data from OG-MoM



DOSBos uses a synthetic segment value of 0xF000 for BIOS and DOS interrupt calls.
We will generically refer to them as ISRs, so we can know what the guest program is doing.




Goal #1: New Game
OG-MoM
run MAGIC.EXE
difficulty = Normal
magic      = Normal
landsize   = Large
opponents  = 4
wizard = Rjak
race   = Orc
banner = Yellow
Artificer
2, 0, 2, 2, 4
home_city_name = RandomCity
slot1_save_name = PrngGoal1
...
backup SAVE1.GAM as the new baseline for Goal #2


Goal #2: Load Game
OG-MoM
run MAGIC.EXE
1. Main Menu → **Load Game** → Load save slot 1 "PrngGoal1"
2. Home-city naming popup: clear the random suggestion, type
   `RandomCity`, Enter.  (Must match `home_city_name` in
   [PrngNewGame.INI](../PrngNewGame.INI).)

home_city_name = RandomCity
slot2_save_name = PrngGoal2
...
backup SAVE2.GAM as the new baseline for Goal #2


Goal #3: Next Turn
load save slot 1
RandomCity
slot3_save_name = PrngGoal3


## Tools
Data Segment Finder
Borland Overlay Finder
OG-MoM Overlay Table


## Targets

### _unit
first change is as seen on line 269 in /home/jbalcomb/STU/devel/ReMoM/MoX/src/LOADSAVE.c - stu_fread(&_unit, 1, 2, file_pointer);
but, in OG-MoM, that is gread() and a DOS INT file read, using _unit as the read buffer
Load_SAVE_GAM() |-> gfread() |-> INT DOS 21h 3Fh
Load_SAVE_GAM() |-> stu_fread()
the specifc gfread call in Load_SAVE_GAM is at offset 05BA
Per Claude, F000:14C1 is DOSBox's fake CS+IP for the native DOS callback.

Init_Overland() |-> WIZ_NextIdleStack() |-> Next_Unit_Nearest_Available()

Sort_Unit_Stack

NX_j_Sort_Unit_Stack
nothing calls the stub\thunk for Sort_Unit_Stack
Select_Unit_Stack() |-> Sort_Unit_Stack()
54E2:0039  E8 E1 00  call near ptr Sort_Unit_Stack

Re: STK_DisplaySort, it is Sort_Unit_Stack, there was stale data in the overlay table file.



WZD
33DF:0000
segment stub051
stru_33DF0 = stub_descr ptr $
CD 3F           int_code
00 00           memswap
90 08           fileoff
00 00 57 02     codesize
2C 00           relsize
02 00           nentries
00 00           prevstub
db 10h dup(0)   workarea
=== "descriptor"
``asm
STRUC stub_descr
    byte int_code[2] = {0xCD, 0x3F}
    word memswap
    word fileoff
    dword codesize
    word relsize
    word nentries
    word prevstub
    byte workarea[16]
```
```c
struct stub_descr
{
    uint8_t int_code[2] = {0xCD, 0x3F}
    uint16_t memswap
    uint16_t fileoff
    uint32_t codesize
    uint16_t relsize
    uint16_t nentries
    uint16_t prevstub
    uint8_t workarea[16]
};
```

I think I see where the confusion is. Let me be more careful about what's resident vs overlay.

When the resolver prints


[OG_OVR]   resolved: WIZARDS.EXE  IDA stub125:0034 = j_Load_MAGIC_SET+0
it's naming the THUNK that lives in stub125, which is a resident-image segment. stub125 itself is loaded once at program start and never moves — its bytes (the 32-byte descriptor + the 5-byte per-function thunks following it) sit at a fixed paragraph in resident memory for the whole session.

j_Load_MAGIC_SET is the label IDA put on the thunk at stub125:0034. The j_ prefix is IDA's convention for "jump thunk" — it's a 5-byte stub whose entire job is to fire INT 3F and let the overlay manager dispatch to the real Load_MAGIC_SET body. The thunk and the real function share a name, but they live in two different places:

Thunk j_Load_MAGIC_SET → in resident segment stub125 at offset 0x0034. Static memory, fixed CS, our single base_offset resolves it correctly.
Real function body Load_MAGIC_SET → in ovr125 (the overlay code segment). Loaded into a buffer on demand at a dynamic runtime CS. This is R6 territory.
The CPU sequence when something calls Load_MAGIC_SET:

CALL stub125:0034 — execution enters the thunk. CS = resident stub125.
CD 3F at stub125:0034 executes — this is what our INT 3F probe sees. CS still resident. Resolver correctly says stub125:j_Load_MAGIC_SET+0. R6 not involved.
Overlay manager handler runs, loads ovr125 if needed, executes JMP FAR ovr125_runtime_buffer:offset_of_Load_MAGIC_SET.
Code runs inside the overlay buffer. CS = dynamic overlay paragraph. Now we're in R6 territory. Our INT 3F probe doesn't see step 4 (it doesn't execute CD 3F); it's og_describe() from og_stage2_unit_write_probe (or whatever future probe) that has to resolve those addresses.
So all four "resolved" lines being accurate is consistent: we're resolving the CD 3F site, which is always in a resident stub. The R6 problem only shows up when we try to resolve a CS:IP captured FROM INSIDE the overlay's loaded code body — like the _unit writes during AI processing.



_ovrbuffer  size of the overlay buffer
WZD  36AA:2080  __ovrbuffer  00 10  dw 406



When an INT 3F triggers, it's at the function address in the stub segment, correct? And, you can offset back to the stub header from there, correct?
Yes, to both. Direct answers:

1. INT 3F at function thunk address in stub segment: correct. When CD 3F executes, the CPU's CS:IP is pointing at that CD 3F instruction. CS is the stubXXX segment paragraph. IP is the offset of that specific function's thunk within the stub segment. Different functions in the same overlay = different IPs in the same stub segment (the thunk table after the 32-byte descriptor).

2. Offset back to the stub header: correct. The stub descriptor is always at offset 0 of the stub segment. From the INT 3F's CS:IP, the header is at the same segment with offset 0 — i.e., the segment paragraph the CD 3F is in, accessed at offset 0..31 reads the descriptor.

Concretely in our existing code that's exactly what og_borland_log_int3f_stub_block does: at every CD 3F fire it reads CS:0 to dump the descriptor for the segment the fire is in.





When you launch WIZARDS.EXE,

DIR()
fopen()
fread()                 9A FE 2E 00 10  1000:2EFE   segment seg000
close()
j_Load_MAGIC_SET()      9A 34 00 90 35  3590:0034   segment stub125


CALLF  04C2:001F (seg001:_main+16)  ->  05BA:000C (seg004:DIR+0)
CALLF  05BA:0017 (seg004:DIR+B)  ->  158F:039E (seg033:Save_Mouse_State+0)
INT    05BA:001E (seg004:DIR+12)  ->  F000:14C0 (INT 21h handler)
INT    05BA:003C (seg004:DIR+30)  ->  F000:14C0 (INT 21h handler)
CALLF  05BA:0064 (seg004:DIR+58)  ->  158F:03B6 (seg033:Restore_Mouse_State+0)
CALLF  04C2:003C (seg001:_main+33)  ->  01A2:214A (seg000:fopen+0)
CALLN  01A2:214D (seg000:fopen+3)  ->  01A2:211F (seg000:__getfp+0)
CALLN  01A2:2164 (seg000:fopen+1A)  ->  01A2:2083 (seg000:__openfp+0)
CALLN  01A2:2098 (seg000:__openfp+15)  ->  01A2:1FC9 (seg000:CheckOpenType+0)
CALLN  01A2:20B7 (seg000:__openfp+34)  ->  01A2:26C3 (seg000:open+0)
CALLN  01A2:26E7 (seg000:open+24)  ->  01A2:1D63 (seg000:_chmod+0)
INT    01A2:1D71 (seg000:_chmod+E)  ->  F000:14C0 (INT 21h handler)
CALLN  01A2:2775 (seg000:open+B2)  ->  01A2:281B (seg000:_open+0)
INT    01A2:2842 (seg000:_open+27)  ->  F000:14C0 (INT 21h handler)
CALLN  01A2:2786 (seg000:open+C3)  ->  01A2:1898 (seg000:ioctl+0)
INT    01A2:18A9 (seg000:ioctl+11)  ->  F000:14C0 (INT 21h handler)
CALLN  01A2:20D8 (seg000:__openfp+55)  ->  01A2:0965 (seg000:isatty+0)
INT    01A2:096E (seg000:isatty+9)  ->  F000:14C0 (INT 21h handler)
CALLN  01A2:20FE (seg000:__openfp+7B)  ->  01A2:2C3F (seg000:_setvbuf+0)
CALLN  01A2:2CE4 (seg000:_setvbuf+A5)  ->  01A2:1A25 (seg000:_malloc+0)
CALLN  01A2:1A5C (seg000:_malloc+37)  ->  01A2:1AC5 (seg000:nExtendHeap+0)
CALLN  01A2:1ACA (seg000:nExtendHeap+5)  ->  01A2:0F2A (seg000:__sbrk+0)
CALLF  04C2:0055 (seg001:_main+4C)  ->  01A2:2240 (seg000:fread+0)
CALLN  01A2:2257 (seg000:fread+17)  ->  01A2:0ABB (seg000:N_LXMUL@+0)
CALLN  01A2:2274 (seg000:fread+34)  ->  01A2:2169 (seg000:_fgetn+0)
CALLN  01A2:221E (seg000:_fgetn+B5)  ->  01A2:24E5 (seg000:_fgetc+0)
CALLN  01A2:24F0 (seg000:_fgetc+B)  ->  01A2:24FA (seg000:fgetc@+0)
CALLN  01A2:253C (seg000:fgetc@+42)  ->  01A2:248C (seg000:_ffill+0)
CALLN  01A2:24AE (seg000:_ffill+22)  ->  01A2:2B71 (seg000:__read+0)
CALLN  01A2:2BB2 (seg000:__read+41)  ->  01A2:0AF3 (seg000:_read+0)
INT    01A2:0B14 (seg000:_read+21)  ->  F000:14C0 (INT 21h handler)
CALLF  04C2:0060 (seg001:_main+57)  ->  01A2:1E8D (seg000:fclose+0)
CALLN  01A2:1EC3 (seg000:fclose+36)  ->  01A2:1956 (seg000:_free+0)
CALLN  01A2:196D (seg000:_free+17)  ->  01A2:1973 (seg000:nFreeLastBlock+0)
CALLN  01A2:19A8 (seg000:nFreeLastBlock+35)  ->  01A2:0F08 (seg000:__brk+0)
CALLN  01A2:1ED4 (seg000:fclose+47)  ->  01A2:1D7E (seg000:close+0)
CALLN  01A2:1DA0 (seg000:close+22)  ->  01A2:1DA6 (seg000:_close+0)
INT    01A2:1DAE (seg000:_close+8)  ->  F000:14C0 (INT 21h handler)
CALLF  04C2:0066 (seg001:_main+5D)  ->  2323:0034 (stub049:j_Load_MAGIC_SET+0)
CALLF  33FE:0837 (ovr049:Load_MAGIC_SET+10)  ->  05BA:000C (seg004:DIR+0)



Yes. 9A is CALL FAR ptr16:16 — always 5 bytes: opcode + 2-byte target offset + 2-byte target segment, both little-endian. So no operand parsing needed; we already know the size and layout.

Adding a generic per-far-call-site probe (one log line per unique src_cs:src_ip so we don't drown the log — modern programs do millions of calls per second). Each line will resolve both source and destination via the existing og_describe:

423 unique call sites in 10s, no cap. And the chain you described is visible:

/home/jbalcomb/STU/devel/STU-Extras/STU-DOSBox/r6-overlay-resolver-test.sh 10 > /dev/null 2>&1

echo "=== overlays command ==="
/home/jbalcomb/STU/devel/STU-Extras/STU-DOSBox/analyze-wizards-calls.py overlays
echo
echo "=== callees of ovr052 (what does ovr052 call?) ==="
/home/jbalcomb/STU/devel/STU-Extras/STU-DOSBox/analyze-wizards-calls.py callees ovr052 | head -20
echo
echo "=== callers of EMM_LBX_EntryLoader (who loads LBX entries?) ==="
/home/jbalcomb/STU/devel/STU-Extras/STU-DOSBox/analyze-wizards-calls.py callers EMM_LBX_EntryLoader | head -10



start+158 -> _main+0
_main+5D -> stub125:j_Load_MAGIC_SET+0     ← caller dispatching to overlay manager via the stub
ovr129:Cast_Chaos_Channels+89 -> seg004:DIR+0  ← overlay-buffer code executing & making far calls
That last line is significant: source CS 46DE is the runtime paragraph where ovr129 is currently loaded. We're seeing the loaded-overlay runtime CS as the source of CALL FARs from overlay code. That cross-references nicely with whatever usbBufferSeg reads we capture from descriptors.

Let me also check whether Init_Overland's call chain is visible:


// WZD o50p01
void Save_SAVE_GAM(int16_t save_gam_idx)
...
4558:05AA 56                                                  push _SI_FileHandle
4558:05AB B8 01 00                                            mov ax, 1
4558:05AE 50                                                  push ax                             ; nitems
4558:05AF B8 02 00                                            mov ax, 2
4558:05B2 50                                                  push ax                             ; size
4558:05B3 33 C0                                               xor ax, ax
4558:05B5 50                                                  push ax                             ; buffer_sgmt
4558:05B6 B8 8E BD                                            mov ax, offset _unit
4558:05B9 50                                                  push ax                             ; ptr
4558:05BA 9A 78 00 9A 16                                      call gfread
4558:05BF 83 C4 0A                                            add sp, 0Ah

169A:0078                                                 proc gfread far                         ; CODE XREF: UU_GUI_LoadClickFile+32P ...
169A:0078                                                 ptr = dword ptr  6
169A:0078                                                 size= word ptr  0Ah
169A:0078                                                 nitems= word ptr  0Ch
169A:0078                                                 uu_file_handle= word ptr  0Eh
169A:0078 55                                                  push bp
169A:0079 8B EC                                               mov bp, sp
169A:0079
169A:0079
169A:007B 8B 46 0A                                            mov ax, [bp+size]
169A:007E F7 6E 0C                                            imul [bp+nitems]
169A:0081 A3 E6 E5                                            mov [_gf_bsize], ax
169A:0081
169A:0081
169A:0084 83 7E 08 00                                         cmp [word ptr bp+ptr+2], 0
169A:0088 75 05                                               jnz short loc_16A2F
169A:008A 8C D8                                               mov ax, ds
169A:008C 89 46 08                                            mov [word ptr bp+ptr+2], ax
169A:008F                                                 loc_16A2F:                              ; CODE XREF: gfread+10j
169A:008F 1E                                                  push ds
169A:008F
169A:008F
169A:0090 8B 1E E8 E5                                         mov bx, [_gf_fhandle]
169A:0094 8B 0E E6 E5                                         mov cx, [_gf_bsize]
169A:0098 8B 46 08                                            mov ax, [word ptr bp+ptr+2]
169A:009B 8E D8                                               mov ds, ax
169A:009D 8B 56 06                                            mov dx, [word ptr bp+ptr]
169A:00A0 B4 3F                                               mov ah, 3Fh
169A:00A2 CD 21                                               int 21h                             ; DOS - 2+ - READ FROM FILE WITH HANDLE
169A:00A2                                                                                         ; BX = file handle, CX = number of bytes to read
169A:00A2                                                                                         ; DS:DX -> buffer
169A:00A4 1F                                                  pop ds
169A:00A4
169A:00A4
169A:00A5 5D                                                  pop bp
169A:00A6 CB                                                  retf
169A:00A6                                                 endp gfread






WIZARDS.EXE
segment stub125

3590:0000                                                 ; ===========================================================================
3590:0000                                                 ; Segment type: Pure code
3590:0000                                                 segment stub125 para public 'CODE' use16
3590:0000                                                     assume cs:stub125
3590:0000                                                     assume es:nothing, ss:nothing, ds:dseg, fs:nothing, gs:nothing
3590:0000 CD 3F 00 00 00 7F 06 00 66 0A A4 00 05 00 00 00+stru_35900  = _stub_descr ptr $
3590:0000 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00     db 0CDh, 3Fh                        ; int_code
3590:0000                                                     dw 0                                ; memswap
3590:0000                                                     dd 67F00h                           ; fileoff
3590:0000                                                     dw 0A66h                            ; codesize
3590:0000                                                     dw 0A4h                             ; relsize
3590:0000                                                     dw 5                                ; nentries
3590:0000                                                     dw 0                                ; prevstub
3590:0000                                                     db 10h dup(0)                       ; workarea
3590:0020                                                 ; =============== S U B R O U T I N E =======================================
3590:0020                                                 ; Attributes: thunk
3590:0020                                                 proc j_Settings_Screen_Draw far         ; DATA XREF: Settings_Screen+281o
3590:0020 EA B8 03 FC A8                                      jmp Settings_Screen_Draw
3590:0020                                                 endp j_Settings_Screen_Draw
3590:0025                                                 ; =============== S U B R O U T I N E =======================================
3590:0025                                                 ; Attributes: thunk
3590:0025                                                 proc j_Settings_Screen far              ; CODE XREF: Load_Screen+461P
3590:0025 EA 00 00 FC A8                                      jmp Settings_Screen
3590:0025                                                 endp j_Settings_Screen
3590:002A                                                 ; =============== S U B R O U T I N E =======================================
3590:002A                                                 ; Attributes: thunk
3590:002A                                                 proc j_GAME_ResetSettings far
3590:002A EA 4E 09 FC A8                                      jmp Set_Default_Game_Settings
3590:002A                                                 endp j_GAME_ResetSettings
3590:002F                                                 ; =============== S U B R O U T I N E =======================================
3590:002F                                                 ; Attributes: thunk
3590:002F                                                 proc j_HLP_Load_Settings far
3590:002F EA 33 0A FC A8                                      jmp Set_Settings_Screen_Help_List
3590:002F                                                 endp j_HLP_Load_Settings
3590:0034                                                 ; =============== S U B R O U T I N E =======================================
3590:0034                                                 ; Attributes: thunk
3590:0034                                                 proc j_Load_MAGIC_SET far               ; CODE XREF: _main+5DP ...
3590:0034 EA 27 08 FC A8                                      jmp Load_MAGIC_SET
3590:0034                                                 endp j_Load_MAGIC_SET
3590:0034                                                 ends stub125
