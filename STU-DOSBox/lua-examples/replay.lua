-- replay.lua
-- Deterministically play back a recorded session (record.lua) by re-injecting
-- the logged keyboard/mouse on the matching frames. Because capture and
-- injection are symmetric, the guest receives the same input it saw live --
-- useful for regression tests and for checking whether the trained model (or
-- ReMoM) reproduces a human session.
--
-- Input file: $DOSBOX_REPLAY_FILE, else /tmp/dosbox_session.txt

local IN = os.getenv("DOSBOX_REPLAY_FILE") or "/tmp/dosbox_session.txt"

local frames = {}
local last = 0
for line in io.lines(IN) do
	if line:sub(1, 1) ~= "#" and line ~= "" then
		local fr  = tonumber(line:match("F=(%-?%d+)"))
		local mdx = tonumber(line:match("mdx=(%-?[%d%.]+)")) or 0
		local mdy = tonumber(line:match("mdy=(%-?[%d%.]+)")) or 0
		local ev  = line:match("ev=(%S*)") or ""
		if fr then
			frames[fr] = { mdx = mdx, mdy = mdy, ev = ev }
			if fr > last then last = fr end
		end
	end
end
dosbox.log(string.format("replay: loaded %d frames from %s", last, IN))

local frame = 0
dosbox.on_frame(function()
	frame = frame + 1
	local rec = frames[frame]
	if not rec then return end

	if rec.mdx ~= 0 or rec.mdy ~= 0 then
		dosbox.mouse_move(rec.mdx, rec.mdy)
	end
	if rec.ev ~= "" then
		for tok in rec.ev:gmatch("[^,]+") do
			local kind, a, p = tok:match("([^:]+):([^:]+):([01])")
			if kind == "k" then
				dosbox.send_key(a, p == "1")
			elseif kind == "mb" then
				dosbox.mouse_button(tonumber(a), p == "1")
			end
		end
		dosbox.log(string.format("replay frame %d: ev=%s", frame, rec.ev))
	end

	if frame >= last then
		dosbox.log("replay: reached end of trace")
	end
end)
