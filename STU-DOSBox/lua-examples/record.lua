-- record.lua
-- Record a human play session: every emulated frame, log the observation
-- (the same guest state your Gym env reads) plus the real keyboard/mouse the
-- human gave during that frame. The resulting trace feeds behavioral cloning
-- (see session_dataset.py) and can be played back deterministically (replay.lua).
--
-- The human just plays normally -- dosbox.record_input(true) taps DOSBox's real
-- input path (KEYBOARD_AddKey / Mouse_*), so nothing special is required.
--
-- Output file: $DOSBOX_RECORD_FILE, else /tmp/dosbox_session.txt
-- Line format:
--   # dosbox-session v1
--   F=<frame> t=<ticks> ax=<ax> mdx=<sum dx> mdy=<sum dy> ev=<tok,tok,...>
--     tok = k:<name>:<0|1>  (key up/down)  |  mb:<button>:<0|1>  (mouse button)

local OUT = os.getenv("DOSBOX_RECORD_FILE") or "/tmp/dosbox_session.txt"
local f = assert(io.open(OUT, "w"))
f:write("# dosbox-session v1\n")
dosbox.record_input(true)
dosbox.log("recording human session -> " .. OUT)

local frame = 0
dosbox.on_frame(function()
	frame = frame + 1

	-- observation: match your Gym env's obs_keys (here: BIOS ticks + AX)
	local t  = dosbox.readd(0x0040, 0x006C)
	local ax = dosbox.get_reg("ax")

	-- input captured since the previous frame
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

	f:write(string.format("F=%d t=%d ax=%d mdx=%.3f mdy=%.3f ev=%s\n",
	                      frame, t, ax, mdx, mdy, table.concat(toks, ",")))
	f:flush()   -- persist every frame so a mid-session quit still saves the trace

	if dropped > 0 then
		dosbox.log(string.format("frame %d: WARNING dropped %d input events", frame, dropped))
	end
end)
