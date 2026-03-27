using System.Reflection;
using System.Runtime.InteropServices;
using RobloxModLoader.Managed.Api;
using RobloxModLoader.Managed.Internal;
using RobloxModLoader.Managed.Modding;

namespace RobloxModLoader.Managed.Host;

public static class ManagedRuntimeHost
{
    [UnmanagedCallersOnly]
    public static int rml_initialize(nint modsRootUtf8, nint generatedRootUtf8)
    {
        return ManagedRuntimeCore.Initialize(modsRootUtf8, generatedRootUtf8);
    }

    [UnmanagedCallersOnly]
    public static int rml_load_mod(nint assemblyPathUtf8)
    {
        return ManagedRuntimeCore.LoadMod(assemblyPathUtf8);
    }

    [UnmanagedCallersOnly]
    public static int rml_unload_mod(nint assemblyPathUtf8)
    {
        return ManagedRuntimeCore.UnloadMod(assemblyPathUtf8);
    }
}

internal static class ManagedRuntimeCore
{
    private static readonly Dictionary<string, (ManagedModLoadContext Context, IMod Mod)> Mods = new(StringComparer.OrdinalIgnoreCase);

    private static string _modsRoot = string.Empty;
    private static string _generatedRoot = string.Empty;

    private static void LogBridgeError(string phase, Exception exception, string? assemblyPath = null)
    {
        try
        {
            var logRoot = string.IsNullOrWhiteSpace(_generatedRoot)
                ? AppContext.BaseDirectory
                : _generatedRoot;

            Directory.CreateDirectory(logRoot);
            var logPath = Path.Combine(logRoot, "managed_bridge_errors.log");

            var lines = new List<string>
            {
                $"[{DateTime.UtcNow:O}] phase={phase}",
                $"assembly={assemblyPath ?? "<null>"}",
                exception.ToString(),
                string.Empty
            };

            if (exception is ReflectionTypeLoadException rtle)
            {
                lines.Add("LoaderExceptions:");
                foreach (var loaderException in rtle.LoaderExceptions)
                {
                    lines.Add(loaderException?.ToString() ?? "<null>");
                }
                lines.Add(string.Empty);
            }

            File.AppendAllLines(logPath, lines);
        }
        catch
        {
            // Best-effort diagnostics only.
        }
    }

    private static int LoadModInternal(string assemblyPath)
    {
        if (string.IsNullOrWhiteSpace(assemblyPath) || !File.Exists(assemblyPath))
        {
            return -1;
        }

        if (Mods.ContainsKey(assemblyPath))
        {
            UnloadModInternal(assemblyPath);
        }

        var context = new ManagedModLoadContext(assemblyPath);

        try
        {
            var assembly = context.LoadFromAssemblyPath(assemblyPath);
            Type[] types;
            try
            {
                types = assembly.GetTypes();
            }
            catch (Exception ex)
            {
                LogBridgeError("assembly.GetTypes", ex, assemblyPath);
                context.Unload();
                return -41;
            }

            var modType = types.FirstOrDefault(type =>
                type.GetCustomAttribute<RmlModAttribute>() is not null &&
                !type.IsAbstract &&
                typeof(IMod).IsAssignableFrom(type));

            if (modType is null)
            {
                context.Unload();
                return -2;
            }

            IMod? modInstance;
            try
            {
                modInstance = (IMod?)Activator.CreateInstance(modType);
            }
            catch (Exception ex)
            {
                LogBridgeError("Activator.CreateInstance", ex, assemblyPath);
                context.Unload();
                return -42;
            }

            if (modInstance is null)
            {
                context.Unload();
                return -3;
            }

            var modRoot = Path.GetDirectoryName(assemblyPath) ?? _modsRoot;
            var generatedDir = Path.Combine(modRoot, "..", "generated");
            generatedDir = Path.GetFullPath(generatedDir);
            Directory.CreateDirectory(generatedDir);

            int initCode;
            try
            {
                initCode = modInstance.Initialize(new ModContext(modRoot, generatedDir));
            }
            catch (Exception ex)
            {
                LogBridgeError("mod.Initialize", ex, assemblyPath);
                context.Unload();
                return -43;
            }

            if (initCode != 0)
            {
                context.Unload();
                return initCode;
            }

            Mods[assemblyPath] = (context, modInstance);
            return 0;
        }
        catch (Exception ex)
        {
            LogBridgeError("LoadModInternal", ex, assemblyPath);
            context.Unload();
            return -4;
        }
    }

    private static int UnloadModInternal(string assemblyPath)
    {
        if (string.IsNullOrWhiteSpace(assemblyPath) || !Mods.TryGetValue(assemblyPath, out var entry))
        {
            return -1;
        }

        try
        {
            entry.Mod.Shutdown();
        }
        catch
        {
            // Ignore managed mod shutdown exceptions to guarantee unload attempt.
        }

        Mods.Remove(assemblyPath);
        entry.Context.Unload();

        for (var i = 0; i < 4; i++)
        {
            GC.Collect();
            GC.WaitForPendingFinalizers();
        }

        return 0;
    }

    internal static int Initialize(nint modsRootUtf8, nint generatedRootUtf8)
    {
        _modsRoot = Marshal.PtrToStringUTF8(modsRootUtf8) ?? string.Empty;
        _generatedRoot = Marshal.PtrToStringUTF8(generatedRootUtf8) ?? string.Empty;

        Directory.CreateDirectory(_modsRoot);
        Directory.CreateDirectory(_generatedRoot);

        return 0;
    }

    internal static int LoadMod(nint assemblyPathUtf8)
    {
        var assemblyPath = Marshal.PtrToStringUTF8(assemblyPathUtf8);
        return LoadModInternal(assemblyPath ?? string.Empty);
    }

    internal static int UnloadMod(nint assemblyPathUtf8)
    {
        var assemblyPath = Marshal.PtrToStringUTF8(assemblyPathUtf8);
        return UnloadModInternal(assemblyPath ?? string.Empty);
    }
}
