using System.Reflection;
using System.Runtime.Loader;

using RML.Core.Api;

using Roblox;

namespace RML.Core.Internal;

internal sealed class ModInfo(AssemblyLoadContext context, IMod instance, DataModelType[]? loadIn, Assembly modAssembly)
{
    public AssemblyLoadContext Context { get; } = context;
    public IMod Instance { get; } = instance;
    public DataModelType[]? LoadInDataModels { get; } = loadIn;
    public Assembly ModAssembly { get; } = modAssembly;
    public bool Initialized { get; set; }
}

internal sealed class ModRegistry
{
    private readonly Dictionary<string, ModInfo> _mods = new();
    private readonly object _lock = new();

    public bool Contains(string path)
    {
        lock (_lock)
        {
            return _mods.ContainsKey(path);
        }
    }

    public bool TryAdd(string path, ModInfo info)
    {
        lock (_lock)
        {
            return _mods.TryAdd(path, info);
        }
    }

    public bool TryRemove(string path, out ModInfo? info)
    {
        lock (_lock)
        {
            return _mods.Remove(path, out info);
        }
    }

    public bool TryGet(string path, out ModInfo? info)
    {
        lock (_lock)
        {
            return _mods.TryGetValue(path, out info);
        }
    }

    public string[] Keys()
    {
        lock (_lock)
        {
            return _mods.Keys.ToArray();
        }
    }

    public ModInfo[] Snapshot()
    {
        lock (_lock)
        {
            return _mods.Values.ToArray();
        }
    }

    public bool GetInitialized(ModInfo info)
    {
        lock (_lock)
        {
            return info.Initialized;
        }
    }

    public void SetInitialized(ModInfo info, bool value)
    {
        lock (_lock)
        {
            info.Initialized = value;
        }
    }
}
