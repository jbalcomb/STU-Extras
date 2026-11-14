-- mom-record.lua
-- Record a human Master of Magic session: the player's real keyboard/mouse plus
-- whatever game state you choose to observe. Feeds session_dataset.py (behavioral
-- cloning) and replay.lua (deterministic playback).
--
-- Two MoM-specific things this handles:
--
--  1. DGROUP detection. on_frame fires at an arbitrary instruction, so the live
--     DS register is NOT always the game's data segment (it may be an overlay's
--     or an ISR's). We verify it with the same Borland sentinel your [capture]
--     confs use -- the string "Borland" at DS:0x0004 -- and only trust reads
--     when it matches. Frames where it doesn't match are still recorded (input
--     is always valid); the game fields are simply marked absent.
--
--  2. MAGIC.EXE and WIZARDS.EXE use DIFFERENT data-segment offsets for the same
--     global, so there is one OBS table per executable. We detect which is
--     running (MAGIC.EXE always launches first) and read with its offsets.
--
-- Output file: $DOSBOX_RECORD_FILE (default /tmp/mom_session.txt)

-- Output file. If DOSBOX_RECORD_FILE is not set, auto-name a UNIQUE timestamped
-- file so back-to-back sessions never overwrite each other.
local function default_out()
	local home = os.getenv("HOME") or "/tmp"
	local ok, stamp = pcall(os.date, "%Y%m%d-%H%M%S")
	if not ok or not stamp then stamp = tostring((os.time and os.time()) or "session") end
	return home .. "/mom_session_" .. stamp .. ".txt"
end
local OUT = os.getenv("DOSBOX_RECORD_FILE") or default_out()

