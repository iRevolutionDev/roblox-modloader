--!nocheck

local passed, failed, parked = 0, 0, 0

local function say(text)
	print("[selftest] " .. text)
	if rml and rml.logger then
		pcall(rml.logger.info, "[selftest] " .. text)
	end
end

local function check(name, body)
	local ok, err = pcall(body)
	if ok then
		passed += 1
		say("pass  " .. name)
	else
		failed += 1
		say("FAIL  " .. name .. "  ->  " .. tostring(err))
	end
end

local function stays_parked(name, wanted, body)
	local ok, err = pcall(body)
	if ok then
		failed += 1
		say("FAIL  " .. name .. "  ->  it answered, so the gap it documents is gone")
	elseif string.find(tostring(err), wanted, 1, true) then
		parked += 1
		say("park  " .. name)
	else
		failed += 1
		say("FAIL  " .. name .. "  ->  " .. tostring(err))
	end
end

say("==== starting ====")

check("the loader installed every debug entry", function()
	local wanted = {
		"getconstants", "getconstant", "setconstant", "getinfo",
		"getproto", "getprotos", "setstack", "getstack",
		"setupvalue", "getupvalue", "getupvalues",
	}

	for _, name in ipairs(wanted) do
		assert(type(debug[name]) == "function", name .. " is a " .. type(debug[name]))
	end
end)

check("roblox's own debug entries survived the merge", function()
	assert(type(debug.traceback) == "function", "traceback went missing")
end)

check("getinfo answers for a function and for a level", function()
	local function sample(a, b)
		return a + b
	end

	local byValue = debug.getinfo(sample)
	assert(type(byValue) == "table", "a function gave back a " .. type(byValue))
	assert(byValue.numparams == 2, "numparams = " .. tostring(byValue.numparams))
	assert(byValue.nups == 0, "nups = " .. tostring(byValue.nups))

	local byLevel = debug.getinfo(1)
	assert(type(byLevel) == "table", "a level gave back a " .. type(byLevel))
	assert(type(byLevel.currentline) == "number", "currentline is missing")
end)

check("getconstants and getconstant tell the same story", function()
	local function speaker()
		return "the original line"
	end

	local all = debug.getconstants(speaker)
	assert(type(all) == "table", "getconstants gave back a " .. type(all))

	local slot
	for i, v in pairs(all) do
		if v == "the original line" then
			slot = i
		end
	end

	assert(slot, "the string never showed up among the constants")
	assert(debug.getconstant(speaker, slot) == "the original line", "getconstant disagrees with getconstants")
end)

check("setconstant rewrites what a function says", function()
	local function speaker()
		return "the original line"
	end

	local slot
	for i, v in pairs(debug.getconstants(speaker)) do
		if v == "the original line" then
			slot = i
		end
	end
	assert(slot, "nothing to patch")

	debug.setconstant(speaker, slot, "the patched line")
	assert(speaker() == "the patched line", "it still says " .. speaker())
end)

check("setconstant refuses to change a constant's type", function()
	local function speaker()
		return "the original line"
	end

	local slot
	for i, v in pairs(debug.getconstants(speaker)) do
		if v == "the original line" then
			slot = i
		end
	end

	assert(not pcall(debug.setconstant, speaker, slot, 12345), "a number went into a string slot")
end)

check("getupvalue reads what a closure captured", function()
	local secret = 4321
	local function reader()
		return secret
	end

	assert(debug.getinfo(reader).nups == 1, "expected exactly one upvalue")
	assert(debug.getupvalue(reader, 1) == 4321, "read back " .. tostring(debug.getupvalue(reader, 1)))
end)

check("setupvalue swaps a captured function, which is the hook", function()
	local function add(a, b)
		return a + b
	end

	local function target(a, b)
		return add(a, b)
	end

	assert(target(2, 3) == 5, "target is not adding to begin with")
	assert(debug.getinfo(target).nups == 1, "target should capture exactly one upvalue")

	local calls = 0
	local installed = debug.setupvalue(target, 1, function(a, b)
		calls += 1
		return a * b
	end)

	assert(installed == true, "setupvalue did not report success")
	assert(target(2, 3) == 6, "the hook never took")
	assert(calls == 1, "the hook ran " .. calls .. " times")
end)

check("a hook can keep the original and wrap it", function()
	local function greet(name)
		return "hello " .. name
	end

	local function target(name)
		return greet(name)
	end

	assert(target("world") == "hello world", "target is not greeting")

	local original = greet
	debug.setupvalue(target, 1, function(name)
		return original(name) .. "!"
	end)

	assert(target("world") == "hello world!", "the wrapper gave " .. target("world"))
end)

check("getstack sees the live frame and setstack writes into it", function()
	local marker = 1337
	local frame = debug.getstack(1)
	assert(type(frame) == "table", "getstack gave back a " .. type(frame))

	local slot
	for i, v in pairs(frame) do
		if v == 1337 then
			slot = i
		end
	end
	assert(slot, "the marker is not on the frame")
	assert(debug.getstack(1, slot) == 1337, "the indexed form disagrees")

	debug.setstack(1, slot, 4242)
	assert(marker == 4242, "the register still holds " .. tostring(marker))
end)

check("setstack refuses to change a register's type", function()
	local marker = 1337
	local slot

	for i, v in pairs(debug.getstack(1)) do
		if v == 1337 then
			slot = i
		end
	end
	assert(slot, "the marker is not on the frame")

	assert(not pcall(debug.setstack, 1, slot, "a string"), "a string went into a number register")
	assert(marker == 1337, "the refusal still moved the register to " .. tostring(marker))
end)

check("getstack walks further than one frame up", function()
	local function inner()
		return debug.getstack(2)
	end

	local function outer()
		local mine = 24680
		local frame = inner()
		assert(mine == 24680, "the local moved under us")
		return frame
	end

	local seen = false
	for _, v in pairs(outer()) do
		if v == 24680 then
			seen = true
		end
	end

	assert(seen, "the outer frame's local never showed up")
end)

check("levels out of range are refused instead of read", function()
	assert(not pcall(debug.getstack, 4096), "a level past the stack was accepted")
	assert(not pcall(debug.getstack, -1), "a negative level was accepted")
end)

stays_parked("getproto waits on Proto.nups", "Proto.nups", function()
	local function host()
		return function() end
	end

	debug.getproto(host, 1)
end)

stays_parked("getprotos waits on Proto.nups", "Proto.nups", function()
	local function host()
		return function() end
	end

	debug.getprotos(host)
end)

say(("==== done: %d passed, %d failed, %d parked ===="):format(passed, failed, parked))

if failed > 0 and rml and rml.logger then
	pcall(rml.logger.warn, ("%d selftest checks failed"):format(failed))
end
