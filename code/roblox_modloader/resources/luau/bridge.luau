--!strict

export type Bridge = {
    call: (self: Bridge, modName: string, functionName: string, ...any) -> any,
    register: (self: Bridge, modName: string, functionName: string, callback: (...any) -> any) -> boolean,
    listen: (self: Bridge, modName: string, eventName: string, callback: (...any) -> any) -> boolean,
}

print("[bridge] Initializing bridge module...")

local bridge = {} :: Bridge
bridge.__index = bridge
setmetatable(bridge, bridge)

local function assertBridge()
    if not rml then
        error("[bridge] Global `rml` is not defined.")
    end
    if not rml.bridge then
        error("[bridge] Global `rml.bridge` is not defined.")
    end
end

function bridge:call(modName: string, functionName: string, ...)
    assertBridge()
    return rml.bridge.call(modName, functionName, ...)
end

function bridge:register(modName: string, functionName: string, callback: (...any) -> any)
    assertBridge()
    return rml.bridge.register(modName, functionName, callback)
end

function bridge:listen(modName: string, eventName: string, callback: (...any) -> any)
    assertBridge()
    return rml.bridge.listen_event(modName, eventName, callback)
end

return bridge
