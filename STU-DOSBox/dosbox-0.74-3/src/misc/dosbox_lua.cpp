/* ==================================================================== *
 *  dosbox_lua.cpp -- embedded Lua scripting engine for DOSBox.
 * ==================================================================== *
 *  Milestone 1: load a user .lua script (from the [lua] config section)
 *  and let it READ guest memory while the emulated machine runs.
 *
 *  Surface exposed to Lua as a global `dosbox` table:
 *    -- read guest memory (milestone 1) --
 *      dosbox.readb/readw/readd(seg, off)  -> value  (real-mode seg:off)
 *      dosbox.read_phys_b/w/d(addr)        -> value  (linear/physical)
 *    -- write guest memory (milestone 2) --
 *      dosbox.writeb/writew/writed(seg, off, val)
 *      dosbox.write_phys_b/w/d(addr, val)
 *    -- CPU registers (milestone 2) --
 *      dosbox.get_reg(name)       -> value   name: ax/eax/al/cs/ip/flags/...
 *      dosbox.set_reg(name, val)
 *    -- input CAPTURE (record human play) --
 *      dosbox.record_input(true|false)       start/stop capturing real input
 *      dosbox.poll_input()  -> events, dropped
 *          events = { {type="key", name=.., pressed=..},
 *                     {type="mouse_button", button=.., pressed=..},
 *                     {type="mouse_move", dx=.., dy=.., x=.., y=..}, ... }
 *          (drains the events seen since the last poll; symmetric with the
 *           injection API above, so a trace can be recorded AND replayed.)
 *    -- input injection (milestone 2) --
 *      dosbox.send_key(name, pressed)        key down/up (KBD_KEYS names)
 *      dosbox.tap_key(name [, hold_ms])      press + auto-release (~50ms)
 *      dosbox.mouse_button(btn, pressed)     btn 0=left 1=right 2=middle
 *      dosbox.mouse_move(dx, dy)             relative move
 *      dosbox.mouse_set(xfrac, yfrac)        absolute [0..1] position
 *    -- savestates: fast RAM+CPU snapshot, for RL episode reset --
 *      dosbox.save_state([slot])        -> bool  (slot 0..3, in-memory)
 *      dosbox.load_state([slot])        -> bool
 *      dosbox.save_state_file(path)     -> bool  (persist across runs)
 *      dosbox.load_state_file(path)     -> bool
 *          Saves guest RAM + cpu_regs + Segs + lazy flags + FPU. Does NOT
 *          save device state (VGA memory, PIC queue, timers, DOS handles).
 *          Requires core=normal. See the block comment above the impl.
 *    -- Python bridge: localhost TCP, newline-delimited text (milestone 3) --
 *      dosbox.bridge_listen(port [, accept_timeout_s])  server; waits for agent
 *      dosbox.bridge_send(line)              -> bool   (send one observation)
 *      dosbox.bridge_recv([timeout_ms])      -> line | nil  (recv one action)
 *      dosbox.bridge_connected()             -> bool
 *      dosbox.bridge_close()
 *    -- misc --
 *      dosbox.log(msg)                       (-> stderr, "[LUA] msg")
 *      dosbox.on_frame(fn)                   (fn called ~10x/sec while running)
 *
 *  The on_frame callback is driven by a recurring PIC event, so it ticks
 *  on the *emulated* timeline (i.e. only while the guest is executing) --
 *  which is exactly when the game we want to observe/control is alive.
 *  Input injection mirrors the SDL layer's own calls (KEYBOARD_AddKey /
 *  Mouse_*), so the guest cannot tell scripted input from real input.
 *
 *  Built as strict C++98 (DOSBox 0.74-3 compiles -std=gnu++98).
 * -------------------------------------------------------------------- */

#include "dosbox.h"
#include "dosbox_lua.h" /* LUA_Init, LUA_Capture* (input taps)         */
#include "setup.h"     /* Section, Section_prop, Property              */
#include "mem.h"       /* real_read/write*, mem_read/write*            */
#include "pic.h"       /* PIC_AddEvent, PIC_RemoveEvents               */
#include "regs.h"      /* cpu_regs, Segs, SegValue/SegSet16            */
#include "keyboard.h"  /* KEYBOARD_AddKey, KBD_KEYS                    */
#include "mouse.h"     /* Mouse_ButtonPressed/Released, CursorMoved    */
#include "paging.h"    /* PAGING_ClearTLB (post-restore safety)        */
#include "fpu.h"       /* FPU_rec fpu -- rides along in a snapshot     */
#include "../cpu/lazyflags.h"  /* LazyFlags lflags -- must ride along  */

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include <string>
#include <cstdio>
#include <cstring>
#include <cstdlib>

/* POSIX TCP transport for the Python bridge (Linux/Unix). On other
 * platforms the bridge_* functions compile to graceful no-ops. */
#if !defined(WIN32) && !defined(_WIN32)
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#define BRIDGE_SUPPORTED 1
#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif
#else
#define BRIDGE_SUPPORTED 0
#endif

/* Capture output goes to stderr (greppable, headless-visible, and consistent
 * with the OG-MoM capture pipeline's [CI]/[RNG-CALL] lines) rather than
 * LOG_MSG, which in a C_DEBUG build is routed to the ncurses debugger window. */
#define LUA_LOG(...) do { fprintf(stderr, "[LUA] " __VA_ARGS__); fputc('\n', stderr); } while (0)

/* Older Lua (<5.2) lacks LUA_OK; 5.4 has it, but keep this defensive. */
#ifndef LUA_OK
#define LUA_OK 0
#endif

