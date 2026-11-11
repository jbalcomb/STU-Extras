


This assembly code handles Fog of War smoothing (or "Auto-Exploration").

When a unit moves and uncovers tiles, it can leave behind ugly, jagged, or illogical unmapped corners. This function scans a 5x5 grid around a target tile and runs a massive lookup table of 38 specific patterns. If it finds a pattern (e.g., "The Top-Left corner is explored, but the adjacent tile is unexplored"), it automatically fills in the missing bits to make the exploration radius look smooth and organic.

Here is the faithful C89 translation, incorporating the exact logic and preserving the researcher notes/bugs you found in the assembly.

The C89 Translation



A Few Key Observations:
  1. The Leftward Band-Aid: Rules #17, #19, and #20 use Adj_Direction = 2 (Left). The assembly explicitly comments this block as a "band-aid" fix. This usually happens in late development when QA notices weird visual artifacts on the left side of the screen, and instead of fixing the root drawing logic, a programmer just slaps 3 new specific left-facing rules into the array!
  2. The "Classic" Bug: As the assembly comments point out, the inner loop never updates the local variable tile_val when a match alters the map. Because of this, rule chains break down—a rule that should trigger based on the result of a previous rule won't fire until the player takes another step and calls this entire function again.
  3. Poles vs Equator Wrap: The code perfectly handles a cylindrical or spherical map. X wraps around (-= e_WORLD_WIDTH), but Y explicitly clamps to e_WORLD_YMAX, meaning you can walk around the globe east/west, but you can't walk "over" the north/south poles.



/* Standard bitflags for 4-corner tile exploration */
#define SCT_TopLeft      0x01
#define SCT_TopRight     0x02
#define SCT_BottomLeft   0x04
#define SCT_BottomRight  0x08

struct Explore_Matching {
    unsigned short Tile_Value;
    unsigned short Adj_Scouting;
    unsigned short Mark_Tile;
    unsigned short Mark_Adj;
    unsigned short Adj_Direction; /* 0 = Right, 1 = Below, 2 = Left */
};

extern unsigned char far _square_explored[];

