using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Runtime.Loader;

using RML.Logging;

namespace RML.NativeHost;

internal static class NativeHost
{
    private static Assembly? _coreAssembly;
    private static AssemblyLoadContext? _coreAssemblyLoadContext;

    private static Action<string>? _loadModBinding;
    private static Action<string>? _unloadModBinding;
    private static Action<ulong, ulong, int>? _notifyDataModelChangedBinding;
    private static Action? _shutdownBinding;

    private static ILogger Logger { get; } = Log.CreateLogger("RML.NativeHost");

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    public static int Initialize(IntPtr modsRootPtr, IntPtr interopTablePtr)
    {
        try
        {
            var modsRoot = Marshal.PtrToStringUTF8(modsRootPtr) ?? throw new ArgumentNullException(nameof(modsRootPtr));

            var hostDirectory = Path.GetDirectoryName(typeof(NativeHost).Assembly.Location)!;
            var corePath = Path.Combine(hostDirectory, "RML.Core.dll");

            _coreAssemblyLoadContext = new CoreLoadContext(corePath);
            _coreAssembly = _coreAssemblyLoadContext.LoadFromAssemblyPath(corePath);

            var entryType = _coreAssembly.GetType("RML.Core.EntryPoint") ??
                            throw new InvalidOperationException("Failed to find RML.Core.EntryPoint type");

            var initializeMethod = entryType.GetMethod("Initialize", BindingFlags.Public | BindingFlags.Static) ??
                                   throw new InvalidOperationException(
                                       "Failed to find RML.Core.EntryPoint.Initialize method");

            var loadModMethod = entryType.GetMethod("LoadMod", BindingFlags.Public | BindingFlags.Static) ??
                                throw new InvalidOperationException(
                                    "Failed to find RML.Core.EntryPoint.LoadMod method");

            var unloadModMethod = entryType.GetMethod("UnloadMod", BindingFlags.Public | BindingFlags.Static) ??
                                  throw new InvalidOperationException(
                                      "Failed to find RML.Core.EntryPoint.UnloadMod method");

            var notifyDataModelChangedMethod =
                entryType.GetMethod("NotifyDataModelChanged", BindingFlags.Public | BindingFlags.Static) ??
                throw new InvalidOperationException(
                    "Failed to find RML.Core.EntryPoint.NotifyDataModelChanged method");

            var shutdownMethod = entryType.GetMethod("Shutdown", BindingFlags.Public | BindingFlags.Static) ??
                                 throw new InvalidOperationException(
                                     "Failed to find RML.Core.EntryPoint.Shutdown method");

            _loadModBinding = loadModMethod.CreateDelegate<Action<string>>();
            _unloadModBinding = unloadModMethod.CreateDelegate<Action<string>>();
            _notifyDataModelChangedBinding = notifyDataModelChangedMethod.CreateDelegate<Action<ulong, ulong, int>>();
            _shutdownBinding = shutdownMethod.CreateDelegate<Action>();

            var result = (int)initializeMethod.Invoke(null, [modsRoot, interopTablePtr])!;

            return result;
        }
        catch (Exception ex)
        {
            Logger.Error($"Initialization failed: {ex}");
            return -1;
        }
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    public static void Shutdown()
    {
        try
        {
            if (_shutdownBinding is null)
            {
                Logger.Error("Shutdown called without successful initialization");
                return;
            }

            _shutdownBinding();
        }
        catch (Exception ex)
        {
            Logger.Error($"Shutdown failed: {ex}");
        }
        finally
        {
            _coreAssemblyLoadContext?.Unload();
        }
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    public static int LoadMod(IntPtr assemblyPathPtr)
    {
        try
        {
            if (_loadModBinding is null)
            {
                Logger.Error("LoadMod called without successful initialization");
                return -1;
            }

            var assemblyPath = Marshal.PtrToStringUTF8(assemblyPathPtr) ??
                               throw new ArgumentNullException(nameof(assemblyPathPtr));

            _loadModBinding(assemblyPath);

            return 0;
        }
        catch (Exception ex)
        {
            Logger.Error($"LoadMod failed: {ex}");
            return -1;
        }
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    public static int UnloadMod(IntPtr assemblyPathPtr)
    {
        try
        {
            if (_unloadModBinding is null)
            {
                Logger.Error("UnloadMod called without successful initialization");
                return -1;
            }

            var assemblyPath = Marshal.PtrToStringUTF8(assemblyPathPtr) ??
                               throw new ArgumentNullException(nameof(assemblyPathPtr));

            _unloadModBinding(assemblyPath);

            return 0;
        }
        catch (Exception ex)
        {
            Logger.Error($"UnloadMod failed: {ex}");
            return -1;
        }
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    public static int NotifyDataModelChanged(ulong oldDataModelPtr, ulong newDataModelPtr, int dataModelType)
    {
        try
        {
            if (_notifyDataModelChangedBinding is null)
            {
                Logger.Error("NotifyDataModelChanged called without successful initialization");
                return -1;
            }

            _notifyDataModelChangedBinding(oldDataModelPtr, newDataModelPtr, dataModelType);
            return 0;
        }
        catch (Exception ex)
        {
            Logger.Error($"NotifyDataModelChanged failed: {ex}");
            return -1;
        }
    }

    private sealed class CoreLoadContext(string mainAssemblyPath) : AssemblyLoadContext("RML.Core")
    {
        private readonly AssemblyDependencyResolver _resolver = new(mainAssemblyPath);

        protected override Assembly? Load(AssemblyName assemblyName)
        {
            var path = _resolver.ResolveAssemblyToPath(assemblyName);
            return path is not null ? LoadFromAssemblyPath(path) : null;
        }

        protected override IntPtr LoadUnmanagedDll(string unmanagedDllName)
        {
            var path = _resolver.ResolveUnmanagedDllToPath(unmanagedDllName);
            return path is not null ? LoadUnmanagedDllFromPath(path) : IntPtr.Zero;
        }
    }
}