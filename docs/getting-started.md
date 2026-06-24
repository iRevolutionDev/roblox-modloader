# Getting started

This guide installs the loader and runs an example mod in Roblox Studio. It assumes Windows.

## 1. Install the loader

You can use the launcher or install manually. The launcher is recommended because it handles
placement and updates for you.

### Option A — launcher

1. Download the latest [rml-launcher release](https://github.com/revolutionxk/rml-launcher/releases).
2. Run it and follow the prompts.
3. Start Roblox Studio through the launcher.

### Option B — manual

1. Download the latest [RML release](https://github.com/revolutionxk/roblox-modloader/releases), or
   [build from source](#building-from-source).
2. Copy `dwmapi.dll` into your Studio directory — the folder that contains `RobloxStudioBeta.exe`,
   usually `%LOCALAPPDATA%\Roblox\Versions\<version>\`.
3. Next to it, create a `RobloxModLoader` folder and place `roblox_modloader.dll` and `config.toml`
   inside.
4. Start Roblox Studio.

If the loader is active, you will see its log output in the console (and in the log files, if
file logging is enabled — see [Configuration](configuration.md)).

## 2. Install a mod

Mods live under `RobloxModLoader/mods/`. Each mod is its own folder, with subfolders by type:

```
RobloxModLoader/mods/
└── your-mod/
    ├── native/     native C++ mod DLLs
    ├── dotnet/     .NET mod assemblies
    └── scripts/    Luau scripts (temporarily disabled)
```

To run one of the bundled examples, build it (see below) and copy its output DLL into the matching
folder. For instance, a built .NET example goes in `your-mod/dotnet/`.

## 3. Write your own

- [Writing .NET mods](writing-dotnet-mods.md) — recommended starting point; the C# API is typed and
  comfortable.
- [Writing native mods](writing-native-mods.md) — for C++ mods that need hooking or low-level access.

The fastest path is to copy an [example](../examples) and adapt it.

## Building from source

If you are not using a prebuilt release:

```powershell
git clone https://github.com/revolutionxk/roblox-modloader.git
cd roblox-modloader

cmake -B build -S . -G "Visual Studio 17 2022"
cmake --build build --config Release
```

Prerequisites: Visual Studio 2022 (MSVC, x64), CMake 3.22.1+, the .NET 10 SDK, and Git. The build
produces the `dwmapi.dll` proxy and the loader, and fetches its dependencies automatically.

## Releases and nightly builds

The install-ready archives (the layout shown above) are produced by CI, not by hand. The
[`Package`](../.github/workflows/package.yml) workflow builds the native core and the .NET runtime,
assembles the package, and uploads `nightly-<platform>.zip` (for example `nightly-windows.zip`).

- **Nightly:** runs on a schedule and on demand; the latest archives are attached to the rolling
  `nightly` prerelease and are also available as workflow artifacts.
- **Versioned:** pushing a `v*` tag publishes a release with the same archives attached.

The bundled default `config.toml` comes from
[`tools/packaging/config.default.toml`](../tools/packaging/config.default.toml).

## Troubleshooting

- **Nothing happens in Studio.** Confirm `dwmapi.dll` sits next to `RobloxStudioBeta.exe` and that
  `roblox_modloader.dll` is inside the `RobloxModLoader` folder beside it.
- **A mod doesn't load.** Check it is in the correct `mods/<name>/<type>/` subfolder and that the
  loader log doesn't report an error for it.
- **A crash on startup.** The native core writes a crash dump near the loader; include it when
  filing an issue.