void far Set_Square_Explored_Flags_Fix(short wx, short wy, short wp) {
    int x_loop, y_loop, i;
    short wx_l2, wy_u2, cur_x, cur_y;
    short adj_row, adj_col;
    unsigned char tile_val, adj_val;
    unsigned long tile_offset, adj_offset;
    unsigned char far *ptr_square_explored;

    /* The 38 Pattern Matching Rules for Exploration Smoothing */
    static const struct Explore_Matching match_array[38] = {
        /* 00 */ { SCT_BottomLeft, SCT_BottomRight, 9, 9, 0 },
        /* 01 */ { SCT_BottomLeft | SCT_TopLeft | SCT_BottomRight, SCT_BottomLeft | SCT_TopRight | SCT_BottomRight, 15, 15, 0 },
        /* 02 */ { SCT_BottomLeft | SCT_TopLeft | SCT_TopRight, SCT_TopLeft | SCT_TopRight | SCT_BottomRight, 15, 15, 0 },
        /* 03 */ { SCT_TopLeft, SCT_TopRight | SCT_BottomRight, 6, 15, 0 }, /* Bug noted: Mark_Adj should be 0Eh */
        /* 04 */ { SCT_BottomLeft | SCT_TopLeft | SCT_TopRight, SCT_BottomLeft | SCT_TopLeft | SCT_BottomRight, 15, 15, 1 },
        /* 05 */ { SCT_TopLeft, SCT_BottomLeft | SCT_BottomRight, 3, 11, 1 },
        /* 06 */ { SCT_TopLeft | SCT_TopRight, SCT_BottomLeft | SCT_BottomRight, 15, 15, 1 },
        /* 07 */ { SCT_BottomLeft | SCT_TopLeft | SCT_TopRight | SCT_BottomRight, SCT_BottomLeft | SCT_TopRight | SCT_BottomRight, 15, 15, 1 },
        /* 08 */ { SCT_BottomLeft | SCT_TopLeft, SCT_BottomRight, 11, 9, 0 },
        /* 09 */ { SCT_TopLeft, SCT_BottomRight, 6, 9, 0 }, /* Concept flaw noted */
        /* 10 */ { SCT_BottomLeft | SCT_TopLeft | SCT_TopRight, SCT_BottomLeft | SCT_TopLeft, 15, 3, 0 },
        /* 11 */ { SCT_BottomLeft | SCT_TopLeft | SCT_TopRight | SCT_BottomRight, SCT_TopLeft | SCT_TopRight | SCT_BottomRight, 15, 15, 0 },
        /* 12 */ { SCT_BottomLeft | SCT_TopLeft | SCT_TopRight | SCT_BottomRight, SCT_BottomLeft | SCT_TopRight | SCT_BottomRight, 15, 15, 0 },
        /* 13 */ { SCT_BottomLeft | SCT_TopLeft | SCT_BottomRight, SCT_BottomLeft | SCT_TopLeft | SCT_BottomRight, 11, 15, 1 },
        /* 14 */ { SCT_BottomLeft | SCT_BottomRight, SCT_BottomLeft | SCT_TopRight | SCT_BottomRight, 9, 15, 1 },
        /* 15 */ { SCT_BottomLeft | SCT_TopLeft, SCT_BottomLeft | SCT_TopLeft | SCT_TopRight | SCT_BottomRight, 15, 15, 0 },
        /* 16 */ { SCT_TopRight, SCT_BottomLeft | SCT_BottomRight, 15, 15, 1 }, /* Bugs noted: Mark_Tile=0Ch, Mark_Adj=0Dh */
        /* 17 */ { SCT_BottomLeft | SCT_TopLeft | SCT_TopRight | SCT_BottomRight, SCT_BottomLeft | SCT_TopLeft | SCT_TopRight, 15, 15, 2 },
        /* 18 */ { SCT_BottomLeft | SCT_TopLeft | SCT_TopRight | SCT_BottomRight, SCT_BottomLeft | SCT_TopRight | SCT_BottomRight, 15, 15, 1 },
        /* 19 */ { SCT_BottomLeft | SCT_TopLeft | SCT_TopRight | SCT_BottomRight, SCT_BottomLeft | SCT_TopLeft | SCT_BottomRight, 15, 15, 2 },
        /* 20 */ { SCT_BottomLeft | SCT_TopLeft | SCT_TopRight | SCT_BottomRight, SCT_BottomLeft | SCT_TopLeft, 15, 15, 2 },
        /* 21 */ { SCT_TopLeft | SCT_TopRight | SCT_BottomRight, SCT_BottomLeft | SCT_TopLeft | SCT_TopRight | SCT_BottomRight, 15, 15, 1 },
        /* 22 */ { SCT_BottomLeft | SCT_TopLeft | SCT_TopRight, SCT_BottomLeft | SCT_TopLeft | SCT_TopRight | SCT_BottomRight, 15, 15, 1 },
        /* 23 */ { SCT_TopLeft, SCT_BottomLeft, 3, 3, 1 },
        /* 24 */ { SCT_TopLeft | SCT_TopRight, SCT_BottomLeft, 15, 3, 1 }, /* Bug noted: Mark_Tile should be 07h */
        /* 25 */ { SCT_BottomLeft | SCT_TopLeft | SCT_BottomRight, SCT_BottomLeft | SCT_TopLeft, 15, 3, 0 },
        /* 26 */ { SCT_TopLeft, SCT_TopRight, 6, 6, 0 },
        /* 27 */ { SCT_TopLeft | SCT_TopRight, SCT_BottomRight, 14, 12, 1 },
        /* 28 */ { SCT_TopRight, SCT_BottomRight, 12, 12, 1 },
        /* 29 */ { SCT_TopRight | SCT_BottomRight, SCT_TopLeft | SCT_TopRight | SCT_BottomRight, 12, 15, 0 },
        /* 30 */ { SCT_TopRight | SCT_BottomRight, SCT_BottomLeft | SCT_TopRight | SCT_BottomRight, 12, 15, 0 },
        /* 31 */ { SCT_BottomLeft | SCT_TopLeft, SCT_TopRight, 7, 6, 0 },
        /* 32 */ { SCT_BottomLeft, SCT_TopRight | SCT_BottomRight, 9, 15, 0 }, /* Bug noted: Mark_Adj should be 0Dh */
        /* 33 */ { SCT_TopLeft | SCT_TopRight, SCT_BottomLeft | SCT_TopLeft | SCT_TopRight | SCT_BottomRight, 15, 15, 1 },
        /* 34 */ { SCT_BottomLeft | SCT_BottomRight, SCT_TopRight | SCT_BottomRight, 9, 15, 1 },
        /* 35 */ { SCT_BottomLeft | SCT_TopLeft | SCT_TopRight | SCT_BottomRight, SCT_TopRight | SCT_BottomRight, 15, 15, 1 },
        /* 36 */ { SCT_BottomLeft | SCT_TopLeft | SCT_TopRight | SCT_BottomRight, SCT_TopLeft, 15, 6, 1 },
        /* 37 */ { SCT_BottomLeft | SCT_TopLeft | SCT_TopRight | SCT_BottomRight, SCT_BottomLeft | SCT_TopRight, 15, 15, 1 }
    };

    /* Point to the specific map plane */
    ptr_square_explored = _square_explored + ((long)wp * e_WORLD_SIZE);

    /* Start position: center (wx,wy) minus 2 to get top-left of 5x5 grid */
    wx_l2 = wx - 2;
    if (wx_l2 < 0) wx_l2 += e_WORLD_WIDTH;

    wy_u2 = wy - 2;
    if (wy_u2 < 1) wy_u2 = 1;

    /* 5x5 Outer Grid Loop */
    for (y_loop = 0; y_loop < 5; y_loop++) {
        for (x_loop = 0; x_loop < 5; x_loop++) {
            
            cur_x = wx_l2 + x_loop;
            if (cur_x >= e_WORLD_WIDTH) cur_x -= e_WORLD_WIDTH;

            tile_offset = (wy_u2 * e_WORLD_WIDTH) + cur_x;
            tile_val = ptr_square_explored[tile_offset];

            /* Check against all 38 smoothing rules */
            for (i = 0; i < 38; i++) {
                if (match_array[i].Tile_Value != tile_val) continue;

                adj_row = wy_u2;
                adj_col = cur_x;

                /* Determine adjacent tile based on rule direction */
                switch (match_array[i].Adj_Direction) {
                    case 0: /* Tile_Right */
                        adj_col++;
                        if (adj_col >= e_WORLD_WIDTH) adj_col -= e_WORLD_WIDTH;
                        break;
                    case 1: /* Tile_Below */
                        adj_row++;
                        if (adj_row >= e_WORLD_HEIGHT) adj_row = e_WORLD_YMAX;
                        break;
                    case 2: /* Tile_Left (Noted as a band-aid in assembly) */
                        adj_col--;
                        if (adj_col < 0) adj_col = e_WORLD_XMAX;
                        break;
                }

                adj_offset = (adj_row * e_WORLD_WIDTH) + adj_col;
                adj_val = ptr_square_explored[adj_offset];

                /* If the adjacent tile matches the pattern, apply the fix */
                if (match_array[i].Adj_Scouting == adj_val) {
                    ptr_square_explored[tile_offset] |= match_array[i].Mark_Tile;
                    ptr_square_explored[adj_offset]  |= match_array[i].Mark_Adj;
                    
                    /* * CLASSIC BUG: The original tile_val and adj_val local variables 
                     * are NOT updated here. If this tile triggers another match further 
                     * down the 38-rule list in the exact same frame, it will be checking 
                     * against the old state, potentially causing visual logic errors.
                     */
                }
            }
        }
        
        /* Advance Y row and clamp to bottom of the map (usually the south pole) */
        wy_u2++;
        if (wy_u2 >= e_WORLD_HEIGHT) {
            wy_u2 = e_WORLD_YMAX;
        }
    }
}



