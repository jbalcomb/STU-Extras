/* ==================================================================== *
 *  dosbox_lua.h -- public hooks for the embedded Lua scripting engine.
 * ==================================================================== *
 *  The engine itself lives in src/misc/dosbox_lua.cpp.  This header only
 *  exposes what OTHER translation units need:
 *    - LUA_Init: the [lua] config-section init (called from dosbox.cpp).
 *    - LUA_Capture*: input-capture taps.  DOSBox's real input choke points
 *      (KEYBOARD_AddKey, Mouse_*) call these so a recording script can log
 *      the human's keyboard/mouse the same way it can inject them.  They are
 *      cheap no-ops unless a script has called dosbox.record_input(true).
 * -------------------------------------------------------------------- */
#ifndef DOSBOX_LUA_H
#define DOSBOX_LUA_H

class Section;

void LUA_Init(Section *sec);

/* Input-capture taps (defined in dosbox_lua.cpp). kbd_key is a KBD_KEYS value
 * (passed as int to avoid pulling keyboard.h into every caller). */
void LUA_CaptureKey(int kbd_key, bool pressed);
void LUA_CaptureMouseButton(int button, bool pressed);
void LUA_CaptureMouseMove(float dx, float dy, float x, float y);

/* MoM screen tracker -- IMPLEMENTED IN src/misc/og_sort_trace.h (compiled into
 * core_normal.o), consumed by the Lua engine. Latches the current screen by
 * detecting entry to a screen dispatch function; default-off until enabled. */
void        OG_LUA_ScreenTrack_Enable(int on);
int         OG_LUA_ScreenTrack_Id(void);
const char* OG_LUA_ScreenTrack_Name(void);

/* Master gate for the matchup project's heavy per-instruction tracing (in
 * og_sort_trace.h). The Lua engine clears it when an ML script is loaded so
 * gameplay/recording runs fast and quiet. Set OG_TRACE_HEAVY in the environment
 * to keep it on (e.g. a matchup + Lua hybrid). */
void        OG_LUA_HeavyTrace(int on);

#endif /* DOSBOX_LUA_H */
