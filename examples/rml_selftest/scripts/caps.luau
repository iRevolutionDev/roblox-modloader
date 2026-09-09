local function run()
	local results = {}

	local function attempt(name, body)
		local ok, err = pcall(body)
		results[name] = { ok = ok, err = tostring(err) }
	end

	attempt("settings", function()
		return settings()
	end)

	attempt("instance", function()
		return Instance.new("Folder")
	end)

	attempt("loadstring", function()
		return loadstring("return 1")
	end)

	attempt("getservice", function()
		return game:GetService("CoreGui")
	end)

	return results
end

return {
	at_load = run(),
	run = run,
}
