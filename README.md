# Roblox ModLoader

A powerful mod loader that revolutionizes Roblox Studio development with native C++/C# mods and internal Luau scripting capabilities.

> [!NOTE]
> This project is still in development and may contain bugs or incomplete features.

> [!WARNING]
> Roblox changed (shuffled) the internal layout of Luau’s structs a few months ago, so the in-memory structures we relied on no longer line up. Because of that I’m building a static-analysis [dumper](https://github.com/revolutionxk/roblox-modloader/tree/develop/dumper) to reconstruct the correct structs and offsets so scripting support can work again. Luau/internal scripting is temporarily disabled while I finish that—native C++/C# mods keep working normally. I’ll re-enable scripting once the [dumper](https://github.com/revolutionxk/roblox-modloader/tree/develop/dumper) produces a stable, reliable mapping.

## Cross-Platform Support

- [x] Windows
- [ ] macOS (planned)
- [ ] Linux Vinegar (planned)

## Quick Start

### Installation

#### Launcher Setup
1. Download the latest release from the [releases page](https://github.com/revolutionxk/rml-launcher/releases)
2. Run the installer and follow the prompts to set up the launcher
3. Launch Roblox Studio through the launcher to automatically load the mod loader and your mods

#### Manual Setup
1. Download the latest release from the [releases page](https://github.com/revolutionxk/roblox-modloader/releases)
2. Extract the archive to your desired location
3. Copy `dwmapi.dll` to your Roblox Studio installation directory (usually `%LOCALAPPDATA%\Roblox Studio`)
4. Create a `RobloxModLoader` folder next to `dwmapi.dll`
5. Place `roblox_modloader.dll` and `config.toml` in the `RobloxModLoader` folder
6. Launch Roblox Studio

### Directory Structure

```
%LOCALAPPDATA%\Roblox Studio\
├── dwmapi.dll                    # Proxy DLL for injection
├── roblox_modloader.dll          # Main mod loader
├── RobloxModLoader/
│   ├── config.toml               # Configuration file
│   └── mods/                     # Mod directory
│       ├── native/               # Native C++ or C# mods
│       │   └── your_mod.dll      # C++ mods
│       └── scripts/              # Luau scripts
│           └── your_script.luau
```

## Creating Mods

### C++ DLL Mods

Create a new CMake project and link against RobloxModLoader:

```cmake
FetchContent_Declare(
        rml
        GIT_REPOSITORY https://github.com/revolutionxk/roblox-modloader.git
        GIT_TAG v0.12.2
        GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(rml)

add_library(your_mod SHARED mod.cpp)
roblox_add_mod(your_mod)
```

Basic mod structure:

```cpp
#include <RobloxModLoader/mod/mod_base.hpp>
#include <RobloxModLoader/logger/logger.hpp>
#include <RobloxModLoader/hooking/hooking.hpp>

namespace mod::hooks {
    static uint64_t* example_hook(uint64_t* instance, uint64_t param1) {
        // Pre-hook logic
        auto result = hooking::get_original<&example_hook>()(instance, param1);
        // Post-hook logic
        return result;
    }
}

class your_mod final : public mod_base {
public:
    your_mod() {
        name = "Your Mod";
        version = "1.0.0";
        author = "Your Name";
        description = "Description of your mod";
    }

    void on_load() override {
        auto logger = logger::get_logger("YourMod");
        logger->info("Mod loaded successfully!");
        
        // Register hooks
        hooking::detour_hook_helper::add<&mod::hooks::example_hook>(
            "EXAMPLE_HOOK", 
            target_address
        );
    }

    void on_unload() override {
        // Cleanup code
    }
};

#define YOUR_MOD_EXPORT __declspec(dllexport)
extern "C" {
    YOUR_MOD_EXPORT mod_base* start_mod() {
        return new your_mod();
    }

    YOUR_MOD_EXPORT void uninstall_mod(const mod_base* mod) {
        delete mod;
    }
}
```

### Luau Script Mods

Create `.luau` files in the `mods/scripts/` directory:

```luau
local logger = require("@rml/logger")
local bridge = require("@rml/bridge")

logger:info("Script mod loaded!")

-- Register functions for other mods to call
bridge:register("YourMod", "greet", function(name)
    return "Hello, " .. name .. "!"
end)

-- Listen to events from other mods
bridge:listen("SomeMod", "player_joined", function(player)
    logger:info("Player joined: " .. player.Name)
end)

-- Call functions from other mods
local result = bridge:call("OtherMod", "getData", "some_parameter")
```

## Building from Source

### Prerequisites

- Visual Studio 2022 or newer
- CMake 3.22.1 or newer
- Git

### Build Steps

```powershell
# Clone the repository
git clone https://github.com/revolutionxk/roblox-modloader.git
cd roblox-modloader

# Configure with Visual Studio generator
cmake -B build -S . -G "Visual Studio 17 2022"

# Build the project
cmake --build build --config Release

# The proxy DLL will be automatically generated and copied to the appropriate directory
```

### Build Options

| Option | Description | Default |
|--------|-------------|---------|
| `ROBLOX_MODLOADER_BUILD_PROXY_GENERATOR` | Build the proxy generator tool | ON |
| `ROBLOX_MODLOADER_BUILD_PROXY_DLL` | Auto-generate dwmapi.dll proxy | ON |
| `ROBLOX_MODLOADER_BUILD_EXAMPLES` | Build example mods | ON |

## Configuration

The `config.toml` file allows customization of the mod loader behavior:

```toml
[logging]
level = "info"
console = true
file = true

[hooking]
enable_crash_handler = true
hook_validation = true

[scripts]
hot_reload = true
execution_timeout = 5000

[mods]
auto_load = true
load_order = []
```

## API Reference

### Hooking System

```cpp
// Define hook function
static RetType hook_func(Args...) {
    auto result = hooking::get_original<&hook_func>()(args...);
    return result;
}

// Register hook
hooking::detour_hook_helper::add<&hook_func>("HOOK_NAME", target_address);
```

### Luau Bridge

```luau
local bridge = require("@rml/bridge")

-- Register function
bridge:register("ModName", "functionName", function(param)
    return result
end)

-- Call function
local result = bridge:call("ModName", "functionName", param)

-- Listen to events
bridge:listen("ModName", "eventName", function(data)
    -- Handle event
end)
```

## Examples

The `examples/` directory contains sample mods demonstrating various features:

- **basic-mod**: Simple C++ mod with hooking
- **discord_rpc**: Discord Rich Presence integration
- **internal_developer**: Enables Internal Roblox Developer tools

## Debugging

### Development Setup

1. Build in Debug configuration
2. Attach Visual Studio debugger to `RobloxStudioBeta.exe`
3. Set breakpoints in your mod code
4. Use the logger for runtime diagnostics

### Common Issues

- **Mod not loading**: Check that exports are correctly defined
- **Hooks not working**: Verify target addresses with a disassembler
- **Scripts failing**: Check Luau syntax and runtime errors in logs
- **Crashes**: Exception handler generates crash dumps in the mod directory

## Contributing

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/amazing-feature`
3. Commit your changes: `git commit -m 'Add amazing feature'`
4. Push to the branch: `git push origin feature/amazing-feature`
5. Open a Pull Request

### Development Guidelines

- Follow the existing code style and patterns
- Add tests for new functionality
- Update documentation for API changes
- Use meaningful commit messages
- Ensure cross-platform compatibility where possible

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Disclaimer

This software is provided for educational and research purposes. Users are responsible for complying with Roblox's Terms of Service and any applicable laws. The developers are not responsible for any consequences resulting from the use of this software.

## Support

- **Issues**: [GitHub Issues](https://github.com/revolutionxk/roblox-modloader/issues)
- **Discussions**: [GitHub Discussions](https://github.com/revolutionxk/roblox-modloader/discussions)
- **Wiki**: [Project Wiki](https://github.com/revolutionxk/roblox-modloader/wiki)

---

**Note**: This project is not affiliated with or endorsed by Roblox Corporation.