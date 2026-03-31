using System.Reflection;
using System.Runtime.Loader;

using RML.Core.Api;
using RML.Core.Internal;
using RML.Core.Modding;

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
                    t.GetCustomAttribute<RmlModAttribute>() is not null && t.IsAssignableTo(typeof(IMod)) &&
                    !t.IsAbstract);

            if (modType is null)
            {
                Console.WriteLine($"No [RmlMod] class implementing IMod found in assembly {assembly.FullName}.");
                context.Unload();
                return;
            }

            var mod = (IMod)Activator.CreateInstance(modType)!;
            mod.OnLoad();
            _mods[path] = new ModInfo(context, mod);
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
            modInfo.Instance.OnUnload();
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

    private static IEnumerable<Assembly> GetCurrentAlcAssemblies()
    {
        var currentAlc = AssemblyLoadContext.GetLoadContext(
            typeof(ModLoader).Assembly);
        return currentAlc?.Assemblies ?? [];
    }

    private record ModInfo(AssemblyLoadContext Context, IMod Instance);
}