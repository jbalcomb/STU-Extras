-- milestone3-bridge.lua
-- DOSBox side of the Python bridge: a synchronous (lockstep) RL-style env.
--
-- On each emulated frame we build an observation from guest state, send it to
-- the Python agent, BLOCK for the agent's action, then inject that action.
-- Because we block per frame, the emulated machine advances exactly one step
-- per agent decision -- the run is reproducible regardless of agent latency.

local PORT = 8642

dosbox.log("bridge env online; listening on port " .. PORT)
if not dosbox.bridge_listen(PORT, 30) then      -- waits for the Python agent
	dosbox.log("no agent connected; running without bridge")
end

local step = 0
dosbox.on_frame(function()
	if not dosbox.bridge_connected() then return end

	-- ---------- observation: whatever guest state your model needs ----------
	step = step + 1
	local ticks = dosbox.readd(0x0040, 0x006C)   -- BIOS timer tick count
	local ax    = dosbox.get_reg("ax")
	local obs   = string.format("step=%d ticks=%d ax=%04X", step, ticks, ax)
	if not dosbox.bridge_send(obs) then
		dosbox.log("bridge send failed; agent gone")
		return
	end

	-- ---------- action: block for the agent's reply, then act ----------
	local action = dosbox.bridge_recv(5000)
	if action == nil then
		dosbox.log("bridge recv timeout/closed; ending episode")
		dosbox.bridge_close()
		return
	end

	if action == "tap:space" then
		dosbox.tap_key("space")
		dosbox.log(string.format("step %d: action=%s -> tapped space", step, action))
	elseif action == "quit" then
		dosbox.log(string.format("step %d: action=quit -> closing bridge", step))
		dosbox.bridge_close()
	elseif action == "noop" then
		dosbox.log(string.format("step %d: action=noop", step))
	else
		dosbox.log(string.format("step %d: action=%s (unknown)", step, action))
	end
end)
