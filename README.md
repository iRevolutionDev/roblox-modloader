# Roblox ModLoader

Roblox ModLoader is a powerful mod loader for Roblox Studio, enabling developers to load custom mods (DLLs and Luau scripts) into Studio for enhanced functionality, automation, and experimentation.

---

## Table of Contents

- [Features](#features)
- [Installation](#installation)
- [Building from Source](#building-from-source)
- [Usage](#usage)
- [Mod Development](#mod-development)
  - [C++ Native Mods](#c-native-mods)
  - [Luau Script Mods](#luau-script-mods)
  - [Mod Configuration](#mod-configuration)
- [Configuration](#configuration)
- [Logging](#logging)
- [Troubleshooting](#troubleshooting)
- [Contributing](#contributing)
- [License](#license)
- [Keywords](#keywords)

---

## Features

- Load C++ DLL mods and Luau scripts into Roblox Studio
- Hot-reload mods and scripts (debug mode)
- Custom mod configuration via TOML files
- Logging to file and console (configurable)
- Event system for mod communication
- Script scheduling and sandboxing
- Example mods provided

---

## Installation

1. **Download the Latest Release**
   - Go to the [releases page](https://github.com/iRevolutionDev/roblox-modloader/releases)
   - Download the latest `roblox-modloader-<version>.zip`

2. **Extract Files**
   - Extract the contents to a folder of your choice

3. **Run the Installer**
   - Run `ROBLOX Mod Loader.exe`
   - Click `Install` and follow the instructions

4. **Open Roblox Studio**
   - Launch Roblox Studio

5. **Open Mods Folder**
   - In Studio, go to `Plugins` → `ROBLOX Mod Loader` → `Open Mods Folder`

6. **Add Mods**
   - Place your mods (DLLs, scripts, configs) in the opened folder

---

## Building from Source

### Prerequisites

- **CMake** >= 3.22.1
- **Visual Studio 2022** or **Clang** (Windows)
- **Ninja** (recommended for fast builds)
- **Git** (for submodules)
- **Roblox Studio** (for testing)

### Steps

1. **Clone the Repository**
   ```sh
   git clone --recursive https://github.com/iRevolutionDev/roblox-modloader.git
   cd roblox-modloader
   ```

2. **Configure the Project**
   ```sh
   cmake -B build -G "Ninja"
   ```

3. **Build**
   ```sh
   cmake --build build --parallel
   ```

4. **(Optional) Run Tests**
   ```sh
   ctest --test-dir build
   ```

5. **Find Output**
   - DLL: `build/Release/roblox_modloader.dll`
   - PDB: `build/Release/roblox_modloader.pdb`

---

## Usage

- Place `roblox_modloader.dll` in the appropriate folder as instructed by the installer.
- Mods should be placed in `RobloxModLoader/mods/` (DLLs, scripts, configs).
- Example mods are available in `examples/`.

---

## Mod Development

### C++ Native Mods

- Create a new folder in `examples/` (e.g., `my_mod/`)
- Add a `main.cpp` implementing a class derived from `mod_base`
- Use the provided CMake functions to build your mod as a DLL

### Luau Script Mods

- Place `.lua` or `.luau` scripts in the `scripts/` subfolder of your mod directory
- Reference scripts in your mod's `mod.toml` configuration

### Mod Configuration

- Each mod should have a `mod.toml` file describing metadata and settings:
  ```toml
  name = "My Mod"
  version = "1.0.0"
  author = "Your Name"
  description = "Description of your mod"

  [runtime]
  enabled = true
  auto_load = true
  priority = 0
  dependencies_path = "deps/"

  [resources]
  max_memory_usage = 104857600
  max_execution_time_ms = 1000
  assets_path = "assets/"
  ```

---

## Configuration

- Core configuration is stored in `RobloxModLoader/config.toml`
- Mod configurations are stored in each mod's folder as `mod.toml`
- See `include/RobloxModLoader/config/config_helpers.hpp` for available config options

---

## Logging

- Logs are written to the directory specified in the config (`log_directory`)
- Console logging can be enabled/disabled via config
- Log retention and max log files are configurable

---

## Troubleshooting

- **DLL Not Loading:** Ensure the DLL is in the correct folder and built for the right architecture.
- **Mod Not Detected:** Check `mod.toml` for errors and ensure `enabled = true`.
- **Script Errors:** Review logs in the log directory for details.
- **Build Issues:** Make sure all dependencies are fetched and CMake version is correct.

---

## Contributing

- Fork the repository and submit pull requests
- See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines

---

## License

- See [LICENSE](LICENSE) for details

---

## Keywords

Roblox, ModLoader, Studio, DLL, Luau, C++, CMake, Ninja, Plugins, Scripting, TOML, Logging, Hot Reload, Event System, Configuration, Example Mods, Native Mods, Script Mods, Automation, Sandbox, Security, Performance, Debugging, Release, Build, Artifacts,