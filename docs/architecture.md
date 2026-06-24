# Architecture

This is a tour of how Roblox ModLoader is put together, from the moment Studio starts to a mod
calling into the engine. It is aimed at contributors and curious users.

## The big picture

```
RobloxStudioBeta.exe
  └─ dwmapi.dll (generated proxy)         loaded via DLL search order; forwards the real exports
       └─ roblox_modloader.dll            the native core
            ├─ engine resolution          pattern scan → function pointers and offsets
            ├─ hooking                     detours + vtable swaps on engine functions
            ├─ mod manager                 discovers and loads mods by type
            ├─ native mods (.dll)          C++
            └─ .NET runtime (CoreCLR)      hosted via hostfxr/nethost
                 ├─ RML.NativeHost         unmanaged entry points
                 ├─ RML.Core               per-mod loader, lifecycle, DataModel awareness
                 ├─ RML.Interop            the marshalling boundary
                 └─ Roblox                 strongly-typed engine wrappers
```

## Injection

Studio is not modified on disk. Instead, the loader ships a generated `dwmapi.dll` proxy that
Windows loads from Studio's own directory before the real system DLL. The proxy forwards every real
export so Studio behaves normally, and uses its load entry point to bring up
`roblox_modloader.dll`. From there the native core owns the process-side integration.

## The native core

Once loaded, the core:

- **Resolves the engine.** It pattern-scans the running Studio image to find the engine functions
  and object offsets it needs. This is what keeps the rest of the system decoupled from any single
  Studio build.
- **Installs hooks.** Using detours and vtable swaps, it intercepts the engine functions it needs to
  observe lifecycle events and to expose hooking to native mods.
- **Manages mods.** A mod manager scans `RobloxModLoader/mods/` and loads each mod by type — native
  DLLs, .NET assemblies, and (when enabled) Luau scripts — each through its own loader.
- **Hosts the .NET runtime.** It boots CoreCLR through `hostfxr`/`nethost` and hands control to the
  managed host.

## The managed runtime

The .NET side is layered so that responsibilities stay separate:

- **RML.NativeHost** exposes the unmanaged entry points the native core calls into.
- **RML.Core** loads each managed mod into its own collectible context, runs the `IMod` lifecycle,
  and notifies `IDataModelAware` mods when a place loads or unloads.
- **RML.Interop** is the boundary: it marshals values between native and managed code through a small
  tagged-union type and a function table shared by both sides.
- **Roblox** is the typed surface mod authors use. Its wrapper classes are generated from the
  engine's reflection metadata, and its value types are laid out to match the engine's memory
  exactly, so reads and writes are byte-accurate rather than approximated.

## The interop boundary

Calls in either direction pass through a fixed-size tagged value and a versioned function table.
Property reads/writes and method calls are routed through the engine's own reflection, so the
managed API is driven by the same descriptors the engine uses internally. Value types such as
`CFrame` and `Vector3` are copied across as raw struct bytes because both sides agree on the layout;
variable-length types such as `NumberSequence` are length-prefixed.

## The type generator

The hundreds of wrapper classes in `Roblox` are not written by hand. A generator
(`Roblox.TypeGenerator`) reads Roblox's published API metadata and emits one class per engine class,
plus a manifest that maps each class name to its wrapper and a factory. Re-running the generator is
how the API surface is kept current with Studio.

## Where things live

```
code/
  roblox_modloader/   native core (injection, hooking, mod manager, interop provider)
  dotnet/
    Runtime/          RML.NativeHost, RML.Core, RML.Interop
    Roblox/           the typed wrappers and the type generator
libs/                 vendored third-party (g3d, bullet, rbxg3d)
examples/             sample mods
```
