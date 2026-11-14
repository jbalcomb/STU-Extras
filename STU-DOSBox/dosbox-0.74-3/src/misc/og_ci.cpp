/* ==================================================================== *
 *  og_ci.cpp -- OG-MoM Capture/Inject capture side (see og_ci.h).
 * ==================================================================== */
#include "og_ci.h"
#include "mem.h"
#include "regs.h"
#include <cstdio>
#include <cstring>

#define OG_CI_WM_PTR_OFF  0x89F2u   /* MGC DGROUP far-ptr slot for _world_maps */
#define OG_CI_WM_OVERRUN  420       /* WORLD_OVERFLOW = 7*WORLD_WIDTH int16 (ReMoM over-alloc past _world_maps[2][40][60]) */
#define OG_CI_MSF_PTR_OFF 0x89CAu   /* MGC DGROUP far-ptr slot for _map_square_flags (uint8/tile) */
#define OG_CI_MSF_OVERRUN 61        /* WORLD_WIDTH + 1 bytes (ReMoM over-alloc past _map_square_flags[2][40][60]) */

enum og_ci_kind { OG_CI_PTR, OG_CI_STACK };

struct og_ci_capfn {
    const char* key;        /* CI variable name                                 */
    const char* site;       /* capture site (a key may have several)            */
    const char* func;       /* og_describe substring confirming identity ("" = skip) */
    Bit32u      off;        /* exe==0 (MGC): trigger overlay offset (== ip_val)  */
                            /* exe==1 (WZD): trigger IDA (== (cs<<4)+ip - base)  */
    int         at_entry;   /* 1 = fire at function entry; 0 = trap FN-EXIT      */
    og_ci_kind  kind;
    Bit16u      ptr_off;    /* PTR: DGROUP far-ptr slot of the block             */
    Bit32u      start;      /* PTR: first element index into the block           */
    int         sp_off;     /* STACK: signed byte offset from SP at the trigger  */
    int         count;      /* number of elements                                */
    int         elem;       /* element size (1 or 2 bytes)                       */
    int         exe;        /* 0 = MAGIC.EXE (Stage 0/1), 1 = WIZARDS.EXE (Stage 2) */
};

/* WORLD_OVERRUN = WORLD_OVERFLOW (7*WORLD_WIDTH = 420) int16 past the logical
 * _world_maps[2][40][60] array -> indices 4800..5219.  This region is OG's
 * adjacent allocation (UU_TBL_1, ...); several functions read it OOB and OG's
 * bytes there differ from ReMoM's over-allocation, so we snapshot it at each
 * reader's entry and inject so ReMoM's OOB reads return OG's bytes:
 *   - post_alloc           : just after Allocate_Data_Space
 *   - simtex_entry         : at Simtex_Autotiling entry (plane-1 row 40 reads)
 *   - animate_oceans_entry : after CRP_NEWG_CreatePathGrids fills UU_TBL_1; what
 *       Animate_Oceans' OGBUG `<=` OOB loop reads at plane-1 row 40.
 *   - gen_ts_entry         : at Generate_Terrain_Specials entry; its OGBUG pick
 *       (wy = itr_wy + Random(radius*2)) overshoots WORLD_HEIGHT on the wp=1
 *       pass, reading the overrun up to ~row 46.  Injected at the function entry
 *       (fires both plane calls; the wp=1 one sets up its reads).
 * The tower `tries` is an uninitialized stack auto [BP-8], read at Generate_Towers
 * entry where SP still points at the near-call return, so [SP-10] == [BP-8]. */
