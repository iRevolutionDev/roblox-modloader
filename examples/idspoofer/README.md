# idspoofer

`idspoofer` is a native Roblox ModLoader mod. It can independently spoof:

- every reflected `Player.UserId` getter path used by the engine;
- `StudioService:GetUserId()`.

The two hooks operate at the Roblox reflection descriptor layer. They do not
use Luau hooks, script states, the task scheduler, or the DataModel watcher.

## Configuration

Edit the four top-level settings in `mod.toml`:

```toml
spoof_player = true
spoof_studio_service = true
player_user_id = 1585524057
studio_service_user_id = 1585524057
```

Both IDs must be positive signed 64-bit integers. Restart Studio after editing
the file. Each hook is installed only when its boolean is `true`.

## Installation layout

```text
RobloxStudio version directory/
  dwmapi.dll
  roblox_modloader.dll
  RobloxModLoader/
    mods/
      idspoofer/
        mod.toml
        native/
          idspoofer.dll
```

This build targets Roblox Studio `0.733.0.7330989` on Windows x64.

## Build

```powershell
cmake -S . -B build-idspoofer -DRML_NATIVE_ONLY=ON
cmake --build build-idspoofer --config Release --target idspoofer
```

The `RML_NATIVE_ONLY` configuration builds only the native runtime,
`dwmapi.dll`, and `idspoofer`; it does not configure the Luau, managed,
scheduler, Qt, RTTI-scanner, crash-dumper, or general hooking subsystems.