static lua_State *lua_ctx        = NULL;
static int        frame_cb_ref   = LUA_NOREF;  /* registry ref to on_frame fn */
static bool       frame_scheduled = false;

#define LUA_FRAME_INTERVAL_MS  100.0f          /* ~10 Hz observation tick */

/* -------------------------------------------------------------------- *
 *  Input capture -- taps on DOSBox's real input path (KEYBOARD_AddKey /
 *  Mouse_*).  Events are buffered here and drained once per frame by
 *  dosbox.poll_input().  Disabled (zero cost) until dosbox.record_input(true).
 *  Fill-then-drop buffer: it is drained every frame, so overflow only happens
 *  under a burst larger than one frame's worth; dropped events are counted.
 * -------------------------------------------------------------------- */
enum lua_in_kind { LIN_KEY, LIN_MBTN, LIN_MMOVE };
struct lua_in_ev {
	unsigned char kind;
	unsigned char pressed;
	short         a;                 /* key code (KBD_KEYS) or mouse button */
	float         dx, dy, x, y;      /* mouse motion */
};

#define LUA_IN_RING 512
static lua_in_ev in_ring[LUA_IN_RING];
static int  in_count      = 0;
static int  in_dropped    = 0;
static bool input_capture = false;

static void in_push(unsigned char kind, short a, unsigned char pressed,
                    float dx, float dy, float x, float y) {
	if (in_count >= LUA_IN_RING) { in_dropped++; return; }
	lua_in_ev &e = in_ring[in_count++];
	e.kind = kind; e.a = a; e.pressed = pressed;
	e.dx = dx; e.dy = dy; e.x = x; e.y = y;
}

/* These three are the public taps declared in dosbox_lua.h and called from
 * keyboard.cpp / ints/mouse.cpp.  External linkage (no `static`). */
void LUA_CaptureKey(int kbd_key, bool pressed) {
	if (input_capture) in_push(LIN_KEY, (short)kbd_key, pressed ? 1 : 0, 0, 0, 0, 0);
}
void LUA_CaptureMouseButton(int button, bool pressed) {
	if (input_capture) in_push(LIN_MBTN, (short)button, pressed ? 1 : 0, 0, 0, 0, 0);
}
void LUA_CaptureMouseMove(float dx, float dy, float x, float y) {
	if (input_capture) in_push(LIN_MMOVE, 0, 0, dx, dy, x, y);
}

/* -------------------------------------------------------------------- *
 *  Lua-callable memory readers
 * -------------------------------------------------------------------- */
static int l_readb(lua_State *L) {
	Bit16u seg = (Bit16u)luaL_checkinteger(L, 1);
	Bit16u off = (Bit16u)luaL_checkinteger(L, 2);
	lua_pushinteger(L, (lua_Integer)real_readb(seg, off));
	return 1;
}

static int l_readw(lua_State *L) {
	Bit16u seg = (Bit16u)luaL_checkinteger(L, 1);
	Bit16u off = (Bit16u)luaL_checkinteger(L, 2);
	lua_pushinteger(L, (lua_Integer)real_readw(seg, off));
	return 1;
}

static int l_readd(lua_State *L) {
	Bit16u seg = (Bit16u)luaL_checkinteger(L, 1);
	Bit16u off = (Bit16u)luaL_checkinteger(L, 2);
	lua_pushinteger(L, (lua_Integer)real_readd(seg, off));
	return 1;
}

static int l_read_phys_b(lua_State *L) {
	PhysPt addr = (PhysPt)luaL_checkinteger(L, 1);
	lua_pushinteger(L, (lua_Integer)mem_readb(addr));
	return 1;
}

static int l_read_phys_w(lua_State *L) {
	PhysPt addr = (PhysPt)luaL_checkinteger(L, 1);
	lua_pushinteger(L, (lua_Integer)mem_readw(addr));
	return 1;
}

static int l_read_phys_d(lua_State *L) {
	PhysPt addr = (PhysPt)luaL_checkinteger(L, 1);
	lua_pushinteger(L, (lua_Integer)mem_readd(addr));
	return 1;
}

static int l_log(lua_State *L) {
	const char *s = luaL_checkstring(L, 1);
	LUA_LOG("%s", s);
	return 0;
}

static int l_on_frame(lua_State *L) {
	luaL_checktype(L, 1, LUA_TFUNCTION);
	if (frame_cb_ref != LUA_NOREF)
		luaL_unref(L, LUA_REGISTRYINDEX, frame_cb_ref);
	lua_pushvalue(L, 1);                             /* dup the function      */
	frame_cb_ref = luaL_ref(L, LUA_REGISTRYINDEX);   /* stash in the registry */
	return 0;
}

/* -------------------------------------------------------------------- *
 *  Lua-callable memory writers
 * -------------------------------------------------------------------- */
static int l_writeb(lua_State *L) {
	Bit16u seg = (Bit16u)luaL_checkinteger(L, 1);
	Bit16u off = (Bit16u)luaL_checkinteger(L, 2);
	real_writeb(seg, off, (Bit8u)luaL_checkinteger(L, 3));
	return 0;
}

static int l_writew(lua_State *L) {
	Bit16u seg = (Bit16u)luaL_checkinteger(L, 1);
	Bit16u off = (Bit16u)luaL_checkinteger(L, 2);
	real_writew(seg, off, (Bit16u)luaL_checkinteger(L, 3));
	return 0;
}

static int l_writed(lua_State *L) {
	Bit16u seg = (Bit16u)luaL_checkinteger(L, 1);
	Bit16u off = (Bit16u)luaL_checkinteger(L, 2);
	real_writed(seg, off, (Bit32u)luaL_checkinteger(L, 3));
	return 0;
}