-- ---------------------------------------------------------------------------
-- OBSERVATION FIELDS -- one table per executable. Offsets are relative to the
-- verified DGROUP (the runtime segment we latch, not IDA's static segment).
-- Fields: name, off, width (1/2/4 bytes), signed (optional -- for int16/int32).
--
-- Seeded from the ReMoM wiki / MoX RE source:
--   g_mouse_x        MGC 31D1:4394   WZD 36AA:78C2   int16  -> logging every frame
--   g_mouse_y        MGC 31D1:4396   WZD 36AA:78C4   int16     makes clicks locatable
--   _current_screen  WZD dseg:9990                   int16  -> WHICH SCREEN (context!)
--
-- _current_screen holds the RAW WIZARDS screen id (per MOM_SCR.h e_SCREENS header):
--   0=City 1=Load 2=Armies 3=Cities 4=Quit 5=Overland(main game) 6=Magic
--   7=RoadBuild 8=Production 9=Items 10=NextTurn 12=Spellbook 14=Advisor 15=Diplomacy
-- (Some values carry a "?default?" note in the source, so the map is CONFIRMED
--  empirically from your recording -- watch `screen` change as you move around.)
-- WZD gameplay globals, confirmed against MOM_DAT.h + the ReMoM wiki:
--   game-state scalars (direct DGROUP int16):  _unit BD8E, _turn BD90, _units BD92,
--     _cities BD94, _difficulty BD96, _magic BD98, _landsize BD9A, _num_players BD9C
--   human resources: the human is ALWAYS _players[0] (per project owner: _human_player_idx
--     is a vestigial artifact of cancelled multiplayer, always 0). _players = struct
--     s_WIZARD array at WZD dseg 0x9ECA, so _players[0] is at 0x9ECA. gold_reserve @+0x356,
--     mana_reserve @+0x25C, fame @+0x24 (all uint16). Entries with player=true read at
--     HUMAN_BASE + off.
-- ---------------------------------------------------------------------------
local HUMAN_BASE  = 0x9ECA   -- _players[0], the human's s_WIZARD record
local WIZARD_SIZE = 0x4C8    -- sizeof(s_WIZARD)=1224; only needed to reach AI players _players[1..]

local OBS_MGC = {
	{ name = "mouse_x", off = 0x4394, width = 2, signed = true },
	{ name = "mouse_y", off = 0x4396, width = 2, signed = true },
}
local OBS_WZD = {
	-- game-state scalars (direct)
	{ name = "turn",        off = 0xBD90, width = 2, signed = true },
	{ name = "units",       off = 0xBD92, width = 2, signed = true },
	{ name = "cities",      off = 0xBD94, width = 2, signed = true },
	{ name = "sel_unit",    off = 0xBD8E, width = 2, signed = true },  -- _unit (selected)
	{ name = "difficulty",  off = 0xBD96, width = 2, signed = true },
	{ name = "magic",       off = 0xBD98, width = 2, signed = true },
	{ name = "landsize",    off = 0xBD9A, width = 2, signed = true },
	{ name = "num_players", off = 0xBD9C, width = 2, signed = true },
	-- human player's resources (direct in _players[0])
	{ name = "gold",        off = 0x356, width = 2, player = true },
	{ name = "mana",        off = 0x25C, width = 2, player = true },
	{ name = "fame",        off = 0x024, width = 2, player = true },
	-- input context
	-- _current_screen is DISPATCH INTENT (the next screen Screen_Control wants), not
	-- the one we're on -- logged as screen_intent; ground-truth `screen` comes from
	-- dosbox.current_screen() (screen-fn entry). Watch them diverge.
	{ name = "screen_intent", off = 0x9990, width = 2 },
	{ name = "mouse_x",       off = 0x78C2, width = 2, signed = true },
	{ name = "mouse_y",       off = 0x78C4, width = 2, signed = true },
}

-- ---------------------------------------------------------------------------

local function read_str(seg, off, n)
	local t = {}
	for i = 0, n - 1 do
		t[#t + 1] = string.char(dosbox.readb(seg, off + i))
	end
	return table.concat(t)
end

local function dgroup_ok(ds)
	return read_str(ds, 0x0004, 7) == "Borland"
end

local function read_field(seg, off, width, signed)
	local v
	if width == 1 then v = dosbox.readb(seg, off)
	elseif width == 2 then v = dosbox.readw(seg, off)
	else v = dosbox.readd(seg, off) end
	if signed then
		if     width == 1 and v >= 0x80        then v = v - 0x100
		elseif width == 2 and v >= 0x8000      then v = v - 0x10000
		elseif width == 4 and v >= 0x80000000  then v = v - 0x100000000 end
	end
	return v
end

local f = assert(io.open(OUT, "w"))
f:write("# dosbox-session v1 game=MasterOfMagic\n")
dosbox.record_input(true)
dosbox.track_screen(true)   -- ground-truth current screen via screen-fn entry
dosbox.log("MoM session recording -> " .. OUT)

local frame = 0
local dgroup = nil       -- DGROUP segment we read game state from
local first_dgroup = nil -- the MAGIC.EXE DGROUP (first one we latch)
local wzd_dgroup = nil   -- WIZARDS.EXE DGROUP, once known -> LOCK to it
local exe = nil          -- "MGC" (setup) or "WZD" (gameplay)

dosbox.on_frame(function()
	frame = frame + 1

	-- ---- observation ----
	local t  = dosbox.readd(0x0040, 0x006C)     -- BIOS ticks (always valid)
	local ds = dosbox.get_reg("ds")

	-- Find the game's DGROUP. The live DS is only the data segment on a minority
	-- of frames (~31%; otherwise it's an overlay's or an ISR's), and the globals
	-- always live at the DGROUP regardless of DS -- so once we know WIZARDS.EXE's
	-- DGROUP we LOCK to it and always read there. MAGIC.EXE launches first (its
	-- DGROUP is the first Borland segment we see); when a DIFFERENT Borland segment
	-- appears, WIZARDS has taken over -> lock. Without the lock, MAGIC's old segment
	-- still passes the Borland check (stale memory) and the latch grabs it -> garbage.
	if wzd_dgroup then
		dgroup = wzd_dgroup                  -- locked to WIZARDS.EXE's DGROUP
	elseif dgroup_ok(ds) then
		dgroup = ds
		if first_dgroup == nil then
			first_dgroup = ds; exe = "MGC"
		elseif ds ~= first_dgroup then
			exe = "WZD"; wzd_dgroup = ds     -- lock from now on
		end
	elseif dgroup and not dgroup_ok(dgroup) then
		dgroup = nil
	end

	local OBS = (exe == "WZD") and OBS_WZD or OBS_MGC
	local fields = {}
	if dgroup and exe then
		for _, o in ipairs(OBS) do
			-- player=true fields live in the human's record, always _players[0].
			local off = o.player and (HUMAN_BASE + o.off) or o.off
			fields[#fields + 1] = string.format("%s=%d", o.name,
				read_field(dgroup, off, o.width, o.signed))
		end
	end

	-- ---- input captured since the previous frame ----
	local events, dropped = dosbox.poll_input()
	local toks, mdx, mdy = {}, 0, 0
	for _, e in ipairs(events) do
		if e.type == "key" then
			toks[#toks + 1] = "k:" .. e.name .. ":" .. (e.pressed and "1" or "0")
		elseif e.type == "mouse_button" then
			toks[#toks + 1] = "mb:" .. e.button .. ":" .. (e.pressed and "1" or "0")
		elseif e.type == "mouse_move" then
			mdx = mdx + e.dx
			mdy = mdy + e.dy
		end
	end

	local screen = dosbox.current_screen()   -- ground-truth screen label ("?" if unknown)

	f:write(string.format("F=%d t=%d exe=%s screen=%s ds=%04X dg=%04X %smdx=%.3f mdy=%.3f ev=%s\n",
		frame, t, exe or "-", screen, ds, dgroup or 0,
		(#fields > 0) and (table.concat(fields, " ") .. " ") or "",
		mdx, mdy, table.concat(toks, ",")))
	f:flush()   -- persist every frame: quitting mid-session still saves the trace

	if dropped > 0 then
		dosbox.log(string.format("frame %d: WARNING dropped %d input events", frame, dropped))
	end
end)
