# Vendored Lua 5.4.7, built as a static library linked into dosbox.
# lua.c (standalone interpreter main) and luac.c (compiler main) are
# intentionally excluded so we don't pull in a second main().
#
# LUA_USE_POSIX enables POSIX os/io facilities without requiring the
# dynamic-C-module loader (LUA_USE_DLOPEN), so no -ldl is needed.
# -w silences Lua's own warnings under DOSBox's warning flags.

AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -DLUA_USE_POSIX
AM_CFLAGS = -w

noinst_LIBRARIES = liblua.a

liblua_a_SOURCES = \
	lua-5.4.7/src/lapi.c \
	lua-5.4.7/src/lcode.c \
	lua-5.4.7/src/lctype.c \
	lua-5.4.7/src/ldebug.c \
	lua-5.4.7/src/ldo.c \
	lua-5.4.7/src/ldump.c \
	lua-5.4.7/src/lfunc.c \
	lua-5.4.7/src/lgc.c \
	lua-5.4.7/src/llex.c \
	lua-5.4.7/src/lmem.c \
	lua-5.4.7/src/lobject.c \
	lua-5.4.7/src/lopcodes.c \
	lua-5.4.7/src/lparser.c \
	lua-5.4.7/src/lstate.c \
	lua-5.4.7/src/lstring.c \
	lua-5.4.7/src/ltable.c \
	lua-5.4.7/src/ltm.c \
	lua-5.4.7/src/lundump.c \
	lua-5.4.7/src/lvm.c \
	lua-5.4.7/src/lzio.c \
	lua-5.4.7/src/lauxlib.c \
	lua-5.4.7/src/lbaselib.c \
	lua-5.4.7/src/lcorolib.c \
	lua-5.4.7/src/ldblib.c \
	lua-5.4.7/src/liolib.c \
	lua-5.4.7/src/lmathlib.c \
	lua-5.4.7/src/loadlib.c \
	lua-5.4.7/src/loslib.c \
	lua-5.4.7/src/lstrlib.c \
	lua-5.4.7/src/ltablib.c \
	lua-5.4.7/src/lutf8lib.c \
	lua-5.4.7/src/linit.c
