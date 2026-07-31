--!nocheck

local function say(text)
	print("[async] " .. text)
	if rml and rml.log then
		pcall(rml.log.info, "[async] " .. text)
	end
end

local function describe(packed)
	if packed.n == 0 then
		return "<no values>"
	end

	local parts = {}
	for i = 1, packed.n do
		local value = packed[i]
		local kind = typeof(value)
		if kind == "Instance" then
			table.insert(parts, ("Instance(%s)"):format(value.ClassName))
		elseif kind == "table" then
			local count = 0
			for _ in pairs(value) do
				count += 1
			end
			table.insert(parts, ("table(%d keys)"):format(count))
		else
			table.insert(parts, ("%s(%s)"):format(kind, tostring(value)))
		end
	end

	return table.concat(parts, ", ")
end

local function probe(name, body)
	local started = os.clock()
	local packed = table.pack(pcall(body))
	local elapsed = (os.clock() - started) * 1000

	local ok = packed[1]
	local results = table.pack(table.unpack(packed, 2, packed.n))

	if not ok then
		say(("%-28s raised after %.1fms: %s"):format(name, elapsed, tostring(results[1])))
		return
	end

	say(("%-28s %.1fms  yielded=%s  returned %d value(s): %s"):format(
		name, elapsed, tostring(elapsed > 1), results.n, describe(results)))
end

say("==== starting ====")

probe("task.wait", function()
	return task.wait(0.05)
end)

probe("GetProductInfo", function()
	return game:GetService("MarketplaceService"):GetProductInfo(1818)
end)

probe("PreloadAsync", function()
	return game:GetService("ContentProvider"):PreloadAsync({})
end)

probe("CreateDockWidgetPluginGui", function()
	local plugin = PluginManager():CreatePlugin()
	return plugin:CreateDockWidgetPluginGuiAsync(
		"RmlAsyncProbe",
		DockWidgetPluginGuiInfo.new(Enum.InitialDockState.Float, true, false, 200, 200, 200, 200))
end)

say("==== done ====")