static int l_write_phys_b(lua_State *L) {
	mem_writeb((PhysPt)luaL_checkinteger(L, 1), (Bit8u)luaL_checkinteger(L, 2));
	return 0;
}

static int l_write_phys_w(lua_State *L) {
	mem_writew((PhysPt)luaL_checkinteger(L, 1), (Bit16u)luaL_checkinteger(L, 2));
	return 0;
}

static int l_write_phys_d(lua_State *L) {
	mem_writed((PhysPt)luaL_checkinteger(L, 1), (Bit32u)luaL_checkinteger(L, 2));
	return 0;
}

/* -------------------------------------------------------------------- *
 *  CPU register access (get_reg / set_reg by name)
 *
 *  reg_* macros expand to lvalues into the global cpu_regs, so we keep
 *  name->pointer tables (address of a global subobject is a valid static
 *  initializer).  Segment regs go through SegValue/SegSet16 so the cached
 *  physical base stays consistent.
 * -------------------------------------------------------------------- */
struct u16reg { const char *name; Bit16u *p; };
struct u32reg { const char *name; Bit32u *p; };
struct u8reg  { const char *name; Bit8u  *p; };
struct segreg { const char *name; int idx; };

static const u16reg u16regs[] = {
	{ "ax", &reg_ax }, { "bx", &reg_bx }, { "cx", &reg_cx }, { "dx", &reg_dx },
	{ "si", &reg_si }, { "di", &reg_di }, { "sp", &reg_sp }, { "bp", &reg_bp },
	{ "ip", &reg_ip }, { NULL, NULL }
};
static const u32reg u32regs[] = {
	{ "eax", &reg_eax }, { "ebx", &reg_ebx }, { "ecx", &reg_ecx }, { "edx", &reg_edx },
	{ "esi", &reg_esi }, { "edi", &reg_edi }, { "esp", &reg_esp }, { "ebp", &reg_ebp },
	{ "eip", &reg_eip }, { NULL, NULL }
};
static const u8reg u8regs[] = {
	{ "al", &reg_al }, { "ah", &reg_ah }, { "bl", &reg_bl }, { "bh", &reg_bh },
	{ "cl", &reg_cl }, { "ch", &reg_ch }, { "dl", &reg_dl }, { "dh", &reg_dh },
	{ NULL, NULL }
};
static const segreg segregs[] = {
	{ "es", es }, { "cs", cs }, { "ss", ss }, { "ds", ds },
	{ "fs", fs }, { "gs", gs }, { NULL, 0 }
};

static int l_get_reg(lua_State *L) {
	const char *n = luaL_checkstring(L, 1);
	const segreg *s; const u16reg *r16; const u32reg *r32; const u8reg *r8;
	for (s = segregs; s->name; ++s)
		if (!strcmp(n, s->name)) { lua_pushinteger(L, (lua_Integer)SegValue((SegNames)s->idx)); return 1; }
	for (r16 = u16regs; r16->name; ++r16)
		if (!strcmp(n, r16->name)) { lua_pushinteger(L, (lua_Integer)*r16->p); return 1; }
	for (r32 = u32regs; r32->name; ++r32)
		if (!strcmp(n, r32->name)) { lua_pushinteger(L, (lua_Integer)*r32->p); return 1; }
	for (r8 = u8regs; r8->name; ++r8)
		if (!strcmp(n, r8->name)) { lua_pushinteger(L, (lua_Integer)*r8->p); return 1; }
	if (!strcmp(n, "flags")) { lua_pushinteger(L, (lua_Integer)cpu_regs.flags); return 1; }
	return luaL_error(L, "unknown register '%s'", n);
}

static int l_set_reg(lua_State *L) {
	const char *n = luaL_checkstring(L, 1);
	lua_Integer v = luaL_checkinteger(L, 2);
	const segreg *s; const u16reg *r16; const u32reg *r32; const u8reg *r8;
	for (s = segregs; s->name; ++s)
		if (!strcmp(n, s->name)) { SegSet16((Bitu)s->idx, (Bit16u)v); return 0; }
	for (r16 = u16regs; r16->name; ++r16)
		if (!strcmp(n, r16->name)) { *r16->p = (Bit16u)v; return 0; }
	for (r32 = u32regs; r32->name; ++r32)
		if (!strcmp(n, r32->name)) { *r32->p = (Bit32u)v; return 0; }
	for (r8 = u8regs; r8->name; ++r8)
		if (!strcmp(n, r8->name)) { *r8->p = (Bit8u)v; return 0; }
	if (!strcmp(n, "flags")) { cpu_regs.flags = (Bitu)v; return 0; }
	return luaL_error(L, "unknown register '%s'", n);
}

/* -------------------------------------------------------------------- *
 *  Keyboard injection
 * -------------------------------------------------------------------- */