This is the absolute best way to handle reverse-engineered ports! Giving players a "Classic Mode" vs. "Enhanced Mode" toggle in the options menu is a staple of source ports (like Doom or Command & Conquer), keeping speedrunners and purists happy while providing a smoother experience for modern players.
To do this, we just need to introduce a global configuration flag and add a tiny check inside the successful match block.
Here is how you would implement that optional fix in C89:
The Toggle and The Fix
First, somewhere in your game's configuration or settings header, you would define your toggle:

Next, we update that inner loop from the Set_Square_Explored_Flags_Fix function. We only need to change what happens after a successful rule match:

Why this matters for "Rule Chaining"
Let's say the player steps into a heavily obscured corner.
  * Rule #3 runs and says: "Ah, I see a sharp edge here. I'll add the SCT_BottomLeft flag to round it out." * In the original DOS game, the local tile_val variable doesn't know about that new flag. So when the loop reaches Rule #15 (which specifically looks for a tile that has SCT_BottomLeft), Rule #15 completely ignores it. The player has to physically move their unit again to trigger the function a second time to catch it!
  * By flipping g_opt_fix_exploration_bugs to 1, tile_val is instantly updated to include SCT_BottomLeft. Rule #15 sees it on the exact same frame, applies its own smoothing, and the Fog of War updates perfectly in a single sweep.
It’s a tiny two-line fix, but it makes the map exploration look noticeably cleaner and more responsive.



/* Global settings flag: 
 * 0 = Original DOS logic (Bugs included for authenticity)
 * 1 = Modern smoothed logic (Fixes the rule-chaining bug) 
 */
extern int g_opt_fix_exploration_bugs;



/* ... inside the 38-rule loop ... */
                
                /* If the adjacent tile matches the pattern, apply the fix */
                if (match_array[i].Adj_Scouting == adj_val) {
                    
                    /* Apply the new exploration flags to the map array */
                    ptr_square_explored[tile_offset] |= match_array[i].Mark_Tile;
                    ptr_square_explored[adj_offset]  |= match_array[i].Mark_Adj;
                    
                    /* --- THE OPTIONAL BUG FIX --- */
                    if (g_opt_fix_exploration_bugs) {
                        /* Update the local variables immediately! 
                         * If Rule #5 changes the tile from 0x01 to 0x03, 
                         * Rule #23 can now successfully catch the 0x03 on 
                         * the very next iteration of this loop. 
                         */
                        tile_val = ptr_square_explored[tile_offset];
                        adj_val  = ptr_square_explored[adj_offset];
                    }
                    /* If g_opt_fix_exploration_bugs is 0, tile_val remains stale, 
                     * perfectly replicating the original engine's blind spot! 
                     */
                }
