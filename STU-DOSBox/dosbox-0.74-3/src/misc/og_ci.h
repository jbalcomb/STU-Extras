/* ==================================================================== *
 *  og_ci -- OG-MoM Capture/Inject (CI) capture side.  (interface)
 * ==================================================================== *
 *  Captures specific OG-MoM memory at registered execution points and
 *  emits one `[CI] <key> <site> = <csv>` line per record.  The pipeline
 *  (extract-ci.py) turns those into og-game-data-capture.fwv, which the
 *  ReMoM side reads back and injects (gd_ci_*), so ReMoM reproduces OG's
 *  exact bytes for values that are otherwise unreproducible (uninitialized
 *  stack autos, OOB-overrun reads, ...).  Distinct from the [GD] capture,
 *  which only *compares* OG vs ReMoM.
 *
 *  Implementation lives in og_ci.cpp (its own TU).  Decoupled from the trace
 *  layer's statics on purpose: all context (MAGIC active?, its DGROUP, the
 *  og_describe resolver) is passed into og_ci_probe(), so og_ci.cpp never
 *  reaches back into og_sort_trace.h.
 * -------------------------------------------------------------------- */
#ifndef DOSBOX_OG_CI_H
#define DOSBOX_OG_CI_H

#include "dosbox.h"
#include <stddef.h>   /* size_t (used in the resolver typedef below) */

/* Resolver passed in from the trace layer (matches og_describe's signature). */
typedef const char* (*og_ci_describe_fn)(Bit16u cs, Bit16u ip, char* buf, size_t buflen);

/* Per-instruction probe (MAGIC or WIZARDS).  Call from og_sort_trace_check():
 *   og_ci_probe(cs, ip, ida, active_exe, dgroup, og_describe);
 * active_exe: 0 = MAGIC.EXE, 1 = WIZARDS.EXE, <0 = neither (probe no-ops).
 * ida = (cs<<4)+ip - active exe base_offset (triggers WZD points). */
void og_ci_probe(Bit16u cs_val, Bit16u ip_val, Bit32u ida, int active_exe,
                 Bit16u dgroup, og_ci_describe_fn describe);

#endif /* DOSBOX_OG_CI_H */
