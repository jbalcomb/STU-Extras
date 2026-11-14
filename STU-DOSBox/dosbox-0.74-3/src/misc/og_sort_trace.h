/*
 *  og_sort_trace.h
 *
 *  Hook from CPU_Core_Normal_Run that instruments OG MoM (WIZARDS.EXE)
 *  for the byte-match audit work in prng-port/baselines/.
 *
 *  The hook is invoked once per executed instruction, right after
 *  LOADIP and before the opcode is fetched.  Each game phase has its
 *  own probe; new phases get a new probe section.
 *
 *  Probe sections (in this file, in order below):
 *    - shared helpers (detect WIZARDS' DS, diagnostic counters)
 *    - Stage 1 -- Sort_Research_List I/O trace (load + immediate save)
 *    - Stage 2 -- _unit write trace             (load + next turn + save)
 *    - Stage 3 -- New Game work                 (placeholder for later)
 *
 *  Output: every probe writes to stderr with a "[OG_XXXX]" prefix so a
 *  single grep separates each probe's lines.
 *
 *  Build: -DOG_SORT_TRACE=1 (set in the #define below).  Disable by
 *  setting it to 0; all probes compile to empty inlines.
 *
 *  Critical gotcha to remember (from the Stage 1 debug saga):
 *    `Bit16u cs = SegValue(cs)` shadows the SegNames enum constant
 *    `cs` inside SegValue and reads an uninitialized local.  All
 *    locals here are named `cs_val`, `ip_val`, `bp_val`, `ss_val`
 *    to avoid that trap.
 */

#ifndef DOSBOX_OG_SORT_TRACE_H
#define DOSBOX_OG_SORT_TRACE_H

#define OG_SORT_TRACE 1

#if OG_SORT_TRACE

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "regs.h"
#include "mem.h"
#include "og_ci.h"   /* OG-MoM Capture/Inject capture side (og_ci.cpp) */


/* ====================================================================== *
 *                         SHARED HELPERS                                  *
 * ====================================================================== */

/*
 *  Detect whether a CS value belongs to an Interrupt Service Routine
 *  (ISR) running as a DOSBox native callback rather than as actual
 *  guest x86 code.
 *
 *  DOSBox implements BIOS interrupts (INT 10h, INT 16h, ...) and DOS
 *  services (INT 21h, ...) as native C functions, not as real-mode
 *  code that runs through the CPU core.  When the guest program
 *  executes an INT instruction, DOSBox dispatches to the native
 *  handler; the per-instruction hook sees a fake CS in the
 *  0xF000+ range as the "current instruction" for the callback.
 *
 *  ISR-attributed memory changes are REAL behavior, not noise: a DOS
 *  INT 21h file-read can write game globals directly into the
 *  guest's data segment when loading a save (see
 *  ~/.claude/projects/.../memory/project_save_load_dos_io.md).
 *  So callers should LABEL these as ISR-origin in the log, not
 *  discard them.
 *
 *  Returns 1 if cs_val is in the ISR/callback range.
 */
static inline int og_cs_is_isr(Bit16u cs_val) {
    return cs_val >= 0xF000;
}


/* ----------------------------------------------------------------------
 *  Byte-window helpers.
 *
 *  Instruction/pattern matching used to read a fixed run of bytes at a
 *  single point.  These format a WINDOW -- bytes before AND after a
 *  point -- so a matched pattern (or a return address) shows its
 *  context on both sides.  General-purpose: any probe can use them.
 * -------------------------------------------------------------------- */

/* Space-separated hex of `count` bytes from guest seg:off. */
static inline const char* og_fmt_bytes(Bit16u seg, Bit16u off, int count,
                                       char* buf, size_t buflen) {
    int p = 0, i;
    if (buflen) buf[0] = '\0';
    for (i = 0; i < count; i++) {
        int w = snprintf(buf + p, buflen - (size_t)p, "%s%02X",
                         i ? " " : "", (unsigned)real_readb(seg, (Bit16u)(off + i)));
        if (w < 0 || (size_t)(p + w) >= buflen) break;
        p += w;
    }
    return buf;
}

/* Window around `center`: `before` bytes preceding, then a `| ` marker,
 * then `after` bytes starting AT center.  Reading toward a return
 * address with before>0 shows the CALL instruction that produced it. */
static inline const char* og_fmt_window(Bit16u seg, Bit16u center,
                                        int before, int after,
                                        char* buf, size_t buflen) {
    int p = 0, i;
    if (buflen) buf[0] = '\0';
    for (i = -before; i < after; i++) {
        const char* pre = (i == 0) ? "| " : "";
        int w = snprintf(buf + p, buflen - (size_t)p, "%s%s%02X",
                         (p ? " " : ""), pre,
                         (unsigned)real_readb(seg, (Bit16u)(center + i)));
        if (w < 0 || (size_t)(p + w) >= buflen) break;
        p += w;
    }
    return buf;
}


/* ====================================================================== *
 *                BORLAND OVERLAY MAP  (runtime_cs <-> seg_idx)            *
 * ====================================================================== *
 *
 *  Built by observing INT 3F traffic.  Every Borland overlay function
 *  call dispatches through a stub of the form
 *
 *      INT 3F                    ; 2 bytes (CD 3F)
 *      .DW <metadata>            ; 2 bytes (read by the handler)
 *      JMP FAR overlay:offset    ; 5 bytes (EA off off seg seg)
 *
 *  AX at INT 3F time == the overlay's SEG_IDX (from the wiki overlay
 *  function table).  The seg word of the trailing JMP FAR is patched
 *  by the manager once the overlay is loaded -- that patched value is
 *  the overlay's runtime CS.
 *
 *  We record (runtime_cs, seg_idx) pairs.  og_resolve_cs() reverse-
 *  lookups runtime CS values back to SEG_IDX so existing probe lines
 *  can self-annotate.
 */

#define OG_OVR_MAP_CAPACITY 64
struct og_ovr_entry {
    Bit16u runtime_cs;       /* 0 == slot empty */
    Bit16u seg_idx;
};
static struct og_ovr_entry og_ovr_map[OG_OVR_MAP_CAPACITY];

/* ----------------------------------------------------------------------
 *  Overlay-table loader (runtime parse of WZD-overlay-table.fwv)
 *
 *  Format (fixed-width, 1-line header + 1 separator line):
 *
 *      SEG_ SEG_EA   SEG_NAME   FUNC_EA  FUNC_NAME
 *      0123456789012345678901234567890123456789...
 *
 *      cols  0..3   SEG_   (4 hex digits)
 *      col   4      space
 *      cols  5..12  SEG_EA (8 hex digits, linear)
 *      col   13     space
 *      cols 14..23  SEG_NAME (left-aligned, space-padded)
 *      col   24     space
 *      cols 25..32  FUNC_EA (8 hex digits, linear)
 *      col   33     space
 *      cols 34..    FUNC_NAME (to EOL)
 *
 *  We pre-compute each entry's IP offset within its segment
 *  (= FUNC_EA - SEG_EA) so runtime lookups don't need the SEG_EA.
 *
 *  Lazy load: the first og_resolve_cs / og_seg_name / og_func_at call
 *  opens the file and parses it.  Failure to open just leaves the
 *  tables empty (everything resolves to "?").
 * -------------------------------------------------------------------- */

#define OG_OVRTAB_PATH  "/home/jbalcomb/STU/devel/STU-Extras/STU-DOSBox/WZD-overlay-table.fwv"
#define OG_OVRTAB_MAX_SEGS   320
#define OG_OVRTAB_MAX_FUNCS  4096

struct og_seg_info {
    Bit16u  seg_idx;
    Bit32u  seg_ea;
    char    seg_name[16];
    /* Derived in og_exe_load_ovrtab once header_size is known.
     * file_offset = seg_ea - 0x10000 + header_size  (where in the
     * static EXE bytes this segment lives).
     * size = distance to next segment's seg_ea, or to file end. */
    Bit32u  file_offset;
    Bit32u  size;
};
struct og_func_info {
    Bit16u  seg_idx;
    Bit16u  ip_offset;        /* FUNC_EA - SEG_EA */
    char    func_name[64];
};

static struct og_seg_info  og_seg_table[OG_OVRTAB_MAX_SEGS];
static struct og_func_info og_func_table[OG_OVRTAB_MAX_FUNCS];
static unsigned og_seg_count           = 0;
static unsigned og_func_count          = 0;
static int      og_ovrtab_load_tried   = 0;

static inline void og_ovrtab_load(void) {
    if (og_ovrtab_load_tried) return;
    og_ovrtab_load_tried = 1;

    FILE* fp = fopen(OG_OVRTAB_PATH, "r");
    if (!fp) {
        fprintf(stderr,
                "[OG_OVR] WARN: cannot open %s -- seg/func names will be '?'\n",
                OG_OVRTAB_PATH);
        return;
    }

    char line[256];
    int  lineno = 0;
    Bit16u last_seg = 0xFFFF;

    while (fgets(line, sizeof line, fp) != NULL) {
        lineno++;
        if (lineno <= 2) continue;            /* header + separator */
        if (strlen(line) < 35) continue;      /* malformed / short */

        Bit16u seg_idx = (Bit16u)strtoul(line, NULL, 16);
        Bit32u seg_ea  = (Bit32u)strtoul(line + 5, NULL, 16);
        Bit32u func_ea = (Bit32u)strtoul(line + 25, NULL, 16);

        /* SEG_NAME: cols 14..23, stop at first space. */
        char sname[16];
        int  i;
        for (i = 0; i < 10 && line[14+i] && line[14+i] != ' '
                 && line[14+i] != '\n'; i++) {
            sname[i] = line[14+i];
        }
        sname[i] = '\0';

        /* FUNC_NAME: col 34 to EOL, strip trailing whitespace. */
        char fname[64];
        int  j;
        for (j = 0; j < 63 && line[34+j] && line[34+j] != '\n'
                 && line[34+j] != '\r'; j++) {
            fname[j] = line[34+j];
        }
        while (j > 0 && (fname[j-1] == ' ' || fname[j-1] == '\t')) j--;
        fname[j] = '\0';

        /* New segment? */
        if (seg_idx != last_seg && og_seg_count < OG_OVRTAB_MAX_SEGS) {
            og_seg_table[og_seg_count].seg_idx = seg_idx;
            og_seg_table[og_seg_count].seg_ea  = seg_ea;
            strncpy(og_seg_table[og_seg_count].seg_name, sname, 15);
            og_seg_table[og_seg_count].seg_name[15] = '\0';
            og_seg_count++;
            last_seg = seg_idx;
        }

        /* Function. */
        if (og_func_count < OG_OVRTAB_MAX_FUNCS) {
            og_func_table[og_func_count].seg_idx   = seg_idx;
            og_func_table[og_func_count].ip_offset = (Bit16u)(func_ea - seg_ea);
            strncpy(og_func_table[og_func_count].func_name, fname, 63);
            og_func_table[og_func_count].func_name[63] = '\0';
            og_func_count++;
        }
    }
    fclose(fp);

    fprintf(stderr,
            "[OG_OVR] loaded %u segments, %u functions from %s\n",
            og_seg_count, og_func_count, OG_OVRTAB_PATH);
}

/*
 *  Look up a segment's name by SEG_IDX.  Returns "?" if not loaded or
 *  not found.
 */
static inline const char* og_seg_name(Bit16u seg_idx) {
    if (!og_ovrtab_load_tried) og_ovrtab_load();
    unsigned i;
    for (i = 0; i < og_seg_count; i++) {
        if (og_seg_table[i].seg_idx == seg_idx) {
            return og_seg_table[i].seg_name;
        }
    }
    return "?";
}

/*
 *  Look up the function containing IP `ip_val` in segment SEG_IDX.
 *  Returns the function name and (in *out_func_offset) the byte offset
 *  from that function's entry.  Returns "?" if not loaded or unknown.
 */
static inline const char* og_func_at(Bit16u seg_idx, Bit16u ip_val,
                                     Bit16u* out_func_offset) {
    if (!og_ovrtab_load_tried) og_ovrtab_load();
    unsigned i;
    Bit16u best_off  = 0;
    const char* best = "?";
    for (i = 0; i < og_func_count; i++) {
        if (og_func_table[i].seg_idx != seg_idx) continue;
        if (og_func_table[i].ip_offset > ip_val) continue;
        if (og_func_table[i].ip_offset >= best_off) {
            best_off = og_func_table[i].ip_offset;
            best     = og_func_table[i].func_name;
        }
    }
    if (out_func_offset) *out_func_offset = (Bit16u)(ip_val - best_off);
    return best;
}

/*
 *  Convenience formatter: writes "ovr061:Next_Unit_Nearest_Available+23D"
 *  (or "?") into `buf`.  Returns `buf` for inline use in fprintf args.
 */
static inline const char* og_describe(Bit16u cs_val, Bit16u ip_val,
                                      char* buf, size_t buflen);


/*
 *  Resolve a runtime CS to a SEG_IDX, or 0 if not yet known.
 *  Linear scan over up to 64 entries -- cheap.
 */
static inline Bit16u og_resolve_cs(Bit16u cs_val) {
    unsigned i;
    if (cs_val == 0) return 0;
    for (i = 0; i < OG_OVR_MAP_CAPACITY; i++) {
        if (og_ovr_map[i].runtime_cs == cs_val) {
            return og_ovr_map[i].seg_idx;
        }
    }
    return 0;
}

/* og_describe body is defined later, after og_exes is declared. */

/*
 *  Insert or update a (runtime_cs, seg_idx) pair.  If a slot for
 *  runtime_cs already exists, optionally remap its seg_idx and log
 *  the change.  Otherwise occupy the first empty slot.
 */
static inline void og_ovr_record(Bit16u runtime_cs, Bit16u seg_idx) {
    unsigned i;
    unsigned empty = OG_OVR_MAP_CAPACITY;
    if (runtime_cs == 0) return;
    for (i = 0; i < OG_OVR_MAP_CAPACITY; i++) {
        if (og_ovr_map[i].runtime_cs == runtime_cs) {
            if (og_ovr_map[i].seg_idx != seg_idx) {
                fprintf(stderr,
                        "[OG_OVR] remapped runtime_cs=%04X  seg_idx %04X -> %04X\n",
                        runtime_cs, og_ovr_map[i].seg_idx, seg_idx);
                og_ovr_map[i].seg_idx = seg_idx;
            }
            return;
        }
        if (og_ovr_map[i].runtime_cs == 0 && empty == OG_OVR_MAP_CAPACITY) {
            empty = i;
        }
    }
    if (empty < OG_OVR_MAP_CAPACITY) {
        og_ovr_map[empty].runtime_cs = runtime_cs;
        og_ovr_map[empty].seg_idx    = seg_idx;
        fprintf(stderr,
                "[OG_OVR] mapped  seg_idx=%04X (%s)  -> runtime_cs=%04X\n",
                seg_idx, og_seg_name(seg_idx), runtime_cs);
    } else {
        fprintf(stderr,
                "[OG_OVR] WARN map full; dropping runtime_cs=%04X seg_idx=%04X\n",
                runtime_cs, seg_idx);
    }
}

/* ----------------------------------------------------------------------
 *  Static EXE image loader, for runtime-byte -> EXE-file matching.
 *
 *  When a fire happens, the runtime bytes around CS:IP exist somewhere
 *  in WIZARDS.EXE (per empirical finding -- the Borland CRT stamps
 *  these bytes from a static template).  By searching the file for
 *  the runtime sequence we get a file offset, which converts to an
 *  IDA-linear address, which the overlay function table maps to a
 *  segment name + function name + offset.
 * -------------------------------------------------------------------- */
#define OG_EXE_COUNT 2

struct og_exe_info {
    const char*    path;
    const char*    ovrtab_path;     /* matching .fwv overlay function table */
    const char*    label;           /* short name for log lines */
    const char*    exename;         /* lowercase __EXENAME__ literal we expect to find in DS */
    Bit32u         exename_file_offset;  /* where __EXENAME__ lives in the EXE file (reference) */
    Bit32u         exename_ida_linear;   /* IDA linear of __EXENAME__ */
    Bit32u         sig_ida_linear;       /* IDA linear of Borland data-segment signature start */
    unsigned char* data;
    size_t         size;
    Bit16u         header_size;
    int            load_tried;
    /* Separately loaded segment + function tables for this binary. */
    struct og_seg_info  segs[OG_OVRTAB_MAX_SEGS];
    struct og_func_info funcs[OG_OVRTAB_MAX_FUNCS];
    unsigned       seg_count;
    unsigned       func_count;
    int            ovrtab_loaded;
    /* Runtime->IDA mapping, anchored on the Borland data-segment
     * signature (the intentional, stable "starting bytes" of the data
     * segment).  base_offset = runtime_sig_linear - sig_ida_linear.
     * NOTE: this mapping is correct for DATA segment reads.  Code
     * segments (resident, overlay buffers) may need their own
     * per-segment mapping; arithmetic CS:IP resolution against this
     * single base_offset may misattribute code locations. */
    int            active;
    Bit32s         base_offset;
    Bit16u         runtime_ds;       /* DS register value observed at detection */
};

static struct og_exe_info og_exes[OG_EXE_COUNT] = {
    /* MGC anchors:                                                       */
    /*   __EXENAME__               at IDA 3158:0248  (linear 0x317C8)     */
    /*   data-segment signature    at IDA 31D1:0000  (linear 0x31D10)     */
    { "/home/jbalcomb/STU_DBWD/MoM131/MAGIC.EXE",
      "/home/jbalcomb/STU/devel/STU-Extras/STU-DOSBox/MGC-overlay-table.fwv",
      "MAGIC.EXE", "magic.exe",   0x00023FC8, 0x000317C8, 0x00031D10,
      NULL, 0, 0, 0,
      {{0}}, {{0}}, 0, 0, 0,
      0, 0, 0 },
    /* WZD anchors:                                                       */
    /*   __EXENAME__               at IDA 3383:0580  (linear 0x33DB0)     */
    /*   data-segment signature    at IDA 36AA:0000  (linear 0x36AA0)     */
    { "/home/jbalcomb/STU_DBWD/MoM131/WIZARDS.EXE",
      "/home/jbalcomb/STU/devel/STU-Extras/STU-DOSBox/WZD-overlay-table.fwv",
      "WIZARDS.EXE", "wizards.exe", 0x000267B0, 0x00033DB0, 0x00036AA0,
      NULL, 0, 0, 0,
      {{0}}, {{0}}, 0, 0, 0,
      0, 0, 0 },
};

/*
 *  Borland C++ DGROUP signature -- a GENERIC, toolchain-emitted
 *  marker.  The Borland C++ linker places these 47 bytes at the head
 *  of DGROUP for every program it builds:
 *
 *    00 00 00 00 "Borland C++ - Copyright 1991 Borland Intl." 00
 *
 *  Finding these bytes at DS:0000 tells us "this is the data segment
 *  of SOME Borland C++ program" -- nothing more.  It does NOT
 *  identify which program.  In particular it does NOT mean the
 *  running EXE is MGC or WZD -- any Borland C++ DOS program built
 *  with this toolchain version would carry the same signature.
 *  Identifying MGC vs WZD (vs "some other Borland C++ EXE we have no
 *  overlay table for") is a separate step using __EXENAME__.
 */
static const unsigned char OG_DGROUP_SIG[] = {
    0x00,0x00,0x00,0x00,
    'B','o','r','l','a','n','d',' ','C','+','+',' ','-',' ',
    'C','o','p','y','r','i','g','h','t',' ','1','9','9','1',' ',
    'B','o','r','l','a','n','d',' ','I','n','t','l','.', 0x00
};
#define OG_DGROUP_SIG_LEN ((Bit16u)sizeof(OG_DGROUP_SIG))

static int og_active_exe_idx = -1;

/* Parse a fixed-width overlay table .fwv into the provided
 * exe_info's seg/func arrays.  Format documented at the top of the
 * existing og_ovrtab_load definition. */
static inline void og_exe_load_ovrtab(struct og_exe_info* e) {
    if (e->ovrtab_loaded) return;
    e->ovrtab_loaded = 1;
    FILE* fp = fopen(e->ovrtab_path, "r");
    if (!fp) {
        fprintf(stderr, "[OG_OVR] WARN cannot open %s\n", e->ovrtab_path);
        return;
    }
    char line[256];
    int lineno = 0;
    Bit16u last_seg = 0xFFFF;
    while (fgets(line, sizeof line, fp) != NULL) {
        lineno++;
        if (lineno <= 2) continue;
        if (strlen(line) < 35) continue;
        Bit16u seg_idx = (Bit16u)strtoul(line, NULL, 16);
        Bit32u seg_ea  = (Bit32u)strtoul(line + 5, NULL, 16);
        Bit32u func_ea = (Bit32u)strtoul(line + 25, NULL, 16);
        char sname[16];
        int  i;
        for (i = 0; i < 10 && line[14+i] && line[14+i] != ' ' && line[14+i] != '\n'; i++) {
            sname[i] = line[14+i];
        }
        sname[i] = '\0';
        char fname[64];
        int j;
        for (j = 0; j < 63 && line[34+j] && line[34+j] != '\n' && line[34+j] != '\r'; j++) {
            fname[j] = line[34+j];
        }
        while (j > 0 && (fname[j-1] == ' ' || fname[j-1] == '\t')) j--;
        fname[j] = '\0';
        if (seg_idx != last_seg && e->seg_count < OG_OVRTAB_MAX_SEGS) {
            e->segs[e->seg_count].seg_idx = seg_idx;
            e->segs[e->seg_count].seg_ea = seg_ea;
            strncpy(e->segs[e->seg_count].seg_name, sname, 15);
            e->segs[e->seg_count].seg_name[15] = '\0';
            e->seg_count++;
            last_seg = seg_idx;
        }
        if (e->func_count < OG_OVRTAB_MAX_FUNCS) {
            e->funcs[e->func_count].seg_idx = seg_idx;
            e->funcs[e->func_count].ip_offset = (Bit16u)(func_ea - seg_ea);
            strncpy(e->funcs[e->func_count].func_name, fname, 63);
            e->funcs[e->func_count].func_name[63] = '\0';
            e->func_count++;
        }
    }
    fclose(fp);

    /* Derive each segment's file_offset and size now that the table is
     * fully loaded and the MZ header_size is known.  These are what
     * the overlay-buffer byte-pattern resolver (R6) compares against. */
    unsigned a;
    for (a = 0; a < e->seg_count; a++) {
        e->segs[a].file_offset =
            e->segs[a].seg_ea - 0x10000u + (Bit32u)e->header_size;
        Bit32u next_ea = (Bit32u)-1;
        unsigned b;
        for (b = 0; b < e->seg_count; b++) {
            if (e->segs[b].seg_ea > e->segs[a].seg_ea
                && e->segs[b].seg_ea < next_ea) {
                next_ea = e->segs[b].seg_ea;
            }
        }
        if (next_ea == (Bit32u)-1) {
            e->segs[a].size = (Bit32u)e->size - e->segs[a].file_offset;
        } else {
            e->segs[a].size = next_ea - e->segs[a].seg_ea;
        }
    }

    fprintf(stderr, "[OG_OVR] loaded %u segs, %u funcs from %s\n",
            e->seg_count, e->func_count, e->ovrtab_path);
}

static inline void og_exe_load(struct og_exe_info* e) {
    if (e->load_tried) return;
    e->load_tried = 1;
    FILE* f = fopen(e->path, "rb");
    if (!f) {
        fprintf(stderr, "[OG_OVR] WARN cannot open %s\n", e->path);
        return;
    }
    fseek(f, 0, SEEK_END);
    e->size = (size_t)ftell(f);
    rewind(f);
    e->data = (unsigned char*)malloc(e->size);
    if (e->data) {
        if (fread(e->data, 1, e->size, f) != e->size) {
            free(e->data);
            e->data = NULL;
        }
    }
    fclose(f);
    if (e->data && e->size > 0x20 && e->data[0] == 'M' && e->data[1] == 'Z') {
        Bit16u cparhdr = e->data[0x08] | (e->data[0x09] << 8);
        e->header_size = (Bit16u)(cparhdr * 16);
    }
    fprintf(stderr, "[OG_OVR] loaded %s (%zu bytes), MZ header=0x%X\n",
            e->label, e->size, e->header_size);
    og_exe_load_ovrtab(e);
}

/*
 *  Generic Borland C++ DGROUP detector.
 *
 *  Scans a tiny window at the start of DS for OG_DGROUP_SIG.  Returns
 *  the offset of the signature in DS, or -1 if not found.  Knows
 *  nothing about MGC, WZD, or any specific program -- this is a
 *  toolchain-level check ("is DS the data segment of some Borland C++
 *  program?").  The signature's IDA linear address is per-EXE and is
 *  consulted only after a specific EXE has been identified by
 *  __EXENAME__ match below.
 */
static inline int og_borland_dgroup_offset(Bit16u ds_val) {
    Bit16u o;
    for (o = 0; o <= 16; o++) {
        if ((Bit32u)o + OG_DGROUP_SIG_LEN > 0x10000u) break;
        Bit16u i;
        int match = 1;
        for (i = 0; i < OG_DGROUP_SIG_LEN; i++) {
            if (real_readb(ds_val, (Bit16u)(o + i)) != OG_DGROUP_SIG[i]) {
                match = 0;
                break;
            }
        }
        if (match) return (int)o;
    }
    return -1;
}

/*
 *  Identify whether the running program is one of the project EXEs we
 *  have IDA overlay tables for (MGC or WZD), and -- if so -- bind the
 *  runtime->IDA base_offset.
 *
 *  Two-layer logic:
 *
 *    Layer A (generic Borland toolchain anchor) -- og_borland_dgroup_offset()
 *        confirms DS points at SOME Borland C++ program's DGROUP.  This
 *        gates resolution: without a Borland DGROUP, we have neither
 *        runtime DGROUP location nor any way to bind base_offset.
 *        Matching here says nothing about which program is running.
 *
 *    Layer B (project-specific identity) -- scan DS for either
 *        "magic.exe\0" or "wizards.exe\0".  Each project EXE has its
 *        own __EXENAME__; any DS occurrence is sufficient because the
 *        literal is unique to its EXE.  If neither matches, the
 *        running program is Borland C++ but NOT one of ours, and we
 *        refuse to resolve (we have no overlay table for it).
 *
 *  base_offset is computed from the signature's runtime location (from
 *  layer A) and the identified EXE's sig_ida_linear (from layer B).
 *  This base_offset is correct for DATA segment reads and resident-
 *  image code.  CS:IP in overlay buffers (loaded at separate runtime
 *  paragraphs) may need per-overlay mapping; see PRD R6.
 */
static inline void og_detect_active_exe(Bit16u ds_val) {
    /* Already bound, and DS still points at that EXE's DGROUP --
     * cheap path, nothing to do. */
    if (og_active_exe_idx >= 0
        && og_exes[og_active_exe_idx].runtime_ds == ds_val) {
        return;
    }

    /* Layer A -- generic Borland DGROUP gate.  If DS isn't a Borland
     * DGROUP right now (the program is briefly addressing non-DGROUP
     * memory, or no DGROUP has appeared yet), leave the current
     * binding alone.  This is the common case after the first bind. */
    int sig_off = og_borland_dgroup_offset(ds_val);
    if (sig_off < 0) return;

    /* Layer B -- is this Borland C++ program one of our project EXEs?
     *
     * Use IDA's known address of __EXENAME__ -- no blind scanning.
     * Per candidate EXE:
     *   1. Hypothesize: base_offset = runtime_sig_linear - sig_ida_linear
     *   2. Compute runtime linear of __EXENAME__:
     *        exename_runtime_linear = exename_ida_linear + base_offset
     *   3. Convert to a real-mode (seg, offset) addressing that
     *      reaches it: seg = linear>>4, off = linear & 0xF.
     *   4. Read the bytes there.  If they EXACTLY equal the EXE's
     *      lowercase exename literal followed by NUL, this is the
     *      active EXE.
     *
     * This is precision over coverage: we don't accept any DS
     * occurrence of the literal (MAGIC.EXE's data contains the
     * uppercase string "WIZARDS.EXE" as a sibling reference, which
     * would false-positive a tolerant scan).  We only accept the
     * literal at __EXENAME__'s known IDA address.
     */
    Bit32u runtime_sig_linear = ((Bit32u)ds_val << 4) + (Bit32u)sig_off;
    int e_i;
    for (e_i = 0; e_i < OG_EXE_COUNT; e_i++) {
        struct og_exe_info* e = &og_exes[e_i];
        og_exe_load(e);     /* lazy load of the EXE bytes + table */
        if (!e->data) continue;

        Bit32s hyp_base       = (Bit32s)((Bit32s)runtime_sig_linear
                                         - (Bit32s)e->sig_ida_linear);
        Bit32s exename_lin_s  = (Bit32s)e->exename_ida_linear + hyp_base;
        if (exename_lin_s < 0 || exename_lin_s > 0x10FFEFL) continue;
        Bit32u exename_lin    = (Bit32u)exename_lin_s;
        Bit16u exename_seg    = (Bit16u)(exename_lin >> 4);
        Bit16u exename_off_lo = (Bit16u)(exename_lin & 0xFu);

        size_t nlen = strlen(e->exename);
        int match = 1;
        size_t j;
        for (j = 0; j < nlen; j++) {
            Bit8u b = real_readb(exename_seg, (Bit16u)(exename_off_lo + j));
            if (b != (Bit8u)e->exename[j]) { match = 0; break; }
        }
        if (!match) continue;
        Bit8u term = real_readb(exename_seg, (Bit16u)(exename_off_lo + nlen));
        if (term != 0) continue;

        /* Match.  Decide whether this is a first-bind, a rebind to a
         * DIFFERENT project EXE (e.g. MAGIC.EXE chained to WIZARDS.EXE
         * mid-session), or just the SAME EXE seen at a different DS
         * (e.g. real-mode aliasing of the same DGROUP via a different
         * paragraph). */
        int prev_idx        = og_active_exe_idx;
        int rebind_diff_exe = (prev_idx >= 0 && prev_idx != e_i);
        int silent_resync   = (prev_idx == e_i);  /* same EXE, DS just shifted */

        Bit32u runtime_sig_linear = ((Bit32u)ds_val << 4) + (Bit32u)sig_off;
        e->base_offset = (Bit32s)((Bit32s)runtime_sig_linear
                                  - (Bit32s)e->sig_ida_linear);
        e->active            = 1;
        e->runtime_ds        = ds_val;
        og_active_exe_idx    = e_i;

        if (silent_resync) {
            /* Same EXE, different DS paragraph -- no log spam. */
            return;
        }
        if (rebind_diff_exe) {
            fprintf(stderr,
                    "[OG_OVR] active EXE CHANGED: %s -> %s (chained process)\n",
                    og_exes[prev_idx].label, e->label);
            /* Mark the previous EXE inactive so probes keyed on its
             * `active` flag (e.g. og_stage2_unit_write_probe checking
             * og_exes[1].active for WZD) flip correctly. */
            og_exes[prev_idx].active = 0;
        }
        fprintf(stderr,
                "[OG_OVR] Borland DGROUP detected: signature at DS:%04X\n",
                (Bit16u)sig_off);
        fprintf(stderr,
                "[OG_OVR] active EXE = %s  __EXENAME__(\"%s\") at runtime %04X:%04X\n",
                e->label, e->exename, exename_seg, exename_off_lo);
        fprintf(stderr,
                "[OG_OVR] base_offset = %08X - %08X = %+ld\n",
                runtime_sig_linear, e->sig_ida_linear,
                (long)e->base_offset);
        return;
    }

    /* Layer A succeeded but layer B did not: this is a Borland C++
     * program, but not MGC or WZD.  No overlay table -> no resolve.
     * Don't disturb any prior binding (e.g. the program is briefly
     * addressing a non-project Borland DGROUP -- unlikely, but safe). */
    if (og_active_exe_idx < 0) {
        fprintf(stderr,
                "[OG_OVR] Borland DGROUP at DS:%04X, but program is neither MGC nor WZD -- not resolving\n",
                (Bit16u)sig_off);
    }
}

/*
 *  Resolve a runtime CS:IP to (seg_name, func_name, offset) using the
 *  currently-identified active EXE's overlay table + the base_offset
 *  determined when __EXENAME__ was located.  No byte search at all --
 *  pure arithmetic.
 */
static inline void og_runtime_to_name(Bit16u cs_val, Bit16u ip_val) {
    if (og_active_exe_idx < 0) {
        fprintf(stderr,
                "[OG_OVR]   resolved: active EXE not yet identified\n");
        return;
    }
    struct og_exe_info* e = &og_exes[og_active_exe_idx];
    Bit32u runtime_linear = ((Bit32u)cs_val << 4) + ip_val;
    Bit32s ida_linear_s   = (Bit32s)runtime_linear - e->base_offset;
    if (ida_linear_s < 0) {
        fprintf(stderr,
                "[OG_OVR]   resolved: cs:ip out of EXE address space (linear=%X, base=%+ld)\n",
                runtime_linear, (long)e->base_offset);
        return;
    }
    Bit32u ida_linear = (Bit32u)ida_linear_s;

    /* Largest seg whose SEG_EA <= ida_linear. */
    unsigned best_seg = (unsigned)-1;
    Bit32u   best_ea  = 0;
    unsigned k;
    for (k = 0; k < e->seg_count; k++) {
        if (e->segs[k].seg_ea <= ida_linear && e->segs[k].seg_ea >= best_ea) {
            best_ea  = e->segs[k].seg_ea;
            best_seg = k;
        }
    }
    if (best_seg == (unsigned)-1) {
        fprintf(stderr,
                "[OG_OVR]   resolved: IDA linear 0x%X not within any %s segment\n",
                ida_linear, e->label);
        return;
    }
    Bit16u seg_idx = e->segs[best_seg].seg_idx;
    Bit16u ip_off  = (Bit16u)(ida_linear - best_ea);

    /* Largest func whose ip_offset <= ip_off, within this seg_idx. */
    unsigned best_func = (unsigned)-1;
    Bit16u   best_func_off = 0;
    for (k = 0; k < e->func_count; k++) {
        if (e->funcs[k].seg_idx != seg_idx) continue;
        if (e->funcs[k].ip_offset > ip_off) continue;
        if (e->funcs[k].ip_offset >= best_func_off) {
            best_func_off = e->funcs[k].ip_offset;
            best_func     = k;
        }
    }
    if (best_func == (unsigned)-1) {
        fprintf(stderr,
                "[OG_OVR]   resolved: %s  IDA %s:%04X  (no labeled function)\n",
                e->label, e->segs[best_seg].seg_name, ip_off);
    } else {
        fprintf(stderr,
                "[OG_OVR]   resolved: %s  IDA %s:%04X = %s+%X\n",
                e->label,
                e->segs[best_seg].seg_name, ip_off,
                e->funcs[best_func].func_name, ip_off - best_func_off);
    }
}

/*
 *  Per-overlay runtime-CS resolver.
 *
 *  When the resident-image base_offset lands a CS:IP inside an `ovrXXX`
 *  segment, the resolver's seg_idx is suspect: overlays don't actually
 *  live at their IDA SEG_EA at runtime -- the overlay manager loads
 *  them into independent buffers.  To identify which overlay is
 *  currently loaded at a given runtime CS, we byte-match a window
 *  starting at runtime CS:IP against each overlay segment's static
 *  bytes at file_offset+ip_val.  The unique match (if any) is the
 *  loaded overlay's seg_idx.
 *
 *  A small (cs -> seg_idx) cache amortizes the search.  Each cache
 *  entry is verified on lookup by checking that 4 bytes at CS:IP still
 *  match the recorded overlay's static bytes -- this catches "buffer
 *  has been reloaded with a different overlay since we cached" without
 *  paying for the full search every time.
 */
/* ---------------------------------------------------------------------
 *  R6 (rev 2) -- overlay-buffer CS resolver.
 *
 *  Problem: at runtime, code in a Borland overlay lives in a buffer at
 *  an independent paragraph (call it `runtime_cs`).  The overlay's
 *  static bytes in WIZARDS.EXE are at some `seg_ea` (per the IDA .fwv
 *  table), prefixed by a variable-size HEADER (shared epilogue +
 *  function-offset table).  At load time the overlay manager ALSO
 *  applies fixups: any `9A xx xx ss ss` (far call) or `B8 ss ss`
 *  (mov ax, segment) has its segment bytes rewritten.  So:
 *
 *      static_offset_in_overlay = runtime_ip + header_offset
 *      runtime_bytes != static_bytes at relocation slots
 *
 *  Algorithm: when we first encounter a runtime `cs`, snapshot 32
 *  bytes from runtime `cs:ip` and find them in the static EXE.
 *  Search must tolerate relocation mismatches (use scored count, not
 *  strict memcmp).  Once we know `(seg_idx, header_offset)` for a
 *  runtime cs, cache it -- every later lookup with the same cs is
 *  pure arithmetic until the overlay buffer gets reloaded with a
 *  different overlay (verified each cache hit by re-reading 8 bytes
 *  at the overlay's static byte 0 via runtime cs:header_offset).
 *
 *  This resolver is general-purpose -- it backs every code-location
 *  description in the trace, not just `_unit` writes.
 * --------------------------------------------------------------------- */

#define OG_OVR_MATCH_WINDOW    32         /* snapshot byte count */
#define OG_OVR_MATCH_MIN_HITS  26         /* need >= this many matching bytes */
#define OG_OVR_MATCH_MARGIN     4         /* best must beat runner-up by this */
#define OG_OVR_CS_CACHE_SIZE   64

struct og_overlay_cs_cache {
    Bit16u   runtime_cs;        /* 0 = empty slot */
    Bit16u   seg_idx;
    unsigned seg_table_idx;
    Bit16u   header_offset;     /* static_offset = ip + header_offset */
    Bit8u    verify_bytes[8];   /* runtime cs:header_offset bytes at learn time */
};
static struct og_overlay_cs_cache og_ovr_cs_cache[OG_OVR_CS_CACHE_SIZE];
static unsigned og_ovr_cs_cache_count = 0;

/*
 *  Count matching bytes between two 32-byte windows.
 *  Cheap; no early-exit because we need the full count for scoring.
 */
static inline int og_window_match_count(const Bit8u* a, const Bit8u* b) {
    int n = 0;
    int i;
    for (i = 0; i < OG_OVR_MATCH_WINDOW; i++) if (a[i] == b[i]) n++;
    return n;
}

/*
 *  Returns 1 if (cs, ip) resolves to an overlay buffer.  On success
 *  fills *out_seg_table_idx and *out_static_offset (the static byte
 *  offset within that overlay's static bytes).  Returns 0 on miss
 *  (no overlay matches the runtime bytes well enough or no clear
 *  best).
 *
 *  Workflow:
 *    1. If cached (cs -> seg, header_offset): verify cache by reading
 *       8 bytes at cs:header_offset and comparing against saved
 *       overlay-byte-0..7 from learn time.  Match -> direct return.
 *       Mismatch -> evict cache entry, fall through to learn.
 *    2. Learn: snapshot 32 bytes at runtime cs:ip.  For each overlay,
 *       slide through every starting offset, scoring matches.  Track
 *       best score across all overlays and second-best globally.
 *       Accept best if score >= MIN_HITS AND best - runner_up >= MARGIN.
 *    3. On learn, cache (cs, seg, header_offset, 8 verify bytes) and
 *       emit [OG_R6_LEARN].  On no-match emit [OG_R6_NOMATCH] with
 *       the 32 runtime bytes (one-shot per cs to avoid log spam).
 */
static inline int og_overlay_seg_for_cs(struct og_exe_info* e,
                                        Bit16u cs_val, Bit16u ip_val,
                                        unsigned* out_seg_table_idx,
                                        Bit16u* out_static_offset) {
    unsigned c;
    int i;

    /* Step 1: cache lookup. */
    for (c = 0; c < og_ovr_cs_cache_count; c++) {
        if (og_ovr_cs_cache[c].runtime_cs != cs_val) continue;
        /* Verify: read 8 bytes at runtime cs:header_offset (= overlay
         * byte 0 in static) and compare against the cached snapshot. */
        Bit16u hdr_off = og_ovr_cs_cache[c].header_offset;
        int ok = 1;
        for (i = 0; i < 8; i++) {
            if (real_readb(cs_val, (Bit16u)(hdr_off + i))
                != og_ovr_cs_cache[c].verify_bytes[i]) {
                ok = 0;
                break;
            }
        }
        if (ok) {
            *out_seg_table_idx = og_ovr_cs_cache[c].seg_table_idx;
            *out_static_offset = (Bit16u)(ip_val + hdr_off);
            return 1;
        }
        /* Stale -- buffer reloaded with different overlay. */
        og_ovr_cs_cache[c].runtime_cs = 0;
    }

    /* Step 2: learn.  Snapshot the runtime window. */
    Bit8u win[OG_OVR_MATCH_WINDOW];
    for (i = 0; i < OG_OVR_MATCH_WINDOW; i++) {
        win[i] = real_readb(cs_val, (Bit16u)(ip_val + i));
    }

    /* Score every position across all ovrXXX segments; track best
     * and second-best WITH positions (so the log tells us where the
     * runner-up actually lives -- crucial for diagnosing ambiguity). */
    int best_hits      = -1;
    unsigned best_seg  = (unsigned)-1;
    Bit32u   best_off  = 0;
    int second_hits    = -1;
    unsigned second_seg = (unsigned)-1;
    Bit32u   second_off = 0;
    unsigned k;
    for (k = 0; k < e->seg_count; k++) {
        if (strncmp(e->segs[k].seg_name, "ovr", 3) != 0) continue;
        Bit32u seg_start = e->segs[k].file_offset;
        if (seg_start >= (Bit32u)e->size) continue;
        Bit32u seg_end = seg_start + e->segs[k].size;
        if (seg_end > (Bit32u)e->size) seg_end = (Bit32u)e->size;
        if (seg_end < (Bit32u)OG_OVR_MATCH_WINDOW) continue;
        Bit32u limit = seg_end - OG_OVR_MATCH_WINDOW;
        Bit32u fo;
        for (fo = seg_start; fo <= limit; fo++) {
            int hits = og_window_match_count(win, e->data + fo);
            if (hits > best_hits) {
                second_hits = best_hits;
                second_seg  = best_seg;
                second_off  = best_off;
                best_hits = hits;
                best_seg  = k;
                best_off  = fo - seg_start;
            } else if (hits > second_hits) {
                second_hits = hits;
                second_seg  = k;
                second_off  = fo - seg_start;
            }
        }
    }
    const char* second_seg_name = (second_seg != (unsigned)-1)
        ? e->segs[second_seg].seg_name : "?";

    if (best_hits < OG_OVR_MATCH_MIN_HITS
        || (best_hits - second_hits) < OG_OVR_MATCH_MARGIN) {
        /* No clear winner -- log diagnostic for pattern analysis.
         * Per-cs so multiple ambiguous cs values all get recorded. */
        fprintf(stderr,
                "[OG_R6_NOMATCH] cs=%04X ip=%04X  best=%d/32 (%s@%X)"
                "  runner=%d/32 (%s@%X)  margin=%d\n",
                cs_val, ip_val, best_hits,
                (best_seg != (unsigned)-1)
                    ? e->segs[best_seg].seg_name : "?",
                (best_seg != (unsigned)-1) ? best_off : 0,
                second_hits, second_seg_name, second_off,
                best_hits - second_hits);
        fprintf(stderr, "[OG_R6_NOMATCH]   runtime bytes:");
        for (i = 0; i < OG_OVR_MATCH_WINDOW; i++) {
            fprintf(stderr, " %02X", win[i]);
        }
        fprintf(stderr, "\n");
        return 0;
    }

    /* Compute header_offset for this (cs, overlay) pairing:
     *   static_offset = ip + header_offset  ->  header_offset = static_offset - ip
     */
    Bit16u header_offset = (Bit16u)((Bit32s)best_off - (Bit32s)ip_val);

    /* Cache for next time.  Save 8 bytes at cs:header_offset for the
     * cheap verify on subsequent lookups. */
    unsigned slot;
    if (og_ovr_cs_cache_count < OG_OVR_CS_CACHE_SIZE) {
        slot = og_ovr_cs_cache_count++;
    } else {
        static unsigned next_evict = 0;
        slot = next_evict;
        next_evict = (next_evict + 1) % OG_OVR_CS_CACHE_SIZE;
    }
    og_ovr_cs_cache[slot].runtime_cs    = cs_val;
    og_ovr_cs_cache[slot].seg_idx       = e->segs[best_seg].seg_idx;
    og_ovr_cs_cache[slot].seg_table_idx = best_seg;
    og_ovr_cs_cache[slot].header_offset = header_offset;
    for (i = 0; i < 8; i++) {
        og_ovr_cs_cache[slot].verify_bytes[i] =
            real_readb(cs_val, (Bit16u)(header_offset + i));
    }

    fprintf(stderr,
            "[OG_R6_LEARN] cs=%04X -> %s  header_offset=%04X"
            "  (cs:ip=%04X:%04X at %s@%04X  hits=%d/32  runner=%d/32 %s@%04X)\n",
            cs_val, e->segs[best_seg].seg_name, header_offset,
            cs_val, ip_val,
            e->segs[best_seg].seg_name, (Bit16u)best_off,
            best_hits,
            second_hits, second_seg_name, second_off);
    /* Dump both the runtime window AND the static bytes at the chosen
     * match position so the trace can be verified by hand against the
     * EXE.  Mismatched byte positions are wrapped in parentheses. */
    Bit32u match_fo = e->segs[best_seg].file_offset + best_off;
    fprintf(stderr, "[OG_R6_LEARN]   runtime cs:ip:");
    for (i = 0; i < OG_OVR_MATCH_WINDOW; i++) {
        fprintf(stderr, " %02X", win[i]);
    }
    fprintf(stderr, "\n[OG_R6_LEARN]   static  %s@%04X (file 0x%X):",
            e->segs[best_seg].seg_name, (Bit16u)best_off, match_fo);
    for (i = 0; i < OG_OVR_MATCH_WINDOW; i++) {
        Bit8u sb = e->data[match_fo + i];
        if (sb == win[i]) fprintf(stderr, " %02X", sb);
        else              fprintf(stderr, "(%02X)", sb);
    }
    fprintf(stderr, "\n");
    if (second_seg != (unsigned)-1) {
        Bit32u runner_fo = e->segs[second_seg].file_offset + second_off;
        fprintf(stderr, "[OG_R6_LEARN]   runner  %s@%04X (file 0x%X):",
                second_seg_name, (Bit16u)second_off, runner_fo);
        for (i = 0; i < OG_OVR_MATCH_WINDOW; i++) {
            Bit8u sb = e->data[runner_fo + i];
            if (sb == win[i]) fprintf(stderr, " %02X", sb);
            else              fprintf(stderr, "(%02X)", sb);
        }
        fprintf(stderr, "\n");
    }

    *out_seg_table_idx = best_seg;
    *out_static_offset = (Bit16u)best_off;
    return 1;
}

/* ----------------------------------------------------------------------
 *  Authoritative overlay-buffer resolver via back-patched stub thunks.
 *
 *  Confirmed empirically (R3-G6 follow probe, fire #2 transition):
 *  Borland C++ 3.0's overlay manager back-patches each `j_Foo` thunk
 *  in its stubXXX segment from "CD 3F XX XX 00" (INT 3F call) to
 *  "EA OO OO SS SS" (JMP FAR ss:oo) after the first dispatch.  The
 *  patched bytes directly encode "Foo currently lives in overlay
 *  buffer SS at offset OO".  No byte-pattern guessing needed.
 *
 *  Strategy:
 *    1. After each INT 3F fire, walk every j_-prefix (or NX_j_-prefix)
 *       function in the IDA .fwv, compute its resident runtime
 *       address via the resident base_offset, read 5 bytes, and --
 *       if the first byte is 0xEA -- record
 *       (target_seg=SS, target_off=OO, funcs_idx) in the
 *       resolved-thunks map.
 *    2. To resolve a runtime CS:IP that lands in an overlay buffer,
 *       find the resolved thunk whose target_seg==CS and largest
 *       target_off <= IP.  Format as ovrXXX:Foo+(IP-target_off),
 *       deriving ovrXXX from the thunk's stubXXX name.
 * --------------------------------------------------------------------- */

#define OG_MAX_RESOLVED_THUNKS 1024

struct og_resolved_thunk {
    Bit16u   target_seg;       /* SS in "EA OO OO SS SS" -- runtime CS of overlay buffer */
    Bit16u   target_off;       /* OO in "EA OO OO SS SS" -- function offset within buffer */
    unsigned funcs_idx;        /* index into og_exes[active].funcs[] of the j_* thunk */
};
static struct og_resolved_thunk og_resolved_thunks[OG_MAX_RESOLVED_THUNKS];
static unsigned og_resolved_thunk_count = 0;

static inline const struct og_seg_info* og_find_seg_by_idx(
        const struct og_exe_info* e, Bit16u seg_idx)
{
    unsigned i;
    for (i = 0; i < e->seg_count; i++) {
        if (e->segs[i].seg_idx == seg_idx) return &e->segs[i];
    }
    return NULL;
}

static inline void og_rebuild_resolved_thunks(struct og_exe_info* e) {
    og_resolved_thunk_count = 0;
    if (!e->active) return;
    unsigned i;
    for (i = 0; i < e->func_count
              && og_resolved_thunk_count < OG_MAX_RESOLVED_THUNKS; i++) {
        struct og_func_info* f = &e->funcs[i];
        const char* nm = f->func_name;
        /* Only j_* and NX_j_* thunks are dispatchers we can detect. */
        if (strncmp(nm, "j_",    2) != 0
            && strncmp(nm, "NX_j_", 5) != 0) continue;
        const struct og_seg_info* sg = og_find_seg_by_idx(e, f->seg_idx);
        if (!sg) continue;
        /* Only consider thunks in stubXXX segments. */
        if (strncmp(sg->seg_name, "stub", 4) != 0) continue;

        /* Compute the thunk's runtime location via the resident base_offset. */
        Bit32u thunk_ida_linear = sg->seg_ea + (Bit32u)f->ip_offset;
        Bit32s rt_linear_s = (Bit32s)thunk_ida_linear + e->base_offset;
        if (rt_linear_s < 0) continue;
        Bit32u rt_linear = (Bit32u)rt_linear_s;
        Bit16u rt_seg = (Bit16u)(rt_linear >> 4);
        Bit16u rt_off = (Bit16u)(rt_linear & 0xF);

        /* Read 5 bytes; if first is 0xEA, it's a JMP FAR -- patched. */
        Bit8u b0 = real_readb(rt_seg, rt_off);
        if (b0 != 0xEA) continue;
        Bit16u t_off = (Bit16u)(real_readb(rt_seg, (Bit16u)(rt_off + 1))
                              | (real_readb(rt_seg, (Bit16u)(rt_off + 2)) << 8));
        Bit16u t_seg = (Bit16u)(real_readb(rt_seg, (Bit16u)(rt_off + 3))
                              | (real_readb(rt_seg, (Bit16u)(rt_off + 4)) << 8));

        og_resolved_thunks[og_resolved_thunk_count].target_seg = t_seg;
        og_resolved_thunks[og_resolved_thunk_count].target_off = t_off;
        og_resolved_thunks[og_resolved_thunk_count].funcs_idx  = i;
        og_resolved_thunk_count++;
    }
}

/*
 *  Identify which ovrXXX is loaded at runtime CS=cs_val.
 *
 *  Once ANY thunk for that overlay has been patched, we have proof of
 *  where the overlay lives at runtime.  Walk the resolved-thunks map
 *  looking for any thunk whose target_seg == cs_val; that thunk's
 *  stubXXX (by name) tells us the corresponding ovrXXX.
 *
 *  Returns 1 + sets *out_ovr_seg_idx on success, 0 on miss.
 */
static inline int og_overlay_at_runtime_cs(
        struct og_exe_info* e, Bit16u cs_val,
        unsigned* out_ovr_seg_idx)
{
    unsigned i, j;
    for (i = 0; i < og_resolved_thunk_count; i++) {
        if (og_resolved_thunks[i].target_seg != cs_val) continue;
        const struct og_func_info* tf = &e->funcs[og_resolved_thunks[i].funcs_idx];
        const struct og_seg_info*  stub_sg = og_find_seg_by_idx(e, tf->seg_idx);
        if (!stub_sg) continue;
        if (strncmp(stub_sg->seg_name, "stub", 4) != 0) continue;
        char ovr_name[16];
        snprintf(ovr_name, sizeof ovr_name, "ovr%s", stub_sg->seg_name + 4);
        for (j = 0; j < e->seg_count; j++) {
            if (strcmp(e->segs[j].seg_name, ovr_name) == 0) {
                *out_ovr_seg_idx = e->segs[j].seg_idx;
                return 1;
            }
        }
    }
    return 0;
}

/*
 *  og_describe -- buffer-returning sibling of og_runtime_to_name, for
 *  inline use in other probes' fprintf args.  Same arithmetic; no
 *  stderr output of its own.
 */
static inline const char* og_describe(Bit16u cs_val, Bit16u ip_val,
                                      char* buf, size_t buflen) {
    if (og_active_exe_idx < 0) {
        snprintf(buf, buflen, "?");
        return buf;
    }
    struct og_exe_info* e = &og_exes[og_active_exe_idx];
    Bit32u runtime_linear = ((Bit32u)cs_val << 4) + ip_val;
    Bit32s ida_linear_s   = (Bit32s)runtime_linear - e->base_offset;

    /* Single-base resolution: find seg whose seg_ea <= ida_linear. */
    unsigned best_seg = (unsigned)-1;
    Bit16u   ip_off   = 0;
    unsigned k;
    if (ida_linear_s >= 0) {
        Bit32u ida_linear = (Bit32u)ida_linear_s;
        Bit32u best_ea = 0;
        for (k = 0; k < e->seg_count; k++) {
            if (e->segs[k].seg_ea <= ida_linear && e->segs[k].seg_ea >= best_ea) {
                best_ea  = e->segs[k].seg_ea;
                best_seg = k;
            }
        }
        if (best_seg != (unsigned)-1) {
            ip_off = (Bit16u)(ida_linear - best_ea);
        }
    }

    /* PRD R3-G6: if the single-base result is an overlay segment,
     * the single base_offset is wrong (overlays don't live at their
     * IDA SEG_EA at runtime).  Authoritative answer comes from the
     * back-patched stub thunks (the Borland manager rewrites each
     * thunk to "EA OO OO SS SS" = JMP FAR ss:oo after first dispatch).
     * Find the patched thunk whose target matches (cs_val, ip_val);
     * format with ovrXXX derived from the thunk's stubXXX name. */
    if (best_seg != (unsigned)-1
        && strncmp(e->segs[best_seg].seg_name, "ovr", 3) == 0) {
        /* Rebuild the thunk map lazily on each overlay-buffer lookup. */
        og_rebuild_resolved_thunks(e);

        /* Use the patched thunks to identify which ovrXXX is loaded
         * at this runtime CS.  Once we know the overlay, look up the
         * function in its FULL func list (the .fwv may have many more
         * functions labeled in the overlay than have j_* thunks --
         * internal functions called only via near calls from within
         * the same overlay don't need a stub). */
        unsigned ovr_seg_idx;
        if (og_overlay_at_runtime_cs(e, cs_val, &ovr_seg_idx)) {
            /* Find the func in ovrXXX whose ip_offset is largest <= ip_val.
             * Empirically the .fwv's static offsets correspond to runtime
             * IPs directly for the user's expected output. */
            unsigned best_func = (unsigned)-1;
            Bit16u   best_func_off = 0;
            unsigned k2;
            unsigned ovr_seg_table_idx = (unsigned)-1;
            for (k2 = 0; k2 < e->seg_count; k2++) {
                if (e->segs[k2].seg_idx == ovr_seg_idx) {
                    ovr_seg_table_idx = k2;
                    break;
                }
            }
            for (k2 = 0; k2 < e->func_count; k2++) {
                if (e->funcs[k2].seg_idx != ovr_seg_idx) continue;
                if (e->funcs[k2].ip_offset > ip_val) continue;
                if (best_func == (unsigned)-1
                    || e->funcs[k2].ip_offset >= best_func_off) {
                    best_func_off = e->funcs[k2].ip_offset;
                    best_func     = k2;
                }
            }
            const char* ovr_name = (ovr_seg_table_idx != (unsigned)-1)
                ? e->segs[ovr_seg_table_idx].seg_name : "ovr?";
            if (best_func == (unsigned)-1) {
                snprintf(buf, buflen, "%s:%04X", ovr_name, (unsigned)ip_val);
            } else {
                snprintf(buf, buflen, "%s:%s+%X",
                         ovr_name,
                         e->funcs[best_func].func_name,
                         (unsigned)(ip_val - best_func_off));
            }
            return buf;
        }
        /* Couldn't identify which overlay is loaded at this runtime CS
         * (no thunk in the resolved map points to it).  Fall through
         * to single-base, which will report a misleading ovrXXX name
         * but at least won't pretend we know the function. */
    }

    if (best_seg == (unsigned)-1) {
        snprintf(buf, buflen, "?");
        return buf;
    }
    Bit16u seg_idx = e->segs[best_seg].seg_idx;

    unsigned best_func = (unsigned)-1;
    Bit16u   best_func_off = 0;
    for (k = 0; k < e->func_count; k++) {
        if (e->funcs[k].seg_idx != seg_idx) continue;
        if (e->funcs[k].ip_offset > ip_off) continue;
        if (e->funcs[k].ip_offset >= best_func_off) {
            best_func_off = e->funcs[k].ip_offset;
            best_func     = k;
        }
    }
    if (best_func == (unsigned)-1) {
        snprintf(buf, buflen, "%s:%04X",
                 e->segs[best_seg].seg_name, ip_off);
    } else {
        snprintf(buf, buflen, "%s:%s+%X",
                 e->segs[best_seg].seg_name,
                 e->funcs[best_func].func_name,
                 (unsigned)(ip_off - best_func_off));
    }
    return buf;
}


/* ----------------------------------------------------------------------
 *  Knowledge base of identified INT 3F MGMT call sites.
 *
 *  Iteratively built from trace runs: when the trace emits
 *  `MGMT UNKNOWN caller=XXXX:YYYY ...`, identify what that call site
 *  is doing in IDA, then add a row here so future runs label it.
 *
 *  Match semantics: 0 in caller_cs/caller_ip is a wildcard; 0xFFFF
 *  in ax_value is a wildcard.  First-match-wins; put more specific
 *  rows before broader ones.
 * -------------------------------------------------------------------- */
struct og_int3f_mgmt_kb_entry {
    Bit16u       caller_cs;     /* 0 = wildcard */
    Bit16u       caller_ip;     /* 0 = wildcard */
    Bit16u       ax_value;      /* 0xFFFF = wildcard */
    const char*  label;         /* short label for the trace line */
    const char*  note;          /* longer prose for humans reading the .h */
};

static const struct og_int3f_mgmt_kb_entry OG_INT3F_MGMT_KB[] = {
    /* { caller_cs, caller_ip, ax_value,  label,   note }, */

    /* ---- add identified entries here as we discover them ----
     * Example shape:
     * { 0x2363, 0x0020, 0x17D4, "INIT_TABLE",
     *   "Borland overlay manager init: passes seg of table in AX." },
     */

    { 0, 0, 0xFFFF, NULL, NULL }   /* terminator */
};

static inline const char* og_int3f_mgmt_lookup(Bit16u caller_cs, Bit16u caller_ip, Bit16u ax) {
    unsigned i;
    for (i = 0; OG_INT3F_MGMT_KB[i].label != NULL; i++) {
        const struct og_int3f_mgmt_kb_entry* e = &OG_INT3F_MGMT_KB[i];
        if (e->caller_cs != 0       && e->caller_cs != caller_cs) continue;
        if (e->caller_ip != 0       && e->caller_ip != caller_ip) continue;
        if (e->ax_value  != 0xFFFF  && e->ax_value  != ax)        continue;
        return e->label;
    }
    return NULL;
}


/*
 *  INT 3F instruction probe.  Fires when the next instruction at
 *  CS:IP is `CD 3F`.  Logs SEG_IDX (from AX) and reads the trailing
 *  JMP FAR target segment at stub+7..+8 -- strategy A from the
 *  og_overlay_probe.PRD.md.
 *
 *  Cheap reject on the first byte; full check on a match.
 *
 *  Caveat on first call to a never-yet-loaded overlay: the JMP FAR
 *  target seg may still be the unpatched placeholder.  In practice
 *  the manager patches and resumes within the same dispatch, so by
 *  the time a subsequent stub fires we read a settled value.  We
 *  filter unreasonable targets (zero, BIOS range) and skip the map
 *  update for those.
 */
/* ----------------------------------------------------------------------
 *  Borland C++ overlay manager helpers (generic).
 *
 *  These helpers don't know anything specific about MGC or WZD.  They
 *  operate on whatever (seg_idx, seg_ea, seg_name) list a caller
 *  hands them, so the same routines work for any Borland C++ DOS
 *  program with overlays whose IDA-exported segment table is loaded.
 * --------------------------------------------------------------------- */

/*
 *  Lookup a Borland overlay segment by index and log one line with
 *  its metadata.  Intended for AX-at-INT-3F-fire-time: AX is normally
 *  the seg_idx the overlay manager is being asked to dispatch to.
 *
 *  Args:
 *    fire_seq     -- the fire counter from the caller (just for log clarity)
 *    ax_val       -- the AX register value at the INT 3F instruction
 *    cs_val       -- runtime CS of the INT 3F (logged so the caller can
 *                    correlate with later runtime_cs assignments)
 *    ip_val       -- runtime IP of the INT 3F
 *    segs         -- caller's segment table (e.g. og_exes[i].segs)
 *    seg_count    -- entries in segs[]
 *    exe_label    -- short label for the EXE the table came from
 *
 *  Emits exactly one [OG_OVR_AX] line per call.  Format:
 *    [OG_OVR_AX] fire #N  cs=XXXX:XXXX  AX=XXXX  -> EXE_LABEL SEG_NAME (SEG_EA=XXXXXXXX)
 *  or, when no entry has seg_idx == ax_val:
 *    [OG_OVR_AX] fire #N  cs=XXXX:XXXX  AX=XXXX  -> EXE_LABEL <no seg_idx match>
 */
static inline void og_borland_log_int3f_ax(
        unsigned long long fire_seq,
        Bit16u ax_val,
        Bit16u cs_val, Bit16u ip_val,
        const struct og_seg_info* segs,
        unsigned seg_count,
        const char* exe_label)
{
    unsigned i;
    for (i = 0; i < seg_count; i++) {
        if (segs[i].seg_idx == ax_val) {
            fprintf(stderr,
                    "[OG_OVR_AX] fire #%llu  cs=%04X:%04X  AX=%04X"
                    "  -> %s %s (SEG_EA=%08X)\n",
                    fire_seq, cs_val, ip_val, ax_val,
                    exe_label, segs[i].seg_name, segs[i].seg_ea);
            return;
        }
    }
    fprintf(stderr,
            "[OG_OVR_AX] fire #%llu  cs=%04X:%04X  AX=%04X"
            "  -> %s <no seg_idx match>\n",
            fire_seq, cs_val, ip_val, ax_val, exe_label);
}

/*
 *  Dump N bytes starting AT the CD 3F instruction, then the same at
 *  cs:0000 (the start of the segment, which is where the per-stub
 *  descriptor lives).
 *
 *  Stub descriptor layout (per jbalcomb RE notes, WZD stub051 at IDA
 *  33DF:0000):
 *
 *      offset  size  field         notes
 *      ------  ----  ------------  --------------------------------------
 *       0      2     int_code      always 0x3FCD (CD 3F)
 *       2      2     memswap
 *       4      2     fileoff
 *       6      4     codesize
 *      10      2     relsize
 *      12      2     nentries
 *      14      2     prevstub
 *      16     16     workarea      overlay-manager-private runtime state
 *
 *  Total: 32 bytes.
 *
 *  Generic: takes CS:IP, knows nothing about MGC/WZD.
 */
static inline void og_borland_log_int3f_stub_block(
        unsigned long long fire_seq,
        Bit16u cs_val, Bit16u ip_val,
        unsigned n_bytes)
{
    /* Raw bytes line. */
    {
        char buf[256];
        int  pos = 0;
        unsigned i;
        pos += snprintf(buf + pos, sizeof buf - pos,
                        "[OG_OVR_STUB] fire #%llu  %04X:%04X (+0..+%u):",
                        fire_seq, cs_val, ip_val, n_bytes - 1);
        for (i = 0; i < n_bytes && pos < (int)sizeof(buf) - 4; i++) {
            Bit8u b = real_readb(cs_val, (Bit16u)(ip_val + i));
            pos += snprintf(buf + pos, sizeof buf - pos, " %02X", b);
        }
        fprintf(stderr, "%s\n", buf);
    }

    if (n_bytes < 32) return;

    /* Dump the 32 bytes at cs:0000 (the START of the segment) -- THIS
     * is the segment's stub_descr.  Always meaningful regardless of
     * where the fire landed within the segment. */
    {
        char buf[256];
        int  pos = 0;
        unsigned i;
        pos += snprintf(buf + pos, sizeof buf - pos,
                        "[OG_OVR_STUB] fire #%llu  %04X:0000 (segment descr):",
                        fire_seq, cs_val);
        for (i = 0; i < 32 && pos < (int)sizeof(buf) - 4; i++) {
            Bit8u b = real_readb(cs_val, (Bit16u)i);
            pos += snprintf(buf + pos, sizeof buf - pos, " %02X", b);
        }
        fprintf(stderr, "%s\n", buf);
    }

    /* Parse cs:0000 per the layout above. */
    Bit8u  b[32];
    unsigned i;
    for (i = 0; i < 32; i++) b[i] = real_readb(cs_val, (Bit16u)i);
    Bit16u int_code = (Bit16u)(b[ 0] | (b[ 1] << 8));
    Bit16u memswap  = (Bit16u)(b[ 2] | (b[ 3] << 8));
    Bit16u fileoff  = (Bit16u)(b[ 4] | (b[ 5] << 8));
    Bit32u codesize = (Bit32u)(b[ 6] | (b[ 7] << 8) | (b[ 8] << 16) | (b[ 9] << 24));
    Bit16u relsize  = (Bit16u)(b[10] | (b[11] << 8));
    Bit16u nentries = (Bit16u)(b[12] | (b[13] << 8));
    Bit16u prevstub = (Bit16u)(b[14] | (b[15] << 8));
    fprintf(stderr,
            "[OG_OVR_STUB] fire #%llu  parsed (from cs=%04X:0000):"
            " int_code=%04X memswap=%04X fileoff=%04X"
            " codesize=%08X relsize=%04X nentries=%04X prevstub=%04X\n",
            fire_seq, cs_val,
            int_code, memswap, fileoff, codesize,
            relsize, nentries, prevstub);
    fprintf(stderr,
            "[OG_OVR_STUB] fire #%llu  workarea:"
            " %02X%02X%02X%02X %02X%02X%02X%02X"
            " %02X%02X%02X%02X %02X%02X%02X%02X\n",
            fire_seq,
            b[16], b[17], b[18], b[19],
            b[20], b[21], b[22], b[23],
            b[24], b[25], b[26], b[27],
            b[28], b[29], b[30], b[31]);
}


/*
 *  Call / ISR probe -- writes EVERY direct call and INT execution
 *  to a dedicated reference log (one per EXE so MAGIC and WIZARDS
 *  don't get jumbled together):
 *
 *      /tmp/magic_exe-calls.log    while og_exes[0] (MAGIC) is active
 *      /tmp/wizards_exe-calls.log  while og_exes[1] (WIZARDS) is active
 *
 *  When MAGIC EXECs WIZARDS (or WIZARDS bounces back to MAGIC), the
 *  active EXE flips and subsequent calls are appended to the right
 *  file.  Files are opened lazy-on-first-use and kept open for the
 *  whole session.
 *
 *  Catches three flavors of control transfer (the direct forms; the
 *  indirect forms FF /2 / FF /3 / etc. require operand decoding and
 *  aren't covered yet):
 *
 *      CALL FAR ptr16:16   9A oo oo ss ss   5 bytes  target = ss:oo
 *      CALL NEAR rel16     E8 oo oo         3 bytes  target = (ip+3) + (sint16)oo
 *      INT imm8            CD nn            2 bytes  target = IVT[nn*4]
 *
 *  Each execution logs:
 *
 *      KIND src_cs:src_ip (src_desc)  ->  dst_cs:dst_ip (dst_desc)
 *
 *  where KIND is one of "CALLF", "CALLN", "INT".  ISR targets come
 *  from real-mode IVT at 0000:(nn*4).
 *
 *  Volume is high (millions of lines for long sessions).  Output
 *  goes to a separate file to keep the main trace readable.  Analyze
 *  off-line with STU-DOSBox/analyze-wizards-calls.py.
 */
static inline void og_far_call_probe(Bit16u cs_val, Bit16u ip_val) {
    Bit8u opcode = real_readb(cs_val, ip_val);
    if (opcode != 0x9A && opcode != 0xE8 && opcode != 0xCD) return;

    /* One log file per EXE.  Index by og_active_exe_idx so calls
     * appear in the file for whichever EXE owned the CPU at the
     * moment of the call. */
    if (og_active_exe_idx < 0) return;
    int idx = og_active_exe_idx;

    static FILE*       fcall_log [OG_EXE_COUNT] = {NULL, NULL};
    static const char* fcall_path[OG_EXE_COUNT] = {
        "/tmp/magic_exe-calls.log",
        "/tmp/wizards_exe-calls.log"
    };
    static int         fcall_failed[OG_EXE_COUNT] = {0, 0};

    if (fcall_failed[idx]) return;
    if (fcall_log[idx] == NULL) {
        fcall_log[idx] = fopen(fcall_path[idx], "w");
        if (fcall_log[idx] == NULL) {
            fprintf(stderr,
                    "[OG_FCALL] WARN cannot open %s -- disabling probe for this EXE\n",
                    fcall_path[idx]);
            fcall_failed[idx] = 1;
            return;
        }
        setvbuf(fcall_log[idx], NULL, _IOLBF, 0);
        fprintf(stderr,
                "[OG_FCALL] %s: writing call/ISR log to %s\n",
                og_exes[idx].label, fcall_path[idx]);
    }
    FILE* fcall_log_f = fcall_log[idx];

    const char* kind;
    Bit16u dst_seg, dst_off;
    char dst_desc[128];

    if (opcode == 0x9A) {                       /* CALL FAR ptr16:16 */
        kind    = "CALLF";
        dst_off = real_readw(cs_val, (Bit16u)(ip_val + 1));
        dst_seg = real_readw(cs_val, (Bit16u)(ip_val + 3));
        og_describe(dst_seg, dst_off, dst_desc, sizeof dst_desc);
    } else if (opcode == 0xE8) {                /* CALL NEAR rel16 */
        kind    = "CALLN";
        Bit16u rel = real_readw(cs_val, (Bit16u)(ip_val + 1));
        dst_seg = cs_val;
        dst_off = (Bit16u)(ip_val + 3 + (Bit16s)rel);
        og_describe(dst_seg, dst_off, dst_desc, sizeof dst_desc);
    } else {                                    /* INT imm8 -- target from IVT */
        Bit8u vec = real_readb(cs_val, (Bit16u)(ip_val + 1));
        kind = "INT";
        /* Real-mode IVT: 4 bytes per entry at linear 0:(vec*4) =
         * offset(word), segment(word).  Read via seg=0, offset=vec*4. */
        dst_off = real_readw(0, (Bit16u)((Bit16u)vec * 4));
        dst_seg = real_readw(0, (Bit16u)((Bit16u)vec * 4 + 2));
        /* Don't run og_describe on the handler address -- it's
         * typically in BIOS/DOS territory or a DOSBox callback
         * (CS=F000+), where the overlay resolver isn't meaningful.
         * Tag with the INT vector number instead. */
        snprintf(dst_desc, sizeof dst_desc, "INT %02Xh handler", vec);
    }

    char src_desc[128];
    og_describe(cs_val, ip_val, src_desc, sizeof src_desc);

    fprintf(fcall_log_f,
            "%-5s  %04X:%04X (%s)  ->  %04X:%04X (%s)\n",
            kind, cs_val, ip_val, src_desc,
            dst_seg, dst_off, dst_desc);
}


static inline void og_stage2_unit_value_snapshot(unsigned long long fire_seq);  /* fwd */

/* ----------------------------------------------------------------------
 *  R3-G6 post-INT-3F instruction-follow probe.
 *
 *  Temporary diagnostic.  When INT 3F fires, og_overlay_int3f_probe
 *  arms this counter to N.  Then on each subsequent per-instruction
 *  hook tick (regardless of whether it's guest code or a DOSBox
 *  native callback), we emit one [OG_OVR_FOLLOW] line with CS:IP
 *  plus 8 bytes of the instruction encoding, and decrement.  When
 *  the counter hits 0 we stop.  A new INT 3F fire re-arms it.
 *
 *  Purpose: observe (not speculate) what the BC++ 3.0 overlay
 *  manager does on dispatch -- handler entry, manager logic, the
 *  transition to overlay code, function entry, and any back-patching
 *  of the originating thunk.  Output destination is the main trace
 *  log (stderr), so the sequence is in chronological order with the
 *  other [OG_OVR] lines we already emit.
 *
 *  This is a temporary probe.  Delete or gate it behind a flag once
 *  the dispatch mechanism is characterized.
 */
#define OG_OVR_FOLLOW_DEPTH 256
static unsigned long long og_ovr_follow_fire_seq    = 0;
static unsigned           og_ovr_follow_remaining   = 0;
static unsigned           og_ovr_follow_index       = 0;

static inline void og_overlay_int3f_followup_probe(Bit16u cs_val, Bit16u ip_val) {
    if (og_ovr_follow_remaining == 0) return;
    og_ovr_follow_index++;
    char buf[160];
    int  pos = 0;
    pos += snprintf(buf + pos, sizeof buf - pos,
                    "[OG_OVR_FOLLOW] fire #%llu +%02u  cs=%04X:%04X  bytes:",
                    og_ovr_follow_fire_seq,
                    og_ovr_follow_index,
                    cs_val, ip_val);
    int i;
    for (i = 0; i < 8; i++) {
        Bit8u b = real_readb(cs_val, (Bit16u)(ip_val + i));
        pos += snprintf(buf + pos, sizeof buf - pos, " %02X", b);
    }
    fprintf(stderr, "%s\n", buf);
    og_ovr_follow_remaining--;
}

static inline void og_overlay_int3f_probe(Bit16u cs_val, Bit16u ip_val) {
    if (real_readb(cs_val, ip_val) != 0xCD) return;
    if (real_readb(cs_val, (Bit16u)(ip_val + 1)) != 0x3F) return;

    /* No model assumptions about layout: dump everything we can.  We
     * don't know what means what yet; we'll figure it out by reading
     * the trace and matching against the disassembly. */
    Bit16u ax = (Bit16u)(reg_eax & 0xFFFFu);
    Bit16u bx = (Bit16u)(reg_ebx & 0xFFFFu);
    Bit16u cx = (Bit16u)(reg_ecx & 0xFFFFu);
    Bit16u dx = (Bit16u)(reg_edx & 0xFFFFu);
    Bit16u di = (Bit16u)(reg_edi & 0xFFFFu);
    Bit16u si = (Bit16u)(reg_esi & 0xFFFFu);
    Bit16u bp = (Bit16u)(reg_ebp & 0xFFFFu);
    Bit16u sp = (Bit16u)(reg_esp & 0xFFFFu);
    Bit16u ss_val = SegValue(ss);   /* *_val: avoid the SegNames-shadow trap */
    Bit16u ds_val = SegValue(ds);
    Bit16u es_val = SegValue(es);

    /* Throttle by (cs, ip): record the fire site once per unique
     * address we see INT 3F executing at. */
    static struct { Bit16u cs; Bit16u ip; } seen[16];
    static unsigned seen_count = 0;
    unsigned k;
    int already = 0;
    for (k = 0; k < seen_count; k++) {
        if (seen[k].cs == cs_val && seen[k].ip == ip_val) { already = 1; break; }
    }
    if (!already && seen_count < 16) {
        seen[seen_count].cs = cs_val;
        seen[seen_count].ip = ip_val;
        seen_count++;
        fprintf(stderr,
                "[OG_OVR] INT 3F site cs=%04X ip=%04X  -- new fire site\n",
                cs_val, ip_val);
    }

    static unsigned long long fires_logged = 0;
    fires_logged++;

    /* ----- per-fire dump -----
     * Header line: registers + SS:SP. */
    fprintf(stderr,
            "[OG_OVR] fire #%llu  cs=%04X:%04X  ax=%04X bx=%04X cx=%04X dx=%04X  di=%04X si=%04X bp=%04X  ds=%04X es=%04X  ss:sp=%04X:%04X\n",
            fires_logged - 1,
            cs_val, ip_val,
            ax, bx, cx, dx, di, si, bp,
            ds_val, es_val, ss_val, sp);

    /* Identify the running EXE by scanning DS for __EXENAME__, then
     * resolve CS:IP via the active EXE's overlay table. */
    og_detect_active_exe(ds_val);
    og_runtime_to_name(cs_val, ip_val);

    /* Generic Borland-overlay AX-at-fire-time lookup: log AX against
     * the active EXE's segment table so we can correlate INT 3F
     * dispatches with the overlay seg_idx the manager is being asked
     * to load.  No-op until layer B has identified an EXE. */
    if (og_active_exe_idx >= 0) {
        struct og_exe_info* ax_e = &og_exes[og_active_exe_idx];
        og_borland_log_int3f_ax(fires_logged - 1, ax,
                                cs_val, ip_val,
                                ax_e->segs, ax_e->seg_count,
                                ax_e->label);
    }

    /* Raw stub-block dump: 32 bytes from CD 3F so we can read the
     * overlay-stub structure off the trace.  Also reads + parses the
     * descriptor at cs:0000 (which IS a TUnitStubBlock when cs is a
     * stubXXX paragraph). */
    og_borland_log_int3f_stub_block(fires_logged - 1, cs_val, ip_val, 32);

    /* Snapshot _unit's current value so we see its trajectory
     * indexed against INT 3F fires (independent of the
     * per-instruction change detector). */
    og_stage2_unit_value_snapshot(fires_logged - 1);

    /* R3-G6: arm the post-INT-3F instruction-follow probe.  Next N
     * per-instruction ticks will each log one [OG_OVR_FOLLOW] line. */
    og_ovr_follow_fire_seq  = fires_logged - 1;
    og_ovr_follow_remaining = OG_OVR_FOLLOW_DEPTH;
    og_ovr_follow_index     = 0;

    /* Rebuild the back-patched thunk map.  This captures any patches
     * the manager applied during previous INT 3F dispatches.  The
     * patch for THIS fire happens during the handler that's about to
     * run; we'll pick it up on the next fire's rebuild. */
    if (og_active_exe_idx >= 0) {
        og_rebuild_resolved_thunks(&og_exes[og_active_exe_idx]);
    }
}


/* ====================================================================== *
 *                       DIAGNOSTIC PROBES                                 *
 * ====================================================================== */

/*
 *  Emit a single line on the first hook call.  Proves the active CPU
 *  core is the one we hooked (normal, not dynamic/simple/full).
 */
static inline void og_diag_first_call(Bit16u cs_val, Bit16u ip_val) {
    static int fired = 0;
    if (!fired) {
        fired = 1;
        fprintf(stderr,
                "[OG_DIAG] FIRST_CALL  trace is alive  cs=%04X ip=%04X\n",
                cs_val, ip_val);
    }
}

/*
 *  Periodic heartbeat: confirms the trace keeps running as the game
 *  proceeds.  Bumps an instruction counter; emits every 5M.
 */
static inline void og_diag_heartbeat(Bit16u cached_ds, Bit16u sort_known_cs) {
    static unsigned long long calls = 0;
    calls++;
    if ((calls % 5000000ULL) == 0) {
        fprintf(stderr,
                "[OG_DIAG] heartbeat calls=%llu  detected_ds=%04X  sort_cs=%04X\n",
                calls, cached_ds, sort_known_cs);
    }
}


/* ====================================================================== *
 *           GENERIC VALUE-CAPTURE                                         *
 * ====================================================================== *
 *
 * Table-driven memory-watch system.  Each row says "while EXE X is
 * active, watch the N-byte value at DGROUP:offset; log a [WATCH] line
 * whenever it changes."  Per-row state (last value + change counter)
 * is kept in parallel arrays indexed by row; nothing here is
 * game-specific -- adding a new watch is a new row.
 *
 * Log format:
 *
 *   [WATCH] EXE=MAGIC.EXE  name=random_seed  value=0x35683568  (change #1  ds:31D1:431E  width=4)
 *
 * Post-process scripts grep by `name=` and pick whichever transition
 * they care about (e.g. first settled value after Randomize()).
 */
struct watch_target {
    const char* name;       /* short label, used in the log line */
    const char* exe_label;  /* must match og_exes[idx].label; NULL means "any active EXE" */
    Bit16u      ds_offset;  /* inline: DGROUP offset of the value.
                             * deref:  DGROUP offset of a FAR pointer (off @ds_offset,
                             *         seg @ds_offset+2) -- watch [ptr]+elem_off. */
    unsigned    width;      /* 1, 2, or 4 bytes */
    int         deref;      /* 0 = inline; 1 = ds_offset is a far-ptr slot (zero-init = inline) */
    Bit16u      elem_off;   /* (deref) byte offset within the pointed-to array */
};

static const struct watch_target WATCH_TARGETS[] = {
    /* Borland writes 4-byte values as two 16-bit stores; the probe
     * sees half-write intermediates as their own changes.  Post-process
     * picks the first settled value (e.g. after Randomize() returns). */
    { "random_seed", "MAGIC.EXE",   0x431E, 4 },
    { "random_seed", "WIZARDS.EXE", 0x7846, 4 },
    /* _players[5] (NEUTRAL_PLAYER_IDX) +0x346 region: 8 bytes (4 words) that
     * diverge in SAVE1.GAM -- OG holds them at +0x346, ReMoM at +0xA6.  Watch
     * each word to capture EXACTLY which instruction writes it.  The meaning of
     * the stored values is NOT assumed -- rows named "p5_" also dump the writer
     * instruction's bytes (see watch_targets_check), so we read the real store
     * and its source operand instead of inferring from the value.
     * _players[5] = 0x6900/0x9ECA + 5*0x4C8 (=0x17E8); +0x346 -> MGC 0x842E /
     * WZD 0xB9F8. */
    { "p5_346w0", "MAGIC.EXE",   0x842E, 2 },
    { "p5_346w1", "MAGIC.EXE",   0x8430, 2 },
    { "p5_346w2", "MAGIC.EXE",   0x8432, 2 },
    { "p5_346w3", "MAGIC.EXE",   0x8434, 2 },
    { "p5_346w0", "WIZARDS.EXE", 0xB9F8, 2 },
    /* --- "the rest": the remaining meaningful SAVE1.GAM diffs, watched to see
     * WHERE (if at all) OG writes them -- so we can tell real init from OG-side
     * uninitialized junk.  Set during the New Game (MGC).  Offsets from the
     * ReMoM dseg wiki.  banner_id is inline in _players; the others are
     * FAR-POINTER arrays (deref=1: ds_offset is the DGROUP far-ptr slot, watch
     * is on [ptr]+elem_off).  "rest_" rows also dump the writer instruction. */
    /* _players[5].banner_id : 0x6900 + 5*0x4C8 + 0x16 = 0x80FE (inline) */
    { "rest_banner_id",  "MAGIC.EXE",   0x80FE, 1 },
    { "rest_banner_id",  "WIZARDS.EXE", 0xB6C8, 1 },
    /* connectivity_grid_land (was UU_TBL_1) far-ptr @ MGC 0x89EE; save +0x3A */
    { "rest_uutbl1_3A",  "MAGIC.EXE",   0x89EE, 2, 1, 0x003A },
    /* _ITEMS far-ptr @ MGC 0x64B0; _ITEMS[9].name bytes (array off 9*50 + 0xE/0x11/0x12) */
    { "rest_items9_1D0", "MAGIC.EXE",   0x64B0, 1, 1, 0x01D0 },
    { "rest_items9_1D3", "MAGIC.EXE",   0x64B0, 1, 1, 0x01D3 },
    { "rest_items9_1D4", "MAGIC.EXE",   0x64B0, 1, 1, 0x01D4 },
    /* hero_names_table far-ptr @ MGC 0x68FC; hero_names[4].name bytes (array off 4*16 + 0/3/4) */
    { "rest_heronm4_40", "MAGIC.EXE",   0x68FC, 1, 1, 0x0040 },
    { "rest_heronm4_43", "MAGIC.EXE",   0x68FC, 1, 1, 0x0043 },
    { "rest_heronm4_44", "MAGIC.EXE",   0x68FC, 1, 1, 0x0044 },
    /* Add new rows here, e.g.:
     *   { "_unit",                 "WIZARDS.EXE", 0xBD8E, 2 },
     *   { "_players_p1_books_nat", "MAGIC.EXE",   0x6E22, 2 },
     */
};
#define WATCH_TARGETS_COUNT \
    ((unsigned)(sizeof(WATCH_TARGETS) / sizeof(WATCH_TARGETS[0])))

/* Per-row state in parallel arrays -- adding/removing rows above
 * doesn't require touching init code. */
static Bit32u             watch_prev_val[WATCH_TARGETS_COUNT] = {0};
static int                watch_seen    [WATCH_TARGETS_COUNT] = {0};
static unsigned long long watch_changes [WATCH_TARGETS_COUNT] = {0};

/* CS:IP of the LAST guest instruction the hook saw -- carried forward
 * from one tick to the next so a detected change is attributed to the
 * instruction that wrote it (which is the instruction that ran on the
 * previous tick, not the one we're about to run now). */
static Bit16u             watch_prev_writer_cs = 0;
static Bit16u             watch_prev_writer_ip = 0;

static inline Bit32u watch_read(Bit16u ds_val, Bit16u off, unsigned width) {
    switch (width) {
        case 1: return (Bit32u)real_readb(ds_val, off);
        case 2: return (Bit32u)real_readw(ds_val, off);
        case 4: {
            Bit16u lo = real_readw(ds_val, off);
            Bit16u hi = real_readw(ds_val, (Bit16u)(off + 2));
            return ((Bit32u)hi << 16) | lo;
        }
        default: return 0;
    }
}

static inline void watch_targets_check(Bit16u cs_val, Bit16u ip_val) {
    int idx = og_active_exe_idx;
    if (idx < 0) {
        watch_prev_writer_cs = cs_val;
        watch_prev_writer_ip = ip_val;
        return;
    }
    struct og_exe_info* e = &og_exes[idx];
    if (!e->active) {
        watch_prev_writer_cs = cs_val;
        watch_prev_writer_ip = ip_val;
        return;
    }

    unsigned i;
    for (i = 0; i < WATCH_TARGETS_COUNT; i++) {
        const struct watch_target* t = &WATCH_TARGETS[i];
        if (t->exe_label && strcmp(t->exe_label, e->label) != 0) continue;

        /* Resolve the watched address.  Inline: DGROUP:ds_offset.  Deref: read
         * the far pointer at DGROUP:ds_offset (off, then seg), then watch the
         * element at [ptr]+elem_off.  Skip while the far-ptr is still null. */
        Bit16u w_seg, w_off;
        if (t->deref) {
            Bit16u p_off = real_readw(e->runtime_ds, t->ds_offset);
            Bit16u p_seg = real_readw(e->runtime_ds, (Bit16u)(t->ds_offset + 2));
            if (p_seg == 0 && p_off == 0) continue;   /* array not allocated yet */
            w_seg = p_seg;
            w_off = (Bit16u)(p_off + t->elem_off);
        } else {
            w_seg = e->runtime_ds;
            w_off = t->ds_offset;
        }

        Bit32u curr = watch_read(w_seg, w_off, t->width);
        if (watch_seen[i] && curr == watch_prev_val[i]) continue;

        watch_changes[i]++;
        /* Resolve writer CS:IP to a stable name when possible. */
        int is_isr = og_cs_is_isr(watch_prev_writer_cs);
        if (is_isr) {
            fprintf(stderr,
                    "[WATCH] EXE=%s  name=%s  value=0x%0*X  writer=%04X:%04X (ISR)  (change #%llu  ds:%04X:%04X  width=%u)\n",
                    e->label, t->name,
                    (int)(t->width * 2), curr,
                    watch_prev_writer_cs, watch_prev_writer_ip,
                    watch_changes[i],
                    w_seg, w_off, t->width);
        } else {
            char desc[128];
            og_describe(watch_prev_writer_cs, watch_prev_writer_ip, desc, sizeof desc);
            fprintf(stderr,
                    "[WATCH] EXE=%s  name=%s  value=0x%0*X  writer=%04X:%04X (%s)  (change #%llu  ds:%04X:%04X  width=%u)\n",
                    e->label, t->name,
                    (int)(t->width * 2), curr,
                    watch_prev_writer_cs, watch_prev_writer_ip, desc,
                    watch_changes[i],
                    w_seg, w_off, t->width);
        }

        /* For the _players[5] probe rows ("p5_"), dump the bytes of the
         * instruction that did the write, so we read the actual store +
         * addressing mode (MOV imm vs MOV from mem/reg vs REP MOVS) instead of
         * inferring meaning from the stored value.  Skipped for ISR writers
         * (their CS:IP isn't a stable code site). */
        if ((strncmp(t->name, "p5_", 3) == 0 || strncmp(t->name, "rest_", 5) == 0)
                && !is_isr) {
            /* Dump a window that includes the loop HEAD (where AX / BX / the
             * source pointer are set up), not just the store -- so we can read
             * where the written value actually comes from.  The JL back-edge is
             * ~0x2F, so the body starts ~0x26 before the store; window =
             * writer-0x2A .. writer+0x0F (0x3A bytes). */
            Bit16u start = (Bit16u)(watch_prev_writer_ip - 0x2A);
            char ib[256]; int n = 0; unsigned k;
            for (k = 0; k < 0x3A; k++)
                n += snprintf(ib + n, (size_t)(sizeof(ib) - (size_t)n),
                              "%s%02X", k ? " " : "",
                              (unsigned)real_readb(watch_prev_writer_cs,
                                          (Bit16u)(start + k)));
            fprintf(stderr,
                    "[WATCH-INSN] %s writer=%04X:%04X window@%04X:%04X bytes=[%s]\n",
                    t->name, watch_prev_writer_cs, watch_prev_writer_ip,
                    watch_prev_writer_cs, start, ib);
        }

        watch_prev_val[i] = curr;
        watch_seen[i]     = 1;
    }

    watch_prev_writer_cs = cs_val;
    watch_prev_writer_ip = ip_val;
}


/* ====================================================================== *
 *           INSTRUCTION-PATTERN CAPTURE                                   *
 * ====================================================================== *
 *
 * Definitively captures a memory value AFTER a specific instruction
 * byte pattern executes -- no function-entry heuristic, no retf
 * counting, no "Nth-observed-change" convention.  When CS:IP points
 * at the configured byte pattern, we set pending; on the very next
 * instruction tick we snapshot the named memory location and emit
 * an [INSTR] line.
 *
 * Use when you know the exact byte sequence that writes the value
 * you care about.  Example: in MAGIC.EXE both Randomize() and
 * Set_Random_Seed() complete their seed update with
 *
 *     A3 20 43    mov [_random_seed+2], ax    ; DS:0x4320
 *
 * Random() advances the seed via a DIFFERENT opcode (`89 3E 20 43`,
 * mov [_random_seed+2], di), so matching `A3 20 43` captures ONLY
 * the genuine "seed was set" events, not LFSR advances.
 *
 * Table-driven, same shape as watch_target.  Each row:
 *   - label + exe_label: event name and which EXE it applies to
 *   - pattern + pattern_len: byte sequence to match at CS:IP
 *   - value_name + ds_offset + width: memory to snapshot AFTER the match
 *
 * Log format:
 *   [INSTR] EXE=MAGIC.EXE  label=seed_set  name=random_seed  value=0x0012697E  (#1  cs:ip=0E5D:00CE)
 *
 * The cs:ip in the log line is the address of the matched instruction
 * (the WHERE it happened), so post-process can disambiguate sites
 * by static IP without needing to know runtime CS values.
 */
struct instr_capture {
    const char*          label;
    const char*          exe_label;
    const unsigned char* pattern;
    unsigned             pattern_len;
    const char*          value_name;
    Bit16u               ds_offset;
    unsigned             width;
};

/* The final 3-byte instruction shared by Randomize() and
 * Set_Random_Seed() that completes the seed update.  Random()'s
 * LFSR advance uses opcode 0x89 (with ModR/M), not 0xA3, so these
 * patterns match ONLY genuine seed-set events.  Byte values
 * extracted from each EXE directly (see scratch-notes):
 *
 *   MGC  Set_Random_Seed @ 1CBB:0080  A3 20 43  (mov [DS:0x4320], ax)
 *   MGC  Randomize       @ 1CBB:00CD  A3 20 43
 *   WZD  Set_Random_Seed @ 1D4D:0084  A3 48 78  (mov [DS:0x7848], ax)
 *   WZD  Randomize       @ 1D4D:00D1  A3 48 78
 *
 * The cs:ip in the log line lets post-process disambiguate which
 * function fired (Set_Random_Seed vs Randomize) by static IP.
 */
static const unsigned char MGC_SEED_SET_FINAL_WRITE[] = {
    0xA3, 0x20, 0x43   /* mov [_random_seed+2], ax  ; DS:0x4320 */
};
static const unsigned char WZD_SEED_SET_FINAL_WRITE[] = {
    0xA3, 0x48, 0x78   /* mov [_random_seed+2], ax  ; DS:0x7848 */
};

static const struct instr_capture INSTR_CAPTURES[] = {
    { "seed_set", "MAGIC.EXE",
      MGC_SEED_SET_FINAL_WRITE, (unsigned)sizeof(MGC_SEED_SET_FINAL_WRITE),
      "random_seed", 0x431E, 4 },
    { "seed_set", "WIZARDS.EXE",
      WZD_SEED_SET_FINAL_WRITE, (unsigned)sizeof(WZD_SEED_SET_FINAL_WRITE),
      "random_seed", 0x7846, 4 },
};
#define INSTR_CAPTURES_COUNT \
    ((unsigned)(sizeof(INSTR_CAPTURES) / sizeof(INSTR_CAPTURES[0])))

static int                instr_pending [INSTR_CAPTURES_COUNT] = {0};
static Bit16u             instr_match_cs[INSTR_CAPTURES_COUNT] = {0};
static Bit16u             instr_match_ip[INSTR_CAPTURES_COUNT] = {0};
static unsigned long long instr_count   [INSTR_CAPTURES_COUNT] = {0};

/* --- RNG segment tracking (n1/n2 resync) ------------------------------
 *  Every Randomize()/Set_Random_Seed() reseed begins a new "segment" of
 *  the RNG stream.  og_rng_segment = which reseed we are in (1 after the
 *  first reseed); og_rng_seg_idx = the Random() call count SINCE that
 *  reseed.  The seed-set capture (instr_captures_check) bumps the segment
 *  on each reseed; og_rng_probe bumps the per-segment index on each
 *  Random().  Shared here so both probes (defined far apart) can reach it.
 *
 *  This is the resync anchor: OG's Randomize() points and ReMoM's matching
 *  Set_Random_Seed() injections delimit the SAME segments, so the streams
 *  align per (segment, seg_idx) even when the global call counts drift. */
static unsigned long og_rng_segment = 0;
static unsigned long og_rng_seg_idx = 0;

/* --- Seed forcing (deterministic replay; OG-Seed-Forcing-PRD.md) -------
 *  When OG_FORCE_SEED is set, overwrite the MAGIC Randomize() result with the
 *  pinned 32-bit value right after its seed store completes, so a run is
 *  reproducible.  P0 (the determinism gate) forces EVERY MAGIC Randomize to the
 *  same value to strip all time-based non-determinism; if two same-seed runs
 *  still differ, the non-determinism is in memory state, not RNG.  Off (env
 *  unset) = no effect.  Done in the instr_captures_check flush block (post-store). */
static int    og_force_seed_inited = 0;
static int    og_force_seed_on     = 0;
static Bit32u og_force_seed        = 0;
static inline void og_force_seed_init(void) {
    if (og_force_seed_inited) return;
    og_force_seed_inited = 1;
    const char* s = getenv("OG_FORCE_SEED");
    if (s && *s) { og_force_seed = (Bit32u)strtoul(s, NULL, 0); og_force_seed_on = 1; }
}

/* Identify the reseed function from the seed-store IP.  Randomize() and
 * Set_Random_Seed() share the `A3 ..` store bytes but sit at distinct,
 * fixed offsets in seg022 (from MGC/WZD-overlay-table.fwv):
 *   MGC: Randomize store @0x00CD, Set_Random_Seed store @0x0080
 *   WZD: Randomize store @0x00D1, Set_Random_Seed store @0x0084          */
static inline const char* og_seed_fn_for_store(const char* exe_label, Bit16u ip) {
    if (strcmp(exe_label, "MAGIC.EXE") == 0) {
        if (ip == 0x00CD) return "Randomize";
        if (ip == 0x0080) return "Set_Random_Seed";
    } else if (strcmp(exe_label, "WIZARDS.EXE") == 0) {
        if (ip == 0x00D1) return "Randomize";
        if (ip == 0x0084) return "Set_Random_Seed";
    }
    return "?";
}

static inline void instr_captures_check(Bit16u cs_val, Bit16u ip_val) {
    int idx = og_active_exe_idx;
    if (idx < 0) return;
    struct og_exe_info* e = &og_exes[idx];
    if (!e->active) return;

    unsigned i;
    for (i = 0; i < INSTR_CAPTURES_COUNT; i++) {
        const struct instr_capture* t = &INSTR_CAPTURES[i];
        if (t->exe_label && strcmp(t->exe_label, e->label) != 0) continue;

        /* Flush pending: the matched instruction executed on the
         * previous tick; the target memory now reflects its effect. */
        if (instr_pending[i]) {
            /* This pattern only matches the Randomize()/Set_Random_Seed()
             * seed store, so every flush is a reseed -> new RNG segment. */
            const char* fn = og_seed_fn_for_store(e->label, instr_match_ip[i]);
            /* Seed forcing FIRST -- overwrite the just-stored seed BEFORE the
             * [INSTR] read below, so the logged value (and extract-seed, which
             * parses [INSTR]) reflect the pinned seed and ReMoM inherits it.
             * MAGIC Randomize only -- the time-based reseed; Set_Random_Seed sets
             * a specific game seed (deterministic given state) so it's left alone.
             * P0 forces every MAGIC Randomize; v2 narrows to the seg1 anchor. */
            og_force_seed_init();
            if (og_force_seed_on && strcmp(e->label, "MAGIC.EXE") == 0 &&
                strcmp(fn, "Randomize") == 0 && t->width == 4) {
                Bit32u was = watch_read(e->runtime_ds, t->ds_offset, t->width);
                real_writew(e->runtime_ds, t->ds_offset,
                            (Bit16u)(og_force_seed & 0xFFFF));
                real_writew(e->runtime_ds, (Bit16u)(t->ds_offset + 2),
                            (Bit16u)(og_force_seed >> 16));
                fprintf(stderr,
                        "[SEED-FORCE] MAGIC Randomize: was 0x%08X now 0x%08X\n",
                        was, og_force_seed);
            }
            Bit32u val = watch_read(e->runtime_ds, t->ds_offset, t->width);
            instr_count[i]++;
            og_rng_segment++;
            og_rng_seg_idx = 0;
            fprintf(stderr,
                    "[INSTR] EXE=%s  label=%s  fn=%s  seg=%lu  name=%s  value=0x%0*X  (#%llu  cs:ip=%04X:%04X)\n",
                    e->label, t->label, fn, og_rng_segment, t->value_name,
                    (int)(t->width * 2), val,
                    instr_count[i],
                    instr_match_cs[i], instr_match_ip[i]);
            instr_pending[i] = 0;
        }

        /* Match the pattern at the current CS:IP. */
        if (real_readb(cs_val, ip_val) != t->pattern[0]) continue;
        int matched = 1;
        unsigned k;
        for (k = 1; k < t->pattern_len; k++) {
            if (real_readb(cs_val, (Bit16u)(ip_val + k)) != t->pattern[k]) {
                matched = 0;
                break;
            }
        }
        if (!matched) continue;

        instr_pending [i] = 1;
        instr_match_cs[i] = cs_val;
        instr_match_ip[i] = ip_val;
    }
}


/* ====================================================================== *
 *           STAGE 1  --  Sort_Research_List I/O TRACE                     *
 * ====================================================================== *
 *
 *  WIZARDS.EXE Sort_Research_List lives in overlay ovr128 at IDA
 *  offset 0xBD6.  We don't know the runtime CS until ovr128 is loaded
 *  by the Borland overlay manager, so we DISCOVER it by scanning for
 *  the function's prologue byte signature:
 *      55 8B EC 83 EC 1A 56 57 8B 7E 06
 *      push bp; mov bp,sp; sub sp,1A; push si; push di; mov di,[bp+6]
 *
 *  When the signature matches at CS:IP=0xBD6, we cache that CS as
 *  ovr128's runtime segment.  Subsequent trace points only fire when
 *  CS equals the cached value.
 *
 *  Trace points (IPs are relative to ovr128's runtime CS):
 *    0xBDE  ENTER  -- right before `mov di,[bp+6]` reads player_idx;
 *                     dump count and player_idx.
 *    0xC47  REALM  -- end of realm-loop body, just before `inc itr`;
 *                     dump itr, bonus, research_points, income.
 *    0xE72  CMP    -- after `cmp ax,[bp+cost2]`, before `jle`;
 *                     dump research_idx2, spell_idx2, research_idx1,
 *                     cost1, cost2, and the SHIFT/STAY decision.
 *    0xEA0  EXIT   -- at the `retf`; dump player_idx as a sentinel.
 *
 *  All BP-relative offsets are from the IDA disassembly:
 *      https://github.com/jbalcomb/ReMoM/wiki/Borland-Overlay-Function-Example
 */

static Bit16u og_sort_known_cs = 0;   /* runtime CS of ovr128, once found */

static inline void og_stage1_detect_sort_overlay(Bit16u cs_val, Bit16u ip_val) {
    /* Cheap reject: the very first byte must be PUSH BP. */
    Bit8u b0 = real_readb(cs_val, ip_val);
    if (b0 != 0x55) return;

    static const Bit8u SIG[11] = {
        0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x1A,
        0x56, 0x57, 0x8B, 0x7E, 0x06
    };
    int i;
    for (i = 1; i < 11; i++) {
        if (real_readb(cs_val, (Bit16u)(ip_val + i)) != SIG[i]) return;
    }
    /* Match. */
    if (cs_val != og_sort_known_cs) {
        fprintf(stderr,
                "[OG_SORT] FOUND  cs=%04X ip=%04X  (expected ip=0BD6, prev_cs=%04X)\n",
                cs_val, ip_val, og_sort_known_cs);
        og_sort_known_cs = cs_val;
    }
}

static inline void og_stage1_emit_trace_points(Bit16u cs_val, Bit16u ip_val) {
    if (og_sort_known_cs == 0 || cs_val != og_sort_known_cs) return;

    Bit16u ss_val = SegValue(ss);
    Bit16u bp_val = (Bit16u)(reg_ebp & 0xFFFFu);

    if (ip_val == 0x0BDE) {
        /* ENTER: read args from caller's stack frame. */
        Bit16u player = real_readw(ss_val, (Bit16u)(bp_val + 6));
        Bit16u count  = real_readw(ss_val, (Bit16u)(bp_val + 8));
        fprintf(stderr,
                "[OG_SORT] cs=%04X ENTER  player=%u count=%u\n",
                cs_val, player, count);
    }
    else if (ip_val == 0x0C47) {
        /* End of realm-loop body, just before `inc itr`. */
        Bit16u itr    = real_readw(ss_val, (Bit16u)(bp_val - 2));
        Bit16u bonus  = real_readw(ss_val, (Bit16u)(bp_val - 4));
        Bit16u rp     = real_readw(ss_val, (Bit16u)(bp_val - 6));
        Bit16u income = real_readw(ss_val,
                                   (Bit16u)(bp_val - 0x1A + (itr * 2)));
        fprintf(stderr,
                "[OG_SORT] cs=%04X REALM  itr=%u bonus=%u rp=%u income=%u\n",
                cs_val, itr, bonus, rp, income);
    }
    else if (ip_val == 0x0E72) {
        /* The decisive `cmp ax,[bp+cost2]` / `jle` -- AX = cost1. */
        Bit16u idx2  = real_readw(ss_val, (Bit16u)(bp_val - 0x0A));
        Bit16u spl2  = real_readw(ss_val, (Bit16u)(bp_val - 0x08));
        Bit16u cost1 = real_readw(ss_val, (Bit16u)(bp_val - 0x0C));
        Bit16u cost2 = real_readw(ss_val, (Bit16u)(bp_val - 0x0E));
        int16_t idx1 = (int16_t)(reg_esi & 0xFFFFu);
        const char* decision = (cost1 <= cost2) ? "STAY" : "SHIFT";
        fprintf(stderr,
                "[OG_SORT] cs=%04X CMP    idx2=%u spl2=%u idx1=%d cost1=%u cost2=%u  -> %s\n",
                cs_val, idx2, spl2, idx1, cost1, cost2, decision);
    }
    else if (ip_val == 0x0EA0) {
        Bit16u player = real_readw(ss_val, (Bit16u)(bp_val + 6));
        fprintf(stderr,
                "[OG_SORT] cs=%04X EXIT   player=%u\n",
                cs_val, player);
    }
}


/* ====================================================================== *
 *           STAGE 2  --  _unit GLOBAL WRITE TRACE                         *
 * ====================================================================== *
 *
 *  Goal: identify every code site that writes to WIZARDS' `_unit`
 *  global (DS:0xBD8E, int16) during a Stage 2 Next-Turn flow.  Used
 *  to track down where OG resets _unit at AI-turn-end (the source of
 *  one byte of the Stage 2 diff against ReMoM).
 *
 *  Detection model:
 *    Our hook fires BEFORE each instruction.  We read _unit each
 *    iteration; if the value changed since last read, the previous
 *    iteration's instruction wrote it.  We log the previous hook's
 *    saved CS:IP and the new value.
 *
 *  Noise filter:
 *    DOSBox runs BIOS/INT handlers as native callbacks through CS in
 *    the F000 range.  Memory state can flip between iterations for
 *    reasons that have nothing to do with game code.  We skip the
 *    log emit when the attributed writer CS is in that range, but
 *    we DO update prev_unit_value so the next real write is detected
 *    relative to the current state.
 */

/*
 *  User-confirmed reference set: the four `_unit` writers observed in
 *  the Stage 2 canonical-save9 -> SAVE2 baseline trace.
 *
 *  WIZARDS.EXE ONLY.  These entries are meaningless when MAGIC.EXE is
 *  the active EXE.
 *
 *  Caveats:
 *    - `cs_val` is the overlay-buffer paragraph in THIS baseline
 *      session.  Borland's overlay manager may load the same overlay
 *      into a different paragraph in future sessions; when that
 *      happens, the (cs, ip) match below will miss and the assertion
 *      simply won't fire (no false positives).
 *    - `ip_val` is IP-within-overlay; that IS stable across sessions
 *      for a given overlay code path.
 *
 *  Source: og_overlay_findings.md (Stage 2 _unit writer references).
 */
struct og_unit_writer_ref {
    Bit16u      cs_val;
    Bit16u      ip_val;
    const char* seg_name;
    const char* func_name;
};
static const struct og_unit_writer_ref OG_UNIT_WRITER_REFS[] = {
    { 0x3FD1, 0x010D, "ovr051", "Init_Overland"               },
    { 0x384E, 0x0266, "ovr062", "Sort_Unit_Stack"             },
    { 0x4681, 0x0B1A, "ovr061", "Next_Unit_Nearest_Available" },
    { 0x45A8, 0x0266, "ovr062", "Sort_Unit_Stack"             },
};
#define OG_UNIT_WRITER_REF_COUNT \
    (sizeof(OG_UNIT_WRITER_REFS) / sizeof(OG_UNIT_WRITER_REFS[0]))

static inline void og_stage2_unit_write_probe(Bit16u cs_val, Bit16u ip_val) {
    static const Bit16u UNIT_OFF = 0xBD8E;
    static const unsigned long long MAX_WRITES_LOGGED = 200;

    static int       initialized      = 0;
    static Bit16u    prev_unit_value  = 0;
    static Bit16u    prev_writer_cs   = 0;
    static Bit16u    prev_writer_ip   = 0;
    static unsigned long long writes_logged = 0;
    static unsigned long long changes_seen  = 0;

    /* WZD-specific probe: no-op when the active EXE is not WIZARDS
     * (or when no EXE has been identified yet).  WIZARDS is og_exes[1]. */
    struct og_exe_info* wzd = &og_exes[1];
    if (!wzd->active) {
        prev_writer_cs = cs_val;
        prev_writer_ip = ip_val;
        return;
    }
    Bit16u wizards_ds = wzd->runtime_ds;

    Bit16u curr = real_readw(wizards_ds, UNIT_OFF);

    if (!initialized) {
        initialized     = 1;
        prev_unit_value = curr;
        fprintf(stderr,
                "[OG_UNIT] initial value=%u  (at ds:%04X:%04X)\n",
                curr, wizards_ds, UNIT_OFF);
        /* Dump 32 bytes around _unit so we can verify what's actually
         * at this address vs what we expect.  16 bytes before, 16
         * after, ASCII side-by-side. */
        char buf[256], asc[33];
        int  pos = 0;
        int  i;
        pos += snprintf(buf + pos, sizeof buf - pos,
                        "[OG_UNIT_DBG] arm-time memory %04X:%04X..+32:",
                        wizards_ds, (Bit16u)(UNIT_OFF - 16));
        for (i = -16; i < 16; i++) {
            Bit8u b = real_readb(wizards_ds, (Bit16u)(UNIT_OFF + i));
            const char* sep = (i == 0) ? " >" : (i == 2 ? "< " : " ");
            pos += snprintf(buf + pos, sizeof buf - pos, "%s%02X", sep, b);
            asc[i + 16] = (b >= 0x20 && b < 0x7F) ? (char)b : '.';
        }
        asc[32] = '\0';
        pos += snprintf(buf + pos, sizeof buf - pos, "  |%s|", asc);
        fprintf(stderr, "%s\n", buf);
    }
    else if (curr != prev_unit_value) {
        changes_seen++;
        int is_isr = og_cs_is_isr(prev_writer_cs);

        /* Log every _unit change.  ISR-attributed writes (CS=F000+,
         * DOSBox native callback range) are REAL behavior -- a DOS
         * INT 21h file-read can write game globals straight into
         * memory when loading a save.  We label them ISR so the
         * reader knows the origin is a callback, not guest x86 code,
         * and we don't run og_describe on the bogus F000-range CS. */
        if (writes_logged < MAX_WRITES_LOGGED) {
            if (is_isr) {
                fprintf(stderr,
                        "[OG_UNIT] writer cs=%04X ip=%04X  ISR  _unit  %u -> %u  (change #%llu)\n",
                        prev_writer_cs, prev_writer_ip,
                        prev_unit_value, curr, changes_seen);
            } else {
                char desc[128];
                og_describe(prev_writer_cs, prev_writer_ip, desc, sizeof desc);
                fprintf(stderr,
                        "[OG_UNIT] writer cs=%04X ip=%04X (%s)  _unit  %u -> %u  (change #%llu)\n",
                        prev_writer_cs, prev_writer_ip, desc,
                        prev_unit_value, curr, changes_seen);

                /* Reference assertion -- only against guest-code writers. */
                unsigned r;
                for (r = 0; r < OG_UNIT_WRITER_REF_COUNT; r++) {
                    const struct og_unit_writer_ref* ref = &OG_UNIT_WRITER_REFS[r];
                    if (ref->cs_val != prev_writer_cs) continue;
                    if (ref->ip_val != prev_writer_ip) continue;
                    int seg_ok  = (strstr(desc, ref->seg_name)  != NULL);
                    int func_ok = (strstr(desc, ref->func_name) != NULL);
                    int pass    = (seg_ok && func_ok);
                    fprintf(stderr,
                            "[OG_UNIT_TEST] cs=%04X ip=%04X  expected=%s:%s  got=%s  %s\n",
                            prev_writer_cs, prev_writer_ip,
                            ref->seg_name, ref->func_name, desc,
                            pass ? "PASS" : "FAIL");
                    break;
                }
            }
            writes_logged++;
        }
        prev_unit_value = curr;
    }

    prev_writer_cs = cs_val;
    prev_writer_ip = ip_val;
}

/*
 *  Snapshot _unit's current value (called from the INT 3F probe at
 *  each fire, so we get a timeline of the value at known anchor
 *  points -- independent of the per-instruction change detector). */
static inline void og_stage2_unit_value_snapshot(unsigned long long fire_seq) {
    struct og_exe_info* wzd = &og_exes[1];
    if (!wzd->active) return;
    Bit16u curr = real_readw(wzd->runtime_ds, 0xBD8E);
    fprintf(stderr,
            "[OG_UNIT_DBG] fire #%llu snapshot:  _unit = %u  (at ds:%04X:BD8E)\n",
            fire_seq, curr, wzd->runtime_ds);
}


/*
 *  Per-write-site _unit probe.
 *
 *  The snapshot/change probe above (og_stage2_unit_write_probe) only
 *  detects when _unit's *value* changes.  It can't see a "write of
 *  the same value" -- e.g. NUNA storing 0 when _unit was already 0,
 *  or SUS storing 5 when it was already 5.  Those are real writes
 *  that the ReMoM-side [_unit ASSIGN] log captures naturally, so for
 *  OG-vs-ReMoM step-by-step comparison we need to log them on the OG
 *  side too.
 *
 *  Strategy: known-instruction-site detection.  We have a table of
 *  (overlay name, IP-within-overlay-buffer, label) for every site
 *  where WIZARDS.EXE writes _unit.  At each guest instruction, check
 *  if (cs_val, ip_val) matches a site (for overlay sites we confirm
 *  the runtime CS belongs to the named overlay via the patched-thunks
 *  map; for ISR sites we match cs_val exactly).  On match: set a
 *  one-shot pending flag.  On the NEXT instruction: read _unit from
 *  WZD DGROUP:BD8E and log "site=<label> _unit_after=<val>".
 *
 *  This gives us every write source-attributed and value-tagged,
 *  whether or not the store actually changed _unit.
 *
 *  Sites are WIZARDS-specific (so we no-op when og_exes[1] is not
 *  active).  Add new sites here when a new write source is observed
 *  in a trace.
 */
struct og_unit_write_site {
    const char* ovr_seg_name; /* "ovrNNN" for overlay-buffer sites; NULL for ISR */
    Bit16u      cs_match;     /* only used when ovr_seg_name == NULL */
    Bit16u      ip_match;     /* IP within the overlay buffer (or ISR IP) */
    const char* label;
};

static const struct og_unit_write_site OG_UNIT_WRITE_SITES[] = {
    /* NOTE: no entry for the F000:14C1 DOS gfread site -- that
     * callback fires for EVERY gfread call (hundreds during a save
     * load), not just the one whose buffer is &_unit.  The existing
     * change-only probe (og_stage2_unit_write_probe) handles the
     * single _unit-targeting gfread cleanly via the 0 -> 5 change
     * it triggers.  Adding a precise DOS-site match here would
     * require parsing gfread's stack arguments to recover the ptr
     * argument and compare it to _unit's address -- not worth the
     * complexity for one extra log line. */
    /* ovr051:Init_Overland+10D: _unit = 0 (overland init reset). */
    { "ovr051", 0,      0x010D, "ovr051:Init_Overland+10D" },
    /* ovr062:Sort_Unit_Stack+149: writes selected stack head to _unit. */
    { "ovr062", 0,      0x0266, "ovr062:Sort_Unit_Stack+149" },
    /* ovr061:Next_Unit_Nearest_Available+23D: Closest_Active branch
     * write site.  IDA 5315:0B1A. */
    { "ovr061", 0,      0x0B1A, "ovr061:Next_Unit_Nearest_Available (Closest_Active branch)" },
    /* TODO: Closest_Waiting branch write site within NUNA.  Per the
     * wiki disassembly (Borland-Overlay-Function-Example), the
     * waiting-branch target is loc_53BC4 (linear 0x53BC4 = ovr061
     * IP 0x0A74) -- the actual _unit store is somewhere inside that
     * block.  Add once observed in a real trace. */
};

static inline void og_stage2_unit_write_site_probe(Bit16u cs_val, Bit16u ip_val) {
    static const unsigned long long MAX_WRITES_LOGGED = 500;
    static int                pending             = 0;
    static const char*        pending_label       = NULL;
    static unsigned long long writes_logged       = 0;

    /* Flush a deferred log first: the previous instruction matched a
     * write site; by now the store has executed, so reading _unit
     * gives us the post-store value. */
    if (pending) {
        struct og_exe_info* wzd = &og_exes[1];
        if (wzd->active && writes_logged < MAX_WRITES_LOGGED) {
            Bit16u curr = real_readw(wzd->runtime_ds, 0xBD8E);
            writes_logged++;
            fprintf(stderr,
                    "[OG_UNIT_WRITE] site=%s  _unit_after=%u  (write #%llu)\n",
                    pending_label, curr, writes_logged);
        }
        pending       = 0;
        pending_label = NULL;
    }

    if (og_active_exe_idx != 1) return; /* WIZARDS only */
    if (writes_logged >= MAX_WRITES_LOGGED) return;

    /* Cheap first filter: IP must match SOME site's IP. */
    unsigned i;
    unsigned n_sites = sizeof OG_UNIT_WRITE_SITES / sizeof OG_UNIT_WRITE_SITES[0];
    int ip_hit = 0;
    for (i = 0; i < n_sites; i++) {
        if (OG_UNIT_WRITE_SITES[i].ip_match == ip_val) { ip_hit = 1; break; }
    }
    if (!ip_hit) return;

    /* IP matched.  Now find the specific site with matching CS context
     * (ISR cs_match for ISR sites; runtime-overlay confirmation for
     * overlay sites). */
    struct og_exe_info* e = &og_exes[og_active_exe_idx];
    int thunks_built = 0;
    for (i = 0; i < n_sites; i++) {
        const struct og_unit_write_site* s = &OG_UNIT_WRITE_SITES[i];
        if (s->ip_match != ip_val) continue;
        if (s->ovr_seg_name == NULL) {
            if (cs_val != s->cs_match) continue;
            pending       = 1;
            pending_label = s->label;
            return;
        }
        /* Overlay site: cs_val must be the runtime CS of s->ovr_seg_name.
         * Walk the patched-thunks map (rebuild on demand, once). */
        if (!thunks_built) { og_rebuild_resolved_thunks(e); thunks_built = 1; }
        unsigned t;
        for (t = 0; t < og_resolved_thunk_count; t++) {
            if (og_resolved_thunks[t].target_seg != cs_val) continue;
            const struct og_func_info* tf = &e->funcs[og_resolved_thunks[t].funcs_idx];
            const struct og_seg_info*  stub_sg = og_find_seg_by_idx(e, tf->seg_idx);
            if (!stub_sg) continue;
            if (strncmp(stub_sg->seg_name, "stub", 4) != 0) continue;
            /* Expected ovrNNN name from "stubNNN". */
            char expected[16];
            snprintf(expected, sizeof expected, "ovr%s", stub_sg->seg_name + 4);
            if (strcmp(expected, s->ovr_seg_name) != 0) continue;
            pending       = 1;
            pending_label = s->label;
            return;
        }
    }
}


/* ====================================================================== *
 *           STAGE 3  --  NEW GAME WORK   (placeholder)                    *
 * ====================================================================== *
 *
 *  No probes yet.  When we start byte-comparing OG vs ReMoM for the
 *  New Game generation path (MAGIC.EXE setup, world gen, wizard
 *  selection writes, starting unit placement, etc.), add the probes
 *  here following the Stage 1/2 patterns above.
 */


/* ====================================================================== *
 *                         MAIN HOOK ENTRY                                 *
 * ====================================================================== *
 *
 *  Called from src/cpu/core_normal.cpp, right after LOADIP at the top
 *  of CPU_Core_Normal_Run's while loop.  Dispatches to each probe.
 *
 *  CS/IP locals are named with `_val` suffixes to avoid shadowing the
 *  SegNames `cs`/`ss` enum constants inside SegValue().
 */
/* -------------------------------------------------------------------- *
 *  Game-data capture engine (CaptureGameData PRD/PLAN, Phase 2).
 *
 *  Table-driven decode of a guest record into human-readable [GD] lines,
 *  one per field:  [GD] <point> <target>[<idx>].<field> = <value>
 *  (strings quoted, integers decimal).  Field tables are generated from the
 *  ReMoM C structs by gen-fieldmap.py -- single source of truth; see
 *  gd_wizard_fields.h.
 * -------------------------------------------------------------------- */
enum { GD_U8 = 0, GD_I8, GD_U16, GD_I16, GD_U32, GD_I32, GD_STR, GD_BYTES };

typedef struct { Bit16u off; Bit8u kind; Bit16u count; const char* name; } gd_field_t;

#include "gd_wizard_fields.h"
#include "gd_node_fields.h"
#include "gd_tower_fields.h"
#include "gd_lair_fields.h"
#include "gd_city_fields.h"
#include "gd_unit_fields.h"
#include "gd_heroes_fields.h"
#include "gd_item_fields.h"
#include "gd_fortress_fields.h"

static inline int gd_elem_size(Bit8u kind) {
    switch (kind) {
        case GD_U16: case GD_I16: return 2;
        case GD_U32: case GD_I32: return 4;
        default:                  return 1;   /* U8/I8/STR/BYTES */
    }
}

static inline long gd_read_elem(Bit16u seg, Bit16u off, Bit8u kind) {
    switch (kind) {
        case GD_I8:  return (long)(int8_t) real_readb(seg, off);
        case GD_U16: return (long)         real_readw(seg, off);
        case GD_I16: return (long)(int16_t)real_readw(seg, off);
        case GD_U32: return (long)         real_readd(seg, off);
        case GD_I32: return (long)(int32_t)real_readd(seg, off);
        default:     return (long)         real_readb(seg, off);  /* U8 */
    }
}

static void gd_decode_record(Bit16u seg, Bit16u base,
                             const gd_field_t* f, int n,
                             const char* target, int idx, const char* point) {
    int i, k;
    char val[1100];
    for (i = 0; i < n; i++) {
        Bit16u foff = (Bit16u)(base + f[i].off);
        if (f[i].kind == GD_STR) {
            int len = f[i].count, p = 0;
            val[p++] = '"';
            for (k = 0; k < len && p < (int)sizeof(val) - 2; k++) {
                Bit8u c = real_readb(seg, (Bit16u)(foff + k));
                if (c == 0) break;
                val[p++] = (c >= 32 && c < 127) ? (char)c : '.';
            }
            val[p++] = '"'; val[p] = 0;
        } else if (f[i].kind == GD_BYTES) {
            int len = f[i].count, p = 0;
            for (k = 0; k < len && p < (int)sizeof(val) - 3; k++)
                p += snprintf(val + p, sizeof(val) - p, "%02X",
                              (unsigned)real_readb(seg, (Bit16u)(foff + k)));
            val[p] = 0;
        } else {
            int es = gd_elem_size(f[i].kind), p = 0;
            for (k = 0; k < f[i].count; k++) {
                long v = gd_read_elem(seg, (Bit16u)(foff + k * es), f[i].kind);
                p += snprintf(val + p, sizeof(val) - p, k ? ",%ld" : "%ld", v);
                if (p > (int)sizeof(val) - 16) break;
            }
        }
        fprintf(stderr, "[GD] %s %s[%d].%s = %s\n",
                point, target, idx, f[i].name, val);
    }
}

/* -------------------------------------------------------------------- *
 *  Spellranks capture probe -- OG side of the new-game AI wizard init.
 *
 *  Dumps _players[].spellranks[] at the point where MAGIC.EXE's
 *  Init_Computer_Players_Wizard_Profile (MGC ovr056, IDA 0x5301E) has
 *  finished.  This is the OG counterpart to ReMoM's [FN-EXIT] LOG_TRACE
 *  in INITGAME.c, so the two traces line up for a value/byte diff.
 *
 *  Trigger.  The function is overlay-buffer code, reached by an
 *  *intra-overlay* near call from Init_Computer_Players (INITGAME.c:82),
 *  so it never passes through its resident INT-3F stub -- a stub-address
 *  trigger on it would never fire (confirmed: the Stage-0 trace logs the
 *  cross-overlay j_ stubs but never the NX_j_ intra-overlay ones).  Its
 *  own runtime CS is session-dynamic and not reliably resolvable through
 *  the single base_offset either.
 *
 *  What makes a *later* resident trigger correct: EVERY write to
 *  spellranks happens before this function returns -- the preset assign,
 *  difficulty erase, trait adds, and the realm merge in
 *  Consolidate_Spell_Book_Realms (itself called from INITGAME.c:895,
 *  INSIDE this function).  Nothing in the rest of MAGIC's new-game setup
 *  touches spellranks again, so the values are final the moment
 *  Init_Computer_Players returns -- the same point ReMoM logs in its
 *  DEBUG loop at INITGAME.c:84.
 *
 *  So we fire on a PROVEN resident landmark that runs after the AI init
 *  and before the MAGIC->WIZARDS handoff: the j_Initialize_Events stub at
 *  IDA linear 0x31BDD (stub056:0x4D, MGC-overlay-table.fwv) -- observed
 *  firing in MAGIC in a real Stage-0 run.  Resident addresses resolve
 *  exactly through base_offset, so no overlay-buffer resolution is needed,
 *  and DS still points at MAGIC.EXE's DGROUP.
 *
 *  Data.  struct s_WIZARD _players[6] at MGC DGROUP offset 0x6900
 *  (ReMoM wiki "Data-Segment Memory Layout"); record stride 0x4C8
 *  (sizeof s_WIZARD); spellranks is int16_t[5] at +0x5A (MOM_DAT.h:1442),
 *  index order [0]=Nature [1]=Sorcery [2]=Chaos [3]=Life [4]=Death
 *  (enum e_SPELL_BOOK_REALM).
 * -------------------------------------------------------------------- */
#define OG_SR_MGC_EXE_IDX     0           /* og_exes[0] == MAGIC.EXE       */
#define OG_SR_WZD_EXE_IDX     1           /* og_exes[1] == WIZARDS.EXE (DGROUP 36AA; post-Load_SAVE_GAM captures) */
#define OG_SR_TRIGGER_IDA     0x00031BDDu /* j_Initialize_Events stub, after AI init */
#define OG_SR_PLAYERS_OFF     0x6900u     /* MGC DGROUP offset of _players  */
#define OG_SR_RECORD_SIZE     0x4C8u      /* sizeof(struct s_WIZARD)        */
#define OG_SR_SPELLRANKS_OFF  0x5Au       /* spellranks[5] within record    */
#define OG_SR_NUMPLAYERS_OFF  0x87D2u     /* MGC DGROUP offset of _num_players */
#define OG_SR_NUM_PLAYERS     6           /* s_WIZARD array length (slots)   */
#define OG_SR_NUM_REALMS      5

static inline void og_spellranks_probe(Bit16u cs_val, Bit16u ip_val) {
    /* Fire once per MAGIC.EXE activation.  The Initialize_Events stub call
     * site is executed more than once per new game (the resolver de-dupes
     * its INT-3F resolved-line, but this probe does not), and the dumped
     * state is identical each time.  Re-arm whenever the active EXE changes
     * so a second new game later in the same session is still captured. */
    static int last_idx = -2;
    static int fired    = 0;
    if (og_active_exe_idx != last_idx) { last_idx = og_active_exe_idx; fired = 0; }

    /* Only meaningful while MAGIC.EXE is the live process. */
    if (og_active_exe_idx != OG_SR_MGC_EXE_IDX || fired) return;
    struct og_exe_info* e = &og_exes[OG_SR_MGC_EXE_IDX];

    Bit32u runtime_linear = ((Bit32u)cs_val << 4) + ip_val;
    Bit32s ida_linear     = (Bit32s)runtime_linear - e->base_offset;
    if (ida_linear < 0 || (Bit32u)ida_linear != OG_SR_TRIGGER_IDA) return;

    fired = 1;
    static unsigned fire_seq = 0;
    fire_seq++;

    Bit16u dgroup = e->runtime_ds;   /* confirmed MAGIC.EXE DGROUP segment */
    int16_t num_players = (int16_t)real_readw(dgroup, (Bit16u)OG_SR_NUMPLAYERS_OFF);
    fprintf(stderr,
            "[OG_SPELLRANKS] fire #%u  Wizard_Profile result (@Initialize_Events)"
            "  _num_players=%d  _players@%04X:%04X  order=[N S C L D]\n",
            fire_seq, num_players, dgroup, (unsigned)OG_SR_PLAYERS_OFF);

    int p, r;
    for (p = 0; p < OG_SR_NUM_PLAYERS; p++) {
        Bit16u sr_base = (Bit16u)(OG_SR_PLAYERS_OFF
                                  + (Bit32u)p * OG_SR_RECORD_SIZE
                                  + OG_SR_SPELLRANKS_OFF);
        int16_t v[OG_SR_NUM_REALMS];
        for (r = 0; r < OG_SR_NUM_REALMS; r++) {
            v[r] = (int16_t)real_readw(dgroup, (Bit16u)(sr_base + (Bit16u)(r * 2)));
        }
        fprintf(stderr,
                "[OG_SPELLRANKS] p=%d  N=%d S=%d C=%d L=%d D=%d%s\n",
                p, v[0], v[1], v[2], v[3], v[4],
                (p >= num_players) ? "  (unused slot)" : "");
    }

    /* Full-record table-driven decode for every active player (Phase 2).
     * Wired here at the proven Initialize_Events landmark to validate the
     * 136-field decode; per-function FN-EXIT traps come next. */
    {
        int act = (num_players >= 1 && num_players <= OG_SR_NUM_PLAYERS)
                  ? num_players : OG_SR_NUM_PLAYERS;
        for (p = 0; p < act; p++) {
            Bit16u rbase = (Bit16u)(OG_SR_PLAYERS_OFF
                                    + (Bit32u)p * OG_SR_RECORD_SIZE);
            gd_decode_record(dgroup, rbase, wizard_fields, WIZARD_FIELD_COUNT,
                             "_players", p, "Initialize_Events");
        }
    }
}

/* -------------------------------------------------------------------- *
 *  _players FN-EXIT capture traps (CaptureGameData PLAN Phase 2b).
 *
 *  Capture _players AFTER each intra-overlay populating function returns.
 *  Entry is detected by (IP == overlay offset) + og_describe() name match --
 *  the SAME identity check the gd_wm/node/lair/city tables below use (this list
 *  used to carry an 8-byte prologue signature instead; converted to share
 *  gd_wm_capfn_t).  The overlay BUFFER segment varies per load, so CS is never
 *  hardcoded.  These are near intra-overlay calls: the return address is the
 *  offset at [SS:SP] with CS unchanged; on entry we record (cs, return_ip), and
 *  when execution reaches it the function has returned and we snapshot _players,
 *  tagged with the point label.  Offsets = func_ea - overlay seg_ea (MGC-overlay-
 *  table); func names are what og_describe resolves (substring-matched). */
struct gd_wm_capfn_t { Bit16u off; const char* func; const char* point; };
static const struct gd_wm_capfn_t gd_capfns[] = {
    { 0x1DA8, "Newgame_Screen_7",                           "000_Newgame_Screen_7_P"          },
    { 0x0D3E, "Init_Computer_Players_Wizard_Profile",       "100_Wizard_Profile_P"            },
    { 0x1726, "Init_Computer_Players_Spell_Library",        "101_Spell_Library_P"             },
    { 0x246D, "Init_Heroes",                                "201_Init_Heroes_P"               },
    { 0x0A07, "Init_Players",                               "203_Init_Players_P"              },
    { 0x096D, "Init_CP_Strategy",                           "204_Init_CP_Strategy_P"          },
    { 0x04B2, "Init_Magic_Personalities_Objectives",        "205_Init_Magic_Pers_Obj_P"       },
    { 0x03C6, "Init_Summoning_Circle_And_Spell_Of_Mastery", "206_Init_Summoning_Circle_P"     },
    /* 105_capital_race_P is an AT-ENTRY capture -- see gd_player_entry_capfns below. */
    { 0x016A, "Init_Diplomatic_Relations",                  "208_Init_Diplomatic_Relations_P" },
    { 0x00C3, "Init_Runtime",                               "209_gold_reserve_P"              },
};

struct gd_pending_t { int active; int fn; Bit16u cs; Bit16u ret_ip; };
static struct gd_pending_t gd_pend[8];

/* AT-ENTRY _players captures: snapshot _players the instant the function is
 * entered (before it runs), for points whose value is finished just before a
 * call boundary.  105_capital_race_P is the capital_race copy loop's result,
 * which is complete right before Init_Diplomatic_Relations is entered. */
static const struct gd_wm_capfn_t gd_player_entry_capfns[] = {
    { 0x016A, "Init_Diplomatic_Relations", "207_capital_race_P" },
};

/* -------------------------------------------------------------------- *
 *  _world_maps capture (terrain grid) at Generate_Climate_Terrain_Types
 *  FN-EXIT.  _world_maps is a FAR POINTER in MGC DGROUP (wiki Data-Segment
 *  layout: MGC 31D1:89F2) to a 9600-byte block = NUM_PLANES x 40 x 60
 *  int16 terrain tiles.  Emits one [GD] line per (plane,row):
 *     [GD] <point> _world_maps[<plane>].y<NN> = csv int16
 *  mirroring ReMoM's gd_dump_world_map so compare-gd-stage0.py diffs them. */
#define OG_WM_PTR_OFF      0x89F2u   /* MGC DGROUP far-ptr slot for _world_maps */
#define OG_WM_PLANES       2
#define OG_WM_WIDTH        60
#define OG_WM_HEIGHT       40
#define OG_WM_PLANE_TILES  (OG_WM_WIDTH * OG_WM_HEIGHT)   /* 2400 */

static void gd_dump_world_map(const char* point) {
    struct og_exe_info* e = &og_exes[OG_SR_MGC_EXE_IDX];
    Bit16u ds      = e->runtime_ds;
    Bit16u blk_off = real_readw(ds, (Bit16u)OG_WM_PTR_OFF);        /* far ptr: off */
    Bit16u blk_seg = real_readw(ds, (Bit16u)(OG_WM_PTR_OFF + 2));  /*          seg */
    int plane, y, x, q;
    char row[OG_WM_WIDTH * 8];
    for (plane = 0; plane < OG_WM_PLANES; plane++) {
        for (y = 0; y < OG_WM_HEIGHT; y++) {
            q = 0;
            for (x = 0; x < OG_WM_WIDTH; x++) {
                unsigned tile = (unsigned)plane * OG_WM_PLANE_TILES
                              + (unsigned)y * OG_WM_WIDTH + (unsigned)x;
                int16_t v = (int16_t)real_readw(blk_seg, (Bit16u)(blk_off + tile * 2));
                q += snprintf(row + q, sizeof(row) - q, x ? ",%d" : "%d", (int)v);
            }
            fprintf(stderr, "[GD] %s _world_maps[%d].y%02d = %s\n", point, plane, y, row);
        }
    }
    /* OOB overrun row p_world_map[1][40][0..59] (int16 offset 4800..4859) -- the
     * south-neighbor Simtex_Autotiling reads at plane-1 row 39.  OG has no
     * over-allocation, so this reads OG's heap just past the _world_maps block;
     * capture it to inject into ReMoM's WORLD_OVERRUN padding so its autotile
     * reads the same bytes OG does (then ReMoM can drop the wp==1&&wy==39 guard). */
    if (strstr(point, "Simtex")) {
        int q2 = 0;
        char orow[OG_WM_WIDTH * 8];
        for (x = 0; x < OG_WM_WIDTH; x++) {
            unsigned tile = (unsigned)OG_WM_PLANES * OG_WM_PLANE_TILES + (unsigned)x;
            int16_t v = (int16_t)real_readw(blk_seg, (Bit16u)(blk_off + tile * 2));
            q2 += snprintf(orow + q2, sizeof(orow) - q2, x ? ",%d" : "%d", (int)v);
        }
        fprintf(stderr, "[GD] %s _world_maps_overrun = %s\n", point, orow);
    }
}

/* -------------------------------------------------------------------- *
 *  _NODES capture (node records) at Generate_Nodes / Rebalance_Node_Types
 *  FN-EXIT.  _NODES is a FAR POINTER in MGC DGROUP (DS:0x89E2, per the dseg
 *  wiki) to NUM_NODES s_NODE records (48 bytes each).  Decoded with the
 *  generic field-map decoder + node_fields, exactly like _players. */
#define OG_NODES_PTR_OFF   0x89E2u   /* MGC DGROUP far-ptr slot for _NODES */
#define OG_NODES_COUNT     30        /* NUM_NODES        */
#define OG_NODE_REC_SIZE   48        /* NODE_RECORD_SIZE */

static void gd_dump_nodes(const char* point) {
    struct og_exe_info* e = &og_exes[OG_SR_MGC_EXE_IDX];
    Bit16u ds      = e->runtime_ds;
    Bit16u blk_off = real_readw(ds, (Bit16u)OG_NODES_PTR_OFF);        /* far ptr: off */
    Bit16u blk_seg = real_readw(ds, (Bit16u)(OG_NODES_PTR_OFF + 2));  /*          seg */
    int i;
    for (i = 0; i < OG_NODES_COUNT; i++) {
        gd_decode_record(blk_seg, (Bit16u)(blk_off + i * OG_NODE_REC_SIZE),
                         node_fields, NODE_FIELD_COUNT, "_NODES", i, point);
    }
}

/* -------------------------------------------------------------------- *
 *  _landmasses capture (landmass-id grid) at Init_Landmasses /
 *  Generate_Landmasses FN-EXIT.  Far pointer in MGC DGROUP (DS:0x89E6) to a
 *  uint8_t[NUM_PLANES*40*60] (1 byte/tile, GET_LANDMASS layout).  Same per-
 *  (plane,row) emit as _world_maps, but uint8. */
#define OG_LM_PTR_OFF   0x89E6u   /* MGC DGROUP far-ptr slot for _landmasses */

static void gd_dump_landmasses(const char* point) {
    struct og_exe_info* e = &og_exes[OG_SR_MGC_EXE_IDX];
    Bit16u ds      = e->runtime_ds;
    Bit16u blk_off = real_readw(ds, (Bit16u)OG_LM_PTR_OFF);
    Bit16u blk_seg = real_readw(ds, (Bit16u)(OG_LM_PTR_OFF + 2));
    int plane, y, x, q;
    char row[OG_WM_WIDTH * 5];
    for (plane = 0; plane < OG_WM_PLANES; plane++) {
        for (y = 0; y < OG_WM_HEIGHT; y++) {
            q = 0;
            for (x = 0; x < OG_WM_WIDTH; x++) {
                unsigned tile = (unsigned)plane * OG_WM_PLANE_TILES
                              + (unsigned)y * OG_WM_WIDTH + (unsigned)x;
                int v = (int)real_readb(blk_seg, (Bit16u)(blk_off + tile));  /* uint8/tile */
                q += snprintf(row + q, sizeof(row) - q, x ? ",%d" : "%d", v);
            }
            fprintf(stderr, "[GD] %s _landmasses[%d].y%02d = %s\n", point, plane, y, row);
        }
    }
}

/* -------------------------------------------------------------------- *
 *  _map_square_terrain_specials capture (1 byte/tile, terrain-special id) at
 *  Generate_Terrain_Specials FN-EXIT.  Far pointer in MGC DGROUP (DS:0x89CE,
 *  the slot immediately before _CITIES at 0x89D2) to uint8_t[NUM_PLANES*40*60].
 *  Same per-(plane,row) emit as _landmasses; compare-gd-stage0.py diffs it. */
#define OG_TS_PTR_OFF   0x89CEu   /* MGC DGROUP far-ptr slot for _map_square_terrain_specials */

static void gd_dump_terrain_specials(const char* point) {
    struct og_exe_info* e = &og_exes[OG_SR_MGC_EXE_IDX];
    Bit16u ds      = e->runtime_ds;
    Bit16u blk_off = real_readw(ds, (Bit16u)OG_TS_PTR_OFF);
    Bit16u blk_seg = real_readw(ds, (Bit16u)(OG_TS_PTR_OFF + 2));
    int plane, y, x, q;
    char row[OG_WM_WIDTH * 5];
    for (plane = 0; plane < OG_WM_PLANES; plane++) {
        for (y = 0; y < OG_WM_HEIGHT; y++) {
            q = 0;
            for (x = 0; x < OG_WM_WIDTH; x++) {
                unsigned tile = (unsigned)plane * OG_WM_PLANE_TILES
                              + (unsigned)y * OG_WM_WIDTH + (unsigned)x;
                int v = (int)real_readb(blk_seg, (Bit16u)(blk_off + tile));  /* uint8/tile */
                q += snprintf(row + q, sizeof(row) - q, x ? ",%d" : "%d", v);
            }
            fprintf(stderr, "[GD] %s _map_square_terrain_specials[%d].y%02d = %s\n",
                    point, plane, y, row);
        }
    }
}

/* -------------------------------------------------------------------- *
 *  _map_square_flags capture (1 byte/tile, per-square flag bits) at
 *  Generate_Roads FN-EXIT.  Far pointer in MGC DGROUP (DS:0x89CA, the slot
 *  immediately before _map_square_terrain_specials at 0x89CE) to
 *  uint8_t[NUM_PLANES*40*60].  Same per-(plane,row) emit as _landmasses. */
#define OG_MSF_PTR_OFF   0x89CAu   /* MGC DGROUP far-ptr slot for _map_square_flags */

static void gd_dump_map_square_flags(const char* point) {
    struct og_exe_info* e = &og_exes[OG_SR_MGC_EXE_IDX];
    Bit16u ds      = e->runtime_ds;
    Bit16u blk_off = real_readw(ds, (Bit16u)OG_MSF_PTR_OFF);
    Bit16u blk_seg = real_readw(ds, (Bit16u)(OG_MSF_PTR_OFF + 2));
    int plane, y, x, q;
    char row[OG_WM_WIDTH * 5];
    for (plane = 0; plane < OG_WM_PLANES; plane++) {
        for (y = 0; y < OG_WM_HEIGHT; y++) {
            q = 0;
            for (x = 0; x < OG_WM_WIDTH; x++) {
                unsigned tile = (unsigned)plane * OG_WM_PLANE_TILES
                              + (unsigned)y * OG_WM_WIDTH + (unsigned)x;
                int v = (int)real_readb(blk_seg, (Bit16u)(blk_off + tile));  /* uint8/tile */
                q += snprintf(row + q, sizeof(row) - q, x ? ",%d" : "%d", v);
            }
            fprintf(stderr, "[GD] %s _map_square_flags[%d].y%02d = %s\n",
                    point, plane, y, row);
        }
    }
}

/* -------------------------------------------------------------------- *
 *  _square_explored capture (1 byte/tile, per-square explored flag) at
 *  Init_Square_Explored FN-EXIT.  Far pointer in MGC DGROUP (DS:0x89C6, the
 *  slot immediately before _map_square_flags at 0x89CA) to
 *  uint8_t[NUM_PLANES*40*60].  Same per-(plane,row) emit as _landmasses. */
#define OG_SE_PTR_OFF   0x89C6u   /* MGC DGROUP far-ptr slot for _square_explored */

static void gd_dump_square_explored(const char* point) {
    struct og_exe_info* e = &og_exes[OG_SR_MGC_EXE_IDX];
    Bit16u ds      = e->runtime_ds;
    Bit16u blk_off = real_readw(ds, (Bit16u)OG_SE_PTR_OFF);
    Bit16u blk_seg = real_readw(ds, (Bit16u)(OG_SE_PTR_OFF + 2));
    int plane, y, x, q;
    char row[OG_WM_WIDTH * 5];
    for (plane = 0; plane < OG_WM_PLANES; plane++) {
        for (y = 0; y < OG_WM_HEIGHT; y++) {
            q = 0;
            for (x = 0; x < OG_WM_WIDTH; x++) {
                unsigned tile = (unsigned)plane * OG_WM_PLANE_TILES
                              + (unsigned)y * OG_WM_WIDTH + (unsigned)x;
                int v = (int)real_readb(blk_seg, (Bit16u)(blk_off + tile));  /* uint8/tile */
                q += snprintf(row + q, sizeof(row) - q, x ? ",%d" : "%d", v);
            }
            fprintf(stderr, "[GD] %s _square_explored[%d].y%02d = %s\n",
                    point, plane, y, row);
        }
    }
}

/* -------------------------------------------------------------------- *
 *  movement_mode_cost_maps capture at Movement_Mode_Cost_Maps FN-EXIT.  Far
 *  pointer in MGC DGROUP (DS:0x89C2, the slot before _square_explored at
 *  0x89C6) to s_MOVE_MODE_COST_MAPS[NUM_PLANES].  Each plane element is 6
 *  movement modes (UU_MvMd, walking, forester, mountaineer, swimming, sailing),
 *  each an int8_t moves2[WORLD_SIZE] (one plane of per-square move cost).  Emits
 *  one [GD] line per (plane,mode,row); compare-gd diffs it, analyze summarizes. */
#define OG_MMCM_PTR_OFF       0x89C2u   /* MGC DGROUP far-ptr slot for movement_mode_cost_maps */
#define OG_MMCM_PLANE_STRIDE  0x3840u   /* sizeof s_MOVE_MODE_COST_MAPS (6 * 0x960) */

static const struct { const char* name; Bit16u off; } og_mmcm_modes[] = {
    { "UU_MvMd",     0x0000u },
    { "walking",     0x0960u },
    { "forester",    0x12C0u },
    { "mountaineer", 0x1C20u },
    { "swimming",    0x2580u },
    { "sailing",     0x2EE0u },
};

static void gd_dump_movement_mode_cost_maps(const char* point) {
    struct og_exe_info* e = &og_exes[OG_SR_MGC_EXE_IDX];
    Bit16u ds      = e->runtime_ds;
    Bit16u blk_off = real_readw(ds, (Bit16u)OG_MMCM_PTR_OFF);
    Bit16u blk_seg = real_readw(ds, (Bit16u)(OG_MMCM_PTR_OFF + 2));
    int nmodes = (int)(sizeof(og_mmcm_modes) / sizeof(og_mmcm_modes[0]));
    int plane, m, y, x, q;
    char row[OG_WM_WIDTH * 5];
    for (plane = 0; plane < OG_WM_PLANES; plane++) {
        for (m = 0; m < nmodes; m++) {
            Bit32u mbase = (Bit32u)plane * OG_MMCM_PLANE_STRIDE + og_mmcm_modes[m].off;
            for (y = 0; y < OG_WM_HEIGHT; y++) {
                q = 0;
                for (x = 0; x < OG_WM_WIDTH; x++) {
                    /* Normalize seg:off -- the block is 28800 bytes, so a far
                     * offset can exceed 16 bits; carry the high bits into seg. */
                    Bit32u abs = (Bit32u)blk_off + mbase
                               + (Bit32u)y * OG_WM_WIDTH + (Bit32u)x;
                    Bit16u rseg = (Bit16u)(blk_seg + (Bit16u)(abs >> 4));
                    Bit16u roff = (Bit16u)(abs & 0xFu);
                    int v = (int)(Bit8s)real_readb(rseg, roff);   /* int8/square */
                    q += snprintf(row + q, sizeof(row) - q, x ? ",%d" : "%d", v);
                }
                fprintf(stderr, "[GD] %s movement_mode_cost_maps[%d].%s_y%02d = %s\n",
                        point, plane, og_mmcm_modes[m].name, y, row);
            }
        }
    }
}

/* -------------------------------------------------------------------- *
 *  UU_TBL_1 / UU_TBL_2 capture at CRP_NEWG_CreatePathGrids FN-EXIT.  Each is a
 *  far pointer in MGC DGROUP (UU_TBL_2 @ DS:0x89EA, UU_TBL_1 @ DS:0x89EE -- the
 *  two slots between _landmasses 0x89E6 and _world_maps 0x89F2) to a path-grid
 *  block.  RECORD_SIZE 2*96 = NUM_PLANES x 96 uint8 (SAMB_ptr = unsigned char*).
 *  Emits one [GD] line per (table,plane); compare-gd diffs it. */
#define OG_UUTBL1_PTR_OFF   0x89EEu   /* MGC DGROUP far-ptr slot for UU_TBL_1 */
#define OG_UUTBL2_PTR_OFF   0x89EAu   /* MGC DGROUP far-ptr slot for UU_TBL_2 */
#define OG_UUTBL_PLANE_LEN  96        /* uint8 entries per plane */

static void gd_dump_one_uu_tbl(const char* point, const char* name, Bit16u ptr_off) {
    struct og_exe_info* e = &og_exes[OG_SR_MGC_EXE_IDX];
    Bit16u ds      = e->runtime_ds;
    Bit16u blk_off = real_readw(ds, ptr_off);
    Bit16u blk_seg = real_readw(ds, (Bit16u)(ptr_off + 2));
    int plane, i, q;
    char row[OG_UUTBL_PLANE_LEN * 5];
    for (plane = 0; plane < OG_WM_PLANES; plane++) {
        q = 0;
        for (i = 0; i < OG_UUTBL_PLANE_LEN; i++) {
            Bit16u o = (Bit16u)(blk_off + plane * OG_UUTBL_PLANE_LEN + i);
            int v = (int)real_readb(blk_seg, o);   /* uint8/entry */
            q += snprintf(row + q, sizeof(row) - q, i ? ",%d" : "%d", v);
        }
        fprintf(stderr, "[GD] %s %s[%d].vals = %s\n", point, name, plane, row);
    }
}

static void gd_dump_uu_tbls(const char* point) {
    gd_dump_one_uu_tbl(point, "UU_TBL_1", OG_UUTBL1_PTR_OFF);
    gd_dump_one_uu_tbl(point, "UU_TBL_2", OG_UUTBL2_PTR_OFF);
}

/* -------------------------------------------------------------------- *
 *  _TOWERS capture (s_TOWER records) at Generate_Towers FN-EXIT.  Far ptr
 *  in MGC DGROUP (DS:0x89DA) to NUM_TOWERS s_TOWER records (4 bytes each).
 *  Decoded with the generic field-map decoder + tower_fields, like _NODES. */
#define OG_TOWERS_PTR_OFF   0x89DAu   /* MGC DGROUP far-ptr slot for _TOWERS */
#define OG_TOWERS_COUNT     6         /* NUM_TOWERS        */
#define OG_TOWER_REC_SIZE   4         /* TOWER_RECORD_SIZE */

static void gd_dump_towers(const char* point) {
    struct og_exe_info* e = &og_exes[OG_SR_MGC_EXE_IDX];
    Bit16u ds      = e->runtime_ds;
    Bit16u blk_off = real_readw(ds, (Bit16u)OG_TOWERS_PTR_OFF);
    Bit16u blk_seg = real_readw(ds, (Bit16u)(OG_TOWERS_PTR_OFF + 2));
    int i;
    for (i = 0; i < OG_TOWERS_COUNT; i++) {
        gd_decode_record(blk_seg, (Bit16u)(blk_off + i * OG_TOWER_REC_SIZE),
                         tower_fields, TOWER_FIELD_COUNT, "_TOWERS", i, point);
    }
}

/* -------------------------------------------------------------------- *
 *  _LAIRS capture (s_LAIR records) at Generate_Lairs FN-EXIT.  Far ptr in
 *  MGC DGROUP (DS:0x89D6) to NUM_LAIRS_102 s_LAIR records (24 bytes each).
 *  Decoded with the generic field-map decoder + lair_fields, like _TOWERS.
 *  Dumps the full 102-record array (matches the save's LAIRS_RECORD_SIZE =
 *  102*24); the game's NUM_LAIRS is 99, so records 99..101 are the unused,
 *  zero-allocated tail and should match on both sides. */
#define OG_LAIRS_PTR_OFF    0x89D6u   /* MGC DGROUP far-ptr slot for _LAIRS */
#define OG_LAIRS_COUNT      102       /* NUM_LAIRS_102 (full allocated array) */
#define OG_LAIR_REC_SIZE    24        /* LAIR_RECORD_SIZE  */

static void gd_dump_lairs(const char* point) {
    struct og_exe_info* e = &og_exes[OG_SR_MGC_EXE_IDX];
    Bit16u ds      = e->runtime_ds;
    Bit16u blk_off = real_readw(ds, (Bit16u)OG_LAIRS_PTR_OFF);
    Bit16u blk_seg = real_readw(ds, (Bit16u)(OG_LAIRS_PTR_OFF + 2));
    int i;
    for (i = 0; i < OG_LAIRS_COUNT; i++) {
        gd_decode_record(blk_seg, (Bit16u)(blk_off + i * OG_LAIR_REC_SIZE),
                         lair_fields, LAIR_FIELD_COUNT, "_LAIRS", i, point);
    }
}

/* -------------------------------------------------------------------- *
 *  _CITIES capture (s_CITY records).  Far ptr in MGC DGROUP (DS:0x89D2) to
 *  NUM_CITIES s_CITY records (114 bytes each).  Dumps the full 100-record
 *  array (matches the save's CITIES_RECORD_SIZE = 100*114); records past the
 *  cities actually placed are unwritten allocator garbage and should be
 *  masked in the compare (like the _LAIRS Item_Values tail). */
#define OG_CITIES_PTR_OFF   0x89D2u   /* MGC DGROUP far-ptr slot for _CITIES */
#define OG_CITIES_COUNT     100       /* NUM_CITIES        */
#define OG_CITIES_NUM_OFF   0x87CAu   /* MGC DGROUP offset of int16_t _cities (placed count);
                                       * empirically 0x87CA (reads 5 @home, 27 @neutral) --
                                       * the supplied 0x87CC is a constant 2 (off by one word). */
#define OG_CITY_REC_SIZE    114       /* CITY_RECORD_SIZE  */
/* WIZARDS.EXE DGROUP (36AA) slots for the post-Load_SAVE_GAM capture (wiki
 * "Data-Segment Memory Layout").  Same s_CITY layout/count, different DGROUP. */
#define OG_WZD_CITIES_PTR_OFF 0x9CBCu /* WZD DGROUP far-ptr slot for _CITIES   */
#define OG_WZD_CITIES_NUM_OFF 0xBD94u /* WZD DGROUP offset of int16_t _cities  */

/* DGROUP-parameterized so the same dump serves MAGIC New-Game (MGC DGROUP/MGC
 * offsets) and the WIZARDS post-load snapshot (WZD DGROUP/WZD offsets). */
static void gd_dump_cities_at(Bit16u ds, Bit16u ptr_off, Bit16u num_off,
                              const char* point) {
    Bit16u blk_off = real_readw(ds, ptr_off);
    Bit16u blk_seg = real_readw(ds, (Bit16u)(ptr_off + 2));
    int i;
    /* placed-city count -> compare-gd masks _CITIES[i >= _cities] (unplaced tail). */
    fprintf(stderr, "[GD] %s _cities = %d\n",
            point, (int)(int16_t)real_readw(ds, num_off));
    for (i = 0; i < OG_CITIES_COUNT; i++) {
        gd_decode_record(blk_seg, (Bit16u)(blk_off + i * OG_CITY_REC_SIZE),
                         city_fields, CITY_FIELD_COUNT, "_CITIES", i, point);
    }
}
static void gd_dump_cities(const char* point) {   /* MAGIC New-Game wrapper */
    gd_dump_cities_at(og_exes[OG_SR_MGC_EXE_IDX].runtime_ds,
                      OG_CITIES_PTR_OFF, OG_CITIES_NUM_OFF, point);
}

/* -------------------------------------------------------------------- *
 *  _UNITS capture (s_UNIT records).  Far ptr in MGC DGROUP (DS:0x68F8) to
 *  NUM_UNITS s_UNIT records (32 bytes each).  Starting city-garrison units are
 *  created inside Generate_Home_Cities / Generate_Neutral_Cities (via
 *  Create_Unit_NewGame), so _UNITS is captured at those same FN-EXITs.  Records
 *  past the active count (_units) are uninitialised garbage, so the count is
 *  emitted first for compare-gd to mask the inactive tail. */
#define OG_UNITS_PTR_OFF    0x68F8u   /* MGC DGROUP far-ptr slot for _UNITS         */
#define OG_UNITS_COUNT      1009      /* NUM_UNITS                                  */
#define OG_UNITS_NUM_OFF    0x87C8u   /* MGC DGROUP offset of int16_t _units (count); WZD dseg:BD92 */
#define OG_UNIT_REC_SIZE    32        /* sizeof(struct s_UNIT) = 0x20              */

static void gd_dump_units(const char* point) {
    struct og_exe_info* e = &og_exes[OG_SR_MGC_EXE_IDX];
    Bit16u ds      = e->runtime_ds;
    Bit16u blk_off = real_readw(ds, (Bit16u)OG_UNITS_PTR_OFF);
    Bit16u blk_seg = real_readw(ds, (Bit16u)(OG_UNITS_PTR_OFF + 2));
    int i;
    /* active-unit count -> compare-gd masks _UNITS[i >= _units] (inactive tail). */
    fprintf(stderr, "[GD] %s _units = %d\n",
            point, (int)(int16_t)real_readw(ds, (Bit16u)OG_UNITS_NUM_OFF));
    for (i = 0; i < OG_UNITS_COUNT; i++) {
        gd_decode_record(blk_seg, (Bit16u)(blk_off + i * OG_UNIT_REC_SIZE),
                         unit_fields, UNIT_FIELD_COUNT, "_UNITS", i, point);
    }
}

/* -------------------------------------------------------------------- *
 *  _HEROES2 capture: per-player heroes.  Unlike _CITIES/_UNITS (one far ptr to
 *  a contiguous array), _HEROES2 is an ARRAY of NUM_PLAYERS far pointers in MGC
 *  DGROUP (DS:0x65AC), each -> a s_HEROES { s_HERO heroes[35] } (420 bytes).
 *  Iterate players, read each far ptr, dump the flattened heroes_fields. */
#define OG_HEROES_PTR_OFF   0x65ACu   /* MGC DGROUP: NUM_PLAYERS far ptrs to s_HEROES; WZD dseg:9232 */
#define OG_HEROES_NPLAYERS  6         /* NUM_PLAYERS */

static void gd_dump_heroes(const char* point) {
    struct og_exe_info* e = &og_exes[OG_SR_MGC_EXE_IDX];
    Bit16u ds = e->runtime_ds;
    int p;
    for (p = 0; p < OG_HEROES_NPLAYERS; p++) {
        Bit16u slot = (Bit16u)(OG_HEROES_PTR_OFF + p * 4);
        Bit16u boff = real_readw(ds, slot);
        Bit16u bseg = real_readw(ds, (Bit16u)(slot + 2));
        if (bseg == 0 && boff == 0) continue;   /* unallocated player slot */
        gd_decode_record(bseg, boff, heroes_fields, HEROES_FIELD_COUNT,
                         "_HEROES2", p, point);
    }
}

/* -------------------------------------------------------------------- *
 *  _ITEMS capture: single far ptr in MGC DGROUP (DS:0x64B0) to NUM_ITEMS
 *  contiguous s_ITEM records (50 bytes each), all loaded from ITEMDATA.LBX by
 *  Initialize_Items -- the whole array is compared (no placement-count tail). */
#define OG_ITEMS_PTR_OFF   0x64B0u   /* MGC DGROUP far-ptr slot for _ITEMS; WZD dseg:9136 */
#define OG_ITEMS_COUNT     138       /* NUM_ITEMS                       */
#define OG_ITEM_REC_SIZE   50        /* sizeof(struct s_ITEM) = 0x32    */

static void gd_dump_items(const char* point) {
    struct og_exe_info* e = &og_exes[OG_SR_MGC_EXE_IDX];
    Bit16u ds      = e->runtime_ds;
    Bit16u blk_off = real_readw(ds, (Bit16u)OG_ITEMS_PTR_OFF);
    Bit16u blk_seg = real_readw(ds, (Bit16u)(OG_ITEMS_PTR_OFF + 2));
    int i;
    for (i = 0; i < OG_ITEMS_COUNT; i++) {
        gd_decode_record(blk_seg, (Bit16u)(blk_off + i * OG_ITEM_REC_SIZE),
                         item_fields, ITEM_FIELD_COUNT, "_ITEMS", i, point);
    }
}

/* ==================================================================== *
 *  POST-LOAD (WIZARDS DGROUP 36AA) game-data capture -- the 300-series.
 *  New Game saves slot 8; WIZARDS loads it; we snapshot every loaded array at
 *  Load_SAVE_GAM on-return (gd_wzd_landmark_probe @ 0x141A7).  Same record
 *  layouts/counts as the MGC New-Game dumps, different DGROUP + offsets (wiki
 *  "Data-Segment Memory Layout").  Numbered 3NN in save-load order.
 * ==================================================================== */
#define OG_WZD_HEROES_PTR_OFF  0x9232u  /* NUM_PLAYERS far ptrs -> s_HEROES        */
#define OG_WZD_PLAYERS_OFF     0x9ECAu  /* DGROUP-resident s_WIZARD[6] (not a ptr) */
#define OG_WZD_NODES_PTR_OFF   0x9CCCu
#define OG_WZD_TOWERS_PTR_OFF  0x9CC4u
#define OG_WZD_LAIRS_PTR_OFF   0x9CC0u
#define OG_WZD_ITEMS_PTR_OFF   0x9136u
#define OG_WZD_UNITS_PTR_OFF   0x9EC2u
#define OG_WZD_UNITS_NUM_OFF   0xBD92u  /* int16_t _units (active count) */
/* UWATCH: which _UNITS slot/field the per-instruction owner_idx value-change
 * watcher tracks (see gd_wzd_landmark_probe).  Change these to retarget 49/50. */
#define OG_WATCH_UNIT_IDX      48       /* _UNITS slot to watch                    */
#define OG_WATCH_OWNER_OFF     0x03u    /* s_UNIT.owner_idx field byte offset      */
#define OG_WZD_FORTRESSES_PTR_OFF 0x9CC8u
#define OG_FORTRESSES_COUNT    6        /* NUM_FORTRESSES          */
#define OG_FORTRESS_REC_SIZE   4        /* sizeof(s_FORTRESS)      */
#define OG_WZD_WM_PTR_OFF      0x9CDCu  /* _world_maps  (int16 tilemap)            */
#define OG_WZD_LM_PTR_OFF      0x9CD0u  /* _landmasses  (uint8 tilemap)            */
#define OG_WZD_TS_PTR_OFF      0x9CB8u  /* _map_square_terrain_specials (uint8)    */
#define OG_WZD_MSF_PTR_OFF     0x9CB4u  /* _map_square_flags            (uint8)    */
/* (OG_WZD_CITIES_PTR_OFF 0x9CBC / OG_WZD_CITIES_NUM_OFF 0xBD94 declared above.) */

/* DGROUP-parameterized far-ptr record-array dump (NODES/TOWERS/LAIRS/ITEMS/UNITS).
 * num_off!=0 emits the dynamic count scalar (num_name) for the compare tail mask. */
static void gd_dump_recs_at(Bit16u ds, Bit16u ptr_off, Bit16u num_off, const char* num_name,
                            const gd_field_t* fields, int fcount, int recsize, int count,
                            const char* name, const char* point) {
    Bit16u blk_off = real_readw(ds, ptr_off);
    Bit16u blk_seg = real_readw(ds, (Bit16u)(ptr_off + 2));
    int i;
    if (num_off && num_name)
        fprintf(stderr, "[GD] %s %s = %d\n", point, num_name,
                (int)(int16_t)real_readw(ds, num_off));
    for (i = 0; i < count; i++)
        gd_decode_record(blk_seg, (Bit16u)(blk_off + i * recsize),
                         fields, fcount, name, i, point);
}

/* _HEROES2: array of NUM_PLAYERS far ptrs at ptr_off, each -> s_HEROES (flattened). */
static void gd_dump_heroes_at(Bit16u ds, Bit16u ptr_off, const char* point) {
    int p;
    for (p = 0; p < OG_HEROES_NPLAYERS; p++) {
        Bit16u slot = (Bit16u)(ptr_off + p * 4);
        Bit16u boff = real_readw(ds, slot);
        Bit16u bseg = real_readw(ds, (Bit16u)(slot + 2));
        if (bseg == 0 && boff == 0) continue;   /* unallocated player slot */
        gd_decode_record(bseg, boff, heroes_fields, HEROES_FIELD_COUNT,
                         "_HEROES2", p, point);
    }
}

/* _players: DGROUP-resident s_WIZARD[NUM_PLAYERS] at base_off (not a far ptr). */
static void gd_dump_players_at(Bit16u ds, Bit16u base_off, const char* point) {
    int p;
    for (p = 0; p < OG_SR_NUM_PLAYERS; p++) {
        Bit16u rbase = (Bit16u)(base_off + (Bit32u)p * OG_SR_RECORD_SIZE);
        gd_decode_record(ds, rbase, wizard_fields, WIZARD_FIELD_COUNT,
                         "_players", p, point);
    }
}

/* DGROUP-parameterized raw tilemap dump (per (plane,row): NUM_PLANES*40*60 cells).
 * elem=2 -> int16 (_world_maps), elem=1 -> uint8 (_landmasses/_terrain_specials/
 * _map_square_flags).  Matches the MGC New-Game tilemap dumps' [GD] line format. */
static void gd_dump_tiles_at(Bit16u ds, Bit16u ptr_off, const char* name,
                             int elem, const char* point) {
    Bit16u blk_off = real_readw(ds, ptr_off);
    Bit16u blk_seg = real_readw(ds, (Bit16u)(ptr_off + 2));
    int plane, y, x, q;
    char row[OG_WM_WIDTH * 8];
    for (plane = 0; plane < OG_WM_PLANES; plane++) {
        for (y = 0; y < OG_WM_HEIGHT; y++) {
            q = 0;
            for (x = 0; x < OG_WM_WIDTH; x++) {
                unsigned tile = (unsigned)plane * OG_WM_PLANE_TILES
                              + (unsigned)y * OG_WM_WIDTH + (unsigned)x;
                int v = (elem == 2)
                    ? (int)(int16_t)real_readw(blk_seg, (Bit16u)(blk_off + tile * 2))
                    : (int)real_readb(blk_seg, (Bit16u)(blk_off + tile));
                q += snprintf(row + q, sizeof(row) - q, x ? ",%d" : "%d", v);
            }
            fprintf(stderr, "[GD] %s %s[%d].y%02d = %s\n", point, name, plane, y, row);
        }
    }
}

/* CAPTURES AT-RETURN */
/* Terrain-gen FN-EXITs to capture the map at, to BISECT where the map first
 * diverges (Landmasses -> BaseTerrain -> ClimateTerrain).  off = ovr051 entry
 * offset (func_ea - seg_ea 0x43580); func = resolver name to confirm (no
 * prologue sig); point = [GD] label (must match the ReMoM gd_dump_world_map).
 * gd_wm_capfn_t is defined above (shared with the _players gd_capfns list). */
static const struct gd_wm_capfn_t gd_wm_capfns[] = {
    { 0x1E09, "Generate_Landmasses",                       "104_Landmasses_W"       },
    { 0x1B76, "Translate_Heightmap_To_Base_Terrain_Types", "105_BaseTerrain_W"      },
    { 0x173B, "Generate_Climate_Terrain_Types",            "106_ClimateTerrain_W"   },
    { 0x0344, "Extend_Islands",                            "112_Extend_Islands_W"   },
    { 0x8A13, "Generate_Terrain_Specials",                 "119_Generate_Terrain_Specials_W" },
    { 0x40BF, "Simtex_Autotiling",                         "121_Simtex_Autotiling_W"},
    { 0x56E3, "Generate_River",                            "122_Generate_River_W"   },
    { 0x5B8F, "River_Autotile",                            "123_River_Autotile_W"   },
    { 0x3D47, "Desert_Autotile",                           "125_Desert_Autotile_W"  },
    { 0x52A1, "Shuffle_Terrains",                          "126_Shuffle_Terrains_W" },
    { 0x98B5, "Animate_Oceans",                            "130_Animate_Oceans_W"   },
};
struct gd_wm_pending_t { int active; Bit16u cs; Bit16u ret_ip; int fn; };
static struct gd_wm_pending_t gd_wm_pend[4];

/* _NODES FN-EXIT captures (same resolver-confirmed near-call return trap). */
static const struct gd_wm_capfn_t gd_node_capfns[] = {
    { 0x20C5, "Generate_Nodes",       "107_Generate_Nodes_N"       },
    { 0x148E, "Rebalance_Node_Types", "108_Rebalance_Node_Types_N" },
};
static struct gd_wm_pending_t gd_node_pend[4];

/* _landmasses FN-EXIT captures (same trap; entries FN_DISC-confirmed). */
static const struct gd_wm_capfn_t gd_landmass_capfns[] = {
    { 0x1450, "Init_Landmasses",     "102_Init_Landmasses_L"     },
    { 0x1E09, "Generate_Landmasses", "103_Generate_Landmasses_L" },
    { 0x0344, "Extend_Islands", "111_Extend_Islands_L"           },
    { 0x3D47, "Desert_Autotile", "124_Desert_Autotile_L"         },
};
static struct gd_wm_pending_t gd_landmass_pend[4];

/* _TOWERS FN-EXIT capture (Generate_Towers entry 0x0715, FN_DISC-confirmed). */
static const struct gd_wm_capfn_t gd_tower_capfns[] = {
    { 0x0715, "Generate_Towers", "110_Generate_Towers_T" },
};
static struct gd_wm_pending_t gd_tower_pend[4];

/* _LAIRS FN-EXIT capture (Generate_Lairs entry 0x2E86, from MGC-overlay-table:
 * func_ea 0x46406 - seg_ea 0x43580).  FN-EXIT = _LAIRS after the whole lair
 * loop, == ReMoM's gd_dump_lairs("12_Generate_Lairs_LR"). */
static const struct gd_wm_capfn_t gd_lair_capfns[] = {
    { 0x2E86, "Generate_Lairs", "113_Generate_Lairs_LR"                               },
    { 0x02B9, "Set_Upper_Lair_Guardian_Count", "131_Set_Upper_Lair_Guardian_Count_LR" },
};
static struct gd_wm_pending_t gd_lair_pend[4];

/* _CITIES FN-EXIT captures.  Generate_Home_Cities (entry 0x08CF) is called once
 * = _CITIES after the home cities.  Generate_Neutral_Cities (entry 0x73C2) is
 * called per plane (2x), last-write-wins = _CITIES after all neutral cities.
 * Offsets from MGC-overlay-table (func_ea - seg_ea 0x43580). */
static const struct gd_wm_capfn_t gd_city_capfns[] = {
    { 0x08CF, "Generate_Home_Cities",    "114_Generate_Home_Cities_C"    },
    { 0x73C2, "Generate_Neutral_Cities", "116_Generate_Neutral_Cities_C" },
};
static struct gd_wm_pending_t gd_city_pend[4];

/* _UNITS FN-EXIT captures.  Generate_Home_Cities / Generate_Neutral_Cities also
 * build the starting city-garrison units (Create_Unit_NewGame), so _UNITS is
 * captured at the same two FN-EXITs as _CITIES (one point number later). */
static const struct gd_wm_capfn_t gd_unit_capfns[] = {
    { 0x08CF, "Generate_Home_Cities",    "115_Generate_Home_Cities_U"    },
    { 0x73C2, "Generate_Neutral_Cities", "117_Generate_Neutral_Cities_U" },
};
static struct gd_wm_pending_t gd_unit_pend[4];

/* _HEROES2 FN-EXIT capture at Init_Heroes (same point/return as 201_Init_Heroes_P,
 * but the per-player hero records instead of _players). */
static const struct gd_wm_capfn_t gd_heroes_capfns[] = {
    { 0x246D, "Init_Heroes", "202_Init_Heroes_HE" },
};
static struct gd_wm_pending_t gd_heroes_pend[4];

/* _ITEMS FN-EXIT capture at Initialize_Items (ovr056 entry 0x38E, from
 * MGC-overlay-table: func_ea 0x5266E - seg_ea 0x522E0).  Loads ITEMDATA.LBX. */
static const struct gd_wm_capfn_t gd_item_capfns[] = {
    { 0x038E, "Initialize_Items", "200_Initialize_Items_I" },
};
static struct gd_wm_pending_t gd_item_pend[4];

/* _map_square_terrain_specials FN-EXIT capture.  Generate_Terrain_Specials
 * (entry 0x8A13, from MGC-overlay-table: func_ea 0x4BF93 - seg_ea 0x43580) runs
 * once per plane (2x); last-write-wins per point = the full map after both. */
static const struct gd_wm_capfn_t gd_ts_capfns[] = {
    { 0x8A13, "Generate_Terrain_Specials", "118_Generate_Terrain_Specials_TS" },
};
static struct gd_wm_pending_t gd_ts_pend[4];

/* _map_square_flags FN-EXIT capture.  Generate_Roads (entry 0x84D3, from
 * MGC-overlay-table: func_ea 0x4BA53 - seg_ea 0x43580) runs once per plane
 * (2x); last-write-wins per point = the full flags map after both. */
static const struct gd_wm_capfn_t gd_msf_capfns[] = {
    { 0x84D3, "Generate_Roads", "120_Generate_Roads_F" },
};
static struct gd_wm_pending_t gd_msf_pend[4];

/* _square_explored FN-EXIT capture.  Init_Square_Explored (entry 0x8F7A, from
 * MGC-overlay-table: func_ea 0x4C4FA - seg_ea 0x43580) is called once. */
static const struct gd_wm_capfn_t gd_se_capfns[] = {
    { 0x8F7A, "Init_Square_Explored", "129_Init_Square_Explored_E" },
};
static struct gd_wm_pending_t gd_se_pend[4];

/* movement_mode_cost_maps FN-EXIT capture.  Movement_Mode_Cost_Maps (entry
 * 0x921C, from MGC-overlay-table: func_ea 0x4C79C - seg_ea 0x43580) runs once
 * per plane (2x); last-write-wins per point = both planes after the second. */
static const struct gd_wm_capfn_t gd_mmcm_capfns[] = {
    { 0x921C, "Movement_Mode_Cost_Maps", "127_Movement_Mode_Cost_Maps_M" },
};
static struct gd_wm_pending_t gd_mmcm_pend[4];

/* UU_TBL_1 / UU_TBL_2 FN-EXIT capture.  CRP_NEWG_CreatePathGrids__WIP (entry
 * 0x89A4, from MGC-overlay-table: func_ea 0x4BF24 - seg_ea 0x43580) runs once
 * per plane (2x); last-write-wins per point = both tables after the second. */
static const struct gd_wm_capfn_t gd_uut_capfns[] = {
    { 0x89A4, "CRP_NEWG_CreatePathGrids", "128_CRP_NEWG_CreatePathGrids_U" },
};
static struct gd_wm_pending_t gd_uut_pend[4];

/* CAPTURES ON-ENTRY*/
/* ENTRY captures: dump the map the instant the function is entered (no return
 * trap needed).  Generate_Towers entry = the map right BEFORE the tower loop,
 * == ReMoM's gd_dump_world_map("PreTowers") after Rebalance_Node_Types. */
static const struct gd_wm_capfn_t gd_wm_entry_capfns[] = {
    { 0x0715, "Generate_Towers", "109_PreTowers_W" },
};

/* ==================================================================== *
 *  FUNCTION ENTRY/EXIT ADDRESS DISCOVERY
 * ==================================================================== *
 *  Build the capture table from OBSERVED runtime addresses, not computed
 *  offsets.  For every unique near (E8) / far (9A) CALL site in MAGIC, log
 *  the caller, the TARGET entry cs:ip (the function's real runtime entry),
 *  and the RETURN cs:ip (the exit address).  The raw addresses come from the
 *  actual CALL instruction, so they're correct even if og_describe's name is
 *  off.  De-duped per call site.  Grep [FN_DISC] for a function name (or scan
 *  the entry addresses) to get its exact entry + the caller's return. */
#define GD_FN_DISC 1
static void gd_fn_discover(Bit16u cs_val, Bit16u ip_val) {
#if GD_FN_DISC
    if (og_active_exe_idx != OG_SR_MGC_EXE_IDX) return;
    Bit8u op = real_readb(cs_val, ip_val);
    if (op != 0xE8 && op != 0x9A) return;     /* near rel16 / far ptr16:16 */

    static Bit32u seen[8192];                 /* open-addressing set; 0 = empty */
    Bit32u key = ((Bit32u)cs_val << 16) | ip_val;
    Bit32u h = (key * 2654435761u) >> 19;     /* 8192 slots */
    while (seen[h]) { if (seen[h] == key) return; h = (h + 1) & 8191u; }
    seen[h] = key;

    Bit16u tgt_cs, tgt_ip, ret_ip;
    if (op == 0xE8) {
        int16_t rel = (int16_t)real_readw(cs_val, (Bit16u)(ip_val + 1));
        tgt_cs = cs_val;
        tgt_ip = (Bit16u)((int)ip_val + 3 + rel);
        ret_ip = (Bit16u)(ip_val + 3);
    } else {
        tgt_ip = real_readw(cs_val, (Bit16u)(ip_val + 1));
        tgt_cs = real_readw(cs_val, (Bit16u)(ip_val + 3));
        ret_ip = (Bit16u)(ip_val + 5);
    }
    char cn[96], tn[96];
    og_describe(cs_val, ip_val, cn, sizeof cn);
    og_describe(tgt_cs, tgt_ip, tn, sizeof tn);
    fprintf(stderr,
            "[FN_DISC] %s  caller=%04X:%04X (%s)  entry=%04X:%04X (%s)  ret=%04X:%04X\n",
            (op == 0xE8) ? "near" : "far ",
            cs_val, ip_val, cn, tgt_cs, tgt_ip, tn, cs_val, ret_ip);
#endif
}

/* Snapshot the whole _players array tagged with `point` (shared by the FN-EXIT
 * and AT-ENTRY _players captures). */
static void gd_dump_players_og(const char* point) {
    Bit16u dgroup = og_exes[OG_SR_MGC_EXE_IDX].runtime_ds;
    int16_t num = (int16_t)real_readw(dgroup, (Bit16u)OG_SR_NUMPLAYERS_OFF);
    int act = (num >= 1 && num <= OG_SR_NUM_PLAYERS) ? num : OG_SR_NUM_PLAYERS;
    int p;
    for (p = 0; p < act; p++) {
        Bit16u rbase = (Bit16u)(OG_SR_PLAYERS_OFF + (Bit32u)p * OG_SR_RECORD_SIZE);
        gd_decode_record(dgroup, rbase, wizard_fields, WIZARD_FIELD_COUNT,
                         "_players", p, point);
    }
}

/* Resolve the address a just-entered function will RET/RETF to, reading the
 * live return frame at [SS:SP].  Works for BOTH call kinds because the return
 * address is always caller_cs:after-call-offset:
 *   - near (E8): caller is in the SAME segment as the callee, so RET pops only
 *     the offset and returns to callee_cs:roff.
 *   - far (9A), incl. through an overlay j_ stub: the original CALL FAR pushed
 *     seg:off and that frame is still intact when the real function body runs,
 *     so RETF returns to [SS:SP+2]:[SS:SP] -- straight back to the real caller.
 * We disambiguate by verifying the near call site (E8 whose rel16 targets this
 * entry); anything else is treated as far and the return segment is read from
 * the next stack word.  callee_cs/entry_ip identify the function just entered. */
static void gd_call_return_addr(Bit16u callee_cs, Bit16u entry_ip,
                                Bit16u* ret_cs, Bit16u* ret_ip) {
    Bit16u ss_val = SegValue(ss);
    Bit16u sp_val = (Bit16u)(reg_esp & 0xFFFFu);
    Bit16u roff   = real_readw(ss_val, sp_val);          /* return offset (both) */
    Bit16u site   = (Bit16u)(roff - 3);                  /* E8 cw is 3 bytes */
    if (real_readb(callee_cs, site) == 0xE8) {
        int16_t rel = (int16_t)real_readw(callee_cs, (Bit16u)(site + 1));
        if ((Bit16u)((int)site + 3 + rel) == entry_ip) {
            *ret_cs = callee_cs;                         /* near: same segment */
            *ret_ip = roff;
            return;
        }
    }
    *ret_cs = real_readw(ss_val, (Bit16u)(sp_val + 2));  /* far: return segment */
    *ret_ip = roff;
}

static void gd_fnexit_probe(Bit16u cs_val, Bit16u ip_val) {
    if (og_active_exe_idx != OG_SR_MGC_EXE_IDX) return;
    int i, j;

    /* (1) return reached -> the function returned; snapshot _players. */
    for (i = 0; i < 8; i++) {
        if (!gd_pend[i].active) continue;
        if (gd_pend[i].cs != cs_val || gd_pend[i].ret_ip != ip_val) continue;
        gd_dump_players_og(gd_capfns[gd_pend[i].fn].point);
        gd_pend[i].active = 0;
    }

    /* (2) entry detect: IP == overlay offset + og_describe name match
     *     (same identity check as the gd_wm/node/lair/city blocks).  Arm the
     *     return trap on the resolved caller cs:ip, so it fires whether the
     *     function was entered by a near call (returns to the same segment) or
     *     a far call (returns to the caller's segment, e.g. via a j_ stub). */
    int nfn = (int)(sizeof(gd_capfns) / sizeof(gd_capfns[0]));
    for (i = 0; i < nfn; i++) {
        if (ip_val != gd_capfns[i].off) continue;
        char pl_nm[96];
        og_describe(cs_val, ip_val, pl_nm, sizeof(pl_nm));
        if (!strstr(pl_nm, gd_capfns[i].func)) continue;
        Bit16u ret_cs, ret_ip2;
        gd_call_return_addr(cs_val, ip_val, &ret_cs, &ret_ip2);
        int slot = -1, exists = 0;
        for (j = 0; j < 8; j++) {
            if (gd_pend[j].active && gd_pend[j].fn == i &&
                gd_pend[j].cs == ret_cs && gd_pend[j].ret_ip == ret_ip2) { exists = 1; break; }
            if (!gd_pend[j].active && slot < 0) slot = j;
        }
        if (!exists && slot >= 0) {
            gd_pend[slot].active = 1; gd_pend[slot].fn = i;
            gd_pend[slot].cs = ret_cs; gd_pend[slot].ret_ip = ret_ip2;
        }
    }

    /* (2b) AT-ENTRY _players capture: same identity check as block (2), but dump
     *      immediately on entry instead of trapping the return.  Used for points
     *      that must observe state *before* a function mutates it. */
    int npe = (int)(sizeof(gd_player_entry_capfns) / sizeof(gd_player_entry_capfns[0]));
    for (i = 0; i < npe; i++) {
        if (ip_val != gd_player_entry_capfns[i].off) continue;
        char pe_nm[96];
        og_describe(cs_val, ip_val, pe_nm, sizeof(pe_nm));
        if (!strstr(pe_nm, gd_player_entry_capfns[i].func)) continue;
        gd_dump_players_og(gd_player_entry_capfns[i].point);
    }

    /* (3) _world_maps capture at terrain-gen FN-EXITs (resolver-confirmed entry,
     *     no prologue sig): when IP hits a function's overlay offset AND
     *     og_describe() names it, trap the near-call return and dump _world_maps
     *     on return.  Each function is called per plane (2x); last write wins per
     *     point = the full post-step map.  Bisects where the map first diverges. */
    int nwm = (int)(sizeof(gd_wm_capfns) / sizeof(gd_wm_capfns[0]));
    for (i = 0; i < 4; i++) {
        if (!gd_wm_pend[i].active) continue;
        if (gd_wm_pend[i].cs != cs_val || gd_wm_pend[i].ret_ip != ip_val) continue;
        gd_wm_pend[i].active = 0;
        gd_dump_world_map(gd_wm_capfns[gd_wm_pend[i].fn].point);
    }
    for (i = 0; i < nwm; i++) {
        if (ip_val != gd_wm_capfns[i].off) continue;
        char wm_nm[96];
        og_describe(cs_val, ip_val, wm_nm, sizeof(wm_nm));
        if (!strstr(wm_nm, gd_wm_capfns[i].func)) continue;
        Bit16u ss_val = SegValue(ss);
        Bit16u sp_val = (Bit16u)(reg_esp & 0xFFFFu);
        Bit16u ret    = real_readw(ss_val, sp_val);   /* near-call return offset */
        int slot = -1;
        for (j = 0; j < 4; j++) {
            if (gd_wm_pend[j].active && gd_wm_pend[j].fn == i &&
                gd_wm_pend[j].cs == cs_val && gd_wm_pend[j].ret_ip == ret) { slot = -2; break; }
            if (!gd_wm_pend[j].active && slot < 0) slot = j;
        }
        if (slot >= 0) {
            gd_wm_pend[slot].active = 1; gd_wm_pend[slot].fn = i;
            gd_wm_pend[slot].cs = cs_val; gd_wm_pend[slot].ret_ip = ret;
        }
    }

    /* (4) ENTRY captures: dump the map immediately on function entry. */
    int nwe = (int)(sizeof(gd_wm_entry_capfns) / sizeof(gd_wm_entry_capfns[0]));
    for (i = 0; i < nwe; i++) {
        if (ip_val != gd_wm_entry_capfns[i].off) continue;
        char we_nm[96];
        og_describe(cs_val, ip_val, we_nm, sizeof(we_nm));
        if (strstr(we_nm, gd_wm_entry_capfns[i].func))
            gd_dump_world_map(gd_wm_entry_capfns[i].point);
    }

    /* (5) _NODES capture at Generate_Nodes / Rebalance_Node_Types FN-EXIT
     *     (same resolver-confirmed near-call return trap as block (3)). */
    int nnf = (int)(sizeof(gd_node_capfns) / sizeof(gd_node_capfns[0]));
    for (i = 0; i < 4; i++) {
        if (!gd_node_pend[i].active) continue;
        if (gd_node_pend[i].cs != cs_val || gd_node_pend[i].ret_ip != ip_val) continue;
        gd_node_pend[i].active = 0;
        gd_dump_nodes(gd_node_capfns[gd_node_pend[i].fn].point);
    }
    for (i = 0; i < nnf; i++) {
        if (ip_val != gd_node_capfns[i].off) continue;
        char nd_nm[96];
        og_describe(cs_val, ip_val, nd_nm, sizeof(nd_nm));
        if (!strstr(nd_nm, gd_node_capfns[i].func)) continue;
        Bit16u ss_val = SegValue(ss);
        Bit16u sp_val = (Bit16u)(reg_esp & 0xFFFFu);
        Bit16u ret    = real_readw(ss_val, sp_val);   /* near-call return offset */
        int slot = -1;
        for (j = 0; j < 4; j++) {
            if (gd_node_pend[j].active && gd_node_pend[j].fn == i &&
                gd_node_pend[j].cs == cs_val && gd_node_pend[j].ret_ip == ret) { slot = -2; break; }
            if (!gd_node_pend[j].active && slot < 0) slot = j;
        }
        if (slot >= 0) {
            gd_node_pend[slot].active = 1; gd_node_pend[slot].fn = i;
            gd_node_pend[slot].cs = cs_val; gd_node_pend[slot].ret_ip = ret;
        }
    }

    /* (6) _landmasses capture at Init_Landmasses / Generate_Landmasses FN-EXIT. */
    int nlf = (int)(sizeof(gd_landmass_capfns) / sizeof(gd_landmass_capfns[0]));
    for (i = 0; i < 4; i++) {
        if (!gd_landmass_pend[i].active) continue;
        if (gd_landmass_pend[i].cs != cs_val || gd_landmass_pend[i].ret_ip != ip_val) continue;
        gd_landmass_pend[i].active = 0;
        gd_dump_landmasses(gd_landmass_capfns[gd_landmass_pend[i].fn].point);
    }
    for (i = 0; i < nlf; i++) {
        if (ip_val != gd_landmass_capfns[i].off) continue;
        char lm_nm[96];
        og_describe(cs_val, ip_val, lm_nm, sizeof(lm_nm));
        if (!strstr(lm_nm, gd_landmass_capfns[i].func)) continue;
        Bit16u ss_val = SegValue(ss);
        Bit16u sp_val = (Bit16u)(reg_esp & 0xFFFFu);
        Bit16u ret    = real_readw(ss_val, sp_val);   /* near-call return offset */
        int slot = -1;
        for (j = 0; j < 4; j++) {
            if (gd_landmass_pend[j].active && gd_landmass_pend[j].fn == i &&
                gd_landmass_pend[j].cs == cs_val && gd_landmass_pend[j].ret_ip == ret) { slot = -2; break; }
            if (!gd_landmass_pend[j].active && slot < 0) slot = j;
        }
        if (slot >= 0) {
            gd_landmass_pend[slot].active = 1; gd_landmass_pend[slot].fn = i;
            gd_landmass_pend[slot].cs = cs_val; gd_landmass_pend[slot].ret_ip = ret;
        }
    }

    /* (7) _TOWERS capture at Generate_Towers FN-EXIT. */
    int ntf = (int)(sizeof(gd_tower_capfns) / sizeof(gd_tower_capfns[0]));
    for (i = 0; i < 4; i++) {
        if (!gd_tower_pend[i].active) continue;
        if (gd_tower_pend[i].cs != cs_val || gd_tower_pend[i].ret_ip != ip_val) continue;
        gd_tower_pend[i].active = 0;
        gd_dump_towers(gd_tower_capfns[gd_tower_pend[i].fn].point);
    }
    for (i = 0; i < ntf; i++) {
        if (ip_val != gd_tower_capfns[i].off) continue;
        char tw_nm[96];
        og_describe(cs_val, ip_val, tw_nm, sizeof(tw_nm));
        if (!strstr(tw_nm, gd_tower_capfns[i].func)) continue;
        Bit16u ss_val = SegValue(ss);
        Bit16u sp_val = (Bit16u)(reg_esp & 0xFFFFu);
        Bit16u ret    = real_readw(ss_val, sp_val);   /* near-call return offset */

        /* Uninitialized auto `tries` lives at [BP-8] (disassembly).  We are AT
         * the entry instruction, before the `PUSH BP; MOV BP,SP` prologue runs,
         * so SP still points at the near-call return addr (see `ret` above).
         * The future frame: PUSH BP -> SP-2 holds old BP; MOV BP,SP -> BP=SP-2;
         * hence [BP-8] = SP-10.  Nothing writes that slot before the tower
         * loop's first `tries++` reads it (the prologue only touches SP-2 and
         * the deeper SI/DI saves; min_distance/itr1 are other slots), so this
         * word IS the garbage `tries` starts from.  Stack autos are NOT
         * zero-initialized: a non-zero value here (a *signed* `> 500` forces an
         * early `min_distance--`) is the suspected divergence from a ReMoM that
         * starts tries=0. */
        {
            Bit16u tries_off = (Bit16u)(sp_val - 10u);
            Bit16u tries_raw = real_readw(ss_val, tries_off);
            fprintf(stderr,
                "[GD_TRIES] Generate_Towers tries@[BP-8] uninit = %d (0x%04X)"
                "  ss=%04X sp=%04X off=%04X ret=%04X\n",
                (int)(Bit16s)tries_raw, tries_raw,
                ss_val, sp_val, tries_off, ret);
        }

        int slot = -1;
        for (j = 0; j < 4; j++) {
            if (gd_tower_pend[j].active && gd_tower_pend[j].fn == i &&
                gd_tower_pend[j].cs == cs_val && gd_tower_pend[j].ret_ip == ret) { slot = -2; break; }
            if (!gd_tower_pend[j].active && slot < 0) slot = j;
        }
        if (slot >= 0) {
            gd_tower_pend[slot].active = 1; gd_tower_pend[slot].fn = i;
            gd_tower_pend[slot].cs = cs_val; gd_tower_pend[slot].ret_ip = ret;
        }
    }

    /* (8) _LAIRS capture at Generate_Lairs FN-EXIT (standard near-call return
     *     trap as blocks (5)/(6); Generate_Lairs is called once). */
    int nlrf = (int)(sizeof(gd_lair_capfns) / sizeof(gd_lair_capfns[0]));
    for (i = 0; i < 4; i++) {
        if (!gd_lair_pend[i].active) continue;
        if (gd_lair_pend[i].cs != cs_val || gd_lair_pend[i].ret_ip != ip_val) continue;
        gd_lair_pend[i].active = 0;
        gd_dump_lairs(gd_lair_capfns[gd_lair_pend[i].fn].point);
    }
    for (i = 0; i < nlrf; i++) {
        if (ip_val != gd_lair_capfns[i].off) continue;
        char lr_nm[96];
        og_describe(cs_val, ip_val, lr_nm, sizeof(lr_nm));
        if (!strstr(lr_nm, gd_lair_capfns[i].func)) continue;
        Bit16u ss_val = SegValue(ss);
        Bit16u sp_val = (Bit16u)(reg_esp & 0xFFFFu);
        Bit16u ret    = real_readw(ss_val, sp_val);   /* near-call return offset */
        int slot = -1;
        for (j = 0; j < 4; j++) {
            if (gd_lair_pend[j].active && gd_lair_pend[j].fn == i &&
                gd_lair_pend[j].cs == cs_val && gd_lair_pend[j].ret_ip == ret) { slot = -2; break; }
            if (!gd_lair_pend[j].active && slot < 0) slot = j;
        }
        if (slot >= 0) {
            gd_lair_pend[slot].active = 1; gd_lair_pend[slot].fn = i;
            gd_lair_pend[slot].cs = cs_val; gd_lair_pend[slot].ret_ip = ret;
        }
    }

    /* (9) _CITIES capture at Generate_Home_Cities FN-EXIT (standard near-call
     *     return trap; Generate_Home_Cities is called once). */
    int ncf = (int)(sizeof(gd_city_capfns) / sizeof(gd_city_capfns[0]));
    for (i = 0; i < 4; i++) {
        if (!gd_city_pend[i].active) continue;
        if (gd_city_pend[i].cs != cs_val || gd_city_pend[i].ret_ip != ip_val) continue;
        gd_city_pend[i].active = 0;
        gd_dump_cities(gd_city_capfns[gd_city_pend[i].fn].point);
    }
    for (i = 0; i < ncf; i++) {
        if (ip_val != gd_city_capfns[i].off) continue;
        char ct_nm[96];
        og_describe(cs_val, ip_val, ct_nm, sizeof(ct_nm));
        if (!strstr(ct_nm, gd_city_capfns[i].func)) continue;
        Bit16u ss_val = SegValue(ss);
        Bit16u sp_val = (Bit16u)(reg_esp & 0xFFFFu);
        Bit16u ret    = real_readw(ss_val, sp_val);   /* near-call return offset */
        int slot = -1;
        for (j = 0; j < 4; j++) {
            if (gd_city_pend[j].active && gd_city_pend[j].fn == i &&
                gd_city_pend[j].cs == cs_val && gd_city_pend[j].ret_ip == ret) { slot = -2; break; }
            if (!gd_city_pend[j].active && slot < 0) slot = j;
        }
        if (slot >= 0) {
            gd_city_pend[slot].active = 1; gd_city_pend[slot].fn = i;
            gd_city_pend[slot].cs = cs_val; gd_city_pend[slot].ret_ip = ret;
        }
    }

    /* (9b) _UNITS capture at Generate_Home_Cities / Generate_Neutral_Cities
     *      FN-EXIT (same near-call return trap as _CITIES; those funcs build the
     *      starting city-garrison units). */
    int nuf = (int)(sizeof(gd_unit_capfns) / sizeof(gd_unit_capfns[0]));
    for (i = 0; i < 4; i++) {
        if (!gd_unit_pend[i].active) continue;
        if (gd_unit_pend[i].cs != cs_val || gd_unit_pend[i].ret_ip != ip_val) continue;
        gd_unit_pend[i].active = 0;
        gd_dump_units(gd_unit_capfns[gd_unit_pend[i].fn].point);
    }
    for (i = 0; i < nuf; i++) {
        if (ip_val != gd_unit_capfns[i].off) continue;
        char ut_nm[96];
        og_describe(cs_val, ip_val, ut_nm, sizeof(ut_nm));
        if (!strstr(ut_nm, gd_unit_capfns[i].func)) continue;
        Bit16u ss_val = SegValue(ss);
        Bit16u sp_val = (Bit16u)(reg_esp & 0xFFFFu);
        Bit16u ret    = real_readw(ss_val, sp_val);   /* near-call return offset */
        int slot = -1;
        for (j = 0; j < 4; j++) {
            if (gd_unit_pend[j].active && gd_unit_pend[j].fn == i &&
                gd_unit_pend[j].cs == cs_val && gd_unit_pend[j].ret_ip == ret) { slot = -2; break; }
            if (!gd_unit_pend[j].active && slot < 0) slot = j;
        }
        if (slot >= 0) {
            gd_unit_pend[slot].active = 1; gd_unit_pend[slot].fn = i;
            gd_unit_pend[slot].cs = cs_val; gd_unit_pend[slot].ret_ip = ret;
        }
    }

    /* (9c) _HEROES2 capture at Init_Heroes FN-EXIT (per-player hero records;
     *      same near-call return trap as the _players 100_Init_Heroes_P capture). */
    int nhf = (int)(sizeof(gd_heroes_capfns) / sizeof(gd_heroes_capfns[0]));
    for (i = 0; i < 4; i++) {
        if (!gd_heroes_pend[i].active) continue;
        if (gd_heroes_pend[i].cs != cs_val || gd_heroes_pend[i].ret_ip != ip_val) continue;
        gd_heroes_pend[i].active = 0;
        gd_dump_heroes(gd_heroes_capfns[gd_heroes_pend[i].fn].point);
    }
    for (i = 0; i < nhf; i++) {
        if (ip_val != gd_heroes_capfns[i].off) continue;
        char he_nm[96];
        og_describe(cs_val, ip_val, he_nm, sizeof(he_nm));
        if (!strstr(he_nm, gd_heroes_capfns[i].func)) continue;
        Bit16u ss_val = SegValue(ss);
        Bit16u sp_val = (Bit16u)(reg_esp & 0xFFFFu);
        Bit16u ret    = real_readw(ss_val, sp_val);   /* near-call return offset */
        int slot = -1;
        for (j = 0; j < 4; j++) {
            if (gd_heroes_pend[j].active && gd_heroes_pend[j].fn == i &&
                gd_heroes_pend[j].cs == cs_val && gd_heroes_pend[j].ret_ip == ret) { slot = -2; break; }
            if (!gd_heroes_pend[j].active && slot < 0) slot = j;
        }
        if (slot >= 0) {
            gd_heroes_pend[slot].active = 1; gd_heroes_pend[slot].fn = i;
            gd_heroes_pend[slot].cs = cs_val; gd_heroes_pend[slot].ret_ip = ret;
        }
    }

    /* (9d) _ITEMS capture at Initialize_Items FN-EXIT (near-call return trap). */
    int nif = (int)(sizeof(gd_item_capfns) / sizeof(gd_item_capfns[0]));
    for (i = 0; i < 4; i++) {
        if (!gd_item_pend[i].active) continue;
        if (gd_item_pend[i].cs != cs_val || gd_item_pend[i].ret_ip != ip_val) continue;
        gd_item_pend[i].active = 0;
        gd_dump_items(gd_item_capfns[gd_item_pend[i].fn].point);
    }
    for (i = 0; i < nif; i++) {
        if (ip_val != gd_item_capfns[i].off) continue;
        char it_nm[96];
        og_describe(cs_val, ip_val, it_nm, sizeof(it_nm));
        if (!strstr(it_nm, gd_item_capfns[i].func)) continue;
        Bit16u ss_val = SegValue(ss);
        Bit16u sp_val = (Bit16u)(reg_esp & 0xFFFFu);
        Bit16u ret    = real_readw(ss_val, sp_val);   /* near-call return offset */
        int slot = -1;
        for (j = 0; j < 4; j++) {
            if (gd_item_pend[j].active && gd_item_pend[j].fn == i &&
                gd_item_pend[j].cs == cs_val && gd_item_pend[j].ret_ip == ret) { slot = -2; break; }
            if (!gd_item_pend[j].active && slot < 0) slot = j;
        }
        if (slot >= 0) {
            gd_item_pend[slot].active = 1; gd_item_pend[slot].fn = i;
            gd_item_pend[slot].cs = cs_val; gd_item_pend[slot].ret_ip = ret;
        }
    }

    /* (9e) _CITIES POST-LOAD capture is NOT here: gd_fnexit_probe returns early
     * for non-MAGIC (line ~3319), so WIZARDS code placed here never runs.  It
     * lives in gd_wzd_landmark_probe (WIZARDS-gated), called from the main hook. */

    /* (10) _map_square_terrain_specials capture at Generate_Terrain_Specials
     *      FN-EXIT (near-call return trap; called once per plane, 2x). */
    int nts = (int)(sizeof(gd_ts_capfns) / sizeof(gd_ts_capfns[0]));
    for (i = 0; i < 4; i++) {
        if (!gd_ts_pend[i].active) continue;
        if (gd_ts_pend[i].cs != cs_val || gd_ts_pend[i].ret_ip != ip_val) continue;
        gd_ts_pend[i].active = 0;
        gd_dump_terrain_specials(gd_ts_capfns[gd_ts_pend[i].fn].point);
    }
    for (i = 0; i < nts; i++) {
        if (ip_val != gd_ts_capfns[i].off) continue;
        char ts_nm[96];
        og_describe(cs_val, ip_val, ts_nm, sizeof(ts_nm));
        if (!strstr(ts_nm, gd_ts_capfns[i].func)) continue;
        Bit16u ss_val = SegValue(ss);
        Bit16u sp_val = (Bit16u)(reg_esp & 0xFFFFu);
        Bit16u ret    = real_readw(ss_val, sp_val);   /* near-call return offset */
        int slot = -1;
        for (j = 0; j < 4; j++) {
            if (gd_ts_pend[j].active && gd_ts_pend[j].fn == i &&
                gd_ts_pend[j].cs == cs_val && gd_ts_pend[j].ret_ip == ret) { slot = -2; break; }
            if (!gd_ts_pend[j].active && slot < 0) slot = j;
        }
        if (slot >= 0) {
            gd_ts_pend[slot].active = 1; gd_ts_pend[slot].fn = i;
            gd_ts_pend[slot].cs = cs_val; gd_ts_pend[slot].ret_ip = ret;
        }
    }

    /* (11) _map_square_flags capture at Generate_Roads FN-EXIT (near-call
     *      return trap; called once per plane, 2x). */
    int nmf = (int)(sizeof(gd_msf_capfns) / sizeof(gd_msf_capfns[0]));
    for (i = 0; i < 4; i++) {
        if (!gd_msf_pend[i].active) continue;
        if (gd_msf_pend[i].cs != cs_val || gd_msf_pend[i].ret_ip != ip_val) continue;
        gd_msf_pend[i].active = 0;
        gd_dump_map_square_flags(gd_msf_capfns[gd_msf_pend[i].fn].point);
    }
    for (i = 0; i < nmf; i++) {
        if (ip_val != gd_msf_capfns[i].off) continue;
        char mf_nm[96];
        og_describe(cs_val, ip_val, mf_nm, sizeof(mf_nm));
        if (!strstr(mf_nm, gd_msf_capfns[i].func)) continue;
        Bit16u ss_val = SegValue(ss);
        Bit16u sp_val = (Bit16u)(reg_esp & 0xFFFFu);
        Bit16u ret    = real_readw(ss_val, sp_val);   /* near-call return offset */
        int slot = -1;
        for (j = 0; j < 4; j++) {
            if (gd_msf_pend[j].active && gd_msf_pend[j].fn == i &&
                gd_msf_pend[j].cs == cs_val && gd_msf_pend[j].ret_ip == ret) { slot = -2; break; }
            if (!gd_msf_pend[j].active && slot < 0) slot = j;
        }
        if (slot >= 0) {
            gd_msf_pend[slot].active = 1; gd_msf_pend[slot].fn = i;
            gd_msf_pend[slot].cs = cs_val; gd_msf_pend[slot].ret_ip = ret;
        }
    }

    /* (12) _square_explored capture at Init_Square_Explored FN-EXIT (near-call
     *      return trap; Init_Square_Explored is called once). */
    int nse = (int)(sizeof(gd_se_capfns) / sizeof(gd_se_capfns[0]));
    for (i = 0; i < 4; i++) {
        if (!gd_se_pend[i].active) continue;
        if (gd_se_pend[i].cs != cs_val || gd_se_pend[i].ret_ip != ip_val) continue;
        gd_se_pend[i].active = 0;
        gd_dump_square_explored(gd_se_capfns[gd_se_pend[i].fn].point);
    }
    for (i = 0; i < nse; i++) {
        if (ip_val != gd_se_capfns[i].off) continue;
        char se_nm[96];
        og_describe(cs_val, ip_val, se_nm, sizeof(se_nm));
        if (!strstr(se_nm, gd_se_capfns[i].func)) continue;
        Bit16u ss_val = SegValue(ss);
        Bit16u sp_val = (Bit16u)(reg_esp & 0xFFFFu);
        Bit16u ret    = real_readw(ss_val, sp_val);   /* near-call return offset */
        int slot = -1;
        for (j = 0; j < 4; j++) {
            if (gd_se_pend[j].active && gd_se_pend[j].fn == i &&
                gd_se_pend[j].cs == cs_val && gd_se_pend[j].ret_ip == ret) { slot = -2; break; }
            if (!gd_se_pend[j].active && slot < 0) slot = j;
        }
        if (slot >= 0) {
            gd_se_pend[slot].active = 1; gd_se_pend[slot].fn = i;
            gd_se_pend[slot].cs = cs_val; gd_se_pend[slot].ret_ip = ret;
        }
    }

    /* (13) movement_mode_cost_maps capture at Movement_Mode_Cost_Maps FN-EXIT
     *      (near-call return trap; called once per plane, 2x). */
    int nmmcm = (int)(sizeof(gd_mmcm_capfns) / sizeof(gd_mmcm_capfns[0]));
    for (i = 0; i < 4; i++) {
        if (!gd_mmcm_pend[i].active) continue;
        if (gd_mmcm_pend[i].cs != cs_val || gd_mmcm_pend[i].ret_ip != ip_val) continue;
        gd_mmcm_pend[i].active = 0;
        gd_dump_movement_mode_cost_maps(gd_mmcm_capfns[gd_mmcm_pend[i].fn].point);
    }
    for (i = 0; i < nmmcm; i++) {
        if (ip_val != gd_mmcm_capfns[i].off) continue;
        char mm_nm[96];
        og_describe(cs_val, ip_val, mm_nm, sizeof(mm_nm));
        if (!strstr(mm_nm, gd_mmcm_capfns[i].func)) continue;
        Bit16u ss_val = SegValue(ss);
        Bit16u sp_val = (Bit16u)(reg_esp & 0xFFFFu);
        Bit16u ret    = real_readw(ss_val, sp_val);   /* near-call return offset */
        int slot = -1;
        for (j = 0; j < 4; j++) {
            if (gd_mmcm_pend[j].active && gd_mmcm_pend[j].fn == i &&
                gd_mmcm_pend[j].cs == cs_val && gd_mmcm_pend[j].ret_ip == ret) { slot = -2; break; }
            if (!gd_mmcm_pend[j].active && slot < 0) slot = j;
        }
        if (slot >= 0) {
            gd_mmcm_pend[slot].active = 1; gd_mmcm_pend[slot].fn = i;
            gd_mmcm_pend[slot].cs = cs_val; gd_mmcm_pend[slot].ret_ip = ret;
        }
    }

    /* (14) UU_TBL_1 / UU_TBL_2 capture at CRP_NEWG_CreatePathGrids FN-EXIT
     *      (near-call return trap; called once per plane, 2x). */
    int nuut = (int)(sizeof(gd_uut_capfns) / sizeof(gd_uut_capfns[0]));
    for (i = 0; i < 4; i++) {
        if (!gd_uut_pend[i].active) continue;
        if (gd_uut_pend[i].cs != cs_val || gd_uut_pend[i].ret_ip != ip_val) continue;
        gd_uut_pend[i].active = 0;
        gd_dump_uu_tbls(gd_uut_capfns[gd_uut_pend[i].fn].point);
    }
    for (i = 0; i < nuut; i++) {
        if (ip_val != gd_uut_capfns[i].off) continue;
        char uu_nm[96];
        og_describe(cs_val, ip_val, uu_nm, sizeof(uu_nm));
        if (!strstr(uu_nm, gd_uut_capfns[i].func)) continue;
        Bit16u ss_val = SegValue(ss);
        Bit16u sp_val = (Bit16u)(reg_esp & 0xFFFFu);
        Bit16u ret    = real_readw(ss_val, sp_val);   /* near-call return offset */
        int slot = -1;
        for (j = 0; j < 4; j++) {
            if (gd_uut_pend[j].active && gd_uut_pend[j].fn == i &&
                gd_uut_pend[j].cs == cs_val && gd_uut_pend[j].ret_ip == ret) { slot = -2; break; }
            if (!gd_uut_pend[j].active && slot < 0) slot = j;
        }
        if (slot >= 0) {
            gd_uut_pend[slot].active = 1; gd_uut_pend[slot].fn = i;
            gd_uut_pend[slot].cs = cs_val; gd_uut_pend[slot].ret_ip = ret;
        }
    }
}

/* -------------------------------------------------------------------- *
 *  MAGIC resident-landmark captures (cross-overlay j_ stubs, resolved exactly
 *  via base_offset).  Save_SAVE_GAM fires when the user saves slot 1 -- after
 *  Init_Runtime fully completes -- so it captures the truly-final _players
 *  record (e.g. gold_reserve, which is 0 at every FN-EXIT point).  One-shot
 *  per landmark per MAGIC activation (re-arm on EXE change).  WIZARDS has its
 *  own gd_wzd_landmark_probe below; gd_landmark_t is the shared landmark type.
 * -------------------------------------------------------------------- */
struct gd_landmark_t { Bit32u ida; const char* point; };
static const struct gd_landmark_t gd_mgc_landmarks[] = {
    { 0x00031800, "Save_SAVE_GAM" },   /* stub048 j_Save_SAVE_GAM */
};

static void gd_mgc_landmark_probe(Bit16u cs_val, Bit16u ip_val) {
    if (og_active_exe_idx != OG_SR_MGC_EXE_IDX) return;
    struct og_exe_info* e = &og_exes[OG_SR_MGC_EXE_IDX];
    static int last_idx = -2;
    static int fired[8];
    int nlm = (int)(sizeof(gd_mgc_landmarks) / sizeof(gd_mgc_landmarks[0])), i, z;
    if (og_active_exe_idx != last_idx) {
        last_idx = og_active_exe_idx;
        for (z = 0; z < 8; z++) fired[z] = 0;
    }
    Bit32s ida = (Bit32s)(((Bit32u)cs_val << 4) + ip_val) - e->base_offset;
    if (ida < 0) return;
    for (i = 0; i < nlm; i++) {
        if ((Bit32u)ida != gd_mgc_landmarks[i].ida || fired[i]) continue;
        fired[i] = 1;
        Bit16u dgroup = e->runtime_ds;
        int16_t num = (int16_t)real_readw(dgroup, (Bit16u)OG_SR_NUMPLAYERS_OFF);
        int act = (num >= 1 && num <= OG_SR_NUM_PLAYERS) ? num : OG_SR_NUM_PLAYERS;
        int p;
        for (p = 0; p < act; p++) {
            Bit16u rbase = (Bit16u)(OG_SR_PLAYERS_OFF + (Bit32u)p * OG_SR_RECORD_SIZE);
            gd_decode_record(dgroup, rbase, wizard_fields, WIZARD_FIELD_COUNT,
                             "_players", p, gd_mgc_landmarks[i].point);
        }
    }
}

/* -------------------------------------------------------------------- *
 *  AI landmass-square scratch arrays capture, at AI_Choose_War_Landmass's
 *  CONTXXX_Map return (605_AI_Choose_War_Landmass_Squares).  Four WZD DGROUP far-ptr slots, each an
 *  int16_t*[2]/int8_t*[2] (per plane) pointing into the CONTXXX EMS page
 *  frame (EmmHndl_CONTXXX; offsets from the ReMoM wiki dseg table).  Mirrors
 *  ReMoM's gd_dump_ai_landmass_squares in AIMOVE.c byte-for-byte:
 *    heads -- all NUM_LANDMASSES (reset every rebuild, always valid)
 *    lists/wx/wy -- only the used node count (their tails are stale scratch);
 *    used = total nodes reachable from all heads (== the builder's count[wp]),
 *    derived by traversal so no _landmasses read is needed. */
#define OG_AICWL_HEADS_OFF       0x9EA0u  /* int16_t*[2]  head node index per landmass */
#define OG_AICWL_LISTS_OFF       0x9EA8u  /* int16_t*[2]  next-node index (linked list) */
#define OG_AICWL_WY_OFF          0x9EB0u  /* int8_t*[2]   node wy coord */
#define OG_AICWL_WX_OFF          0x9EB8u  /* int8_t*[2]   node wx coord */
#define OG_AICWL_NUM_LANDMASSES  60
#define OG_AICWL_MAX_NODES       1600
#define OG_AICWL_PLANES          2

static void gd_dump_ai_landmass_squares(Bit16u ds, const char* point) {
    static char buf[1024];
    int plane, i, q, used, node, guard, base, end;
    for (plane = 0; plane < OG_AICWL_PLANES; plane++) {
        Bit16u h_off  = real_readw(ds, (Bit16u)(OG_AICWL_HEADS_OFF + plane * 4));
        Bit16u h_seg  = real_readw(ds, (Bit16u)(OG_AICWL_HEADS_OFF + plane * 4 + 2));
        Bit16u l_off  = real_readw(ds, (Bit16u)(OG_AICWL_LISTS_OFF + plane * 4));
        Bit16u l_seg  = real_readw(ds, (Bit16u)(OG_AICWL_LISTS_OFF + plane * 4 + 2));
        Bit16u wy_off = real_readw(ds, (Bit16u)(OG_AICWL_WY_OFF + plane * 4));
        Bit16u wy_seg = real_readw(ds, (Bit16u)(OG_AICWL_WY_OFF + plane * 4 + 2));
        Bit16u wx_off = real_readw(ds, (Bit16u)(OG_AICWL_WX_OFF + plane * 4));
        Bit16u wx_seg = real_readw(ds, (Bit16u)(OG_AICWL_WX_OFF + plane * 4 + 2));

        /* used = total nodes reachable from all heads == builder count[plane].
         * A node index outside [0,MAX_NODES) is the list terminator (ST_UNDEFINED
         * or stale); guard bounds the walk against a corrupt cycle. */
        used = 0;
        for (i = 0; i < OG_AICWL_NUM_LANDMASSES; i++) {
            node = (short)real_readw(h_seg, (Bit16u)(h_off + i * 2));
            guard = 0;
            while (node >= 0 && node < OG_AICWL_MAX_NODES && guard++ < OG_AICWL_MAX_NODES) {
                used++;
                node = (short)real_readw(l_seg, (Bit16u)(l_off + node * 2));
            }
        }

        /* Emit <=100 values per [GD] line, tagged with the start index (@base),
         * matching the ReMoM capfn so the two logs diff line-for-line. */
        for (base = 0; base < OG_AICWL_NUM_LANDMASSES; base += 100) {
            end = (base + 100 < OG_AICWL_NUM_LANDMASSES) ? base + 100 : OG_AICWL_NUM_LANDMASSES;
            q = 0; buf[0] = 0;
            for (i = base; i < end; i++)
                q += snprintf(buf + q, sizeof(buf) - q, i > base ? ",%d" : "%d",
                              (int)(short)real_readw(h_seg, (Bit16u)(h_off + i * 2)));
            fprintf(stderr, "[GD] %s _ai_landmass_land_squares[%d].heads@%d = %s\n", point, plane, base, buf);
        }
        for (base = 0; base < used; base += 100) {
            end = (base + 100 < used) ? base + 100 : used;
            q = 0; buf[0] = 0;
            for (i = base; i < end; i++)
                q += snprintf(buf + q, sizeof(buf) - q, i > base ? ",%d" : "%d",
                              (int)(short)real_readw(l_seg, (Bit16u)(l_off + i * 2)));
            fprintf(stderr, "[GD] %s _ai_landmass_land_squares[%d].lists@%d = %s\n", point, plane, base, buf);
        }
        for (base = 0; base < used; base += 100) {
            end = (base + 100 < used) ? base + 100 : used;
            q = 0; buf[0] = 0;
            for (i = base; i < end; i++)
                q += snprintf(buf + q, sizeof(buf) - q, i > base ? ",%d" : "%d",
                              (int)(signed char)real_readb(wx_seg, (Bit16u)(wx_off + i)));
            fprintf(stderr, "[GD] %s _ai_landmass_land_squares[%d].wx_array@%d = %s\n", point, plane, base, buf);
        }
        for (base = 0; base < used; base += 100) {
            end = (base + 100 < used) ? base + 100 : used;
            q = 0; buf[0] = 0;
            for (i = base; i < end; i++)
                q += snprintf(buf + q, sizeof(buf) - q, i > base ? ",%d" : "%d",
                              (int)(signed char)real_readb(wy_seg, (Bit16u)(wy_off + i)));
            fprintf(stderr, "[GD] %s _ai_landmass_land_squares[%d].wy_array@%d = %s\n", point, plane, base, buf);
        }
    }
}

/* Dock-square arrays -- parallel to the land squares above, same CONTXXX EMS frame.
 * DGROUP far-ptr offsets from the ReMoM wiki dseg table: heads 9D2A, lists 9D32,
 * wy 9D3A, wx 9D42.  Built once at load by Build_Dock_Linked_List; visible here
 * because AICWL's CONTXXX_Map() maps the CONTXXX pages.  Mirrors ReMoM's
 * gd_dump_ai_dock_squares byte-for-byte. */
#define OG_AICWL_DOCK_HEADS_OFF  0x9D2Au
#define OG_AICWL_DOCK_LISTS_OFF  0x9D32u
#define OG_AICWL_DOCK_WY_OFF     0x9D3Au
#define OG_AICWL_DOCK_WX_OFF     0x9D42u

static void gd_dump_ai_dock_squares(Bit16u ds, const char* point) {
    static char buf[1024];
    int plane, i, q, used, node, guard, base, end;
    for (plane = 0; plane < OG_AICWL_PLANES; plane++) {
        Bit16u h_off  = real_readw(ds, (Bit16u)(OG_AICWL_DOCK_HEADS_OFF + plane * 4));
        Bit16u h_seg  = real_readw(ds, (Bit16u)(OG_AICWL_DOCK_HEADS_OFF + plane * 4 + 2));
        Bit16u l_off  = real_readw(ds, (Bit16u)(OG_AICWL_DOCK_LISTS_OFF + plane * 4));
        Bit16u l_seg  = real_readw(ds, (Bit16u)(OG_AICWL_DOCK_LISTS_OFF + plane * 4 + 2));
        Bit16u wy_off = real_readw(ds, (Bit16u)(OG_AICWL_DOCK_WY_OFF + plane * 4));
        Bit16u wy_seg = real_readw(ds, (Bit16u)(OG_AICWL_DOCK_WY_OFF + plane * 4 + 2));
        Bit16u wx_off = real_readw(ds, (Bit16u)(OG_AICWL_DOCK_WX_OFF + plane * 4));
        Bit16u wx_seg = real_readw(ds, (Bit16u)(OG_AICWL_DOCK_WX_OFF + plane * 4 + 2));

        used = 0;
        for (i = 0; i < OG_AICWL_NUM_LANDMASSES; i++) {
            node = (short)real_readw(h_seg, (Bit16u)(h_off + i * 2));
            guard = 0;
            while (node >= 0 && node < OG_AICWL_MAX_NODES && guard++ < OG_AICWL_MAX_NODES) {
                used++;
                node = (short)real_readw(l_seg, (Bit16u)(l_off + node * 2));
            }
        }

        for (base = 0; base < OG_AICWL_NUM_LANDMASSES; base += 100) {
            end = (base + 100 < OG_AICWL_NUM_LANDMASSES) ? base + 100 : OG_AICWL_NUM_LANDMASSES;
            q = 0; buf[0] = 0;
            for (i = base; i < end; i++)
                q += snprintf(buf + q, sizeof(buf) - q, i > base ? ",%d" : "%d",
                              (int)(short)real_readw(h_seg, (Bit16u)(h_off + i * 2)));
            fprintf(stderr, "[GD] %s _ai_landmass_dock_squares[%d].heads@%d = %s\n", point, plane, base, buf);
        }
        for (base = 0; base < used; base += 100) {
            end = (base + 100 < used) ? base + 100 : used;
            q = 0; buf[0] = 0;
            for (i = base; i < end; i++)
                q += snprintf(buf + q, sizeof(buf) - q, i > base ? ",%d" : "%d",
                              (int)(short)real_readw(l_seg, (Bit16u)(l_off + i * 2)));
            fprintf(stderr, "[GD] %s _ai_landmass_dock_squares[%d].lists@%d = %s\n", point, plane, base, buf);
        }
        for (base = 0; base < used; base += 100) {
            end = (base + 100 < used) ? base + 100 : used;
            q = 0; buf[0] = 0;
            for (i = base; i < end; i++)
                q += snprintf(buf + q, sizeof(buf) - q, i > base ? ",%d" : "%d",
                              (int)(signed char)real_readb(wx_seg, (Bit16u)(wx_off + i)));
            fprintf(stderr, "[GD] %s _ai_landmass_dock_squares[%d].wx_array@%d = %s\n", point, plane, base, buf);
        }
        for (base = 0; base < used; base += 100) {
            end = (base + 100 < used) ? base + 100 : used;
            q = 0; buf[0] = 0;
            for (i = base; i < end; i++)
                q += snprintf(buf + q, sizeof(buf) - q, i > base ? ",%d" : "%d",
                              (int)(signed char)real_readb(wy_seg, (Bit16u)(wy_off + i)));
            fprintf(stderr, "[GD] %s _ai_landmass_dock_squares[%d].wy_array@%d = %s\n", point, plane, base, buf);
        }
    }
}

/* g_ai_evaluation_map -- per-plane WORLD_SIZE uint16 occupation map in the CONTXXX
 * EMS frame (the occupiable gate AI_Evaluate_Continents reads).  DGROUP far-ptr
 * array uint16_t* g_ai_evaluation_map[2] @ 0x9C94.  Chunked <=100/line @base, %u
 * (values can be a cost OR-ed with AI_TARGET_SITE 0x8000).  Mirrors ReMoM. */
#define OG_EVAL_MAP_OFF     0x9C94u
#define OG_EVAL_WORLD_SIZE  2400

static void gd_dump_ai_evaluation_map(Bit16u ds, const char* point) {
    static char buf[1024];
    int plane, base, end, i, q;
    for (plane = 0; plane < OG_AICWL_PLANES; plane++) {
        Bit16u m_off = real_readw(ds, (Bit16u)(OG_EVAL_MAP_OFF + plane * 4));
        Bit16u m_seg = real_readw(ds, (Bit16u)(OG_EVAL_MAP_OFF + plane * 4 + 2));
        for (base = 0; base < OG_EVAL_WORLD_SIZE; base += 100) {
            end = (base + 100 < OG_EVAL_WORLD_SIZE) ? base + 100 : OG_EVAL_WORLD_SIZE;
            q = 0; buf[0] = 0;
            for (i = base; i < end; i++)
                q += snprintf(buf + q, sizeof(buf) - q, i > base ? ",%u" : "%u",
                              (unsigned)real_readw(m_seg, (Bit16u)(m_off + i * 2)));
            fprintf(stderr, "[GD] %s g_ai_evaluation_map[%d]@%d = %s\n", point, plane, base, buf);
        }
    }
}

/* -------------------------------------------------------------------- *
 *  _ai_continents capture (604): struct s_AI_CONTINENTS in WZD DGROUP.
 *  plane[2] x player[6] x s_LANDMASSES { wx_array[60]@0x00, wy_array[60]@0x3C,
 *  type_array[60]@0x78 } (all uint8; 0xB4/player, 0x438/plane).  type_array is
 *  the continent status the AI_Choose_War_Landmass reevaluation switch reads.
 *  Flat record idx = plane*6 + player.  Mirrors ReMoM's gd_dump_ai_continents. */
#define OG_WZD_AICONT_OFF 0x8890u   /* WZD DGROUP offset of _ai_continents */

static void gd_dump_ai_continents_at(Bit16u ds, Bit16u base_off, const char* point) {
    static char buf[1024];
    int plane, pl, lm, q, flat;
    for (plane = 0; plane < 2; plane++) {
        for (pl = 0; pl < 6; pl++) {
            Bit16u rec = (Bit16u)(base_off + plane * 0x438 + pl * 0xB4);
            flat = plane * 6 + pl;
            q = 0; buf[0] = 0;
            for (lm = 0; lm < 60; lm++)
                q += snprintf(buf + q, sizeof(buf) - q, lm ? ",%d" : "%d",
                              (int)real_readb(ds, (Bit16u)(rec + 0x00 + lm)));
            fprintf(stderr, "[GD] %s _ai_continents[%d].wx_array = %s\n", point, flat, buf);
            q = 0; buf[0] = 0;
            for (lm = 0; lm < 60; lm++)
                q += snprintf(buf + q, sizeof(buf) - q, lm ? ",%d" : "%d",
                              (int)real_readb(ds, (Bit16u)(rec + 0x3C + lm)));
            fprintf(stderr, "[GD] %s _ai_continents[%d].wy_array = %s\n", point, flat, buf);
            q = 0; buf[0] = 0;
            for (lm = 0; lm < 60; lm++)
                q += snprintf(buf + q, sizeof(buf) - q, lm ? ",%d" : "%d",
                              (int)real_readb(ds, (Bit16u)(rec + 0x78 + lm)));
            fprintf(stderr, "[GD] %s _ai_continents[%d].type_array = %s\n", point, flat, buf);
        }
    }
}

/* WIZARDS resident-landmark captures (post-load).  Same base_offset->IDA trick,
 * WIZARDS-gated, reading the WIZARDS DGROUP.  0x141A7 = seg001:_main+2E0 = the
 * instruction Load_SAVE_GAM(8) RETURNS TO (verified in WIZARDS.EXE: `MOV AX,8;
 * PUSH AX; CALLF j_Load_SAVE_GAM; <0x141A7> POP CX`) -- i.e. on-return/FN-EXIT.
 * seg001 is resident so base_offset resolution is exact, and DS there is settled
 * to the WIZARDS DGROUP (unlike the INT-3F overlay entry).  Matches ReMoM's
 * gd_dump_cities("310_Load_Cities_C") called right after Load_SAVE_GAM(8). */
static const struct gd_landmark_t gd_wzd_landmarks[] = {
    { 0x000141A7, "post_load" },        /* seg001:_main+2E0, return from Load_SAVE_GAM(8); fires the 300-series */
    { 0x00033DE0, "save_slot1_entry" }, /* stub050:j_Save_SAVE_GAM entry; fires the 400-series, gated save_gam_idx==0 (slot-1 canonical save) */
    { 0x00033DE0, "save_slot5_entry" }, /* same entry; fires the 500-series, gated save_gam_idx==4 (slot-5, Stage 2 Next-Turn save) */
    { 0x00036A04, "aicwl_entry" },      /* stub162:j_AI_Choose_War_Landmass on-entry; 601 _FORTRESSES + 602 _CITIES snapshot (DGROUP, readable at entry -- unlike the EMS landmass arrays) */
};  /* NOTE: 605_AI_Choose_War_Landmass_Squares (the landmass EMS arrays) is NOT a
    * landmark -- it needs CONTXXX mapped, so it triggers off the j_CONTXXX_Map
    * stub return, handled inline in gd_wzd_landmark_probe above. */

static void gd_wzd_landmark_probe(Bit16u cs_val, Bit16u ip_val) {
    if (og_active_exe_idx != OG_SR_WZD_EXE_IDX) return;
    struct og_exe_info* e = &og_exes[OG_SR_WZD_EXE_IDX];
    static int last_idx = -2;
    static int fired[8];
    /* AICWL: capture the AI landmass-square arrays at AI_Choose_War_Landmass's
     * CONTXXX_Map() return -- the only point where the CONTXXX EMS pages are
     * mapped into the frame AND the far-ptrs are (re)assigned (the builder leaves
     * EMMDATAH mapped, not CONTXXX; on-entry would read stale/unmapped memory).
     * CONTXXX_Map is called from many sites, so arm on the resident j_CONTXXX_Map
     * stub ONLY when its far-call return offset lands in AI_Choose_War_Landmass's
     * overlay-offset range, then fire when control returns there.  The return is
     * OVERLAY code, so match it by physical (cs:ip) -- its IDA wouldn't match a
     * resident landmark, and og_resolve_cs can't map the overlay CS at this point.
     * One capture per activation (arrays are map-derived / player-independent). */
    static Bit16u aicwl_ret_seg = 0, aicwl_ret_off = 0;
    static int    aicwl_armed = 0, aicwl_done = 0;
    /* AI_Evaluate_Continents (610 on-entry / 611 at-return) capture state. */
    static Bit16u aievalcont_ret_seg = 0, aievalcont_ret_off = 0;
    static int    aievalcont_armed = 0, aievalcont_done = 0;
    /* 628/629: _players before/after Diplomacy_Growth (diplomacy-phase modifier pass). */
    static Bit16u dipgro_ret_seg = 0, dipgro_ret_off = 0;
    static int    dipgro_armed = 0, dipgro_done = 0;
    /* 622/626: _CITIES before/after Apply_City_Changes (per-turn city-update pass). */
    static Bit16u appcity_ret_seg = 0, appcity_ret_off = 0;
    static int    appcity_armed = 0, appcity_done = 0;
    /* 623 sub: _UNITS after City_Apply_Production (intra-overlay call in Apply_City_Changes;
     * learn ovr140 CS from j_City_Current_Product_Cost, fire at the return IP 0x1987). */
    static Bit16u cap_cs = 0;
    static int    cap_done = 0, cap_n = 0;   /* cap_n: City_Apply_Production return counter (fire on 10th == city 9) */
    /* 624 sub: _UNITS after Apply_Consecration (cross-overlay j_Apply_Consecration; only fires
     * for consecrated cities -- none in the current turn).  Far-return into Apply_City_Changes. */
    static Bit16u consec_ret_seg = 0, consec_ret_off = 0;
    static int    consec_armed = 0, consec_done = 0;
    /* 612/613: _UNITS before/after AI_Set_Unit_Orders (per-AI-player order/movement). */
    static Bit16u aiuo_ret_seg = 0, aiuo_ret_off = 0;
    static int    aiuo_armed = 0, aiuo_done = 0;
    /* 614/615: _UNITS before/after AI_Execute_Orders (per-AI-player order EXECUTION). */
    static Bit16u aiexo_ret_seg = 0, aiexo_ret_off = 0;
    static int    aiexo_armed = 0, aiexo_done = 0;
    /* 620: Determine_Event random-event gate (event_pressure_accumulator + _event_delay_count). */
    static int    de_armed = 0, de_done = 0;
    /* 616/617: _UNITS after Make_Raiders / Make_Monsters (NPC unit generation, at-return). */
    static Bit16u mkraid_ret_seg = 0, mkraid_ret_off = 0;
    static int    mkraid_armed = 0, mkraid_done = 0;
    static Bit16u mkmon_ret_seg = 0, mkmon_ret_off = 0;
    static int    mkmon_armed = 0, mkmon_done = 0;
    /* 618: _UNITS after AI_Next_Turn (whole AI turn).  AI_Next_Turn EVICTS its caller
     * Next_Turn_Calc (ovr119, reloads at a different segment on return) -> a captured
     * far-return segment goes stale and never matches.  Fire on the first Next_Turn_Calc
     * landmark AFTER the return instead (see the block below). */
    static int    aint_armed = 0, aint_done = 0;
    /* 633: _UNITS after the LATE (end-of-turn) All_City_Calculations (3rd call; ovr119
     * evicted so fire at the Get_Random_Seed landmark, not the stale far-return). */
    static int    acc_armed = 0, acc_done = 0;
    /* 631: _UNITS after Do_All_Units_XP_Check (single Next_Turn_Calc call +0x20B; no eviction). */
    static Bit16u xpchk_ret_seg = 0, xpchk_ret_off = 0;
    static int    xpchk_armed = 0, xpchk_done = 0;
    /* 632: _UNITS after Heal_All_Units (single Next_Turn_Calc call; no eviction -> plain far-return). */
    static Bit16u heal_ret_seg = 0, heal_ret_off = 0;
    static int    heal_armed = 0, heal_done = 0;
    /* 621: _UNITS after All_Outpost_Population_Growth (ovr119 evicted -> fire at the
     * j_Apply_City_Changes landmark, not the stale far-return). */
    static int    aopg_armed = 0, aopg_done = 0;
    /* 600/630/634: _UNITS after each Delete_Dead_Units turn call.  The two Next_Turn_Proc__WIP
     * (ovr060) calls are INTRA-overlay (bypass the stub) -> fire at the j_All_Units_In_Towers
     * landmark that follows each one.  DDU#1 is FIRST (before Next_Turn_Calc) -> 600 on the 1st
     * hit.  DDU#2 is LAST (after Next_Turn_Calc) -> 634; but All_Units_In_Towers is ALSO called
     * from other early sites, so the raw 2nd hit lands early -- gate 634 on acc_done (633 = the
     * last NTC capture) so we take the first hit AFTER Next_Turn_Calc = the true ATow#2.  The
     * one Next_Turn_Calc call is cross-overlay (+0x1E8) -> gated far-return (no eviction). */
    static int    ddu_ntp_n = 0, ddu634_done = 0;
    static Bit16u ddu_ntc_ret_seg = 0, ddu_ntc_ret_off = 0;
    static int    ddu_ntc_armed = 0, ddu_ntc_done = 0;
    /* 627: _UNITS after Determine_Offer (single Next_Turn_Calc call +0x11A; no eviction). */
    static Bit16u doff_ret_seg = 0, doff_ret_off = 0;
    static int    doff_armed = 0, doff_done = 0;
    /* AI_Choose_War_Landmass Get_Weighted_Choice input (607) capture state. */
    static int    gwc_done = 0;
    /* 607 aux: per-landmass min_delta_distance shadow.  min_delta_distance is a
     * scalar reused each landmass iteration, so snapshot Closest_LoadTile_Dist at
     * AICWL loc_F8CA9 (overlay offset 0x43B9) into this array for the 607 dump. */
    static Bit16s og_aicwl_min_delta[OG_AICWL_NUM_LANDMASSES];
    /* 315/316: land/dock squares at Build_*_Linked_List return capture state. */
    static int    land_bld_pending = 0, land_bld_done = 0;
    static int    dock_bld_pending = 0, dock_bld_done = 0;
    /* 608: g_ai_evaluation_map at AI_Evaluation_Map return capture state. */
    static Bit16u evalmap_ret_seg = 0, evalmap_ret_off = 0;
    static int    evalmap_armed = 0, evalmap_done = 0;
    /* 608 per-unit strength: capture AX at each j_Effective_Unit_Strength return that
     * lands back in AI_Evaluation_Map (ovr162 [0x3AAB,0x3F14)), indexed by SI (== itr,
     * captured at the call).  eus_capture gates it to the first AI_Evaluation_Map call. */
    static Bit16u eus_ret_seg = 0, eus_ret_off = 0, eus_si = 0;
    static int    eus_armed = 0, eus_capture = 0;
    /* 317: _ai_reevaluate_continents_countdown at Loaded_Game_Update return state. */
    static Bit16u lgu_ret_seg = 0, lgu_ret_off = 0;
    static int    lgu_armed = 0, lgu_done = 0;
    /* UWATCH: per-instruction value-change watch on _UNITS[OG_WATCH_UNIT_IDX].owner_idx.
     * Reports every time that byte's value changes and names the WRITING instruction:
     * this probe fires pre-instruction (top of the core loop, after LOADIP), so the
     * store landed on the immediately-preceding cs:ip (uw_prev_*).  Reads the live
     * far-ptr each step so it survives a _UNITS realloc.  uw_last==0x7FFFFFFF: unset.
     * uc_last: same watch on the _units active-count scalar (DGROUP word, no far-ptr). */
    static int    uw_last = 0x7FFFFFFF;
    static int    uc_last = 0x7FFFFFFF;
    static Bit16u uw_prev_cs = 0, uw_prev_ip = 0;
    int nlm = (int)(sizeof(gd_wzd_landmarks) / sizeof(gd_wzd_landmarks[0])), i, z;
    if (og_active_exe_idx != last_idx) {
        last_idx = og_active_exe_idx;
        for (z = 0; z < 8; z++) fired[z] = 0;
        aicwl_armed = 0; aicwl_done = 0;
        aievalcont_armed = 0; aievalcont_done = 0;
        dipgro_armed = 0; dipgro_done = 0;
        appcity_armed = 0; appcity_done = 0;
        cap_cs = 0; cap_done = 0; cap_n = 0;
        consec_armed = 0; consec_done = 0;
        aiuo_armed = 0; aiuo_done = 0;
        aiexo_armed = 0; aiexo_done = 0;
        de_armed = 0; de_done = 0;
        mkraid_armed = 0; mkraid_done = 0;
        mkmon_armed = 0; mkmon_done = 0;
        aint_armed = 0; aint_done = 0;
        acc_armed = 0; acc_done = 0;
        xpchk_armed = 0; xpchk_done = 0;
        heal_armed = 0; heal_done = 0;
        aopg_armed = 0; aopg_done = 0;
        ddu_ntp_n = 0; ddu634_done = 0;
        ddu_ntc_armed = 0; ddu_ntc_done = 0;
        doff_armed = 0; doff_done = 0;
        gwc_done = 0;
        for (z = 0; z < OG_AICWL_NUM_LANDMASSES; z++) og_aicwl_min_delta[z] = 0;
        land_bld_pending = 0; land_bld_done = 0;
        dock_bld_pending = 0; dock_bld_done = 0;
        evalmap_armed = 0; evalmap_done = 0;
        eus_armed = 0; eus_capture = 0;
        lgu_armed = 0; lgu_done = 0;
        uw_last = 0x7FFFFFFF; uc_last = 0x7FFFFFFF; uw_prev_cs = 0; uw_prev_ip = 0;
    }

    /* ---- UWATCH: _UNITS[OG_WATCH_UNIT_IDX].owner_idx value-change watch (decl above).
     * Runs on EVERY WIZARDS instruction, BEFORE the `ida < 0` early-out below, so the
     * writer attribution (uw_prev_*) is always the true preceding instruction even when
     * the store or the detecting instruction sits in low memory (BIOS/DOS, ida < 0). */
    {
        Bit16u wds = e->runtime_ds;
        if (wds != 0) {
            Bit16u u_off = real_readw(wds, OG_WZD_UNITS_PTR_OFF);
            Bit16u u_seg = real_readw(wds, (Bit16u)(OG_WZD_UNITS_PTR_OFF + 2));
            if (u_seg != 0) {
                Bit16u eoff = (Bit16u)(u_off
                             + OG_WATCH_UNIT_IDX * OG_UNIT_REC_SIZE + OG_WATCH_OWNER_OFF);
                int cur = (int)(Bit8s)real_readb(u_seg, eoff);
                if (cur != uw_last) {
                    char wbuf[160];
                    og_describe(uw_prev_cs, uw_prev_ip, wbuf, sizeof(wbuf));
                    if (uw_last == 0x7FFFFFFF)
                        fprintf(stderr, "[UWATCH] _UNITS[%d].owner_idx = %d (first observed)"
                                "  @ %04X:%04X %s\n",
                                OG_WATCH_UNIT_IDX, cur, uw_prev_cs, uw_prev_ip, wbuf);
                    else
                        fprintf(stderr, "[UWATCH] _UNITS[%d].owner_idx  %d -> %d"
                                "  writer %04X:%04X %s  (detected @ %04X:%04X)\n",
                                OG_WATCH_UNIT_IDX, uw_last, cur,
                                uw_prev_cs, uw_prev_ip, wbuf, cs_val, ip_val);
                    uw_last = cur;
                }
            }
            /* _units active-count scalar (DGROUP word; independent of the far-ptr). */
            int ucur = (int)(Bit16s)real_readw(wds, OG_WZD_UNITS_NUM_OFF);
            if (ucur != uc_last) {
                char wbuf[160];
                og_describe(uw_prev_cs, uw_prev_ip, wbuf, sizeof(wbuf));
                if (uc_last == 0x7FFFFFFF)
                    fprintf(stderr, "[UWATCH] _units = %d (first observed)"
                            "  @ %04X:%04X %s\n",
                            ucur, uw_prev_cs, uw_prev_ip, wbuf);
                else
                    fprintf(stderr, "[UWATCH] _units  %d -> %d"
                            "  writer %04X:%04X %s  (detected @ %04X:%04X)\n",
                            uc_last, ucur, uw_prev_cs, uw_prev_ip, wbuf, cs_val, ip_val);
                uc_last = ucur;
            }
        }
        uw_prev_cs = cs_val; uw_prev_ip = ip_val;
    }

    Bit32s ida = (Bit32s)(((Bit32u)cs_val << 4) + ip_val) - e->base_offset;
    if (ida < 0) return;

    /* 315/316: land/dock square arrays at their Build_*_Linked_List return, captured
     * right before the builder's EMMDATAH_Map() remaps CONTXXX away (arrays fully
     * built AND CONTXXX still mapped).  Arm a pending flag at the builder's resident
     * far stub (j_Build_Land 0x361C0 / j_Build_Dock 0x361C5), then fire at
     * EMMDATAH_Map's entry (seg013 0x18361, before it executes).  Both builders run
     * once at load (Loaded_Game_Update), sequentially, so no overlap.  Fire once. */
    if (ida == 0x000361C0L && !land_bld_done) land_bld_pending = 1;
    if (ida == 0x000361C5L && !dock_bld_done) dock_bld_pending = 1;
    if (ida == 0x00018361L) {          /* EMMDATAH_Map entry -- CONTXXX still mapped */
        if (land_bld_pending) {
            land_bld_pending = 0; land_bld_done = 1;
            gd_dump_ai_landmass_squares(e->runtime_ds, "315_Build_Land_Linked_List_Squares");
        } else if (dock_bld_pending) {
            dock_bld_pending = 0; dock_bld_done = 1;
            gd_dump_ai_dock_squares(e->runtime_ds, "316_Build_Dock_Linked_List_Squares");
        }
    }

    if (!aicwl_done) {
        if (aicwl_armed && cs_val == aicwl_ret_seg && ip_val == aicwl_ret_off) {
            aicwl_armed = 0;
            aicwl_done  = 1;
            gd_dump_ai_landmass_squares(e->runtime_ds, "605_AI_Choose_War_Landmass_Squares");
            gd_dump_ai_dock_squares(e->runtime_ds, "606_AI_Choose_War_Landmass_Dock_Squares");
        } else if (ida == 0x000361BBL) {   /* j_CONTXXX_Map resident stub entry */
            Bit16u ss_val = SegValue(ss);
            Bit16u r_off  = real_readw(ss_val, (Bit16u)reg_sp);         /* far-call ret off */
            Bit16u r_seg  = real_readw(ss_val, (Bit16u)(reg_sp + 2));   /* far-call ret seg */
            /* Gate by the return's OVERLAY OFFSET within AI_Choose_War_Landmass:
             * [0x3F14, 0x46A2) = func EA 0xF8804 - ovr162 seg EA 0xF48F0, up to
             * the next func (0xF8F92 - 0xF48F0).  og_resolve_cs can't map the
             * overlay CS here (returns 0), but this offset is unique to the AICWL
             * caller among all CONTXXX_Map call sites (others return to 0x3AB8 /
             * 0x2B30 / 0x004C, all outside the range). */
            if (r_off >= 0x3F14 && r_off < 0x46A2) {
                aicwl_ret_seg = r_seg; aicwl_ret_off = r_off; aicwl_armed = 1;
            }
        }
    }

    /* AI_Evaluate_Continents (ovr162): the every-turn, non-throttled continent
     * evaluator that (re)builds the continent status (type_array) + stage points.
     * Capture _ai_continents on-ENTRY (610) and at-RETURN (611).  It is called
     * cross-overlay from AI_Next_Turn (ovr145), so it has a resident far stub
     * j_AI_Evaluate_Continents @ 0x369FF that DOES fire in this per-instruction
     * probe (unlike an intra-overlay NX_j_ near call).  At the stub entry DS = WZD
     * DGROUP (readable) and the far-call arg player_idx is at [SS:SP+4]; the
     * far-call return ([SS:SP]=off, [SS:SP+2]=seg) is where it returns into
     * AI_Next_Turn, so arm 611 there (fires after the full rebuild + EMMDATAH_Map).
     * Fire once (first AI player's call). */
    /* 608: g_ai_evaluation_map at AI_Evaluation_Map return.  Cross-overlay far stub
     * j_AI_Evaluation_Map @ 0x369FA fires; arm there, capture the far-return, fire the
     * dump at the return.  AI_Evaluation_Map has no EMMDATAH_Map() at its end, so
     * CONTXXX is still mapped at return -> the map is readable.  Fire once. */
    if (!evalmap_done) {
        if (evalmap_armed && cs_val == evalmap_ret_seg && ip_val == evalmap_ret_off) {
            evalmap_armed = 0; evalmap_done = 1; eus_capture = 0;
            gd_dump_ai_evaluation_map(e->runtime_ds, "608_AI_Evaluation_Map");
        } else if (eus_capture && eus_armed && cs_val == eus_ret_seg && ip_val == eus_ret_off) {
            /* Effective_Unit_Strength returned into AI_Evaluation_Map's unit loop:
             * AX = raw strength (before the caller's /10), eus_si = itr (unit index
             * captured at the call, so SI-preservation is irrelevant).  Emit indexed
             * by unit -- mirrors ReMoM's per-unit LOG_DEBUG in AI_Evaluation_Map. */
            eus_armed = 0;
            if (eus_si < OG_UNITS_COUNT)
                fprintf(stderr, "[GD] 608_AI_Evaluation_Map_Strength unit_strength[%u].s = %d\n",
                        (unsigned)eus_si, (int)(Bit16s)reg_ax);
        } else if (ida == 0x000369FAL) {   /* j_AI_Evaluation_Map resident far stub */
            Bit16u ss_val = SegValue(ss);
            /* on-entry: the inputs AI_Evaluation_Map sums into g_ai_evaluation_map
             * (_players, _UNITS, _LAIRS, _CITIES, _NODES) -- all DGROUP, readable at
             * the stub -- to pin which drives the 608 strength divergence. */
            gd_dump_players_at(e->runtime_ds, OG_WZD_PLAYERS_OFF, "608_AI_Evaluation_Map_P");
            gd_dump_recs_at(e->runtime_ds, OG_WZD_UNITS_PTR_OFF, OG_WZD_UNITS_NUM_OFF, "_units", unit_fields, UNIT_FIELD_COUNT, OG_UNIT_REC_SIZE, OG_UNITS_COUNT, "_UNITS", "608_AI_Evaluation_Map_U");
            gd_dump_recs_at(e->runtime_ds, OG_WZD_LAIRS_PTR_OFF, 0, 0, lair_fields, LAIR_FIELD_COUNT, OG_LAIR_REC_SIZE, OG_LAIRS_COUNT, "_LAIRS", "608_AI_Evaluation_Map_L");
            gd_dump_cities_at(e->runtime_ds, OG_WZD_CITIES_PTR_OFF, OG_WZD_CITIES_NUM_OFF, "608_AI_Evaluation_Map_C");
            gd_dump_recs_at(e->runtime_ds, OG_WZD_NODES_PTR_OFF, 0, 0, node_fields, NODE_FIELD_COUNT, OG_NODE_REC_SIZE, OG_NODES_COUNT, "_NODES", "608_AI_Evaluation_Map_N");
            evalmap_ret_off = real_readw(ss_val, (Bit16u)reg_sp);
            evalmap_ret_seg = real_readw(ss_val, (Bit16u)(reg_sp + 2));
            evalmap_armed = 1;
            eus_capture = 1;   /* capture per-unit strengths until this call returns */
        } else if (eus_capture && ida == 0x00035797L) {   /* j_Effective_Unit_Strength stub */
            /* Arm on the resident far stub; gate by the far-call return offset landing
             * inside AI_Evaluation_Map (ovr162 [0x3AAB,0x3F14)) so only its EUS calls
             * (not nested EUS-from-EUS, which returns into ovr121) arm.  Snapshot SI=itr
             * now (before EUS runs) so the return handler needn't trust SI-preservation. */
            Bit16u ss_val = SegValue(ss);
            Bit16u r_off  = real_readw(ss_val, (Bit16u)reg_sp);
            Bit16u r_seg  = real_readw(ss_val, (Bit16u)(reg_sp + 2));
            if (r_off >= 0x3AAB && r_off < 0x3F14) {
                eus_ret_off = r_off; eus_ret_seg = r_seg;
                eus_si = (Bit16u)(reg_si);
                eus_armed = 1;
            }
        }
    }

    /* 317: _ai_reevaluate_continents_countdown (uint8_t[NUM_PLAYERS] via a DGROUP
     * far-ptr @ 0x8FAA) at Loaded_Game_Update return -- the AI_Evaluate_Continents
     * throttle, initialized at load.  Resident seg001 Loaded_Game_Update @ 0x144A7
     * (large-model far call); arm at entry, fire at the far-return.  Fire once.
     * NOTE: OG stores this UNSIGNED 1-byte; ReMoM stores int16_t -- values match at
     * load (all 0) but can diverge after the AI turn decrements it. */
    if (!lgu_done) {
        if (lgu_armed && cs_val == lgu_ret_seg && ip_val == lgu_ret_off) {
            Bit16u p_off = real_readw(e->runtime_ds, 0x8FAAu);
            Bit16u p_seg = real_readw(e->runtime_ds, 0x8FACu);
            char buf[256]; int k, q = 0; buf[0] = 0;
            lgu_armed = 0; lgu_done = 1;
            for (k = 0; k < 6; k++)   /* NUM_PLAYERS */
                q += snprintf(buf + q, sizeof(buf) - q, k ? ",%d" : "%d",
                              (int)(unsigned char)real_readb(p_seg, (Bit16u)(p_off + k)));
            fprintf(stderr, "[GD] 317_Loaded_Game_Update_Countdown _ai_reevaluate_continents_countdown = %s\n", buf);
        } else if (ida == 0x000144A7L) {   /* Loaded_Game_Update seg001 entry */
            Bit16u ss_val = SegValue(ss);
            lgu_ret_off = real_readw(ss_val, (Bit16u)reg_sp);
            lgu_ret_seg = real_readw(ss_val, (Bit16u)(reg_sp + 2));
            lgu_armed = 1;
        }
    }

    if (!aievalcont_done) {
        if (aievalcont_armed && cs_val == aievalcont_ret_seg && ip_val == aievalcont_ret_off) {
            aievalcont_armed = 0;
            aievalcont_done  = 1;
            gd_dump_ai_continents_at(e->runtime_ds, OG_WZD_AICONT_OFF, "611_AI_Evaluate_Continents_Return_Continents");
        } else if (ida == 0x000369FFL) {   /* j_AI_Evaluate_Continents resident far stub */
            Bit16u ss_val = SegValue(ss);
            /* on-entry (610): _ai_continents before the rebuild. */
            gd_dump_ai_continents_at(e->runtime_ds, OG_WZD_AICONT_OFF, "610_AI_Evaluate_Continents_Entry_Continents");
            fprintf(stderr, "[GD] 610_AI_Evaluate_Continents_Entry_Continents player_idx = %d\n",
                    (int)(short)real_readw(ss_val, (Bit16u)(reg_sp + 4)));
            /* arm 611 at the far-call return into AI_Next_Turn. */
            aievalcont_ret_off = real_readw(ss_val, (Bit16u)reg_sp);
            aievalcont_ret_seg = real_readw(ss_val, (Bit16u)(reg_sp + 2));
            aievalcont_armed   = 1;
        }
    }

    /* 628/629: _players BEFORE (628) and AFTER (629) Diplomacy_Growth -- the next-turn
     * diplomacy pass that updates the treaty/peace/exchange modifiers, downstream of the
     * All_Outpost_Population_Growth RNG desync.  Called cross-overlay (ovr085) so the
     * resident far stub j_Diplomacy_Growth @ 0x34B05 fires in this per-instruction probe;
     * arm there, dump _players (625), capture the far-call return, dump again at the
     * return (629).  Diplomacy_Growth loops players internally -> one call/turn.  Fire once. */
    if (!dipgro_done) {
        if (dipgro_armed && cs_val == dipgro_ret_seg && ip_val == dipgro_ret_off) {
            dipgro_armed = 0; dipgro_done = 1;
            gd_dump_players_at(e->runtime_ds, OG_WZD_PLAYERS_OFF, "629_Diplomacy_Growth_Return_P");
        } else if (ida == 0x00034B05L) {   /* j_Diplomacy_Growth resident far stub */
            gd_dump_players_at(e->runtime_ds, OG_WZD_PLAYERS_OFF, "628_Diplomacy_Growth_Entry_P");
            Bit16u ss_val = SegValue(ss);
            dipgro_ret_off = real_readw(ss_val, (Bit16u)reg_sp);
            dipgro_ret_seg = real_readw(ss_val, (Bit16u)(reg_sp + 2));
            dipgro_armed   = 1;
        }
    }

    /* 622/626: _CITIES BEFORE (622) and AFTER (626) Apply_City_Changes -- the per-turn
     * city-update pass (outpost graduation/failure + population growth), called
     * cross-overlay from Next_Turn_Calc (ovr119) so the resident far stub
     * j_Apply_City_Changes @ 0x35F0B (ovr140) fires here; arm there, dump _CITIES (622),
     * capture the far-call return, dump again at the return (626).  Apply_City_Changes
     * loops all cities internally and ends with Volcano_Counts() before returning, so
     * the return (621-style far return into Next_Turn_Calc) is post-Volcano_Counts --
     * matching ReMoM's 631 placement.  Called once/turn.  Fire once. */
    if (!appcity_done) {
        if (appcity_armed && cs_val == appcity_ret_seg && ip_val == appcity_ret_off) {
            appcity_armed = 0; appcity_done = 1;
            gd_dump_cities_at(e->runtime_ds, OG_WZD_CITIES_PTR_OFF, OG_WZD_CITIES_NUM_OFF, "626_Apply_City_Changes_Return_C");
            gd_dump_recs_at(e->runtime_ds, OG_WZD_UNITS_PTR_OFF, OG_WZD_UNITS_NUM_OFF, "_units", unit_fields, UNIT_FIELD_COUNT, OG_UNIT_REC_SIZE, OG_UNITS_COUNT, "_UNITS", "626_Apply_City_Changes_Return_U");
            /* 625_Volcano_Counts: Volcano_Counts is Apply_City_Changes's LAST op, so "after
             * Volcano_Counts" == this Apply_City_Changes return state.  Captured here (robust)
             * rather than at a Volcano_Counts far-return (eviction risk, no landmark after it). */
            gd_dump_recs_at(e->runtime_ds, OG_WZD_UNITS_PTR_OFF, OG_WZD_UNITS_NUM_OFF, "_units", unit_fields, UNIT_FIELD_COUNT, OG_UNIT_REC_SIZE, OG_UNITS_COUNT, "_UNITS", "625_Volcano_Counts_Return_U");
        } else if (ida == 0x00035F0BL) {   /* j_Apply_City_Changes resident far stub */
            gd_dump_cities_at(e->runtime_ds, OG_WZD_CITIES_PTR_OFF, OG_WZD_CITIES_NUM_OFF, "622_Apply_City_Changes_Entry_C");
            Bit16u ss_val = SegValue(ss);
            appcity_ret_off = real_readw(ss_val, (Bit16u)reg_sp);
            appcity_ret_seg = real_readw(ss_val, (Bit16u)(reg_sp + 2));
            appcity_armed   = 1;
        }
    }

    /* 623 sub: _UNITS AFTER City_Apply_Production (ReMoM NEXTTURN.c:3141) -- the per-city
     * production pass that calls Create_Unit() for completed unit builds.  It's INTRA-overlay
     * (ovr140:Apply_City_Changes -> ovr140:City_Apply_Production, CALLN), so no resident stub
     * fires for it; learn ovr140's runtime CS from the cross-overlay stub j_City_Current_-
     * Product_Cost @ 0x35FEB (called inside City_Apply_Production -- its far-return seg IS the
     * ovr140 CS), then count City_Apply_Production returns (IP 0x1987 = Apply_City_Changes
     * ovr140-offset 0x14ED + CALLN return 0x49A).  Matches ReMoM's `itr_cities == 9` gate:
     * City_Apply_Production is called once per loop iteration in order (no skip/continue), so
     * the 10th return == itr_cities 9 -- one of the 3 cities (9/13/17) whose production creates
     * the divergent units.  Gated to the Apply_City_Changes window by appcity_armed.  Fire once. */
    if (!cap_done && appcity_armed) {
        if (cap_cs && cs_val == cap_cs && ip_val == 0x1987) {
            if (++cap_n == 10) {   /* 10th City_Apply_Production return == itr_cities 9 */
                cap_done = 1;
                gd_dump_recs_at(e->runtime_ds, OG_WZD_UNITS_PTR_OFF, OG_WZD_UNITS_NUM_OFF, "_units", unit_fields, UNIT_FIELD_COUNT, OG_UNIT_REC_SIZE, OG_UNITS_COUNT, "_UNITS", "623_City_Apply_Production_Return_U");
            }
        } else if (!cap_cs && ida == 0x00035FEBL) {   /* j_City_Current_Product_Cost -- learn ovr140 CS */
            Bit16u ss_val = SegValue(ss);
            cap_cs = real_readw(ss_val, (Bit16u)(reg_sp + 2));
        }
    }

    /* 624 sub: _UNITS AFTER Apply_Consecration (ReMoM NEXTTURN.c:3152) -- the CONSECRATION
     * enchantment handler in Apply_City_Changes's loop.  Cross-overlay stub
     * j_Apply_Consecration @ 0x35B15; gated to the Apply_City_Changes context by appcity_armed;
     * arm at the stub, capture the far-return into Apply_City_Changes, dump _UNITS there.
     * Fires only for consecrated cities -- NONE this turn, so untested (may need the
     * eviction/landmark treatment if it ever fires and misses).  Fire once. */
    if (!consec_done && appcity_armed) {
        if (consec_armed && cs_val == consec_ret_seg && ip_val == consec_ret_off) {
            consec_armed = 0; consec_done = 1;
            gd_dump_recs_at(e->runtime_ds, OG_WZD_UNITS_PTR_OFF, OG_WZD_UNITS_NUM_OFF, "_units", unit_fields, UNIT_FIELD_COUNT, OG_UNIT_REC_SIZE, OG_UNITS_COUNT, "_UNITS", "624_Apply_Consecration_Return_U");
        } else if (ida == 0x00035B15L) {   /* j_Apply_Consecration resident far stub */
            Bit16u ss_val = SegValue(ss);
            consec_ret_off = real_readw(ss_val, (Bit16u)reg_sp);
            consec_ret_seg = real_readw(ss_val, (Bit16u)(reg_sp + 2));
            consec_armed   = 1;
        }
    }

    /* 612/613: _UNITS BEFORE (612) and AFTER (613) AI_Set_Unit_Orders -- the per-AI-
     * player order/movement setter (sets dst/Status/moves via AI_Move_Out_Boats + the
     * stack-expedition logic; the 911 movement divergence).  Called cross-overlay from
     * AI_Next_Turn (ovr145) so the resident far stub j_AI_Set_Unit_Orders @ 0x36710
     * (ovr158) fires here; arm there, dump _UNITS + player_idx ([SS:SP+4], the far-call
     * arg) (612), capture the far return into AI_Next_Turn, dump _UNITS again at the
     * return (613).  Called once per AI player; fire once (first, matching ReMoM). */
    if (!aiuo_done) {
        if (aiuo_armed && cs_val == aiuo_ret_seg && ip_val == aiuo_ret_off) {
            aiuo_armed = 0; aiuo_done = 1;
            gd_dump_recs_at(e->runtime_ds, OG_WZD_UNITS_PTR_OFF, OG_WZD_UNITS_NUM_OFF, "_units", unit_fields, UNIT_FIELD_COUNT, OG_UNIT_REC_SIZE, OG_UNITS_COUNT, "_UNITS", "613_AI_Set_Unit_Orders_Return_U");
        } else if (ida == 0x00036710L) {   /* j_AI_Set_Unit_Orders resident far stub */
            Bit16u ss_val = SegValue(ss);
            gd_dump_recs_at(e->runtime_ds, OG_WZD_UNITS_PTR_OFF, OG_WZD_UNITS_NUM_OFF, "_units", unit_fields, UNIT_FIELD_COUNT, OG_UNIT_REC_SIZE, OG_UNITS_COUNT, "_UNITS", "612_AI_Set_Unit_Orders_Entry_U");
            fprintf(stderr, "[GD] 612_AI_Set_Unit_Orders_Entry_U player_idx = %d\n",
                    (int)(short)real_readw(ss_val, (Bit16u)(reg_sp + 4)));
            aiuo_ret_off = real_readw(ss_val, (Bit16u)reg_sp);
            aiuo_ret_seg = real_readw(ss_val, (Bit16u)(reg_sp + 2));
            aiuo_armed   = 1;
        }
    }

    /* 614/615: _UNITS BEFORE (614) and AFTER (615) AI_Execute_Orders -- the per-AI-player
     * order EXECUTION pass that dispatches each unit by Status (the 911 movement
     * divergence; where dst/Status get applied).  ReMoM AI_Execute_Orders == OG
     * AI_MoveUnits (ovr100; user reconciled the __WIP name in the .fwv), called
     * cross-overlay from AI_Next_Turn (ovr145) so the resident far stub
     * j_AI_Execute_Orders @ 0x35010 (stub100) fires here; arm there, dump _UNITS +
     * player_idx ([SS:SP+4]) (614), capture the far return into AI_Next_Turn, dump _UNITS
     * again at the return (615).  Called once per AI player; fire once (first). */
    if (!aiexo_done) {
        if (aiexo_armed && cs_val == aiexo_ret_seg && ip_val == aiexo_ret_off) {
            aiexo_armed = 0; aiexo_done = 1;
            gd_dump_recs_at(e->runtime_ds, OG_WZD_UNITS_PTR_OFF, OG_WZD_UNITS_NUM_OFF, "_units", unit_fields, UNIT_FIELD_COUNT, OG_UNIT_REC_SIZE, OG_UNITS_COUNT, "_UNITS", "615_AI_Execute_Orders_Return_U");
        } else if (ida == 0x00035010L) {   /* j_AI_Execute_Orders resident far stub (ovr100 AI_MoveUnits) */
            Bit16u ss_val = SegValue(ss);
            gd_dump_recs_at(e->runtime_ds, OG_WZD_UNITS_PTR_OFF, OG_WZD_UNITS_NUM_OFF, "_units", unit_fields, UNIT_FIELD_COUNT, OG_UNIT_REC_SIZE, OG_UNITS_COUNT, "_UNITS", "614_AI_Execute_Orders_Entry_U");
            fprintf(stderr, "[GD] 614_AI_Execute_Orders_Entry_U player_idx = %d\n",
                    (int)(short)real_readw(ss_val, (Bit16u)(reg_sp + 4)));
            aiexo_ret_off = real_readw(ss_val, (Bit16u)reg_sp);
            aiexo_ret_seg = real_readw(ss_val, (Bit16u)(reg_sp + 2));
            aiexo_armed   = 1;
        }
    }

    /* 620: OG-SIDE diagnostic capture of Determine_Event's random-event GATE inputs --
     * event_pressure_accumulator ([BP-2], the value Random(512) is compared against at
     * EVENTS.c:301) and the global _event_delay_count (WZD DGROUP offset 0x4A32, seg
     * 36AA == runtime_ds).  Determine_Event (ovr080) calls Random exactly once (resident
     * seg022 @ 0x1D5A8) at +0x19E; at Random's entry BP is still Determine_Event's frame
     * (Random's prologue hasn't run yet) and SS is unchanged, so [SS:BP-2] is the
     * finalized accumulator.  Arm at the j_Determine_Event stub (0x34950), capture at the
     * first Random entry after that.  Called once/turn (only when random events are
     * enabled); fire once.  OG-side only (no paired ReMoM dump requested). */
    if (!de_done) {
        if (de_armed && ida == 0x0001D5A8L) {   /* Random entry, reached from Determine_Event */
            Bit16u ss_val = SegValue(ss);
            Bit16s epa = (Bit16s)real_readw(ss_val, (Bit16u)(reg_bp - 2));       /* event_pressure_accumulator @ [BP-2] */
            Bit16s edc = (Bit16s)real_readw(e->runtime_ds, (Bit16u)0x4A32);      /* _event_delay_count @ DGROUP:0x4A32 */
            fprintf(stderr, "[GD] 620_Determine_Event event_pressure_accumulator = %d\n", (int)epa);
            fprintf(stderr, "[GD] 620_Determine_Event _event_delay_count = %d\n", (int)edc);
            de_armed = 0; de_done = 1;
        } else if (ida == 0x00034950L) {   /* j_Determine_Event resident stub -- arm */
            de_armed = 1;
        }
    }

    /* 616/617: _UNITS AFTER Make_Raiders (616) and Make_Monsters (617) -- the NPC
     * unit-generation passes, called cross-overlay once/turn from AI_Next_Turn (ovr145)
     * so the resident far stubs j_Make_Raiders @ 0x36A7A and j_Make_Monsters @ 0x36A75
     * (both ovr164) fire here; arm at the stub, capture the far return into AI_Next_Turn,
     * dump _UNITS at the return (after-only -- these create the divergent late unit
     * records, e.g. _UNITS[48..50]).  Fire once each. */
    if (!mkraid_done) {
        if (mkraid_armed && cs_val == mkraid_ret_seg && ip_val == mkraid_ret_off) {
            mkraid_armed = 0; mkraid_done = 1;
            gd_dump_recs_at(e->runtime_ds, OG_WZD_UNITS_PTR_OFF, OG_WZD_UNITS_NUM_OFF, "_units", unit_fields, UNIT_FIELD_COUNT, OG_UNIT_REC_SIZE, OG_UNITS_COUNT, "_UNITS", "616_Make_Raiders_Return_U");
        } else if (ida == 0x00036A7AL) {   /* j_Make_Raiders resident far stub */
            Bit16u ss_val = SegValue(ss);
            mkraid_ret_off = real_readw(ss_val, (Bit16u)reg_sp);
            mkraid_ret_seg = real_readw(ss_val, (Bit16u)(reg_sp + 2));
            mkraid_armed   = 1;
        }
    }

    if (!mkmon_done) {
        if (mkmon_armed && cs_val == mkmon_ret_seg && ip_val == mkmon_ret_off) {
            mkmon_armed = 0; mkmon_done = 1;
            gd_dump_recs_at(e->runtime_ds, OG_WZD_UNITS_PTR_OFF, OG_WZD_UNITS_NUM_OFF, "_units", unit_fields, UNIT_FIELD_COUNT, OG_UNIT_REC_SIZE, OG_UNITS_COUNT, "_UNITS", "617_Make_Monsters_Return_U");
        } else if (ida == 0x00036A75L) {   /* j_Make_Monsters resident far stub */
            Bit16u ss_val = SegValue(ss);
            mkmon_ret_off = real_readw(ss_val, (Bit16u)reg_sp);
            mkmon_ret_seg = real_readw(ss_val, (Bit16u)(reg_sp + 2));
            mkmon_armed   = 1;
        }
    }

    /* 618: _UNITS AFTER AI_Next_Turn (the whole AI turn).  AI_Next_Turn (ovr145) is long
     * and evicts its caller Next_Turn_Calc (ovr119), which reloads at a DIFFERENT segment
     * on return -- so matching a captured far-return (seg,off) fails (the seg is stale;
     * ovr119 is observed at 9 different segments).  Instead: arm at the j_AI_Next_Turn
     * stub (0x36175), then fire at the first cross-overlay call Next_Turn_Calc makes on
     * return -- j_Next_Turn_Process_Purify (stub061 @ 0x342E6, from Next_Turn_Calc+0x32).
     * That stub is called EXACTLY ONCE, right after AI_Next_Turn and never during it, so
     * _UNITS there is the AI turn's net state, before any further Next_Turn_Calc phase.
     * Called once/turn.  Fire once. */
    if (!aint_done) {
        if (aint_armed && ida == 0x000342E6L) {   /* j_Next_Turn_Process_Purify -- 1st NTC call after AI_Next_Turn */
            aint_armed = 0; aint_done = 1;
            gd_dump_recs_at(e->runtime_ds, OG_WZD_UNITS_PTR_OFF, OG_WZD_UNITS_NUM_OFF, "_units", unit_fields, UNIT_FIELD_COUNT, OG_UNIT_REC_SIZE, OG_UNITS_COUNT, "_UNITS", "618_AI_Next_Turn_Return_U");
        } else if (ida == 0x00036175L) {   /* j_AI_Next_Turn resident far stub -- arm */
            aint_armed = 1;
        }
    }

    /* 633: _UNITS AFTER the LATE (end-of-turn) All_City_Calculations.  It's called 3x
     * (Init_Overland load + Next_Turn_Calc+0x23 early + Next_Turn_Calc+0x278 late) through
     * one stub j_All_City_Calculations @ 0x35EC5 -- gate to the LATE call by its far-return
     * offset 0x27D (early returns to 0x28, load to Init_Overland+0x1B8).  All_City_Calculations
     * evicts its caller ovr119 (reloads at a new seg on return), so -- like 618 -- don't
     * match the stale far-return; fire at the first Next_Turn_Calc call on return,
     * Get_Random_Seed (seg022 @ 0x1D559, Next_Turn_Calc+0x27D, not called during ACC).
     * Fire once. */
    if (!acc_done) {
        if (acc_armed && ida == 0x0001D559L) {   /* Get_Random_Seed -- 1st NTC call after late ACC */
            acc_armed = 0; acc_done = 1;
            gd_dump_recs_at(e->runtime_ds, OG_WZD_UNITS_PTR_OFF, OG_WZD_UNITS_NUM_OFF, "_units", unit_fields, UNIT_FIELD_COUNT, OG_UNIT_REC_SIZE, OG_UNITS_COUNT, "_UNITS", "633_All_City_Calculations_Return_U");
        } else if (ida == 0x00035EC5L) {   /* j_All_City_Calculations stub -- arm only for the LATE call */
            Bit16u ss_val = SegValue(ss);
            if (real_readw(ss_val, (Bit16u)reg_sp) == 0x027D) acc_armed = 1;
        }
    }

    /* 631: _UNITS AFTER Do_All_Units_XP_Check -- single call from Next_Turn_Calc (+0x20B), runs
     * right before Heal_All_Units.  Cross-overlay stub j_Do_All_Units_XP_Check @ 0x35F38; no
     * eviction (ovr119 stays), so plain far-return: arm at the stub, capture the far return into
     * Next_Turn_Calc (+0x210, the Heal_All_Units call), dump _UNITS there.  Fire once. */
    if (!xpchk_done) {
        if (xpchk_armed && cs_val == xpchk_ret_seg && ip_val == xpchk_ret_off) {
            xpchk_armed = 0; xpchk_done = 1;
            gd_dump_recs_at(e->runtime_ds, OG_WZD_UNITS_PTR_OFF, OG_WZD_UNITS_NUM_OFF, "_units", unit_fields, UNIT_FIELD_COUNT, OG_UNIT_REC_SIZE, OG_UNITS_COUNT, "_UNITS", "631_Do_All_Units_XP_Check_Return_U");
        } else if (ida == 0x00035F38L) {   /* j_Do_All_Units_XP_Check resident far stub */
            Bit16u ss_val = SegValue(ss);
            xpchk_ret_off = real_readw(ss_val, (Bit16u)reg_sp);
            xpchk_ret_seg = real_readw(ss_val, (Bit16u)(reg_sp + 2));
            xpchk_armed   = 1;
        }
    }

    /* 632: _UNITS AFTER Heal_All_Units -- single call from Next_Turn_Calc (+0x210).  Unlike
     * 618/633, Heal_All_Units does NOT evict its caller ovr119 (it stays at its segment on
     * return -- verified), so the plain far-return works: arm at j_Heal_All_Units @ 0x35F2E,
     * capture the far return into Next_Turn_Calc (+0x215, the Record_History call), dump
     * _UNITS there.  Fire once. */
    if (!heal_done) {
        if (heal_armed && cs_val == heal_ret_seg && ip_val == heal_ret_off) {
            heal_armed = 0; heal_done = 1;
            gd_dump_recs_at(e->runtime_ds, OG_WZD_UNITS_PTR_OFF, OG_WZD_UNITS_NUM_OFF, "_units", unit_fields, UNIT_FIELD_COUNT, OG_UNIT_REC_SIZE, OG_UNITS_COUNT, "_UNITS", "632_Heal_All_Units_Return_U");
        } else if (ida == 0x00035F2EL) {   /* j_Heal_All_Units resident far stub */
            Bit16u ss_val = SegValue(ss);
            heal_ret_off = real_readw(ss_val, (Bit16u)reg_sp);
            heal_ret_seg = real_readw(ss_val, (Bit16u)(reg_sp + 2));
            heal_armed   = 1;
        }
    }

    /* 621: _UNITS AFTER All_Outpost_Population_Growth -- single call from Next_Turn_Calc
     * (+0x86).  It EVICTS ovr119 (4530->4606 on return), so -- like 618/629 -- don't match
     * the stale far-return; fire at the first Next_Turn_Calc call on return, which is
     * j_Apply_City_Changes (0x35F0B, Next_Turn_Calc+0x8B, right after; not called during
     * All_Outpost).  Shares the 0x35F0B trigger with the 622/623 block (independent -- both
     * fire at that stub; this dumps _UNITS, that dumps _CITIES).  Fire once. */
    if (!aopg_done) {
        if (aopg_armed && ida == 0x00035F0BL) {   /* j_Apply_City_Changes -- 1st NTC call after All_Outpost */
            aopg_armed = 0; aopg_done = 1;
            gd_dump_recs_at(e->runtime_ds, OG_WZD_UNITS_PTR_OFF, OG_WZD_UNITS_NUM_OFF, "_units", unit_fields, UNIT_FIELD_COUNT, OG_UNIT_REC_SIZE, OG_UNITS_COUNT, "_UNITS", "621_All_Outpost_Population_Growth_Return_U");
        } else if (ida == 0x00036004L) {   /* j_All_Outpost_Population_Growth stub -- arm */
            aopg_armed = 1;
        }
    }

    /* 600/634: _UNITS AFTER the two Next_Turn_Proc__WIP (ovr060) Delete_Dead_Units calls.
     * They are INTRA-overlay (direct CALLN to ovr060:Delete_Dead_Units, NOT via the stub), so
     * trigger on the cross-overlay landmark that immediately follows each one -- j_All_Units_In_
     * Towers (stub059 @ 0x341FC).  Call order in Next_Turn_Proc is:  DDU#1 -> ATow#1 (FIRST,
     * before Next_Turn_Calc) ... Next_Turn_Calc() [601..633] ... DDU#2 -> ATow#2 (LAST, after
     * Next_Turn_Calc).  DDU#1 = the 1st landmark hit -> 600.  For DDU#2 we CANNOT use "the 2nd
     * hit": All_Units_In_Towers is also called from other early sites, so the raw 2nd hit lands
     * before Next_Turn_Calc (was the old 634 bug -- captured pre-city-production, _units=48 /
     * XP=0).  Instead gate on acc_done (633 = the last Next_Turn_Calc capture, at Get_Random_
     * Seed) and take the FIRST hit after it -- that is the true post-NTC ATow#2. */
    if (ida == 0x000341FCL) {   /* j_All_Units_In_Towers -- follows each Next_Turn_Proc DDU */
        ddu_ntp_n++;
        if (ddu_ntp_n == 1)
            gd_dump_recs_at(e->runtime_ds, OG_WZD_UNITS_PTR_OFF, OG_WZD_UNITS_NUM_OFF, "_units", unit_fields, UNIT_FIELD_COUNT, OG_UNIT_REC_SIZE, OG_UNITS_COUNT, "_UNITS", "600_Delete_Dead_Units_NextTurnProc_1_U");
        if (acc_done && !ddu634_done) {   /* first All_Units_In_Towers after 633 == post-NTC ATow#2 */
            ddu634_done = 1;
            gd_dump_recs_at(e->runtime_ds, OG_WZD_UNITS_PTR_OFF, OG_WZD_UNITS_NUM_OFF, "_units", unit_fields, UNIT_FIELD_COUNT, OG_UNIT_REC_SIZE, OG_UNITS_COUNT, "_UNITS", "634_Delete_Dead_Units_NextTurnProc_2_U");
        }
    }

    /* 630: _UNITS AFTER the Next_Turn_Calc (ovr119) Delete_Dead_Units (+0x1E8) -- this one IS
     * cross-overlay, so stub j_Delete_Dead_Units @ 0x34250 fires for it (and the load call);
     * gate to it by its far-return offset 0x1ED.  No eviction here (ovr119 stays), so plain
     * far-return.  Fire once. */
    if (!ddu_ntc_done) {
        if (ddu_ntc_armed && cs_val == ddu_ntc_ret_seg && ip_val == ddu_ntc_ret_off) {
            ddu_ntc_armed = 0; ddu_ntc_done = 1;
            gd_dump_recs_at(e->runtime_ds, OG_WZD_UNITS_PTR_OFF, OG_WZD_UNITS_NUM_OFF, "_units", unit_fields, UNIT_FIELD_COUNT, OG_UNIT_REC_SIZE, OG_UNITS_COUNT, "_UNITS", "630_Delete_Dead_Units_NextTurnCalc_U");
        } else if (ida == 0x00034250L) {   /* j_Delete_Dead_Units stub -- arm only for the NTCalc call */
            Bit16u ss_val = SegValue(ss);
            if (real_readw(ss_val, (Bit16u)reg_sp) == 0x01ED) {
                ddu_ntc_ret_off = real_readw(ss_val, (Bit16u)reg_sp);
                ddu_ntc_ret_seg = real_readw(ss_val, (Bit16u)(reg_sp + 2));
                ddu_ntc_armed   = 1;
            }
        }
    }

    /* 627: _UNITS AFTER Determine_Offer (Next_Turn_Calc+0x11A).  Single call, no eviction
     * (ovr119 stays at its segment on return), so plain far-return: arm at the stub
     * j_Determine_Offer @ 0x35F1A, gate to the Next_Turn_Calc call by return offset 0x11F
     * (vs any ItemMake caller), dump _UNITS at the return.  Fire once. */
    if (!doff_done) {
        if (doff_armed && cs_val == doff_ret_seg && ip_val == doff_ret_off) {
            doff_armed = 0; doff_done = 1;
            gd_dump_recs_at(e->runtime_ds, OG_WZD_UNITS_PTR_OFF, OG_WZD_UNITS_NUM_OFF, "_units", unit_fields, UNIT_FIELD_COUNT, OG_UNIT_REC_SIZE, OG_UNITS_COUNT, "_UNITS", "627_Determine_Offer_Return_U");
        } else if (ida == 0x00035F1AL) {   /* j_Determine_Offer stub */
            Bit16u ss_val = SegValue(ss);
            if (real_readw(ss_val, (Bit16u)reg_sp) == 0x011F) {
                doff_ret_off = real_readw(ss_val, (Bit16u)reg_sp);
                doff_ret_seg = real_readw(ss_val, (Bit16u)(reg_sp + 2));
                doff_armed   = 1;
            }
        }
    }

    /* 607 aux: snapshot per-landmass min_delta_distance (Closest_LoadTile_Dist @
     * [BP-8]) at AICWL loc_F8CA9 (overlay offset 0x43B9 = 0xF8CA9 - 0xF48F0), once
     * per landmass, keyed by itr_landmasses @ [BP-0x0A].  Gate by AICWL's overlay CS
     * (aicwl_ret_seg, learned at the CONTXXX_Map return) so it can't fire in another
     * overlay sharing the segment at the same offset. */
    if (!gwc_done && aicwl_ret_seg && cs_val == aicwl_ret_seg && ip_val == 0x43B9) {
        Bit16u ss_val = SegValue(ss);
        Bit16s itr = (Bit16s)real_readw(ss_val, (Bit16u)(reg_bp - 0x0A));
        if (itr >= 0 && itr < OG_AICWL_NUM_LANDMASSES)
            og_aicwl_min_delta[itr] = (Bit16s)real_readw(ss_val, (Bit16u)(reg_bp - 8));
    }

    /* 607: final_landmass_weights -- AICWL's Continent_Values_2 @ [BP-0x10A], 60
     * int16, the array whose sum is the diverging Get_Weighted_Choice arg.  Trigger
     * at Get_Weighted_Choice's resident entry (WZD seg001 IDA 0x14596), gated by the
     * return offset landing in AICWL's overlay range [0x3F14,0x46A2) -- AICWL's only
     * Get_Weighted_Choice call site.  At the callee's first instruction BP is still
     * AICWL's frame, so read the array + wp ([BP-6]) + player_idx ([BP+6]) +
     * first_hostile_player_idx ([BP-0x18]) BP-relative from SS.  Fire once. */
    if (!gwc_done && ida == 0x00014596L) {
        Bit16u ss_val = SegValue(ss);
        Bit16u r_off  = real_readw(ss_val, (Bit16u)reg_sp);   /* call return offset */
        if (r_off >= 0x3F14 && r_off < 0x46A2) {
            char buf[1024]; int q = 0, k;
            gwc_done = 1;
            buf[0] = 0;
            for (k = 0; k < OG_AICWL_NUM_LANDMASSES; k++) {
                short v = (short)real_readw(ss_val, (Bit16u)(reg_bp - 0x10A + 2 * k));
                q += snprintf(buf + q, sizeof(buf) - (size_t)q, k ? ",%d" : "%d", (int)v);
            }
            fprintf(stderr, "[GD] 607_AI_Choose_War_Landmass_Weights final_landmass_weights = %s\n", buf);
            /* Continent_Values @ [BP-0x92], 60 int16 (retained) -- the C in C*1000/d. */
            q = 0; buf[0] = 0;
            for (k = 0; k < OG_AICWL_NUM_LANDMASSES; k++) {
                short v = (short)real_readw(ss_val, (Bit16u)(reg_bp - 0x92 + 2 * k));
                q += snprintf(buf + q, sizeof(buf) - (size_t)q, k ? ",%d" : "%d", (int)v);
            }
            fprintf(stderr, "[GD] 607_AI_Choose_War_Landmass_Weights Continent_Values = %s\n", buf);
            /* min_delta_distance -- the d in C*1000/d (per-landmass shadow). */
            q = 0; buf[0] = 0;
            for (k = 0; k < OG_AICWL_NUM_LANDMASSES; k++)
                q += snprintf(buf + q, sizeof(buf) - (size_t)q, k ? ",%d" : "%d", (int)og_aicwl_min_delta[k]);
            fprintf(stderr, "[GD] 607_AI_Choose_War_Landmass_Weights min_delta_distance = %s\n", buf);
            fprintf(stderr, "[GD] 607_AI_Choose_War_Landmass_Weights wp = %d\n",
                    (int)(short)real_readw(ss_val, (Bit16u)(reg_bp - 6)));
            fprintf(stderr, "[GD] 607_AI_Choose_War_Landmass_Weights player_idx = %d\n",
                    (int)(short)real_readw(ss_val, (Bit16u)(reg_bp + 6)));
            fprintf(stderr, "[GD] 607_AI_Choose_War_Landmass_Weights first_hostile_player_idx = %d\n",
                    (int)(short)real_readw(ss_val, (Bit16u)(reg_bp - 0x18)));
        }
    }

    for (i = 0; i < nlm; i++) {
        if ((Bit32u)ida != gd_wzd_landmarks[i].ida || fired[i]) continue;
        Bit16u ds = e->runtime_ds;
        if (strcmp(gd_wzd_landmarks[i].point, "aicwl_entry") == 0) {
            /* AI_Choose_War_Landmass on-entry (resident stub -> DS = WZD DGROUP).
             * _FORTRESSES and _CITIES live in DGROUP, not the CONTXXX EMS frame,
             * so both are readable here -- capture the state that gates the
             * landmass weighting: the included-landmass set (_FORTRESSES) and the
             * per-landmass city ownership that builds Continent_Values (_CITIES).
             * Reuses the save-time field maps. */
            fired[i] = 1;
            gd_dump_recs_at(ds, OG_WZD_FORTRESSES_PTR_OFF, 0, 0, fortress_fields, FORTRESS_FIELD_COUNT, OG_FORTRESS_REC_SIZE, OG_FORTRESSES_COUNT, "_FORTRESSES", "601_AI_Choose_War_Landmass_F");
            gd_dump_cities_at(ds, OG_WZD_CITIES_PTR_OFF, OG_WZD_CITIES_NUM_OFF, "602_AI_Choose_War_Landmass_C");
            gd_dump_ai_continents_at(ds, OG_WZD_AICONT_OFF, "603_AI_Choose_War_Landmass_Continents");
            gd_dump_players_at(ds, OG_WZD_PLAYERS_OFF, "604_AI_Choose_War_Landmass_P");
            /* player_idx: the AI_Choose_War_Landmass arg on the far-call stack
             * ([SS:SP]=ret_off, +2=ret_seg, +4=arg) -- which player this (first)
             * call is for.  Tells us whose _ai_continents/_players row the reevaluation reads. */
            fprintf(stderr, "[GD] 604_AI_Choose_War_Landmass_P player_idx = %d\n",
                    (int)(short)real_readw(SegValue(ss), (Bit16u)(reg_sp + 4)));
            continue;
        }
        if (strcmp(gd_wzd_landmarks[i].point, "save_slot1_entry") == 0) {
            /* stub050:j_Save_SAVE_GAM entry.  The CALLF arg save_gam_idx sits on
             * the stack (Borland far call: [SS:SP]=ret_off, +2=ret_seg, +4=arg).
             * Capture ONLY the canonical slot-1 save (idx 0); leave fired[] clear
             * for other slots (e.g. the slot-9 autosave) so the slot-1 call still
             * fires.  This is the in-memory state about to become SAVE1.GAM. */
            Bit16u ss_val = SegValue(ss);
            if (real_readw(ss_val, (Bit16u)(reg_sp + 4)) != 0) continue;
            fired[i] = 1;
            /* 400-series: same arrays as the 300-series (load), captured here at
             * save-ENTRY instead of load-exit. */
            gd_dump_heroes_at (ds, OG_WZD_HEROES_PTR_OFF, "401_Save_Heroes_H");
            gd_dump_players_at(ds, OG_WZD_PLAYERS_OFF,    "402_Save_Players_P");
            gd_dump_tiles_at(ds, OG_WZD_WM_PTR_OFF, "_world_maps", 2, "403_Save_World_Maps_W");
            gd_dump_tiles_at(ds, OG_WZD_LM_PTR_OFF, "_landmasses", 1, "404_Save_Landmasses_L");
            gd_dump_recs_at(ds, OG_WZD_NODES_PTR_OFF,  0, 0,        node_fields,  NODE_FIELD_COUNT,  OG_NODE_REC_SIZE, OG_NODES_COUNT,  "_NODES",  "405_Save_Nodes_N");
            gd_dump_recs_at(ds, OG_WZD_FORTRESSES_PTR_OFF, 0, 0,    fortress_fields, FORTRESS_FIELD_COUNT, OG_FORTRESS_REC_SIZE, OG_FORTRESSES_COUNT, "_FORTRESSES", "406_Save_Fortresses_F");
            gd_dump_recs_at(ds, OG_WZD_TOWERS_PTR_OFF, 0, 0,        tower_fields, TOWER_FIELD_COUNT, OG_TOWER_REC_SIZE, OG_TOWERS_COUNT, "_TOWERS", "407_Save_Towers_T");
            gd_dump_recs_at(ds, OG_WZD_LAIRS_PTR_OFF,  0, 0,        lair_fields,  LAIR_FIELD_COUNT,  OG_LAIR_REC_SIZE, OG_LAIRS_COUNT,  "_LAIRS",  "408_Save_Lairs_L");
            gd_dump_recs_at(ds, OG_WZD_ITEMS_PTR_OFF,  0, 0,        item_fields,  ITEM_FIELD_COUNT,  OG_ITEM_REC_SIZE, OG_ITEMS_COUNT,  "_ITEMS",  "409_Save_Items_I");
            gd_dump_cities_at(ds, OG_WZD_CITIES_PTR_OFF, OG_WZD_CITIES_NUM_OFF,                                                                    "410_Save_Cities_C");
            gd_dump_recs_at(ds, OG_WZD_UNITS_PTR_OFF, OG_WZD_UNITS_NUM_OFF, "_units", unit_fields, UNIT_FIELD_COUNT, OG_UNIT_REC_SIZE, OG_UNITS_COUNT, "_UNITS", "411_Save_Units_U");
            gd_dump_tiles_at(ds, OG_WZD_TS_PTR_OFF,  "_map_square_terrain_specials", 1, "412_Save_Terrain_Specials_T");
            gd_dump_tiles_at(ds, OG_WZD_MSF_PTR_OFF, "_map_square_flags",            1, "413_Save_Map_Flags_M");
            continue;
        }
        if (strcmp(gd_wzd_landmarks[i].point, "save_slot5_entry") == 0) {
            /* Same as save_slot1_entry but for Stage 2's slot-5 (SAVE5) save.
             * This is the in-memory state AFTER Next Turn, about to become
             * SAVE5.GAM -- so the 500-series captures the post-turn divergences
             * that the 300-series (load) can't see.  Gated save_gam_idx==4;
             * leave fired[] clear for other slots. */
            Bit16u ss_val = SegValue(ss);
            if (real_readw(ss_val, (Bit16u)(reg_sp + 4)) != 4) continue;
            fired[i] = 1;
            gd_dump_heroes_at (ds, OG_WZD_HEROES_PTR_OFF, "901_Save_Heroes_H");
            gd_dump_players_at(ds, OG_WZD_PLAYERS_OFF,    "902_Save_Players_P");
            gd_dump_tiles_at(ds, OG_WZD_WM_PTR_OFF, "_world_maps", 2, "903_Save_World_Maps_W");
            gd_dump_tiles_at(ds, OG_WZD_LM_PTR_OFF, "_landmasses", 1, "904_Save_Landmasses_L");
            gd_dump_recs_at(ds, OG_WZD_NODES_PTR_OFF,  0, 0,        node_fields,  NODE_FIELD_COUNT,  OG_NODE_REC_SIZE, OG_NODES_COUNT,  "_NODES",  "905_Save_Nodes_N");
            gd_dump_recs_at(ds, OG_WZD_FORTRESSES_PTR_OFF, 0, 0,    fortress_fields, FORTRESS_FIELD_COUNT, OG_FORTRESS_REC_SIZE, OG_FORTRESSES_COUNT, "_FORTRESSES", "906_Save_Fortresses_F");
            gd_dump_recs_at(ds, OG_WZD_TOWERS_PTR_OFF, 0, 0,        tower_fields, TOWER_FIELD_COUNT, OG_TOWER_REC_SIZE, OG_TOWERS_COUNT, "_TOWERS", "907_Save_Towers_T");
            gd_dump_recs_at(ds, OG_WZD_LAIRS_PTR_OFF,  0, 0,        lair_fields,  LAIR_FIELD_COUNT,  OG_LAIR_REC_SIZE, OG_LAIRS_COUNT,  "_LAIRS",  "908_Save_Lairs_L");
            gd_dump_recs_at(ds, OG_WZD_ITEMS_PTR_OFF,  0, 0,        item_fields,  ITEM_FIELD_COUNT,  OG_ITEM_REC_SIZE, OG_ITEMS_COUNT,  "_ITEMS",  "909_Save_Items_I");
            gd_dump_cities_at(ds, OG_WZD_CITIES_PTR_OFF, OG_WZD_CITIES_NUM_OFF,                                                                    "910_Save_Cities_C");
            gd_dump_recs_at(ds, OG_WZD_UNITS_PTR_OFF, OG_WZD_UNITS_NUM_OFF, "_units", unit_fields, UNIT_FIELD_COUNT, OG_UNIT_REC_SIZE, OG_UNITS_COUNT, "_UNITS", "911_Save_Units_U");
            gd_dump_tiles_at(ds, OG_WZD_TS_PTR_OFF,  "_map_square_terrain_specials", 1, "912_Save_Terrain_Specials_T");
            gd_dump_tiles_at(ds, OG_WZD_MSF_PTR_OFF, "_map_square_flags",            1, "913_Save_Map_Flags_M");
            continue;
        }
        fired[i] = 1;
        /* 300-series: every save-loaded array, in load order, at Load_SAVE_GAM
         * on-return.  Gaps (303 _world_maps, 304 _landmasses, 306 _FORTRESSES,
         * 312/313 raw maps) are Phase 2 -- they need ReMoM-side dumps/field maps. */
        gd_dump_heroes_at (ds, OG_WZD_HEROES_PTR_OFF, "301_Load_Heroes_H");
        gd_dump_players_at(ds, OG_WZD_PLAYERS_OFF,    "302_Load_Players_P");
        gd_dump_tiles_at(ds, OG_WZD_WM_PTR_OFF, "_world_maps", 2, "303_Load_World_Maps_W");
        gd_dump_tiles_at(ds, OG_WZD_LM_PTR_OFF, "_landmasses", 1, "304_Load_Landmasses_L");
        gd_dump_recs_at(ds, OG_WZD_NODES_PTR_OFF,  0, 0,        node_fields,  NODE_FIELD_COUNT,  OG_NODE_REC_SIZE, OG_NODES_COUNT,  "_NODES",  "305_Load_Nodes_N");
        gd_dump_recs_at(ds, OG_WZD_FORTRESSES_PTR_OFF, 0, 0,    fortress_fields, FORTRESS_FIELD_COUNT, OG_FORTRESS_REC_SIZE, OG_FORTRESSES_COUNT, "_FORTRESSES", "306_Load_Fortresses_F");
        gd_dump_recs_at(ds, OG_WZD_TOWERS_PTR_OFF, 0, 0,        tower_fields, TOWER_FIELD_COUNT, OG_TOWER_REC_SIZE, OG_TOWERS_COUNT, "_TOWERS", "307_Load_Towers_T");
        gd_dump_recs_at(ds, OG_WZD_LAIRS_PTR_OFF,  0, 0,        lair_fields,  LAIR_FIELD_COUNT,  OG_LAIR_REC_SIZE, OG_LAIRS_COUNT,  "_LAIRS",  "308_Load_Lairs_L");
        gd_dump_recs_at(ds, OG_WZD_ITEMS_PTR_OFF,  0, 0,        item_fields,  ITEM_FIELD_COUNT,  OG_ITEM_REC_SIZE, OG_ITEMS_COUNT,  "_ITEMS",  "309_Load_Items_I");
        gd_dump_cities_at(ds, OG_WZD_CITIES_PTR_OFF, OG_WZD_CITIES_NUM_OFF,                                                                    "310_Load_Cities_C");
        gd_dump_recs_at(ds, OG_WZD_UNITS_PTR_OFF, OG_WZD_UNITS_NUM_OFF, "_units", unit_fields, UNIT_FIELD_COUNT, OG_UNIT_REC_SIZE, OG_UNITS_COUNT, "_UNITS", "311_Load_Units_U");
        gd_dump_tiles_at(ds, OG_WZD_TS_PTR_OFF,  "_map_square_terrain_specials", 1, "312_Load_Terrain_Specials_T");
        gd_dump_tiles_at(ds, OG_WZD_MSF_PTR_OFF, "_map_square_flags",            1, "313_Load_Map_Flags_M");
        /* 314: _ai_continents (WZD DGROUP struct @ 0x8890) is derived AI scratch
         * state, NOT part of the save -- appended after the 13 canonical load
         * arrays as an at-load in-memory baseline (compare vs 603/610). Readable
         * here: DS = WZD DGROUP at the post_load landmark. */
        gd_dump_ai_continents_at(ds, OG_WZD_AICONT_OFF, "314_Load_Continents");
    }
}

/* ==================================================================== *
 *  RNG-input stream capture + diagnostics (CaptureGameData PRD G11/§5.11)
 *  See STU-Extras/STU-DOSBox/OG-RNG-Instrumentation.md.
 * ==================================================================== *
 *
 *  Hook the resident Random() entry (MGC seg022:Random, IDA 0x1CC84) and log
 *  the argument `n` of every call, mirroring ReMoM's [RNG-CALL] lines so the
 *  two RNG-draw streams diff -> pins the first RNG-input divergence.
 *
 *  Most callers reach Random by a FAR call: at entry the stack is
 *  [SS:SP]=ret_IP, [SS:SP+2]=ret_CS, [SS:SP+4]=n (Borland large model, 16-bit
 *  int) -- so n is at +4.  But some calls (notably an SS=0192 caller) read a
 *  bogus value at +4.  The arg MUST be on the stack or in a register, so
 *  rather than assume +4 we now (a) classify the actual CALL instruction from
 *  the bytes preceding the return address and read n at the CORRECT
 *  displacement, (b) track SS/DS/ES changes so a stack switch before Random is
 *  visible, and (c) flag known-good instances so they don't masquerade as
 *  divergences.
 * -------------------------------------------------------------------- */

/* --- 2b. Segment-register change tracking ----------------------------
 *  Per-instruction (MAGIC only).  Rings on SS changes -- the mystery --
 *  capturing full state; DS/ES values ride along in each record.  Set
 *  OG_SEGCHG_LIVE 1 to also stream every SS/DS/ES change to stderr. */
#define OG_SEGCHG_LIVE 0
#define OG_SEGCHG_RING 64

struct og_segchg_rec {
    unsigned long seq;
    Bit16u cs, ip;
    Bit16u ss, ds, es, sp, bp;
    Bit16u old_ss;
};
static struct og_segchg_rec og_segchg[OG_SEGCHG_RING];
static unsigned       og_segchg_head = 0;   /* next write slot */
static unsigned long  og_segchg_seq  = 0;   /* total SS changes recorded */

static void og_segreg_track(Bit16u cs_val, Bit16u ip_val) {
    static int    last_idx = -2;
    static int    inited   = 0;
    static Bit16u p_ss = 0, p_ds = 0, p_es = 0;
    if (og_active_exe_idx != last_idx) { last_idx = og_active_exe_idx; inited = 0; }
    if (og_active_exe_idx != OG_SR_MGC_EXE_IDX) return;

    Bit16u ss_val = SegValue(ss);   /* *_val: avoid the SegNames-shadow trap */
    Bit16u ds_val = SegValue(ds);
    Bit16u es_val = SegValue(es);
    if (!inited) { p_ss = ss_val; p_ds = ds_val; p_es = es_val; inited = 1; return; }

#if OG_SEGCHG_LIVE
    if (ss_val != p_ss || ds_val != p_ds || es_val != p_es) {
        char nm[96];
        fprintf(stderr, "[SEGCHG] %s%s%scs:ip=%04X:%04X (%s) ss=%04X->%04X ds=%04X->%04X es=%04X->%04X sp=%04X bp=%04X\n",
                (ss_val != p_ss) ? "SS " : "", (ds_val != p_ds) ? "DS " : "", (es_val != p_es) ? "ES " : "",
                cs_val, ip_val, og_describe(cs_val, ip_val, nm, sizeof(nm)),
                p_ss, ss_val, p_ds, ds_val, p_es, es_val,
                (Bit16u)(reg_esp & 0xFFFFu), (Bit16u)(reg_ebp & 0xFFFFu));
    }
#endif

    if (ss_val != p_ss) {   /* ring on SS change; DS/ES captured for context */
        struct og_segchg_rec* r = &og_segchg[og_segchg_head];
        og_segchg_head = (og_segchg_head + 1) % OG_SEGCHG_RING;
        r->seq    = ++og_segchg_seq;
        r->cs     = cs_val; r->ip = ip_val;
        r->ss     = ss_val; r->ds = ds_val; r->es = es_val;
        r->sp     = (Bit16u)(reg_esp & 0xFFFFu);
        r->bp     = (Bit16u)(reg_ebp & 0xFFFFu);
        r->old_ss = p_ss;
    }
    p_ss = ss_val; p_ds = ds_val; p_es = es_val;
}

/* Dump the most recent `n` SS-change ring records (oldest first). */
static void og_segchg_dump_recent(unsigned long call_no, unsigned want) {
    unsigned have = (og_segchg_seq < OG_SEGCHG_RING)
                  ? (unsigned)og_segchg_seq : OG_SEGCHG_RING;
    if (want > have) want = have;
    unsigned i;
    char nm[96];
    for (i = 0; i < want; i++) {
        /* head points at next-write; last written is head-1 */
        unsigned idx = (og_segchg_head + OG_SEGCHG_RING - want + i) % OG_SEGCHG_RING;
        struct og_segchg_rec* r = &og_segchg[idx];
        fprintf(stderr,
                "[OG_RNG_SEG] call=%lu seq=%lu cs:ip=%04X:%04X (%s) ss=%04X->%04X ds=%04X es=%04X sp=%04X bp=%04X\n",
                call_no, r->seq, r->cs, r->ip,
                og_describe(r->cs, r->ip, nm, sizeof(nm)),
                r->old_ss, r->ss, r->ds, r->es, r->sp, r->bp);
    }
}

/* --- 2a. CALL-instruction classifier ---------------------------------
 *  The bytes ending immediately before a return address are the CALL
 *  that produced it.  Return the displacement (from entry SP) of the
 *  first pushed argument; -1 if unrecognized.  `kind` (>=8 bytes) gets
 *  a short mnemonic. */
static inline int og_call_arg_disp(Bit16u ret_seg, Bit16u ret_ip,
                                   char* kind, size_t kindlen) {
    /* Direct forms first (more specific). */
    if (real_readb(ret_seg, (Bit16u)(ret_ip - 5)) == 0x9A) {  /* CALLF ptr16:16 */
        snprintf(kind, kindlen, "farD");  return 4;
    }
    if (real_readb(ret_seg, (Bit16u)(ret_ip - 3)) == 0xE8) {  /* CALL rel16 */
        snprintf(kind, kindlen, "nearD"); return 2;
    }
    /* Indirect FF /reg: reg=3 -> CALLF m16:16, reg=2 -> CALL near m16.
     * Variable length; scan a few preceding bytes for the FF + modrm. */
    int j;
    for (j = 2; j <= 6; j++) {
        if (real_readb(ret_seg, (Bit16u)(ret_ip - j)) != 0xFF) continue;
        Bit8u modrm = real_readb(ret_seg, (Bit16u)(ret_ip - j + 1));
        int reg = (modrm >> 3) & 7;
        if (reg == 3) { snprintf(kind, kindlen, "farI");  return 4; }
        if (reg == 2) { snprintf(kind, kindlen, "nearI"); return 2; }
    }
    snprintf(kind, kindlen, "?");
    return -1;
}

/* --- 2c. Flag / ignore table -----------------------------------------
 *  Known instances, matched by resolved-caller substring (NULL=wildcard),
 *  SS (0xFFFF=wildcard) and/or raw +4 value (-1=wildcard).  Re-key on
 *  caller_substr once the byte window names the caller. */
struct og_rng_flag {
    const char* caller_substr;
    Bit16u      ss;
    int         raw4;
    int         expect_n;
    const char* note;
};
static const struct og_rng_flag og_rng_flags[] = {
    /* call ~67-71: caller ret=9A08:B1D2 on stack SS=0192; +4 reads 1762
     * but the equivalent ReMoM call passes 14 (known-good per user). */
    { NULL, 0x0192, 1762, 14,
      "KNOWN-GOOD: +4 reads 1762 but true n should be 14 -- arg not at far +4 here" },
    { NULL, 0xFFFF, -1, -1, NULL }   /* terminator */
};

static inline const struct og_rng_flag*
og_rng_flag_lookup(const char* caller, Bit16u ss, int raw4) {
    unsigned i;
    for (i = 0; og_rng_flags[i].note != NULL; i++) {
        const struct og_rng_flag* f = &og_rng_flags[i];
        if (f->caller_substr && (!caller || !strstr(caller, f->caller_substr))) continue;
        if (f->ss != 0xFFFF && f->ss != ss) continue;
        if (f->raw4 != -1 && f->raw4 != raw4) continue;
        return f;
    }
    return NULL;
}

#define OG_RNG_RANDOM_IDA_MGC  0x0001CC84u   /* MGC seg022:Random entry */
#define OG_RNG_RANDOM_IDA_WZD  0x0001D5A8u   /* WZD seg022:Random entry */
#define OG_RNG_DETAIL_CAP  120               /* _BYTES/_STK/_SEG cap (+ flagged) */

static void og_rng_probe(Bit16u cs_val, Bit16u ip_val) {
    /* Capture Random() in BOTH EXEs: MAGIC (New Game / segment 1...) and
     * WIZARDS (Load Game / later segment).  Per-EXE Random entry IDA. */
    int idx = og_active_exe_idx;
    Bit32u want_ida;
    if      (idx == 0) want_ida = OG_RNG_RANDOM_IDA_MGC;   /* MAGIC.EXE   */
    else if (idx == 1) want_ida = OG_RNG_RANDOM_IDA_WZD;   /* WIZARDS.EXE */
    else return;
    struct og_exe_info* e = &og_exes[idx];
    Bit32s ida = (Bit32s)(((Bit32u)cs_val << 4) + ip_val) - e->base_offset;
    if (ida < 0 || (Bit32u)ida != want_ida) return;

    static unsigned long call_no = 0;
    call_no++;
    og_rng_seg_idx++;               /* Random count since the current reseed */
    Bit16u ss_val = SegValue(ss);   /* *_val: avoid the SegNames-shadow trap */
    Bit16u sp     = (Bit16u)(reg_esp & 0xFFFFu);
    Bit16u ret_ip = real_readw(ss_val, sp);
    Bit16u ret_cs = real_readw(ss_val, (Bit16u)(sp + 2));
    int    raw4   = (int)(int16_t)real_readw(ss_val, (Bit16u)(sp + 4));

    /* Classify the CALL from the bytes before the return address, then read
     * the arg at the CORRECT displacement instead of assuming far +4. */
    char kind[8];
    int  disp = og_call_arg_disp(ret_cs, ret_ip, kind, sizeof(kind));
    int  n    = (disp > 0)
              ? (int)(int16_t)real_readw(ss_val, (Bit16u)(sp + disp))
              : raw4;
    int  ndisp = (disp > 0) ? disp : 4;

    char caller[96];
    og_describe(ret_cs, ret_ip, caller, sizeof(caller));

    const struct og_rng_flag* flag = og_rng_flag_lookup(caller, ss_val, raw4);

    /* OG random_seed (LCG state) at this entry == the "before" seed for THIS
     * draw -- the probe fires at Random's prologue, before `seed = seed*K+1`.
     * Value-diffing this against ReMoM's [RNG-CALL] before= pins where the two
     * RNG streams first disagree despite matching args/counts:
     *   same call#, DIFFERENT seed  -> seed already offset (unlogged call upstream)
     *   same seed,  DIFFERENT result -> per-n reduction difference.
     * MGC random_seed @ DS:0x431E, WZD @ DS:0x7846 (4 bytes). */
    Bit16u seed_off = (idx == 0) ? 0x431Eu : 0x7846u;
    Bit16u ds_seed  = e->runtime_ds;
    Bit32u og_seed  = (Bit32u)real_readw(ds_seed, seed_off)
                    | ((Bit32u)real_readw(ds_seed, (Bit16u)(seed_off + 2)) << 16);

    fprintf(stderr,
            "[OG_RNG] seg=%lu  sidx=%lu  call=%lu  exe=%s  cs=%04X:%04X  ss=%04X:%04X  kind=%s  arg@+%d=%d  raw4=%d  seed=%08lX  ret=%04X:%04X=%s%s%s\n",
            og_rng_segment, og_rng_seg_idx, call_no, e->label,
            cs_val, ip_val, ss_val, sp, kind, ndisp, n, raw4, (unsigned long)og_seed,
            ret_cs, ret_ip, caller,
            flag ? "  [KNOWN-GOOD] " : "",
            flag ? flag->note : "");

    if (call_no <= OG_RNG_DETAIL_CAP || flag) {
        char w[160];
        /* The CALL instruction that produced ret_ip (8 before | 2 after). */
        fprintf(stderr, "[OG_RNG_BYTES] call=%lu callsite ret=%04X:%04X  %s\n",
                call_no, ret_cs, ret_ip,
                og_fmt_window(ret_cs, ret_ip, 8, 2, w, sizeof(w)));
        /* Confirm the executing point really is Random's prologue. */
        fprintf(stderr, "[OG_RNG_BYTES] call=%lu random   %04X:%04X  %s\n",
                call_no, cs_val, ip_val,
                og_fmt_window(cs_val, ip_val, 4, 8, w, sizeof(w)));
        /* Wide stack window. */
        { int p = 0, k;
          for (k = 0; k <= 16; k += 2)
              p += snprintf(w + p, sizeof(w) - p, " +%d=%04X",
                            k, (unsigned)real_readw(ss_val, (Bit16u)(sp + k)));
          fprintf(stderr, "[OG_RNG_STK] call=%lu ss:sp=%04X:%04X%s\n",
                  call_no, ss_val, sp, w); }
        /* Recent SS-change history -> exposes a stack switch before Random. */
        og_segchg_dump_recent(call_no, 6);
    }
}

/* ============================================================================
 *  Lua screen tracker  (CLAUDE 2026-07: for the ML session recorder)
 *
 *  Latches which top-level SCREEN FUNCTION is currently executing, by watching
 *  for execution at a screen dispatch function's resident INT-3F stub entry.
 *  This is the GROUND-TRUTH "current screen" (the code actually running the
 *  screen's input/output), unlike _current_screen (WZD dseg:9990) which
 *  Screen_Control() overwrites with the NEXT screen it intends to dispatch.
 *
 *  Reuses the proven resolver: og_resolve_cs(cs) -> seg_idx, matched against a
 *  small table of screen-function (seg_idx, ip_offset) entry points precomputed
 *  once from og_func_table (the loaded WZD overlay table).  Names are normalized
 *  (drop "j_"/"NX_j_" prefix and "__WIP" suffix) before matching.
 *
 *  Purely additive and DEFAULT-OFF.  OG_LUA_ScreenTrack_Enable(1) (from the Lua
 *  engine's dosbox.track_screen(true)) arms it; the matchup pipeline leaves it
 *  off and pays a single bool compare per instruction.  ids match the raw WZD
 *  e_SCREENS values where documented; the undocumented extras use ids >= 200.
 * ========================================================================== */
static const struct { const char* name; int id; const char* label; }
OG_SCREEN_FUNCS[] = {
    { "Main_Screen",           5, "Overland"     },
    { "City_Screen",           0, "City"         },
    { "Magic_Screen",          6, "Magic"        },
    { "Production_Screen",     8, "Production"    },
    { "Items_Screen",          9, "Items"        },
    { "Cities_Screen",         3, "Cities"       },
    { "Armies_Screen",         2, "Armies"       },
    { "Road_Build_Screen",     7, "RoadBuild"    },
    { "Advisor_Screen",       14, "Advisor"      },
    { "Diplomacy_Screen",     15, "Diplomacy"    },
    { "Spellbook_Screen",     12, "Spellbook"    },
    { "Enemy_City_Screen",   200, "EnemyCity"    },
    { "Mirror_Screen",       201, "Mirror"       },
    { "Outpost_Screen",      202, "Outpost"      },
    { "Npc_Diplomacy_Screen",203, "NpcDiplomacy" },
    { "Combat_Screen",       204, "Combat"       },
    { "Surveyor_Screen",     205, "Surveyor"     },
    { "Hall_Of_Fame_Screen", 206, "HallOfFame"   },
    { "Settings_Screen",     207, "Settings"     },
    { "Item_Make_Screen",    208, "ItemMake"     },
    { "Spell_Casting_Screen",209, "SpellCasting" },
    { NULL, 0, NULL }
};

/* Runtime linear address of a screen-function's resident stub entry. */
struct og_screen_stub { Bit32u lin; int id; const char* label; };
static struct og_screen_stub og_screen_stubs[64];
static int         og_screen_stub_n        = 0;
static int         og_screen_stub_built     = 0;
static int         og_screen_track_on       = 0;
/* Master gate for the HEAVY matchup instrumentation in og_sort_trace_check.
 * Default 1 (matchup unchanged). The Lua engine clears it when an ML script is
 * loaded, so gameplay/recording runs skip ~11 per-instruction probes -> fast +
 * quiet. The lightweight screen tracker still runs. */
static int         og_heavy_on              = 1;
static Bit32u      og_screen_lo             = 0xFFFFFFFFu;  /* fast-reject band */
static Bit32u      og_screen_hi             = 0;
static int         og_current_screen_id     = -1;
static const char* og_current_screen_label  = "?";

/* Copy func_name minus a leading "NX_j_"/"j_" and a trailing "__WIP" into out. */
static inline void og_screen_normname(const char* in, char* out, size_t n) {
    size_t len;
    if      (strncmp(in, "NX_j_", 5) == 0) in += 5;
    else if (strncmp(in, "j_",    2) == 0) in += 2;
    strncpy(out, in, n - 1);
    out[n - 1] = '\0';
    len = strlen(out);
    if (len >= 5 && strcmp(out + len - 5, "__WIP") == 0) out[len - 5] = '\0';
}

/* True once WIZARDS.EXE is the active exe (its base_offset is then valid). */
static inline int og_screen_is_wzd(void) {
    return og_active_exe_idx >= 0
        && og_exes[og_active_exe_idx].exename
        && strcmp(og_exes[og_active_exe_idx].exename, "wizards.exe") == 0;
}

/* Build the screen-stub table using the SAME resident-code resolution og_describe
 * uses: runtime_linear = IDA_linear + base_offset (the single DOS load relocation,
 * shared by all resident segments incl. the INT-3F stubs). Deferred until WZD is
 * active + its overlay table is loaded. */
static inline void og_screen_build(void) {
    struct og_exe_info* e;
    unsigned f, sg; int s;
    if (!og_screen_is_wzd()) return;
    e = &og_exes[og_active_exe_idx];
    if (e->func_count == 0 || e->seg_count == 0) return;   /* tables not loaded yet */

    og_screen_stub_n = 0;
    og_screen_lo = 0xFFFFFFFFu;
    og_screen_hi = 0;
    for (f = 0; f < e->func_count; f++) {
        char norm[64];
        /* Resident INT-3F thunks only ("j_..."). The bare overlay-body function
         * and the intra-overlay "NX_j_" thunks live at dynamic runtime addresses
         * where base_offset does NOT apply, so their computed address would be
         * bogus. The cross-overlay screen dispatch always goes through the
         * resident "j_" stub, which is what we want to detect. */
        if (strncmp(e->funcs[f].func_name, "j_", 2) != 0) continue;
        og_screen_normname(e->funcs[f].func_name, norm, sizeof norm);
        for (s = 0; OG_SCREEN_FUNCS[s].name; s++) {
            Bit32u seg_ea, ida_ea, rt;
            if (strcmp(norm, OG_SCREEN_FUNCS[s].name) != 0) continue;
            seg_ea = 0xFFFFFFFFu;
            for (sg = 0; sg < e->seg_count; sg++)
                if (e->segs[sg].seg_idx == e->funcs[f].seg_idx) {
                    seg_ea = e->segs[sg].seg_ea; break;
                }
            if (seg_ea == 0xFFFFFFFFu) break;
            ida_ea = seg_ea + e->funcs[f].ip_offset;
            rt     = (Bit32u)((Bit32s)ida_ea + e->base_offset);
            if (og_screen_stub_n < 64) {
                og_screen_stubs[og_screen_stub_n].lin   = rt;
                og_screen_stubs[og_screen_stub_n].id    = OG_SCREEN_FUNCS[s].id;
                og_screen_stubs[og_screen_stub_n].label = OG_SCREEN_FUNCS[s].label;
                if (rt < og_screen_lo) og_screen_lo = rt;
                if (rt > og_screen_hi) og_screen_hi = rt;
                og_screen_stub_n++;
            }
            break;
        }
    }
    if (og_screen_stub_n > 0) {
        og_screen_stub_built = 1;
        fprintf(stderr,
                "[OG_SCREEN] built %d screen entry points (base=%+ld, lin %06X..%06X)\n",
                og_screen_stub_n, (long)e->base_offset,
                (unsigned)og_screen_lo, (unsigned)og_screen_hi);
    }
}

/* Per-instruction hook: if CS:IP is exactly a screen-function stub entry, latch
 * it as the current screen. Fast-rejects most instructions with a band compare. */
static inline void og_screen_track(Bit16u cs_val, Bit16u ip_val) {
    Bit32u lin;
    int    i;
    if (!og_screen_track_on) return;
    if (!og_screen_stub_built) { og_screen_build(); if (!og_screen_stub_built) return; }
    lin = ((Bit32u)cs_val << 4) + ip_val;
    if (lin < og_screen_lo || lin > og_screen_hi) return;
    for (i = 0; i < og_screen_stub_n; i++) {
        if (og_screen_stubs[i].lin == lin) {
            og_current_screen_id    = og_screen_stubs[i].id;
            og_current_screen_label = og_screen_stubs[i].label;
            return;
        }
    }
}

/* Cross-TU bridge for the Lua engine (dosbox_lua.cpp).  NON-static on purpose:
 * og_sort_trace.h is #included by exactly one TU (core_normal.cpp), so these are
 * defined once.  Declarations live in include/dosbox_lua.h.  (See handoff note.) */
void OG_LUA_ScreenTrack_Enable(int on) {
    og_screen_track_on = on ? 1 : 0;
    if (!on) { og_current_screen_id = -1; og_current_screen_label = "?"; }
}
int         OG_LUA_ScreenTrack_Id(void)   { return og_current_screen_id; }
const char* OG_LUA_ScreenTrack_Name(void) { return og_current_screen_label; }
void        OG_LUA_HeavyTrace(int on)     { og_heavy_on = on ? 1 : 0; }

static inline void og_sort_trace_check(void) {
    Bit16u cs_val = SegValue(cs);
    Bit16u ip_val = (Bit16u)(reg_eip & 0xFFFFu);
    Bit16u ds_val = SegValue(ds);

    /* Generic Borland DGROUP gate + project EXE identity bind.
     * Cheap no-op once og_active_exe_idx >= 0.  Also lazy-loads the active EXE's
     * overlay table, which the screen tracker below depends on. */
    og_detect_active_exe(ds_val);

    /* Lua screen tracker: lightweight and self-gated, and needs only the overlay
     * table + base_offset that og_detect_active_exe set up. Runs on BOTH paths. */
    og_screen_track(cs_val, ip_val);

    /* ML/gameplay fast path.  Everything below is the matchup project's heavy
     * per-instruction instrumentation, which the ML/recording path does not need
     * and cannot afford in real time.  The Lua engine clears og_heavy_on when an
     * ML script is loaded (see LUA_Init), so those runs skip it entirely. */
    if (!og_heavy_on) return;

    /* Segment-register (SS/DS/ES) change tracking -- feeds the RNG probe's
     * SS-change history dump (MAGIC only; cheap no-op otherwise). */
    og_segreg_track(cs_val, ip_val);

    /* Diagnostics. */
    og_diag_first_call(cs_val, ip_val);

    /* R3-G6 post-INT-3F instruction-follow probe.  Runs FIRST so the
     * first follow-up line is the very next instruction after CD 3F
     * (and the counter doesn't include CD 3F itself, which is logged
     * by og_overlay_int3f_probe below). */
    og_overlay_int3f_followup_probe(cs_val, ip_val);

    /* INT 3F overlay manager probe (also arms the followup counter). */
    og_overlay_int3f_probe(cs_val, ip_val);

    /* Far-CALL (opcode 9A) probe: one-shot per unique call site. */
    og_far_call_probe(cs_val, ip_val);

    /* NUNA branching-criteria probe (OG side).
     * IDA disassembly of Next_Unit_Nearest_Available (per ReMoM wiki
     * "Borland-Overlay-Function-Example") puts the branch decision at
     * 5315:0A22 with the instruction:
     *
     *   cmp [bp+Closest_Active_Unit], -1    encoded as  83 7E F0 FF
     *
     * Then at 5315:0A2B:
     *
     *   cmp [bp+Closest_Waiting_Unit], -1   encoded as  83 7E F4 FF
     *
     * with locals at:
     *   Closest_Active_Unit  = [BP-0x10]
     *   Closest_Waiting_Unit = [BP-0x0C]
     *
     * Within ovr061 the runtime IP of the first branch instruction is
     * 0x0A22 (Next_Unit_Nearest_Available starts at static/runtime
     * offset 0x8DD; the branch is at NUNA+0x145 = 0xA22).  We detect
     * by matching the exact 4-byte signature `83 7E F0 FF` at IP =
     * 0x0A22 -- cheap, and the signature disambiguates from incidental
     * IP collisions in other overlays. */
    if (ip_val == 0x0A22
        && real_readb(cs_val, ip_val)               == 0x83
        && real_readb(cs_val, (Bit16u)(ip_val + 1)) == 0x7E
        && real_readb(cs_val, (Bit16u)(ip_val + 2)) == 0xF0
        && real_readb(cs_val, (Bit16u)(ip_val + 3)) == 0xFF) {
        Bit16u ss_val = SegValue(ss);
        Bit16u bp_val = (Bit16u)(reg_ebp & 0xFFFFu);
        int16_t closest_active  = (int16_t)real_readw(ss_val, (Bit16u)(bp_val - 0x10));
        int16_t closest_waiting = (int16_t)real_readw(ss_val, (Bit16u)(bp_val - 0x0C));
        const char* branch =
            (closest_active  != -1) ? "Active branch"
          : (closest_waiting != -1) ? "Waiting branch"
          : "neither (plane switch or unavailable)";
        fprintf(stderr,
                "[NUNA] BRANCH  Closest_Active_Unit=%d  Closest_Waiting_Unit=%d  -> %s\n",
                closest_active, closest_waiting, branch);
    }

    /* Stage 1 -- Sort_Research_List trace. */
    og_stage1_detect_sort_overlay(cs_val, ip_val);
    og_stage1_emit_trace_points(cs_val, ip_val);

    /* Stage 2 -- _unit write trace.  No-op unless active EXE is WZD. */
    og_stage2_unit_write_probe(cs_val, ip_val);
    og_stage2_unit_write_site_probe(cs_val, ip_val);
    watch_targets_check(cs_val, ip_val);
    instr_captures_check(cs_val, ip_val);

    /* New-game AI wizard spellranks capture (MAGIC.EXE only). */
    og_spellranks_probe(cs_val, ip_val);

    /* Function entry/exit address discovery (build the capture table). */
    gd_fn_discover(cs_val, ip_val);

    /* Phase 2b: per-function FN-EXIT capture traps (MAGIC.EXE only). */
    gd_fnexit_probe(cs_val, ip_val);

    /* Resident-landmark captures (Save_SAVE_GAM = final record). */
    gd_mgc_landmark_probe(cs_val, ip_val);

    /* WIZARDS post-load resident landmark (Load_SAVE_GAM on-return -> _CITIES). */
    gd_wzd_landmark_probe(cs_val, ip_val);

    /* RNG-input stream: log the arg `n` of every Random() call (MAGIC). */
    og_rng_probe(cs_val, ip_val);

    /* Capture/Inject capture side (og_ci.cpp): emit [CI] records at registered
     * points.  Pass the ACTIVE exe (MAGIC or WIZARDS) + its DGROUP + IDA + the name
     * resolver so og_ci.cpp stays decoupled from this header's static state. */
    {
        int aexe = (og_active_exe_idx == OG_SR_MGC_EXE_IDX) ? 0 :
                   (og_active_exe_idx == OG_SR_WZD_EXE_IDX) ? 1 : -1;
        Bit16u dg = 0;
        Bit32u ci_ida = 0;
        if (aexe >= 0) {
            struct og_exe_info* ae = &og_exes[aexe == 0 ? OG_SR_MGC_EXE_IDX : OG_SR_WZD_EXE_IDX];
            dg = ae->runtime_ds;
            Bit32s d = (Bit32s)(((Bit32u)cs_val << 4) + ip_val) - ae->base_offset;
            ci_ida = (d < 0) ? 0xFFFFFFFFu : (Bit32u)d;   /* 0xFFFFFFFF = never matches a real off */
        }
        og_ci_probe(cs_val, ip_val, ci_ida, aexe, dg, og_describe);
    }
}

#else  /* !OG_SORT_TRACE */

static inline void og_sort_trace_check(void) {}

/* Screen-tracker bridge stubs so the Lua engine still links when tracing is
 * compiled out. */
void        OG_LUA_ScreenTrack_Enable(int on) { (void)on; }
int         OG_LUA_ScreenTrack_Id(void)       { return -1; }
const char* OG_LUA_ScreenTrack_Name(void)     { return "?"; }
void        OG_LUA_HeavyTrace(int on)         { (void)on; }

#endif /* OG_SORT_TRACE */

#endif /* DOSBOX_OG_SORT_TRACE_H */
