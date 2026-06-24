# Writing .NET mods

A .NET mod is an ordinary class library that the loader discovers, loads into its own isolated
context, and drives through a small lifecycle. You write against a strongly-typed view of the
engine in the `Roblox` namespace.

A complete, runnable reference is [`examples/example_dotnet`](../examples/example_dotnet); a real
integration is [`examples/discord_rpc_dotnet`](../examples/discord_rpc_dotnet).

## Project setup

Target .NET 10 and reference `RML.Core`. That single reference also brings in the typed `Roblox`
API.

```xml
<Project Sdk="Microsoft.NET.Sdk">
  <PropertyGroup>
    <TargetFramework>net10.0</TargetFramework>
    <Nullable>enable</Nullable>
    <ImplicitUsings>enable</ImplicitUsings>
  </PropertyGroup>
  <ItemGroup>
    <ProjectReference Include="path/to/code/dotnet/Runtime/src/RML.Core/RML.Core.csproj" />
  </ItemGroup>
</Project>
```

Build the project, then copy the resulting assembly into a mod folder under the loader:
`RobloxModLoader/mods/<your-mod>/dotnet/`.

## The mod class

Annotate a class with `[Mod]` and implement the lifecycle. You can implement `IMod` directly or
extend the `Mod` base class, which adds a ready-to-use `Logger`.

```csharp
using RML.Core.Api;
using RML.Core.Modding;

[Mod("my-mod", "1.0.0", Author = "You", Description = "What it does")]
public sealed class MyMod : Mod
{
    public override int OnLoad()
    {
        Logger.Info("Loaded.");
        return 0; // non-zero signals a load failure
    }

    public override void OnUnload()
    {
        Logger.Info("Unloaded.");
    }
}
```

- `OnLoad` runs when the mod is loaded; return `0` for success.
- `OnUnload` runs when the mod is unloaded. Release anything you started here — the mod's context is
  collectible, so leaks keep it alive.

## Reacting to a place

Most mods want to act when a place (a `DataModel`) becomes available. Implement `IDataModelAware`:

```csharp
using RML.Core.Api;
using RML.Core.Modding;
using Roblox;

[Mod("place-watcher", "1.0.0", Author = "You")]
public sealed class PlaceWatcher : Mod, IDataModelAware
{
    public override int OnLoad() => 0;
    public override void OnUnload() { }

    public void OnDataModelLoaded(DataModel game, DataModelType type)
    {
        Logger.Info($"Place loaded as {type}.");

        var workspace = game.GetService("Workspace")?.As<Workspace>();
        Logger.Info($"Camera: {workspace?.CurrentCamera?.CFrame}");
    }

    public void OnDataModelUnloaded(DataModel game, DataModelType type) { }
}
```

`DataModelType` distinguishes the context (for example, an edit session versus play-testing), so you
can enable behavior only where it makes sense.

## Working with the engine

The `Roblox` namespace mirrors the engine's object model with ordinary C# types.

- **Services and lookups:** `game.GetService("Workspace")`, `instance.FindFirstChild("Name")`,
  `instance.GetChildren()`.
- **Typed casts:** `instance.As<Part>()` returns the typed wrapper (or `null`).
- **Properties:** read and write them directly — `part.Name`, `part.Position`, `part.CFrame`,
  `part.BrickColor`.
- **Value types:** `Vector3`, `Vector2`, `CFrame`, `Color3`, `UDim`, `UDim2`, `Rect`, `Region3`,
  `Ray`, `NumberRange`, `Faces`, `Axes`, `BrickColor`, `NumberSequence`, `ColorSequence` are all
  real structs you can construct and pass around.
- **Events:** subscribe with `+=` and unsubscribe with `-=`.

```csharp
var workspace = game.GetService("Workspace")?.As<Workspace>();

workspace!.DescendantAdded += instance =>
    Logger.Info($"Added {instance.Name} ({instance.ClassName})");

var part = workspace.FindFirstChild("Baseplate")?.As<Part>();
if (part is not null)
{
    part.BrickColor = BrickColor.FromName("Bright red");
    part.CFrame = new CFrame(new Vector3(0, 10, 0));
}
```

## Notes

- **Threading.** You can do background work (for example with `Task.Run`), but engine objects are
  not free-threaded — keep engine access on the appropriate thread and avoid blocking it.
- **Unsubscribe and dispose on unload.** Anything you connect or start in `OnLoad` /
  `OnDataModelLoaded` should be undone in `OnUnload`, so the mod can be cleanly unloaded.