static const og_ci_capfn og_ci_capfns[] = {
    /* --- MAGIC.EXE (Stage 0/1), exe=0 --- */
    { "world_maps_overrun", "post_alloc",           "Allocate_Data_Space",       0x0000, 0, OG_CI_PTR,   OG_CI_WM_PTR_OFF, 4800,   0, OG_CI_WM_OVERRUN, 2, 0 },
    { "world_maps_overrun", "simtex_entry",         "Simtex_Autotiling",         0x40BF, 1, OG_CI_PTR,   OG_CI_WM_PTR_OFF, 4800,   0, OG_CI_WM_OVERRUN, 2, 0 },
    { "world_maps_overrun", "animate_oceans_entry", "CRP_NEWG_CreatePathGrids",  0x89A4, 0, OG_CI_PTR,   OG_CI_WM_PTR_OFF, 4800,   0, OG_CI_WM_OVERRUN, 2, 0 },
    { "world_maps_overrun", "gen_ts_entry",         "Generate_Terrain_Specials", 0x8A13, 1, OG_CI_PTR,   OG_CI_WM_PTR_OFF, 4800,   0, OG_CI_WM_OVERRUN, 2, 0 },
    /* _map_square_flags (uint8/tile) overrun: Generate_Roads' wp=1 pass writes/reads
     * _map_square_flags[(1*2400)+(40*60)+60]=4860, one past the logical 4800.  OG's
     * adjacent bytes != ReMoM's padding, so capture+inject at Generate_Roads entry. */
    { "map_square_flags_overrun", "gen_roads_entry", "Generate_Roads",           0x84D3, 1, OG_CI_PTR,   OG_CI_MSF_PTR_OFF, 4800,  0, OG_CI_MSF_OVERRUN, 1, 0 },
    { "tries",              "gen_towers",           "Generate_Towers",           0x0715, 1, OG_CI_STACK, 0,                    0, -10,   1, 2, 0 },
    /* --- WIZARDS.EXE (Stage 2), exe=1.  WZD points trigger by IDA (resident stub),
     * func="" skips the name check (the stub IDA is unique).  Captured at the far-stub
     * entry where SP == far return, so [SP + sp_off] reads the callee's not-yet-set
     * stack auto (sp_off = -(2 + |BP-offset|)). --- */
    /* AI_Evaluate_Continents uninitialized centroid: wy=[BP-10h], wx=[BP-0Eh].  Read
     * uninitialized in Phase 6 (before Phase 9 sets it).  Captured as [wy, wx] from
     * SP-12h at the j_AI_Evaluate_Continents stub (IDA 0x369FF). */
    { "centroid",           "ai_eval_cont",         "",                          0x000369FF, 1, OG_CI_STACK, 0,               0, -0x12,  2, 2, 1 },
};
#define OG_CI_NCAP ((int)(sizeof(og_ci_capfns) / sizeof(og_ci_capfns[0])))

/* FN-EXIT pending traps (one per non-at_entry record in flight). */
struct og_ci_pend { int active; int fn; Bit16u cs; Bit16u ret_ip; };
static og_ci_pend og_ci_pend_slots[4];

/* Return address of a just-entered function -- near (E8, same segment, returns
 * to callee_cs:roff) or far (returns to the segment word at [SS:SP+2]).  Same
 * logic as the gd trap so it works through overlay j_ stubs. */
static void og_ci_return_addr(Bit16u callee_cs, Bit16u entry_ip,
                              Bit16u* rcs, Bit16u* rip) {
    Bit16u ss_v = SegValue(ss);
    Bit16u sp_v = (Bit16u)(reg_esp & 0xFFFFu);
    Bit16u roff = real_readw(ss_v, sp_v);
    Bit16u site = (Bit16u)(roff - 3);
    if (real_readb(callee_cs, site) == 0xE8) {
        Bit16s rel = (Bit16s)real_readw(callee_cs, (Bit16u)(site + 1));
        if ((Bit16u)((int)site + 3 + rel) == entry_ip) { *rcs = callee_cs; *rip = roff; return; }
    }
    *rcs = real_readw(ss_v, (Bit16u)(sp_v + 2));
    *rip = roff;
}