struct keymap_e { const char *name; KBD_KEYS code; };
static const keymap_e keymap[] = {
	{ "a", KBD_a }, { "b", KBD_b }, { "c", KBD_c }, { "d", KBD_d }, { "e", KBD_e },
	{ "f", KBD_f }, { "g", KBD_g }, { "h", KBD_h }, { "i", KBD_i }, { "j", KBD_j },
	{ "k", KBD_k }, { "l", KBD_l }, { "m", KBD_m }, { "n", KBD_n }, { "o", KBD_o },
	{ "p", KBD_p }, { "q", KBD_q }, { "r", KBD_r }, { "s", KBD_s }, { "t", KBD_t },
	{ "u", KBD_u }, { "v", KBD_v }, { "w", KBD_w }, { "x", KBD_x }, { "y", KBD_y },
	{ "z", KBD_z },
	{ "0", KBD_0 }, { "1", KBD_1 }, { "2", KBD_2 }, { "3", KBD_3 }, { "4", KBD_4 },
	{ "5", KBD_5 }, { "6", KBD_6 }, { "7", KBD_7 }, { "8", KBD_8 }, { "9", KBD_9 },
	{ "f1", KBD_f1 }, { "f2", KBD_f2 }, { "f3", KBD_f3 }, { "f4", KBD_f4 },
	{ "f5", KBD_f5 }, { "f6", KBD_f6 }, { "f7", KBD_f7 }, { "f8", KBD_f8 },
	{ "f9", KBD_f9 }, { "f10", KBD_f10 }, { "f11", KBD_f11 }, { "f12", KBD_f12 },
	{ "esc", KBD_esc }, { "tab", KBD_tab }, { "backspace", KBD_backspace },
	{ "enter", KBD_enter }, { "space", KBD_space },
	{ "leftalt", KBD_leftalt }, { "rightalt", KBD_rightalt },
	{ "leftctrl", KBD_leftctrl }, { "rightctrl", KBD_rightctrl },
	{ "leftshift", KBD_leftshift }, { "rightshift", KBD_rightshift },
	{ "capslock", KBD_capslock }, { "scrolllock", KBD_scrolllock },
	{ "numlock", KBD_numlock },
	{ "grave", KBD_grave }, { "minus", KBD_minus }, { "equals", KBD_equals },
	{ "backslash", KBD_backslash }, { "leftbracket", KBD_leftbracket },
	{ "rightbracket", KBD_rightbracket }, { "semicolon", KBD_semicolon },
	{ "quote", KBD_quote }, { "period", KBD_period }, { "comma", KBD_comma },
	{ "slash", KBD_slash },
	{ "insert", KBD_insert }, { "home", KBD_home }, { "pageup", KBD_pageup },
	{ "delete", KBD_delete }, { "end", KBD_end }, { "pagedown", KBD_pagedown },
	{ "left", KBD_left }, { "up", KBD_up }, { "down", KBD_down }, { "right", KBD_right },
	{ "kp0", KBD_kp0 }, { "kp1", KBD_kp1 }, { "kp2", KBD_kp2 }, { "kp3", KBD_kp3 },
	{ "kp4", KBD_kp4 }, { "kp5", KBD_kp5 }, { "kp6", KBD_kp6 }, { "kp7", KBD_kp7 },
	{ "kp8", KBD_kp8 }, { "kp9", KBD_kp9 },
	{ "kpdivide", KBD_kpdivide }, { "kpmultiply", KBD_kpmultiply },
	{ "kpminus", KBD_kpminus }, { "kpplus", KBD_kpplus },
	{ "kpenter", KBD_kpenter }, { "kpperiod", KBD_kpperiod },
	{ NULL, KBD_NONE }
};

static KBD_KEYS lookup_key(const char *n) {
	const keymap_e *k;
	for (k = keymap; k->name; ++k)
		if (!strcmp(n, k->name)) return k->code;
	return KBD_NONE;
}

static const char *key_name(int code) {
	const keymap_e *k;
	for (k = keymap; k->name; ++k)
		if ((int)k->code == code) return k->name;
	return "?";
}

static int l_send_key(lua_State *L) {
	const char *n = luaL_checkstring(L, 1);
	bool pressed = (lua_toboolean(L, 2) != 0);
	KBD_KEYS code = lookup_key(n);
	if (code == KBD_NONE) return luaL_error(L, "unknown key '%s'", n);
	KEYBOARD_AddKey(code, pressed);
	return 0;
}

/* Auto-release for tap_key: the keycode rides in the PIC event value. */
static void LUA_KeyRelease(Bitu val) {
	KEYBOARD_AddKey((KBD_KEYS)val, false);
}

static int l_tap_key(lua_State *L) {
	const char *n = luaL_checkstring(L, 1);
	float hold_ms = (float)luaL_optnumber(L, 2, 50.0);
	KBD_KEYS code = lookup_key(n);
	if (code == KBD_NONE) return luaL_error(L, "unknown key '%s'", n);
	KEYBOARD_AddKey(code, true);
	PIC_AddEvent(LUA_KeyRelease, hold_ms, (Bitu)code);
	return 0;
}

/* -------------------------------------------------------------------- *
 *  Mouse injection (mirrors sdlmain.cpp's own Mouse_* calls)
 * -------------------------------------------------------------------- */
static int l_mouse_button(lua_State *L) {
	Bit8u button = (Bit8u)luaL_checkinteger(L, 1);
	bool pressed = (lua_toboolean(L, 2) != 0);
	if (pressed) Mouse_ButtonPressed(button);
	else         Mouse_ButtonReleased(button);
	return 0;
}

static int l_mouse_move(lua_State *L) {   /* relative delta */
	float dx = (float)luaL_checknumber(L, 1);
	float dy = (float)luaL_checknumber(L, 2);
	Mouse_CursorMoved(dx, dy, 0.0f, 0.0f, true);
	return 0;
}

static int l_mouse_set(lua_State *L) {    /* absolute fraction [0..1] of screen */
	float x = (float)luaL_checknumber(L, 1);
	float y = (float)luaL_checknumber(L, 2);
	Mouse_CursorMoved(0.0f, 0.0f, x, y, false);
	return 0;
}

/* -------------------------------------------------------------------- *
 *  Python bridge -- newline-delimited text over a localhost TCP socket.
 *
 *  DOSBox is the server: bridge_listen() binds + accepts one client (the
 *  Python agent).  bridge_send()/bridge_recv() then exchange one line each
 *  per on_frame in synchronous lockstep -- the emulated machine advances
 *  exactly one step per agent action, so a training run is reproducible.
 *  A recv timeout keeps a dead agent from hanging the emulator forever.
 * -------------------------------------------------------------------- */
