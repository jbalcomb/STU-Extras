-- mom-gym-env.lua
-- Master of Magic Gym environment (DOSBox side). Pairs with mom_gym.py.
--
-- Flow:
--   1. Observe MoM state each frame (latched DGROUP, WIZARDS.EXE offsets).
--   2. Snapshot the episode-start machine state (slot 0) the first time we reach
--      a stable Overland screen -- or immediately (once a DGROUP is latched) if
--      DOSBOX_GYM_NOWAIT is set, for headless plumbing tests.
--   3. Open the bridge and serve a synchronous lockstep loop: send observation,
--      block for the agent's action, inject it. "reset" -> load_state(0) (back to
--      the episode-start snapshot); "quit" -> close; any other string -> tap_key.
--
-- Env vars: DOSBOX_GYM_PORT (default 8642), DOSBOX_GYM_NOWAIT (test mode).

local PORT          = tonumber(os.getenv("DOSBOX_GYM_PORT") or "8642")
local NOWAIT        = os.getenv("DOSBOX_GYM_NOWAIT") ~= nil
local STABLE_FRAMES = 5          -- Overland must persist this long before snapshot
local OVERLAND_ID   = 5          -- current_screen() id for the overland/main game

-- ---- MoM WZD memory map (see mom-record.lua / project offsets) ----
local HUMAN_BASE = 0x9ECA        -- _players[0] (human) s_WIZARD record base
local SCALARS = {                -- direct DGROUP int16
	{ "turn", 0xBD90 }, { "units", 0xBD92 }, { "cities", 0xBD94 },
	{ "sel_unit", 0xBD8E }, { "difficulty", 0xBD96 }, { "magic", 0xBD98 },
	{ "landsize", 0xBD9A }, { "num_players", 0xBD9C },
	{ "mouse_x", 0x78C2 }, { "mouse_y", 0x78C4 },
}
local RESOURCES = {              -- in _players[0], uint16
	{ "gold", 0x356 }, { "mana", 0x25C }, { "fame", 0x24 },
}

local function read_str(seg, off, n)
	local t = {}
	for i = 0, n - 1 do t[#t + 1] = string.char(dosbox.readb(seg, off + i)) end
	return table.concat(t)
end
local function dgroup_ok(ds) return read_str(ds, 0x0004, 7) == "Borland" end
local function s16(v) if v >= 0x8000 then return v - 0x10000 end return v end

dosbox.track_screen(true)
dosbox.log("mom-gym env online (port " .. PORT .. (NOWAIT and ", NOWAIT test mode)" or ")"))

local dgroup, first_dgroup, wzd_dgroup, exe = nil, nil, nil, nil
local snap_ready, bridge_open   = false, false
local overland_frames           = 0
local click_held                = false   -- a grid-click's button held into the next step

local function build_obs()
	local parts = {}
	for _, o in ipairs(SCALARS) do
		parts[#parts + 1] = string.format("%s=%d", o[1], s16(dosbox.readw(dgroup, o[2])))
	end
	for _, o in ipairs(RESOURCES) do
		parts[#parts + 1] = string.format("%s=%d", o[1], dosbox.readw(dgroup, HUMAN_BASE + o[2]))
	end
	local _, sid = dosbox.current_screen()
	parts[#parts + 1] = string.format("screen=%d", sid)
	return table.concat(parts, " ")
end

dosbox.on_frame(function()
	-- Find the game's DGROUP, then LOCK to WIZARDS.EXE's (globals always live there;
	-- MAGIC's old segment still passes the Borland check, so an unlocked latch grabs
	-- it and reads garbage). Same scheme as mom-record.lua.
	local ds = dosbox.get_reg("ds")
	if wzd_dgroup then
		dgroup = wzd_dgroup
	elseif dgroup_ok(ds) then
		dgroup = ds
		if first_dgroup == nil then first_dgroup = ds; exe = "MGC"
		elseif ds ~= first_dgroup then exe = "WZD"; wzd_dgroup = ds end
	elseif dgroup and not dgroup_ok(dgroup) then
		dgroup = nil
	end

	-- Phase 1: reach the episode-start snapshot (needs a latched DGROUP to read state)
	if not snap_ready then
		if not dgroup then return end
		local _, sid = dosbox.current_screen()
		if NOWAIT or sid == OVERLAND_ID then
			overland_frames = overland_frames + 1
		else
			overland_frames = 0
		end
		if overland_frames >= (NOWAIT and 1 or STABLE_FRAMES) then
			if dosbox.save_state(0) then
				snap_ready = true
				dosbox.log("episode-start snapshot captured (slot 0)")
			end
		end
		return
	end

	-- Phase 2: open the bridge once, at the ready position (blocks until the agent connects)
	if not bridge_open then
		bridge_open = true
		if not dosbox.bridge_listen(PORT, 300) then
			dosbox.log("no agent connected within timeout; idling")
		end
	end
	if not dosbox.bridge_connected() then return end

	-- Release a grid-click held from the previous step (the button was held one
	-- full step so MoM's INT-33 poll registers the click at the set position).
	if click_held then dosbox.mouse_button(0, false); click_held = false end

	-- Phase 3: synchronous lockstep -- observe, act
	if not dosbox.bridge_send(build_obs()) then return end
	local action = dosbox.bridge_recv(10000)
	if action == nil then
		dosbox.log("agent gone (recv timeout/closed); closing bridge")
		dosbox.bridge_close()
	elseif action == "reset" then
		dosbox.load_state(0)                 -- back to the episode-start snapshot
	elseif action == "quit" then
		dosbox.log("agent requested quit")
		dosbox.bridge_close()
	elseif action:sub(1, 6) == "click:" then
		local xf, yf = action:match("click:([%-%d%.]+),([%-%d%.]+)")
		dosbox.mouse_set(tonumber(xf), tonumber(yf))  -- move cursor to the cell center
		dosbox.mouse_button(0, true)                  -- press; released next step
		click_held = true
	elseif action ~= "noop" then
		dosbox.tap_key(action)                        -- key-name action -> tap
	end
end)