/* Read one element (PTR: normalized far addr; STACK: SS-relative). */
static int og_ci_read_elem_ptr(Bit16u bseg, Bit16u boff, Bit32u idx, int elem) {
    Bit32u abs = (Bit32u)boff + idx * (Bit32u)elem;
    Bit16u rs  = (Bit16u)(bseg + (Bit16u)(abs >> 4));
    Bit16u ro  = (Bit16u)(abs & 0xFu);
    return (elem == 2) ? (int)(Bit16s)real_readw(rs, ro) : (int)real_readb(rs, ro);
}

static void og_ci_emit(const og_ci_capfn* c, Bit16u dgroup) {
    char buf[3600];   /* up to OG_CI_WM_OVERRUN (420) int16 values, ~7 chars each */
    int  q = 0, i;
    if (c->kind == OG_CI_PTR) {
        Bit16u boff = real_readw(dgroup, c->ptr_off);
        Bit16u bseg = real_readw(dgroup, (Bit16u)(c->ptr_off + 2));
        for (i = 0; i < c->count; i++) {
            int v = og_ci_read_elem_ptr(bseg, boff, c->start + (Bit32u)i, c->elem);
            q += snprintf(buf + q, sizeof(buf) - q, i ? ",%d" : "%d", v);
        }
    } else { /* OG_CI_STACK */
        Bit16u ss_v = SegValue(ss);
        Bit16u sp_v = (Bit16u)(reg_esp & 0xFFFFu);
        for (i = 0; i < c->count; i++) {
            Bit16u o = (Bit16u)((int)sp_v + c->sp_off + i * c->elem);
            int v = (c->elem == 2) ? (int)(Bit16s)real_readw(ss_v, o)
                                   : (int)real_readb(ss_v, o);
            q += snprintf(buf + q, sizeof(buf) - q, i ? ",%d" : "%d", v);
        }
    }
    fprintf(stderr, "[CI] %s %s = %s\n", c->key, c->site, buf);
}

void og_ci_probe(Bit16u cs_val, Bit16u ip_val, Bit32u ida, int active_exe,
                 Bit16u dgroup, og_ci_describe_fn describe) {
    if (active_exe < 0) return;   /* not MAGIC or WIZARDS */
    int i, j;

    /* (1) FN-EXIT fires (only for pending traps of the active exe). */
    for (i = 0; i < 4; i++) {
        og_ci_pend* p = &og_ci_pend_slots[i];
        if (p->active && og_ci_capfns[p->fn].exe == active_exe &&
            p->cs == cs_val && p->ret_ip == ip_val) {
            og_ci_emit(&og_ci_capfns[p->fn], dgroup);
            p->active = 0;
        }
    }

    /* (2) entry: fire (at_entry) or arm the return trap (FN-EXIT).  MGC points
     * trigger by overlay offset (== ip_val); WZD points by IDA. */
    for (i = 0; i < OG_CI_NCAP; i++) {
        const og_ci_capfn* c = &og_ci_capfns[i];
        if (c->exe != active_exe) continue;
        int hit = c->exe ? (ida == c->off) : (ip_val == (Bit16u)c->off);
        if (!hit) continue;
        if (c->func[0]) {   /* "" skips the name check (unique IDA gate) */
            char nm[96];
            describe(cs_val, ip_val, nm, sizeof(nm));
            if (!strstr(nm, c->func)) continue;
        }
        if (c->at_entry) {
            og_ci_emit(c, dgroup);
            continue;
        }
        Bit16u rcs, rip;
        og_ci_return_addr(cs_val, ip_val, &rcs, &rip);
        int slot = -1, exists = 0;
        for (j = 0; j < 4; j++) {
            if (og_ci_pend_slots[j].active && og_ci_pend_slots[j].fn == i &&
                og_ci_pend_slots[j].cs == rcs && og_ci_pend_slots[j].ret_ip == rip) { exists = 1; break; }
            if (!og_ci_pend_slots[j].active && slot < 0) slot = j;
        }
        if (!exists && slot >= 0) {
            og_ci_pend_slots[slot].active = 1;
            og_ci_pend_slots[slot].fn     = i;
            og_ci_pend_slots[slot].cs     = rcs;
            og_ci_pend_slots[slot].ret_ip = rip;
        }
    }
}
