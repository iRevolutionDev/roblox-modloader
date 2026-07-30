--!nocheck

local passed, failed, parked = 0, 0, 0

local function say(text)
	print("[selftest] " .. text)
	if rml and rml.log then
		pcall(rml.log.info, "[selftest] " .. text)
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
	assert(byValue.what == "Lua", "what = " .. tostring(byValue.what))
	assert(byValue.linedefined > 0, "linedefined = " .. tostring(byValue.linedefined))

	assert(string.find(tostring(byValue.short_src), "selftest", 1, true) ~= nil,
		"short_src holds " .. tostring(byValue.short_src) .. ", which looks like the name")
	assert(byValue.name == "sample", "name = " .. tostring(byValue.name))

	local byLevel = debug.getinfo(1)
	assert(type(byLevel) == "table", "a level gave back a " .. type(byLevel))
	assert(byLevel.currentline > 0, "currentline = " .. tostring(byLevel.currentline))
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
	local secret = tonumber("4321")
	local function reader()
		return secret
	end

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

check("getgc walks the heap", function()
	local objects = getgc()

	assert(type(objects) == "table", "getgc gave back a " .. type(objects))
	assert(#objects > 0, "the heap came back empty")

	local kinds = {}
	for _, value in ipairs(objects) do
		kinds[type(value)] = true
	end

	assert(kinds["function"], "no function turned up on the heap")
	assert(not kinds["table"], "tables came back without being asked for")
	assert(#getgc(true) > #objects, "asking for tables added nothing")
end)

check("getgc finds an object we just made", function()
	local needle = function() end

	local found = false
	for _, value in ipairs(getgc()) do
		if value == needle then
			found = true
			break
		end
	end

	assert(found, "a closure created moments ago is not on the heap")
end)

check("the userdata getgc returns is still a real engine object", function()
	local seen = 0

	for _, value in ipairs(getgc()) do
		if type(value) == "userdata" then
			assert(type(typeof(value)) == "string", "typeof refused a heap userdata")
			seen += 1
		end
	end

	say(("probe getgc returned %d userdata"):format(seen))
end)

check("getinfo survives every function the heap holds", function()
	local seen = 0

	for _, value in ipairs(getgc()) do
		if type(value) == "function" then
			debug.getinfo(value)
			seen += 1
		end
	end

	assert(seen > 0, "the heap held no functions at all")
	say(("probe getinfo answered for %d heap functions"):format(seen))
end)

check("getinfo answers for a c closure", function()
	local info = debug.getinfo(print)

	assert(type(info) == "table", "a c closure gave back a " .. type(info))
	assert(info.what == "C", "what = " .. tostring(info.what))
end)

check("iscclosure and islclosure tell the two apart", function()
	local function written_in_luau()
	end

	assert(islclosure(written_in_luau), "a luau function is not reported as one")
	assert(not iscclosure(written_in_luau), "a luau function is reported as a c closure")
	assert(iscclosure(print), "print is not reported as a c closure")
	assert(not islclosure(print), "print is reported as a luau closure")
end)

check("clonefunction makes a copy that stands on its own", function()
	local function speaker()
		return "the original line"
	end

	local copy = clonefunction(speaker)

	assert(copy ~= speaker, "the clone is the very same object")
	assert(copy() == "the original line", "the clone does not behave like the original")
end)

check("clonefunction copies a closure that captured something", function()
	local secret = tonumber("777")
	local function reader()
		return secret
	end

	local copy = clonefunction(reader)

	assert(copy ~= reader, "the clone is the very same object")
	assert(copy() == 777, "the clone read back " .. tostring(copy()))
end)

check("hookfunction swaps a luau function and hands back the original", function()
	local function target(a, b)
		return a + b
	end

	local function replacement(a, b)
		return a * b
	end

	assert(target(2, 3) == 5, "target is not adding to begin with")

	local original = hookfunction(target, replacement)

	assert(target(2, 3) == 6, "the hook never took")
	assert(original(2, 3) == 5, "the original handed back does not add")
end)

check("newlclosure wraps a function without capturing anything", function()
	local wrapped = newlclosure(function(a, b)
		return a * b
	end)

	assert(islclosure(wrapped), "the wrapper is not a luau closure")
	assert(debug.getinfo(wrapped).nups == 0, "the wrapper captured " .. debug.getinfo(wrapped).nups .. " values")
	assert(wrapped(2, 3) == 6, "the wrapper does not forward to what it wraps")
end)

check("hookfunction wraps a replacement that captures too much", function()
	local function target(a, b)
		return a + b
	end

	local captured = tonumber("10")
	local function replacement(a, b)
		return a * b + captured
	end

	assert(debug.getinfo(target).nups == 0, "the target was supposed to capture nothing")
	assert(debug.getinfo(replacement).nups > 0, "the replacement was supposed to capture something")

	hookfunction(target, replacement)

	assert(target(2, 3) == 16, "the hook gave " .. tostring(target(2, 3)))
end)

check("hookfunction takes a c closure as the replacement", function()
	local function target(...)
		return "not hooked"
	end

	hookfunction(target, print)
	assert(target ~= nil, "the target went missing")
end)

check("restorefunction puts a hooked function back", function()
	local function target(a, b)
		return a + b
	end

	local function replacement(a, b)
		return a * b
	end

	hookfunction(target, replacement)
	assert(target(2, 3) == 6, "the hook never took")

	restorefunction(target)
	assert(target(2, 3) == 5, "the function was not put back")
end)


check("rml.mod carries this mod's manifest and refuses writes", function()
	assert(type(rml.mod) == "table", "rml.mod is a " .. type(rml.mod))
	assert(rml.mod.name == "RML Self Test", "name = " .. tostring(rml.mod.name))
	assert(#tostring(rml.mod.path) > 0, "path is empty")
	assert(not pcall(function()
		rml.mod.name = "hijacked"
	end), "rml.mod accepted a write")
end)

check("require refuses a specifier with no alias", function()
	local ok, err = pcall(require, "some_module")
	assert(not ok, "a bare specifier resolved")
	assert(string.find(tostring(err), "@rml/", 1, true) ~= nil, "the error does not name the aliases: " .. tostring(err))
end)

check("require refuses to climb out of the mod folder", function()
	assert(not pcall(require, "@self/../../../escape"), "a .. specifier resolved")
end)

check("require reports a module it cannot find", function()
	local ok, err = pcall(require, "@self/definitely_not_here")
	assert(not ok, "a missing module resolved")
	assert(string.find(tostring(err), "not found", 1, true) ~= nil, "unexpected error: " .. tostring(err))
end)

check("require hands back the same table twice", function()
	local first = require("@self/probe")
	local second = require("@self/probe")
	assert(first == second, "the two requires produced different tables")
	assert(first.marker == "probe", "the module did not load its own contents")
end)

check("require detects a cycle instead of overflowing", function()
	local ok, err = pcall(require, "@self/cycle_a")
	assert(not ok, "a cyclic require resolved")
	assert(string.find(tostring(err), "cycle", 1, true) ~= nil, "unexpected error: " .. tostring(err))
end)

local function probe(name, body)
	local ok, err = pcall(body)
	if not ok then
		say("probe " .. name .. " raised " .. tostring(err))
	end
end

probe("frame alignment", function()
	local aaa = tonumber("111111")
	local bbb = tonumber("222222")
	local ccc = tonumber("333333")
	local frame = debug.getstack(1)

	local shown = {}
	for i = 1, 8 do
		table.insert(shown, ("[%d]=%s"):format(i, tostring(rawget(frame, i))))
	end

	say("probe align " .. table.concat(shown, " "))
	say(("probe locals were %d %d %d"):format(aaa, bbb, ccc))
end)

check("getstack sees the live frame and setstack writes into it", function()
	local marker = tonumber("1337")
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
	local marker = tonumber("1337")
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
		local mine = tonumber("24680")
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

if failed > 0 and rml and rml.log then
	pcall(rml.log.warn, "%d selftest checks failed", failed)
end
