--!nocheck

local passed, failed, parked = 0, 0, 0

local function say(text)
	print("[yield] " .. text)
	if rml and rml.log then
		pcall(rml.log.info, "[yield] " .. text)
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

check("a script yields at the top level and comes back", function()
	local before = os.clock()
	task.wait(0.05)
	assert(os.clock() > before, "the clock did not move across the wait")
end)

check("a yield inside pcall keeps the script alive", function()
	local ok, err = pcall(function()
		task.wait(0.05)
		return "through"
	end)

	assert(ok, "the wait raised " .. tostring(err))
	assert(err == "through", "pcall returned " .. tostring(err))
end)

check("a yield inside a nested function call still works", function()
	local function inner()
		task.wait(0.05)
		return 42
	end

	local function outer()
		return inner()
	end

	assert(outer() == 42, "the nested call did not return")
end)

check("task.spawn runs a thread of our own to completion", function()
	local reached = false

	task.spawn(function()
		task.wait(0.05)
		reached = true
	end)

	local waited = 0
	while not reached and waited < 2 do
		waited += task.wait(0.05)
	end

	assert(reached, "the spawned thread never finished")
end)

check("a coroutine we make can yield through the engine", function()
	local co = coroutine.create(function()
		task.wait(0.05)
		return "done"
	end)

	local ok, first = coroutine.resume(co)
	assert(ok, "the first resume raised " .. tostring(first))

	local waited = 0
	while coroutine.status(co) ~= "dead" and waited < 2 do
		waited += task.wait(0.05)
	end

	assert(coroutine.status(co) == "dead", "the coroutine is " .. coroutine.status(co))
end)

check("an engine call that yields answers on our thread", function()
	if game == nil then
		say("probe engine-yield skipped: this thread has no engine globals")
		return
	end

	local http = game:GetService("HttpService")
	assert(http ~= nil, "HttpService is missing")

	local ok, err = pcall(function()
		return game:GetService("ContentProvider"):PreloadAsync({})
	end)

	assert(ok, "a yielding engine call raised " .. tostring(err))
end)

stays_parked("require still cannot load a module that yields", "yieldable", function()
	require("@self/yielding_module")
end)

say(("==== done: %d passed, %d failed, %d parked ===="):format(passed, failed, parked))

if failed > 0 and rml and rml.log then
	pcall(rml.log.warn, "%d yield checks failed", failed)
end