static int         bridge_listen_fd = -1;
static int         bridge_client_fd = -1;
static std::string bridge_rx;                 /* stream buffer for line assembly */

static void bridge_client_disconnect(void) {
#if BRIDGE_SUPPORTED
	if (bridge_client_fd >= 0) { close(bridge_client_fd); bridge_client_fd = -1; }
#endif
	bridge_rx.clear();
}

static void bridge_shutdown(void) {
#if BRIDGE_SUPPORTED
	if (bridge_client_fd >= 0) { close(bridge_client_fd); bridge_client_fd = -1; }
	if (bridge_listen_fd >= 0) { close(bridge_listen_fd); bridge_listen_fd = -1; }
#endif
	bridge_rx.clear();
}

static int l_bridge_listen(lua_State *L) {
#if BRIDGE_SUPPORTED
	int port = (int)luaL_checkinteger(L, 1);
	double tmo = luaL_optnumber(L, 2, 60.0);   /* accept timeout, seconds; <0 = block */
	struct sockaddr_in addr;
	int yes = 1;
	fd_set rf;
	struct timeval tv;
	int sel;

	bridge_shutdown();

	bridge_listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (bridge_listen_fd < 0) { LUA_LOG("bridge: socket() failed"); lua_pushboolean(L, 0); return 1; }
	setsockopt(bridge_listen_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	addr.sin_port = htons((unsigned short)port);
	if (bind(bridge_listen_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		LUA_LOG("bridge: bind(port %d) failed: %s", port, strerror(errno));
		bridge_shutdown(); lua_pushboolean(L, 0); return 1;
	}
	if (listen(bridge_listen_fd, 1) < 0) {
		LUA_LOG("bridge: listen() failed: %s", strerror(errno));
		bridge_shutdown(); lua_pushboolean(L, 0); return 1;
	}
	LUA_LOG("bridge: listening on 127.0.0.1:%d, waiting for agent (timeout %.0fs)...", port, tmo);

	FD_ZERO(&rf); FD_SET(bridge_listen_fd, &rf);
	tv.tv_sec  = (long)tmo;
	tv.tv_usec = (long)((tmo - (double)(long)tmo) * 1.0e6);
	sel = select(bridge_listen_fd + 1, &rf, NULL, NULL, (tmo < 0) ? NULL : &tv);
	if (sel <= 0) { LUA_LOG("bridge: no agent connected (timeout)"); lua_pushboolean(L, 0); return 1; }

	bridge_client_fd = accept(bridge_listen_fd, NULL, NULL);
	if (bridge_client_fd < 0) { LUA_LOG("bridge: accept() failed"); lua_pushboolean(L, 0); return 1; }
	LUA_LOG("bridge: agent connected");
	lua_pushboolean(L, 1);
	return 1;
#else
	LUA_LOG("bridge: TCP sockets not supported on this platform build");
	lua_pushboolean(L, 0);
	return 1;
#endif
}

static int l_bridge_send(lua_State *L) {
#if BRIDGE_SUPPORTED
	size_t len;
	const char *s = luaL_checklstring(L, 1, &len);
	std::string out;
	size_t off;
	if (bridge_client_fd < 0) { lua_pushboolean(L, 0); return 1; }
	out.assign(s, len);
	out += '\n';
	off = 0;
	while (off < out.size()) {
		ssize_t n = send(bridge_client_fd, out.data() + off, out.size() - off, MSG_NOSIGNAL);
		if (n <= 0) { bridge_client_disconnect(); lua_pushboolean(L, 0); return 1; }
		off += (size_t)n;
	}
	lua_pushboolean(L, 1);
	return 1;
#else
	lua_pushboolean(L, 0);
	return 1;
#endif
}

static int l_bridge_recv(lua_State *L) {
#if BRIDGE_SUPPORTED
	double tmo_ms = luaL_optnumber(L, 1, 5000.0);   /* recv timeout, ms; <0 = block */
	if (bridge_client_fd < 0) { lua_pushnil(L); return 1; }
	for (;;) {
		std::string::size_type nl = bridge_rx.find('\n');
		if (nl != std::string::npos) {
			std::string line = bridge_rx.substr(0, nl);
			if (!line.empty() && line[line.size() - 1] == '\r')
				line.erase(line.size() - 1);       /* tolerate CRLF */
			bridge_rx.erase(0, nl + 1);
			lua_pushlstring(L, line.data(), line.size());
			return 1;
		}
		{
			fd_set rf;
			struct timeval tv;
			int sel;
			char buf[2048];
			ssize_t n;
			FD_ZERO(&rf); FD_SET(bridge_client_fd, &rf);
			tv.tv_sec  = (long)(tmo_ms / 1000.0);
			tv.tv_usec = (long)((tmo_ms - (double)tv.tv_sec * 1000.0) * 1000.0);
			sel = select(bridge_client_fd + 1, &rf, NULL, NULL, (tmo_ms < 0) ? NULL : &tv);
			if (sel <= 0) { lua_pushnil(L); return 1; }          /* timeout / error */
			n = recv(bridge_client_fd, buf, sizeof(buf), 0);
			if (n <= 0) { bridge_client_disconnect(); lua_pushnil(L); return 1; } /* peer closed */
			bridge_rx.append(buf, (size_t)n);
		}
	}
#else
	lua_pushnil(L);
	return 1;
#endif
}

static int l_bridge_connected(lua_State *L) {
	lua_pushboolean(L, (bridge_client_fd >= 0) ? 1 : 0);
	return 1;
}

static int l_bridge_close(lua_State *L) {
	(void)L;
	bridge_shutdown();
	return 0;
}

/* -------------------------------------------------------------------- *
 *  Input capture -- Lua side (enable + drain)
 * -------------------------------------------------------------------- */
static int l_record_input(lua_State *L) {
	input_capture = (lua_toboolean(L, 1) != 0);
	in_count = 0;                       /* reset buffer on toggle */
	in_dropped = 0;
	return 0;
}

static int l_poll_input(lua_State *L) {
	int i;
	lua_createtable(L, in_count, 0);    /* array of event tables */
	for (i = 0; i < in_count; ++i) {
		lua_in_ev &e = in_ring[i];
		lua_createtable(L, 0, 5);
		if (e.kind == LIN_KEY) {
			lua_pushstring(L, "key");        lua_setfield(L, -2, "type");
			lua_pushstring(L, key_name(e.a));lua_setfield(L, -2, "name");
			lua_pushboolean(L, e.pressed);   lua_setfield(L, -2, "pressed");
		} else if (e.kind == LIN_MBTN) {
			lua_pushstring(L, "mouse_button"); lua_setfield(L, -2, "type");
			lua_pushinteger(L, e.a);           lua_setfield(L, -2, "button");
			lua_pushboolean(L, e.pressed);     lua_setfield(L, -2, "pressed");
		} else { /* LIN_MMOVE */
			lua_pushstring(L, "mouse_move"); lua_setfield(L, -2, "type");
			lua_pushnumber(L, e.dx);         lua_setfield(L, -2, "dx");
			lua_pushnumber(L, e.dy);         lua_setfield(L, -2, "dy");
			lua_pushnumber(L, e.x);          lua_setfield(L, -2, "x");
			lua_pushnumber(L, e.y);          lua_setfield(L, -2, "y");
		}
		lua_rawseti(L, -2, i + 1);
	}
	lua_pushinteger(L, in_dropped);     /* 2nd return: events dropped since last poll */
	in_count = 0;                       /* drain */
	in_dropped = 0;
	return 2;
}

/* -------------------------------------------------------------------- *
 *  Savestates -- fast RAM + CPU snapshot/restore, for RL episode reset.
 *
 *  WHAT IS SAVED: all guest RAM (GetMemBase()..MEM_TotalPages()), the CPU
 *  registers (cpu_regs), the segment registers *and* their cached physical
 *  bases (Segs), the lazy-flag state (lflags) and the FPU (fpu).
 *
 *  Restoring cpu_regs/Segs from inside a frame callback is safe because the
 *  CPU core re-derives its instruction pointer from reg_eip + SegBase(cs) at
 *  the top of every instruction (core_normal.cpp LOADIP), so execution resumes
 *  at the snapshot point on the very next instruction.  lflags rides along so
 *  the first conditional branch after a restore evaluates against the flags
 *  that belonged to the snapshot, not to the instruction we interrupted.
 *
 *  WHAT IS *NOT* SAVED (by design -- this is a fast state snapshot, not a
 *  full machine savestate): device state such as VGA video memory and CRTC
 *  registers, the PIC event queue, timers, DMA/sound, EMS/XMS *mappings*
 *  (the paged data itself lives in RAM and is saved), and host-side DOS
 *  state such as open file handles.  For RAM-based observations -- which is
 *  what this ML pipeline uses -- that is sufficient: the guest program's
 *  state is restored exactly and the game redraws the screen itself.
 *
 *  Use core=normal.  Dynamic cores cache translated blocks and a raw memory
 *  restore bypasses the write handlers that would invalidate them.
 * -------------------------------------------------------------------- */
#define LUA_STATE_SLOTS 4

struct lua_state_slot {
	unsigned char *mem;
	Bitu           mem_size;
	CPU_Regs       regs;
	Segments       segs;
	LazyFlags      lazy;
#if C_FPU
	FPU_rec        fpustate;
#endif
	bool           valid;
};

static lua_state_slot state_slots[LUA_STATE_SLOTS];  /* zero-init: mem=NULL, valid=false */
static lua_state_slot state_filebuf;                 /* scratch for file save/load       */

static bool state_capture(lua_state_slot &s) {
	Bitu   bytes = MEM_TotalPages() * 4096;
	HostPt base  = GetMemBase();
	if (!base || !bytes) return false;
	if (s.mem && s.mem_size != bytes) { free(s.mem); s.mem = NULL; }
	if (!s.mem) {
		s.mem = (unsigned char *)malloc(bytes);
		if (!s.mem) return false;
		s.mem_size = bytes;
	}
	memcpy(s.mem, base, bytes);
	s.regs = cpu_regs;
	s.segs = Segs;
	s.lazy = lflags;
#if C_FPU
	s.fpustate = fpu;
#endif
	s.valid = true;
	return true;
}

static bool state_restore(const lua_state_slot &s) {
	Bitu bytes = MEM_TotalPages() * 4096;
	if (!s.valid || !s.mem || s.mem_size != bytes) return false;
	memcpy(GetMemBase(), s.mem, bytes);
	cpu_regs = s.regs;
	Segs     = s.segs;
	lflags   = s.lazy;
#if C_FPU
	fpu      = s.fpustate;
#endif
	PAGING_ClearTLB();
	return true;
}

static void state_free_all(void) {
	int i;
	for (i = 0; i < LUA_STATE_SLOTS; ++i) {
		if (state_slots[i].mem) { free(state_slots[i].mem); state_slots[i].mem = NULL; }
		state_slots[i].valid = false;
	}
	if (state_filebuf.mem) { free(state_filebuf.mem); state_filebuf.mem = NULL; }
	state_filebuf.valid = false;
}

struct lua_state_hdr {
	char   magic[8];        /* "DBXLUA1" */
	Bit32u mem_size;
	Bit32u regs_size;
	Bit32u segs_size;
	Bit32u lazy_size;
	Bit32u fpu_size;
};

static int l_save_state(lua_State *L) {
	int slot = (int)luaL_optinteger(L, 1, 0);
	if (slot < 0 || slot >= LUA_STATE_SLOTS)
		return luaL_error(L, "save_state: slot %d out of range (0..%d)", slot, LUA_STATE_SLOTS - 1);
	lua_pushboolean(L, state_capture(state_slots[slot]) ? 1 : 0);
	return 1;
}

static int l_load_state(lua_State *L) {
	int slot = (int)luaL_optinteger(L, 1, 0);
	if (slot < 0 || slot >= LUA_STATE_SLOTS)
		return luaL_error(L, "load_state: slot %d out of range (0..%d)", slot, LUA_STATE_SLOTS - 1);
	lua_pushboolean(L, state_restore(state_slots[slot]) ? 1 : 0);
	return 1;
}

static int l_save_state_file(lua_State *L) {
	const char *path = luaL_checkstring(L, 1);
	lua_state_hdr h;
	FILE *fp;
	if (!state_capture(state_filebuf)) { lua_pushboolean(L, 0); return 1; }
	fp = fopen(path, "wb");
	if (!fp) { LUA_LOG("save_state_file: cannot open '%s'", path); lua_pushboolean(L, 0); return 1; }
	memset(&h, 0, sizeof(h));
	memcpy(h.magic, "DBXLUA1", 7);
	h.mem_size  = (Bit32u)state_filebuf.mem_size;
	h.regs_size = (Bit32u)sizeof(CPU_Regs);
	h.segs_size = (Bit32u)sizeof(Segments);
	h.lazy_size = (Bit32u)sizeof(LazyFlags);
#if C_FPU
	h.fpu_size  = (Bit32u)sizeof(FPU_rec);
#endif
	fwrite(&h, sizeof(h), 1, fp);
	fwrite(&state_filebuf.regs, sizeof(CPU_Regs), 1, fp);
	fwrite(&state_filebuf.segs, sizeof(Segments), 1, fp);
	fwrite(&state_filebuf.lazy, sizeof(LazyFlags), 1, fp);
#if C_FPU
	fwrite(&state_filebuf.fpustate, sizeof(FPU_rec), 1, fp);
#endif
	fwrite(state_filebuf.mem, 1, state_filebuf.mem_size, fp);
	fclose(fp);
	LUA_LOG("save_state_file: wrote '%s' (%lu bytes of RAM)", path,
	        (unsigned long)state_filebuf.mem_size);
	lua_pushboolean(L, 1);
	return 1;
}

static int l_load_state_file(lua_State *L) {
	const char *path = luaL_checkstring(L, 1);
	lua_state_hdr h;
	FILE *fp = fopen(path, "rb");
	if (!fp) { LUA_LOG("load_state_file: cannot open '%s'", path); lua_pushboolean(L, 0); return 1; }
	if (fread(&h, sizeof(h), 1, fp) != 1 || memcmp(h.magic, "DBXLUA1", 7) != 0) {
		LUA_LOG("load_state_file: '%s' is not a DBXLUA1 savestate", path);
		fclose(fp); lua_pushboolean(L, 0); return 1;
	}
	if (h.mem_size  != (Bit32u)(MEM_TotalPages() * 4096) ||
	    h.regs_size != (Bit32u)sizeof(CPU_Regs) ||
	    h.segs_size != (Bit32u)sizeof(Segments) ||
	    h.lazy_size != (Bit32u)sizeof(LazyFlags)) {
		LUA_LOG("load_state_file: '%s' layout mismatch (different memsize/build?)", path);
		fclose(fp); lua_pushboolean(L, 0); return 1;
	}
	if (state_filebuf.mem && state_filebuf.mem_size != h.mem_size) {
		free(state_filebuf.mem); state_filebuf.mem = NULL;
	}
	if (!state_filebuf.mem) {
		state_filebuf.mem = (unsigned char *)malloc(h.mem_size);
		if (!state_filebuf.mem) { fclose(fp); lua_pushboolean(L, 0); return 1; }
		state_filebuf.mem_size = h.mem_size;
	}
	fread(&state_filebuf.regs, sizeof(CPU_Regs), 1, fp);
	fread(&state_filebuf.segs, sizeof(Segments), 1, fp);
	fread(&state_filebuf.lazy, sizeof(LazyFlags), 1, fp);
#if C_FPU
	if (h.fpu_size == (Bit32u)sizeof(FPU_rec))
		fread(&state_filebuf.fpustate, sizeof(FPU_rec), 1, fp);
#endif
	if (fread(state_filebuf.mem, 1, h.mem_size, fp) != h.mem_size) {
		LUA_LOG("load_state_file: '%s' truncated", path);
		fclose(fp); lua_pushboolean(L, 0); return 1;
	}
	fclose(fp);
	state_filebuf.valid = true;
	lua_pushboolean(L, state_restore(state_filebuf) ? 1 : 0);
	return 1;
}

/* -------------------------------------------------------------------- *
 *  MoM screen tracker (impl in og_sort_trace.h). Ground-truth "current
 *  screen" from screen-function entry, vs the unreliable _current_screen.
 * -------------------------------------------------------------------- */
static int l_track_screen(lua_State *L) {
	OG_LUA_ScreenTrack_Enable(lua_toboolean(L, 1) ? 1 : 0);
	return 0;
}

static int l_current_screen(lua_State *L) {
	lua_pushstring(L, OG_LUA_ScreenTrack_Name());   /* label, e.g. "Overland" */
	lua_pushinteger(L, OG_LUA_ScreenTrack_Id());    /* id, or -1 if unknown   */
	return 2;
}

static const luaL_Reg dosbox_lib[] = {
	{ "readb",        l_readb },
	{ "readw",        l_readw },
	{ "readd",        l_readd },
	{ "read_phys_b",  l_read_phys_b },
	{ "read_phys_w",  l_read_phys_w },
	{ "read_phys_d",  l_read_phys_d },
	{ "writeb",       l_writeb },
	{ "writew",       l_writew },
	{ "writed",       l_writed },
	{ "write_phys_b", l_write_phys_b },
	{ "write_phys_w", l_write_phys_w },
	{ "write_phys_d", l_write_phys_d },
	{ "get_reg",      l_get_reg },
	{ "set_reg",      l_set_reg },
	{ "send_key",     l_send_key },
	{ "tap_key",      l_tap_key },
	{ "mouse_button",    l_mouse_button },
	{ "mouse_move",      l_mouse_move },
	{ "mouse_set",       l_mouse_set },
	{ "record_input",    l_record_input },
	{ "poll_input",      l_poll_input },
	{ "track_screen",    l_track_screen },
	{ "current_screen",  l_current_screen },
	{ "save_state",      l_save_state },
	{ "load_state",      l_load_state },
	{ "save_state_file", l_save_state_file },
	{ "load_state_file", l_load_state_file },
	{ "bridge_listen",   l_bridge_listen },
	{ "bridge_send",     l_bridge_send },
	{ "bridge_recv",     l_bridge_recv },
	{ "bridge_connected",l_bridge_connected },
	{ "bridge_close",    l_bridge_close },
	{ "log",             l_log },
	{ "on_frame",        l_on_frame },
	{ NULL, NULL }
};

static void register_api(lua_State *L) {
	lua_newtable(L);
	luaL_setfuncs(L, dosbox_lib, 0);   /* Lua 5.2+: fill table on top of stack */
	lua_setglobal(L, "dosbox");
}

/* -------------------------------------------------------------------- *
 *  Per-frame dispatch (recurring PIC event on the emulated timeline)
 * -------------------------------------------------------------------- */
static void LUA_FrameEvent(Bitu /*val*/) {
	frame_scheduled = false;
	if (lua_ctx && frame_cb_ref != LUA_NOREF) {
		lua_rawgeti(lua_ctx, LUA_REGISTRYINDEX, frame_cb_ref);
		if (lua_pcall(lua_ctx, 0, 0, 0) != LUA_OK) {
			LUA_LOG("on_frame error: %s", lua_tostring(lua_ctx, -1));
			lua_pop(lua_ctx, 1);
		}
	}
	if (lua_ctx) {                     /* re-arm for the next tick */
		frame_scheduled = true;
		PIC_AddEvent(LUA_FrameEvent, LUA_FRAME_INTERVAL_MS);
	}
}

/* -------------------------------------------------------------------- *
 *  Lifecycle (section init / destroy)
 * -------------------------------------------------------------------- */
static void LUA_ShutDown(Section * /*sec*/) {
	PIC_RemoveEvents(LUA_FrameEvent);
	PIC_RemoveEvents(LUA_KeyRelease);   /* drop any pending tap_key releases */
	bridge_shutdown();                  /* close any Python-bridge sockets    */
	input_capture = false;              /* stop input capture                 */
	in_count = 0; in_dropped = 0;
	state_free_all();                   /* release savestate slot buffers     */
	frame_scheduled = false;
	if (lua_ctx) {
		if (frame_cb_ref != LUA_NOREF) {
			luaL_unref(lua_ctx, LUA_REGISTRYINDEX, frame_cb_ref);
			frame_cb_ref = LUA_NOREF;
		}
		lua_close(lua_ctx);
		lua_ctx = NULL;
	}
}

void LUA_Init(Section *sec) {
	Section_prop *section = static_cast<Section_prop *>(sec);

	/* Support config re-init: tear down any prior state first. */
	LUA_ShutDown(NULL);

	std::string script = section->Get_string("script");
	if (script.empty())
		return;   /* no script configured -> engine stays dormant */

	/* A script is configured -> this is an ML gameplay/recording run, not a
	 * matchup trace run. Turn off the heavy per-instruction og instrumentation so
	 * the game runs at full speed and stops spamming stderr. Set OG_TRACE_HEAVY in
	 * the environment to keep it on (matchup + Lua hybrid). */
	if (!getenv("OG_TRACE_HEAVY"))
		OG_LUA_HeavyTrace(0);

	lua_ctx = luaL_newstate();
	if (!lua_ctx) {
		LUA_LOG("failed to create Lua state");
		return;
	}
	luaL_openlibs(lua_ctx);
	register_api(lua_ctx);

	if (luaL_dofile(lua_ctx, script.c_str()) != LUA_OK) {
		LUA_LOG("error loading '%s': %s",
		        script.c_str(), lua_tostring(lua_ctx, -1));
		lua_pop(lua_ctx, 1);
	} else {
		LUA_LOG("loaded script '%s'", script.c_str());
	}

	/* Start the observation tick; fires once emulation advances time. */
	if (!frame_scheduled) {
		frame_scheduled = true;
		PIC_AddEvent(LUA_FrameEvent, LUA_FRAME_INTERVAL_MS);
	}

	section->AddDestroyFunction(&LUA_ShutDown);
}
