local logger = require("@rml/logger")

local RunService = game:GetService("RunService")
local MemStorageService = game:GetService("MemStorageService")
local HttpService = game:GetService("HttpService")
local StudioService = game:GetService("StudioService")
local ScriptEditorService = game:GetService("ScriptEditorService")

if not game:IsLoaded() then
    game.Loaded:Wait()
end

local data = HttpService:JSONEncode({
    id = tostring(game.GameId),
    placeId = tostring(game.PlaceId),
})

MemStorageService:Fire("place_changed", data)

local selectionConnection: RBXScriptConnection? = nil
local isPlaying: boolean = false

local function getLastOpenDocument(): ScriptDocument?
	local documents = ScriptEditorService:GetScriptDocuments()
	for i = #documents, 1, -1 do
		local document = documents[i]
		if not document:IsCommandBar() then
			return document
		end
	end
	return nil
end

StudioService:GetPropertyChangedSignal("ActiveScript"):Connect(function()
    local activeScript = StudioService.ActiveScript :: Script?

    if not activeScript then
        MemStorageService:Fire("back_to_editing")
        return
    end

    if selectionConnection then
        selectionConnection:Disconnect()
        selectionConnection = nil
    end

    local scriptDocument = getLastOpenDocument()
    if not scriptDocument then
        logger.warn("[discord_rpc] No script document found for active script: %s", activeScript.Name)
        return
    end

    local startLine, startCharacter = scriptDocument:GetSelectionStart()

    currentScript = activeScript

    MemStorageService:Fire("active_script_changed", HttpService:JSONEncode({
        name = tostring(activeScript.Name),
        class = tostring(activeScript.ClassName),
        startLine = startLine,
        startCharacter = startCharacter,
    }))

    selectionConnection = scriptDocument.SelectionChanged:Connect(function()
        local newStartLine, newStartCharacter = scriptDocument:GetSelectionStart()
        if newStartLine ~= startLine or newStartCharacter ~= startCharacter then
            MemStorageService:Fire("active_script_changed", HttpService:JSONEncode({
                name = tostring(activeScript.Name),
                class = tostring(activeScript.ClassName),
                startLine = newStartLine,
                startCharacter = newStartCharacter,
            }))
            startLine, startCharacter = newStartLine, newStartCharacter
        end
    end)
end)

RunService.Heartbeat:Connect(function()
    if not isPlaying and game:GetService("RunService"):IsStudio() then
        isPlaying = true
        MemStorageService:Fire("playing")
    elseif isPlaying and not game:GetService("RunService"):IsStudio() then
        isPlaying = false
        MemStorageService:Fire("back_to_editing")
    end
end)