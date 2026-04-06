--!optimize 2
--!strict

type Logger = {
    --- Logs an informational message.
    --- **This show logs on the RML console, not the Roblox Studio console.**
    --- @param self Logger
    --- @param message string
    --- @param ... any Additional arguments to format the message.
    info: (message: string, ...any) -> (),

    --- Logs a warning message.
    --- **This show logs on the RML console, not the Roblox Studio console.**
    --- @param self Logger
    --- @param message string
    --- @param ... any Additional arguments to format the message.
    warn: (message: string, ...any) -> (),

    --- Logs an error message.
    --- **This show logs on the RML console, not the Roblox Studio console.**
    --- @param self Logger
    --- @param message string
    --- @param ... any Additional arguments to format the message.
    error: (message: string, ...any) -> (),

    --- Logs a debug message.
    --- **This show logs on the RML console, not the Roblox Studio console.**
    --- @param self Logger
    --- @param message string
    --- @param ... any Additional arguments to format the message.
    debug: (message: string, ...any) -> (),
}

local logger: Logger = {
    info = function(self, message, ...)
       local source, line = debug.info(2, "sl")

        rml.logger:info(`[{source}:{line}] {message} {table.concat({...}, " ")}`)
    end,
    warn = function(self, message, ...)
        local source, line = debug.info(2, "sl")

        rml.logger:warn(`[{source}:{line}] {message} {table.concat({...}, " ")}`)
    end,
    error = function(self, message, ...)
        local source, line = debug.info(2, "sl")

        rml.logger:error(`[{source}:{line}] {message} {table.concat({...}, " ")}`)
    end,
    debug = function(self, message, ...)
        local source, line = debug.info(2, "sl")

        rml.logger:debug(`[{source}:{line}] {message} {table.concat({...}, " ")}`)
    end
}

return logger