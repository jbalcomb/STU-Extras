-- milestone2-write-regs-input.lua
-- Proves the DOSBox Lua engine milestone 2 (the "act" half of the ML loop):
--   * memory WRITES round-trip,
--   * CPU register get/set works,
--   * KEYBOARD INPUT injection actually reaches the guest.
--
-- Each check prints "-> OK" or "-> FAIL" so it is greppable headless.

dosbox.log("milestone2 script online")

local frame = 0

dosbox.on_frame(function()
	frame = frame + 1

	-- (1) memory write round-trip: scratch area at phys 0x510 (BIOS/DOS
	--     inter-application comm area, unused at an idle prompt).
	if frame == 1 then
		dosbox.write_phys_w(0x00000510, 0xBEEF)
		local rb = dosbox.read_phys_w(0x00000510)
		dosbox.log(string.format("write : wrote 0xBEEF, read 0x%04X -> %s",
		                         rb, rb == 0xBEEF and "OK" or "FAIL"))

		-- (2) registers: read live regs, then a set/get round-trip on ax
		--     that we immediately restore (net zero effect on the guest).
		local cs, ip, ax = dosbox.get_reg("cs"), dosbox.get_reg("ip"), dosbox.get_reg("ax")
		dosbox.set_reg("ax", 0x1234)
		local ax2 = dosbox.get_reg("ax")
		dosbox.set_reg("ax", ax)                       -- restore
		dosbox.log(string.format("regs  : cs=%04X ip=%04X ax=%04X ; set ax->1234 read 0x%04X -> %s",
		                         cs, ip, ax, ax2, ax2 == 0x1234 and "OK" or "FAIL"))
	end

	-- (3) input injection: hold Left-Shift, then watch the BIOS keyboard
	--     flags byte at 0040:0017 (bit 0x02 = left shift) that the guest's
	--     own INT 9 handler maintains -- independent of anyone reading keys.
	if frame == 3 then
		dosbox.send_key("leftshift", true)             -- press & hold
	elseif frame == 5 then
		local f = dosbox.readb(0x0040, 0x0017)
		dosbox.log(string.format("input : shift HELD, kbd_flags=0x%02X -> %s",
		                         f, (f & 0x02) ~= 0 and "OK" or "FAIL"))
		dosbox.send_key("leftshift", false)            -- release
	elseif frame == 7 then
		local f = dosbox.readb(0x0040, 0x0017)
		dosbox.log(string.format("input : shift RELEASED, kbd_flags=0x%02X -> %s",
		                         f, (f & 0x02) == 0 and "OK" or "FAIL"))
		dosbox.log("milestone2 checks complete")
	end
end)
