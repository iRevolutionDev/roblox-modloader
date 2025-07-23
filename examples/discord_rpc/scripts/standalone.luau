local MarketplaceService = game:GetService("MarketplaceService")
local MemStorageService = game:GetService("MemStorageService")
local HttpRbxApiService = game:GetService("HttpRbxApiService")
local HttpService = game:GetService("HttpService")

local bridge = require("@rml/bridge")
local logger = require("@rml/logger")

local placeEditing

local function getPlaceInfo(placeId: number): { name: string, description: string }
    local success, placeInfo = pcall(MarketplaceService.GetProductInfo, MarketplaceService, placeId, Enum.InfoType.Asset)
    if not success then
        logger:error("Failed to get place info for ID %d: %s", placeId, placeInfo)
        return { name = "Unknown Place", description = "No description available." }
    end
    return {
        name = placeInfo.Name,
        description = placeInfo.Description or "No description available."
    }
end

local function getPlaceThumbnail(id: string): string
    local response = HttpRbxApiService:GetAsyncFullUrl(`https://thumbnails.roblox.com/v1/games/icons?universeIds={id}&returnPolicy=PlaceHolder&size=420x420&format=Png&isCircular=false`)
    local data = HttpService:JSONDecode(response)
    if not data or not data.data or #data.data == 0 then
        logger:error("Failed to get thumbnail for place ID %d", id)
        return "https://www.roblox.com/favicon.ico"
    end

    return data.data[1].imageUrl
end

local function onPlaceChanged(placeInfo: {id: string, placeId: string, name: string})
    local placeId = tonumber(placeInfo.placeId)
    if not placeId then
        return
    end

    local place = getPlaceInfo(placeId)
    local thumbnailUrl = getPlaceThumbnail(placeInfo.id)

    placeEditing = {
        details = `Editing {place.name}`,
        state = "In a new place",
        large_image = thumbnailUrl,
        small_image = "studio",
    }

    bridge:call("discord_rpc", "update_activity", placeEditing)
end

MemStorageService:Bind("place_changed", function(data: string)
    local placeInfo = HttpService:JSONDecode(data)
    if not placeInfo or not placeInfo.id or not placeInfo.placeId then
        logger:error("Invalid place changed data: %s", data)
        return
    end

    onPlaceChanged(placeInfo)
end)


MemStorageService:Bind("active_script_changed", function(data: string)
    local scriptInfo = HttpService:JSONDecode(data)
    if not scriptInfo or not scriptInfo.name then
        logger:error("Invalid active script changed data: %s", data)
        return
    end

    local startLine = scriptInfo.startLine or 1
    local startCharacter = scriptInfo.startCharacter or 1

    bridge:call("discord_rpc", "update_activity", {
        details = `Editing {scriptInfo.name} - Line: {startLine}:{startCharacter}`,
        state = "In a script",
        large_image = "luau",
        large_image_text = scriptInfo.class,
        small_image = "studio",
    })
end)

MemStorageService:Bind("back_to_editing", function()
    bridge:call("discord_rpc", "update_activity", placeEditing)
end)

MemStorageService:Bind("playing", function()
    if not placeEditing then
        logger:warn("No place editing activity set, cannot update to playing state.")
        return
    end

    bridge:call("discord_rpc", "update_activity", {
        details = placeEditing.details,
        state = "Testing",
        large_image = placeEditing.large_image,
        small_image = "play",
    })
end)
