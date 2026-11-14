-- milestone1-bios-ticks.lua
-- Proves the DOSBox Lua engine (milestone 1) end-to-end:
--   * the [lua] section loaded this script,
--   * dosbox.on_frame() fires on the emulated timeline,
--   * dosbox.read* actually reads live guest memory.
--
-- We read the BIOS Data Area, which is valid under any guest:
--   0040:006C  dword  timer tick count (increments ~18.2x/sec)
--   0040:0010  word   installed-equipment flags
--
-- If BIOS_ticks climbs across frames, the whole pipeline works.

dosbox.log("milestone1 script online")

local frame = 0
dosbox.on_frame(function()
	frame = frame + 1
	if frame <= 5 or frame % 20 == 0 then
		local ticks = dosbox.readd(0x0040, 0x006C)   -- BIOS timer tick count
		local equip = dosbox.readw(0x0040, 0x0010)   -- BIOS equipment word
		dosbox.log(string.format("frame %d: BIOS_ticks=%d equip=0x%04X",
		                         frame, ticks, equip))
	end
end)
