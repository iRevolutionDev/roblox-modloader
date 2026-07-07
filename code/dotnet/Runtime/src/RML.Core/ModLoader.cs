using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.Loader;

using RML.Core.Api;
using RML.Core.Internal;
using RML.Core.Modding;

using Roblox;

namespace RML.Core;

internal static class ModLoader
{
    private static readonly Dictionary<string, ModInfo> _mods = new();
    private static readonly object _lock = new();
    private static string _modsRoot = string.Empty;

    internal static void Initialize(string modsRoot) => _modsRoot = modsRoot;

    internal static void LoadMod(string path)
    {
        lock (_lock)
        {
            if (_mods.ContainsKey(path))
            {
                RuntimeLog.Warn($"Mod at path {path} is already loaded.");
                return;
            }
        }

        try
        {
            var sharedAssemblies = AssemblyLoadContext.Default.Assemblies
                .Concat(GetCurrentAlcAssemblies())
                .ToList();

            var context = new ModAssemblyContext(path, sharedAssemblies);
            var assembly = context.LoadFromAssemblyPath(path);

            var modType = assembly.GetTypes()
                .FirstOrDefault(t =>
                    t.GetCustomAttribute<ModAttribute>() is not null && t.IsAssignableTo(typeof(IMod)) &&
                    !t.IsAbstract);

            if (modType is null)
            {
                RuntimeLog.Warn($"No [Mod] class implementing IMod found in assembly {assembly.FullName}.");
                context.Unload();
                return;
            }

            var mod = (IMod)Activator.CreateInstance(modType)!;

            var attr = modType.GetCustomAttribute<ModAttribute>()!;
            var loadIn = attr.LoadInDataModels;

            var modContext = new ModContext(
                new ModDescriptor(attr.Id, attr.Version, attr.Author, attr.Description), path, assembly);

            ModContext.Register(modContext);
            if (mod is ModBase modBase)
            {
                modBase.Context = modContext;
            }

            var info = new ModInfo(context, mod, loadIn, assembly);

            lock (_lock)
            {
                if (!_mods.TryAdd(path, info))
                {
                    RuntimeLog.Warn($"Mod at path {path} is already loaded.");
                    ModContext.Unregister(assembly);
                    context.Unload();
                    return;
                }
            }

            if (loadIn == null || loadIn.Length == 0)
            {
                try
                {
                    mod.OnLoad();
                    lock (_lock)
                    {
                        info.Initialized = true;
                    }
                }
                catch (Exception e)
                {
                    RuntimeLog.Error($"Error while calling OnLoad for mod at {path}: {e}");
                }
            }
        }
        catch (Exception e)
        {
            RuntimeLog.Error($"Failed to load mod at {path}: {e}");
            throw;
        }
    }

    internal static void UnloadMod(string path)
    {
        ModInfo? modInfo;
        lock (_lock)
        {
            if (!_mods.Remove(path, out modInfo))
            {
                RuntimeLog.Warn($"No mod loaded from path {path}.");
                return;
            }
        }

        var weakContext = UnloadModCore(path, modInfo);

        for (var i = 0; weakContext.IsAlive && i < 10; i++)
        {
            GC.Collect();
            GC.WaitForPendingFinalizers();
        }

        if (weakContext.IsAlive)
        {
            RuntimeLog.Warn(
                $"The AssemblyLoadContext for '{path}' did not unload — it is still rooted " +
                "(a leaked event handler, GCHandle, static reference, or running thread).");
        }
    }

    [MethodImpl(MethodImplOptions.NoInlining)]
    private static WeakReference UnloadModCore(string path, ModInfo modInfo)
    {
        bool wasInitialized;
        lock (_lock)
        {
            wasInitialized = modInfo.Initialized;
        }

        try
        {
            if (wasInitialized)
            {
                modInfo.Instance.OnUnload();
            }
        }
        catch (Exception e)
        {
            RuntimeLog.Error($"Error while unloading mod from {path}: {e}");
        }

        ModContext.Unregister(modInfo.ModAssembly);

        var weak = new WeakReference(modInfo.Context);
        modInfo.Context.Unload();
        return weak;
    }

    internal static void Shutdown()
    {
        string[] paths;
        lock (_lock)
        {
            paths = _mods.Keys.ToArray();
        }

        foreach (var path in paths)
        {
            UnloadMod(path);
        }
    }

    internal static void OnDataModelChanged(ulong oldDataModelPtr, ulong newDataModelPtr, DataModelType dataModelType)
    {
        var oldModel = DataModel.FromHandle((nuint)oldDataModelPtr);
        var newModel = DataModel.FromHandle((nuint)newDataModelPtr);

        ModInfo[] snapshot;
        lock (_lock)
        {
            snapshot = _mods.Values.ToArray();
        }

        foreach (var modInfo in snapshot)
        {
            // var interested = modInfo.LoadInDataModels;
            // if (interested == null || interested.Length == 0)
            //     continue;
            //
            // if (!interested.Contains(dataModelType))
            //     continue;

            if (newModel != null)
            {
                bool wasInitialized;
                lock (_lock)
                {
                    wasInitialized = modInfo.Initialized;
                }

                if (!wasInitialized)
                {
                    try
                    {
                        modInfo.Instance.OnLoad();
                    }
                    catch (Exception e)
                    {
                        RuntimeLog.Error($"Error while calling OnLoad for mod: {e}");
                    }

                    lock (_lock)
                    {
                        modInfo.Initialized = true;
                    }
                }

                if (modInfo.Instance is IDataModelAware aware)
                {
                    try
                    {
                        aware.OnDataModelLoaded(newModel, dataModelType);
                    }
                    catch (Exception e)
                    {
                        RuntimeLog.Error($"Error in IDataModelAware.OnDataModelLoaded: {e}");
                    }
                }
            }
            else
            {
                if (oldModel != null && modInfo.Instance is IDataModelAware aware)
                {
                    try
                    {
                        aware.OnDataModelUnloaded(oldModel, dataModelType);
                    }
                    catch (Exception e)
                    {
                        RuntimeLog.Error($"Error in IDataModelAware.OnDataModelUnloaded: {e}");
                    }
                }

                bool wasInitialized;
                lock (_lock)
                {
                    wasInitialized = modInfo.Initialized;
                }

                if (wasInitialized)
                {
                    try
                    {
                        modInfo.Instance.OnUnload();
                    }
                    catch (Exception e)
                    {
                        RuntimeLog.Error($"Error while calling OnUnload for mod: {e}");
                    }

                    lock (_lock)
                    {
                        modInfo.Initialized = false;
                    }
                }
            }
        }
    }

    private static IEnumerable<Assembly> GetCurrentAlcAssemblies()
    {
        var currentAlc = AssemblyLoadContext.GetLoadContext(
            typeof(ModLoader).Assembly);
        return currentAlc?.Assemblies ?? [];
    }

    private class ModInfo(AssemblyLoadContext context, IMod instance, DataModelType[]? loadIn, Assembly modAssembly)
    {
        public AssemblyLoadContext Context { get; } = context;
        public IMod Instance { get; } = instance;
        public DataModelType[]? LoadInDataModels { get; } = loadIn;
        public Assembly ModAssembly { get; } = modAssembly;
        public bool Initialized { get; set; }
    }
}