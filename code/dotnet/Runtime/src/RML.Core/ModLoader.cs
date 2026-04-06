using System.Reflection;
using System.Runtime.Loader;

using RML.Core.Api;
using RML.Core.Internal;
using RML.Core.Modding;

using Roblox;

namespace RML.Core;

internal static class ModLoader
{
    private static readonly Dictionary<string, ModInfo> _mods = new();
    private static string _modsRoot = string.Empty;


    internal static void Initialize(string modsRoot) => _modsRoot = modsRoot;

    internal static void LoadMod(string path)
    {
        if (_mods.ContainsKey(path))
        {
            Console.WriteLine($"Mod at path {path} is already loaded.");
            return;
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
                Console.WriteLine($"No [RmlMod] class implementing IMod found in assembly {assembly.FullName}.");
                context.Unload();
                return;
            }

            var mod = (IMod)Activator.CreateInstance(modType)!;

            var attr = modType.GetCustomAttribute<ModAttribute>();
            var loadIn = attr?.LoadInDataModels;

            var info = new ModInfo(context, mod, loadIn);
            _mods[path] = info;

            if (loadIn == null || loadIn.Length == 0)
            {
                try
                {
                    mod.OnLoad();
                    info.Initialized = true;
                }
                catch (Exception e)
                {
                    Console.WriteLine($"Error while calling OnLoad for mod at {path}: {e}");
                }
            }
        }
        catch (Exception e)
        {
            Console.WriteLine(e);
            throw;
        }
    }

    internal static void UnloadMod(string path)
    {
        if (!_mods.TryGetValue(path, out var modInfo))
        {
            Console.WriteLine($"No mod loaded from path {path}.");
            return;
        }

        try
        {
            if (modInfo.Initialized)
            {
                modInfo.Instance.OnUnload();
            }
        }
        catch (Exception e)
        {
            Console.WriteLine($"Error while unloading mod from {path}: {e}");
        }
        finally
        {
            _mods.Remove(path);
            modInfo.Context.Unload();

            // yes i ik this is ugly but works
            for (var i = 0; i < 3 && modInfo.Context.IsCollectible; i++)
            {
                GC.Collect();
                GC.WaitForPendingFinalizers();
            }
        }
    }

    internal static void Shutdown()
    {
        foreach (var path in _mods.Keys.ToArray())
        {
            UnloadMod(path);
        }
    }

    internal static void OnDataModelChanged(ulong oldDataModelPtr, ulong newDataModelPtr, DataModelType dataModelType)
    {
        var oldModel = DataModel.FromHandle((nuint)oldDataModelPtr);
        var newModel = DataModel.FromHandle((nuint)newDataModelPtr);

        foreach (var kv in _mods.ToArray())
        {
            var modInfo = kv.Value;

            // var interested = modInfo.LoadInDataModels;
            // if (interested == null || interested.Length == 0)
            //     continue;
            //
            // if (!interested.Contains(dataModelType))
            //     continue;

            if (newModel != null)
            {
                if (!modInfo.Initialized)
                {
                    try
                    {
                        modInfo.Instance.Game = newModel;
                        modInfo.Instance.OnLoad();
                    }
                    catch (Exception e)
                    {
                        Console.WriteLine($"Error while calling OnLoad for mod: {e}");
                    }

                    modInfo.Initialized = true;
                }
                else
                {
                    // update reference
                    modInfo.Instance.Game = newModel;
                }

                if (modInfo.Instance is IDataModelAware aware)
                {
                    try
                    {
                        aware.OnDataModelLoaded(newModel, dataModelType);
                    }
                    catch (Exception e)
                    {
                        Console.WriteLine($"Error in IDataModelAware.OnDataModelLoaded: {e}");
                    }
                }
            }
            else
            {
                if (modInfo.Instance is IDataModelAware aware)
                {
                    try
                    {
                        aware.OnDataModelUnloaded(oldModel, dataModelType);
                    }
                    catch (Exception e)
                    {
                        Console.WriteLine($"Error in IDataModelAware.OnDataModelUnloaded: {e}");
                    }
                }

                if (modInfo.Initialized)
                {
                    try
                    {
                        modInfo.Instance.OnUnload();
                    }
                    catch (Exception e)
                    {
                        Console.WriteLine($"Error while calling OnUnload for mod: {e}");
                    }

                    modInfo.Instance.Game = null;
                    modInfo.Initialized = false;
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

    private class ModInfo
    {
        public ModInfo(AssemblyLoadContext context, IMod instance, DataModelType[]? loadIn)
        {
            Context = context;
            Instance = instance;
            LoadInDataModels = loadIn;
            Initialized = false;
        }

        public AssemblyLoadContext Context { get; }
        public IMod Instance { get; }
        public DataModelType[]? LoadInDataModels { get; }
        public bool Initialized { get; set; }
    }
}