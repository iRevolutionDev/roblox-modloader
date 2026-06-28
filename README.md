<p align="center">
  <p align="center">
	<img width="150" height="150" src="./assets/logo.png" alt="Logo">
  </p>
  <h1 align="center"><b>Roblox ModLoader</b></h1>
  <p align="center">
    A modding framework for Roblox Studio, enabling native, C#, and internal Luau script mods.
  </p>
</p>

<div align="center">

![GitHub Workflow Status](https://img.shields.io/github/actions/workflow/status/revolutionxk/roblox-modloader/build.yml?style=for-the-badge&branch=develop&logo=github&label=develop%20build)
![GitHub Workflow Status](https://img.shields.io/github/actions/workflow/status/revolutionxk/roblox-modloader/build.yml?style=for-the-badge&branch=main&logo=github&label=main%20build)
[![GitHub License](https://img.shields.io/github/license/revolutionxk/roblox-modloader?style=for-the-badge)](LICENSE)

</div>

<div align="center">

<div background="none" class="previewbox previewDiscordText"><div class="discordtext"><svg color="#FFFFFF" fill="#2C2F33" width="60" height="60" viewBox="0 0 48 48" class="discord-logo-container"><rect width="100%" height="100%" fill="currentfill"></rect> <foreignObject style="width: 100%; height: 100%;"><body xmlns="http://www.w3.org/1999/xhtml"><canvas id="discordCanvas"></canvas></body></foreignObject> <defs><g><defs><path id="discord-def-face" fill="currentcolor" d="M40,12C40,12,35.415,8.412,30,8L29.512,8.976C34.408,10.174,36.654,11.891,39,14C34.955,11.935,30.961,10,24,10S13.045,11.935,9,14C11.346,11.891,14.018,9.985,18.488,8.976L18,8C12.319,8.537,8,12,8,12S2.879,19.425,2,34C7.162,39.953,15,40,15,40L16.639,37.815C13.857,36.848,10.715,35.121,8,32C11.238,34.45,16.125,37,24,37S36.762,34.45,40,32C37.285,35.121,34.143,36.848,31.361,37.815L33,40C33,40,40.838,39.953,46,34C45.121,19.425,40,12,40,12Z"></path> <g id="discord-def-face-eyes"><path id="discord-def-face-left-eye" fill="currentfill" d="M17.5,30C15.567,30,14,28.209,14,26C14,23.791,15.567,22,17.5,22S21,23.791,21,26C21,28.209,19.433,30,17.5,30Z"></path> <path id="discord-def-face-right-eye" fill="currentfill" d="M30.5,30C28.567,30,27,28.209,27,26C27,23.791,28.567,22,30.5,22S34,23.791,34,26C34,28.209,32.433,30,30.5,30Z"></path></g></defs> <g id="discordFaceID76"><use href="#discord-def-face"></use> <g id="discord-logo-eyes"><mask id="mask-right-eye-wink"><ellipse fill="#FFFFFF" ry="15" rx="15" cy="39.7" cx="35"></ellipse> <ellipse fill="#000000" ry="15" rx="15" cy="40.5" cx="34"></ellipse></mask> <mask id="mask-eyes-angry"><rect height="48" width="48" y="0" x="0" fill="#FFFFFF"></rect> <rect transform="rotate(45 24,14.5)" height="24" width="24" y="2.5" x="12" fill="#000000"></rect></mask> <g class="discord-eyes"><use xlink:href="#discord-def-face-eyes"></use></g></g></g> <mask id="mask-outer-layer"><rect width="100%" height="100%" fill="#FFFFFF"></rect> <circle r="42%" cx="50%" cy="50%" fill="#000000"></circle></mask> <mask id="mask-middle-layer"><rect width="100%" height="100%" fill="#000000"></rect> <circle r="43%" cx="50%" cy="50%" fill="#FFFFFF"></circle> <circle r="32%" cx="50%" cy="50%" fill="#000000"></circle></mask> <mask id="mask-inner-layer"><rect width="100%" height="100%" fill="#000000"></rect> <circle r="32%" cx="50%" cy="50%" fill="#FFFFFF"></circle></mask></g></defs> <g class="discord-logo swirl-animation"><use class="discord-original" xlink:href="#discordFaceID76"></use><use class="discord-inner-layer" xlink:href="#discordFaceID76" mask="url(#mask-inner-layer)"></use><use class="discord-middle-layer" xlink:href="#discordFaceID76" mask="url(#mask-middle-layer)"></use><use class="discord-outer-layer" xlink:href="#discordFaceID76" mask="url(#mask-outer-layer)"></use></g> <a href="https://robloxmodloader.com"><rect width="100%" height="100%" fill-opacity="0"></rect></a> <!----></svg> <svg height="30" preserveAspectRatio="xMinYMin" class="speechbubble" viewBox="0 0 955.359375 200"><g class="pathElementGroup" transform="translate(955.359375, 0)"><path transform="scale(-1,1)" fill="#FFFFFF" d="M 919.859375,0 L 20.5,0 C 9.2,0 0,9.2 0,20.6 L 0,155.8 C 0,167.2 9.2,177 20.5,176.4 L 899.259375,176.4 L 893.959375,157.9 L 906.759375,169.8 L 918.859375,181 L 940.359375,200 L 940.359375,20.6 C 940.359375,9.2 931.159375,0 919.859375,0 Z" class="pathElement"></path></g> <text fill="#2C2F33" font-size="90" x="95" y="57%" class="textElement">Join us on Discord!</text> <a href="https://robloxmodloader.com"><rect width="100%" height="100%" fill-opacity="0"></rect></a></svg></div></div>

<style type='text/css'>* { font-family: 'Avenir', Helvetica, Arial, sans-serif; } .discord-logo { transform: scale(0.7); transform-origin: 24px 24px; } .speechbubble { position: relative; transform: translateY(-50%); } .discordtext { opacity: 0.75; } .discordtext:hover { opacity: 1; } .discordtext a { text-decoration: none; }.discord-logo.swirl-animation .discord-outer-layer { transition: transform 800ms cubic-bezier(0.7, 1, 0.7, 1); transform-origin: 50% 50%; } .discord-logo-container:hover .swirl-animation .discord-outer-layer, .animated .swirl-animation .discord-outer-layer { transform: scale(1.5) rotate(360deg); } .discord-logo.swirl-animation .discord-middle-layer { transition: transform 800ms cubic-bezier(0.5, 1, 0.5, 1); transform-origin: 50% 50%; } .discord-logo-container:hover .swirl-animation .discord-middle-layer, .animated .swirl-animation .discord-middle-layer { transform: scale(1.4) rotate(360deg); } .discord-logo.swirl-animation .discord-inner-layer { transition: transform 800ms cubic-bezier(0.3, 1, 0.3, 1); transform-origin: 50% 50%; } .discord-logo-container:hover .swirl-animation .discord-inner-layer, .animated .swirl-animation .discord-inner-layer { transform: scale(1.3) rotate(360deg); } .discord-logo.swirl-animation .discord-original { transition: visibility 0ms; transition-delay: 800ms; } .discord-logo-container:hover .swirl-animation .discord-original, .animated .swirl-animation .discord-original { visibility: hidden; transition-delay: 0ms; }</style>

</div>

> [!NOTE]
> This project is still in development and may contain bugs or incomplete features.

> [!WARNING]
> Roblox changed (shuffled) the internal layout of Luau’s structs a few months ago, so the in-memory structures we
> relied on no longer line up. Because of that I’m building a
> static-analysis [dumper](https://github.com/revolutionxk/roblox-modloader/tree/develop/dumper) to reconstruct the
> correct structs and offsets so scripting support can work again. Luau/internal scripting is temporarily disabled while
> I
> finish that—native C++/C# mods keep working normally. I’ll re-enable scripting once
> the [dumper](https://github.com/revolutionxk/roblox-modloader/tree/develop/dumper) produces a stable, reliable
> mapping.

## Cross-Platform Support

- [x] Windows
- [x] Linux Vinegar
- [ ] macOS (planned)

## Quick Start

### Installation

RML targets your local Roblox **Studio** installation (typically under
`%LOCALAPPDATA%\Roblox\Versions\<version>\`).

### Using the launcher (Not finished yet)

The launcher handles placement and keeps the loader up to date.

1. Download the latest [rml-launcher release](https://github.com/revolutionxk/rml-launcher/releases).
2. Run it and follow the prompts.
3. Start Roblox Studio through the launcher — the loader and your mods are applied automatically.

### Manual installation

1. Download the latest [RML release](https://github.com/revolutionxk/roblox-modloader/releases).
2. Open your Studio directory — the folder that contains `RobloxStudioBeta.exe`, usually
   `%LOCALAPPDATA%\Roblox\Versions\<version>\`.
3. Extract the archive into that folder.
4. Start Roblox Studio.

The release archive is already laid out so a plain extract lands everything in the right place:

```
<Studio directory>/
├── RobloxStudioBeta.exe
├── dwmapi.dll                    proxy, loaded by Studio
└── RobloxModLoader/
    ├── roblox_modloader.dll      native core
    ├── config.toml               created with defaults if missing
    ├── runtime/                  the .NET host and bundled runtime
    └── mods/                     your mods, one folder each
        └── your-mod/
            ├── native/           native C++ mod DLLs
            ├── dotnet/           .NET mod assemblies
            └── scripts/          Luau scripts (temporarily disabled)
```

## Writing a mod

The quickest way to start is to copy one of the [examples](#examples) and adapt it. Full guides:
[Writing .NET mods](docs/writing-dotnet-mods.md) and
[Writing native mods](docs/writing-native-mods.md).

## Examples

| Example                                             | Surface    | What it shows                                                     |
|-----------------------------------------------------|------------|-------------------------------------------------------------------|
| [`basic-mod`](examples/basic-mod)                   | C++        | Minimal native mod skeleton and the hooking entry points          |
| [`internal_developer`](examples/internal_developer) | C++        | Enables Studio's internal developer tools                         |
| [`discord_rpc`](examples/discord_rpc)               | C++ / Luau | Discord Rich Presence, native + script bridge                     |
| [`example_dotnet`](examples/example_dotnet)         | C#         | Services, instances, properties, and events through the typed API |
| [`discord_rpc_dotnet`](examples/discord_rpc_dotnet) | C#         | A managed Discord Rich Presence integration                       |

## Building from source

### Prerequisites

- Windows with Visual Studio 2022 (MSVC, x64)
- CMake 3.22.1 or newer
- .NET 10 SDK (for the managed runtime and mods)
- Git

### Steps

```powershell
git clone https://github.com/revolutionxk/roblox-modloader.git
cd roblox-modloader

cmake -B build -S . -G "Visual Studio 17 2022"
cmake --build build --config Release
```

The build generates the `dwmapi.dll` proxy and the loader. Dependencies are fetched automatically
via CMake.

### Build options

| Option                                   | Description                          | Default |
|------------------------------------------|--------------------------------------|---------|
| `ROBLOX_MODLOADER_BUILD_PROXY_GENERATOR` | Build the proxy generator tool       | ON      |
| `ROBLOX_MODLOADER_BUILD_PROXY_DLL`       | Auto-generate the `dwmapi.dll` proxy | ON      |
| `ROBLOX_MODLOADER_BUILD_EXAMPLES`        | Build the example mods               | ON      |

## Roadmap

Current focus areas:

- Finish the [dumper](docs/dumper.md)
- Re-enable Luau/internal scripting support
- Add macOS support (long-term goal)
- Finish .NET modding support (Better API, async support, etc.)
- Add a mod browser and installer

## Contributing

Contributions are welcome. Please open an issue to discuss substantial changes first, keep pull
requests focused, follow the existing code style, and add tests where it makes sense. The
[`docs/`](docs/) directory is the best starting point for understanding the codebase.

## License

Released under the MIT License. See [LICENSE](LICENSE).

## Disclaimer

This project is provided for educational and research purposes. You are responsible for complying
with Roblox's Terms of Service and any applicable laws. It is not affiliated with or endorsed by
Roblox Corporation.
