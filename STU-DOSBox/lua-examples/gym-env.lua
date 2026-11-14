-- gym-env.lua
-- DOSBox side of the Gymnasium wrapper (dosbox_gym.py). Same lockstep bridge
-- as milestone 3, but the observation line now carries everything the Gym env
-- needs: state fields + reward + episode-done, and we honour a "reset" action.
--
-- Observation format (plain decimal, unambiguous for the parser):
--     t=<bios_ticks> ax=<ax_reg> reward=<r> done=<0|1>
-- Actions understood: "noop", "tap:space", "reset", "quit".

local PORT      = 8642
local MAX_STEPS = 6         -- synthetic episode length (done=1 at this step)

dosbox.log("gym env online; listening on port " .. PORT)
dosbox.track_screen(true)   -- ground-truth current screen (id) for the observation
if not dosbox.bridge_listen(PORT, 30) then
	dosbox.log("no agent connected; running without bridge")
end

local step = 0
local snapshot_taken = false

dosbox.on_frame(function()
	if not dosbox.bridge_connected() then return end

	-- Take the episode-start snapshot once. Every later reset restores exactly
	-- this machine state, so all episodes begin identically (true RL reset).
	if not snapshot_taken then
		snapshot_taken = true
		dosbox.log("episode-start snapshot -> slot 0: " .. tostring(dosbox.save_state(0)))
	end

	-- ---------- observation (state + reward + done) ----------
	step = step + 1
	local ticks  = dosbox.readd(0x0040, 0x006C)   -- BIOS timer tick count
	local ax     = dosbox.get_reg("ax")
	local _, sid = dosbox.current_screen()        -- screen id (-1 until in-game)
	local reward = 1.0                             -- synthetic: +1 per surviving step
	local done   = (step >= MAX_STEPS) and 1 or 0
	local obs = string.format("t=%d ax=%d screen=%d reward=%.1f done=%d",
	                          ticks, ax, sid, reward, done)
	if not dosbox.bridge_send(obs) then
		dosbox.log("bridge send failed; agent gone")
		return
	end

	-- ---------- action (block for the agent's reply) ----------
	local action = dosbox.bridge_recv(5000)
	if action == nil then
		dosbox.log("bridge recv timeout/closed; ending")
		dosbox.bridge_close()
		return
	end

	if action == "tap:space" then
		dosbox.tap_key("space")
	elseif action == "reset" then
		-- TRUE reset: restore the exact machine state (RAM + CPU) captured at
		-- episode start, so every episode begins from identical conditions.
		local ok = dosbox.load_state(0)
		step = 0
		dosbox.log("episode reset: load_state(0) -> " .. tostring(ok))
	elseif action == "quit" then
		dosbox.log("agent requested quit; closing bridge")
		dosbox.bridge_close()
	-- "noop" and anything else: do nothing
	end
end)
