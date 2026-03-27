using System.Runtime.InteropServices;
using RobloxModLoader.Managed.Api;

namespace RobloxModLoader.Managed.Bootstrap;

public static class ManagedModBootstrap
{
    private static IMod? _current;

    public static void Register(IMod mod)
    {
        _current = mod;
    }

    public static int InitializeManaged(nint modRootUtf8, nint generatedOutputUtf8)
    {
        if (_current is null)
        {
            return -10;
        }

        var modRoot = Marshal.PtrToStringUTF8(modRootUtf8) ?? string.Empty;
        var generated = Marshal.PtrToStringUTF8(generatedOutputUtf8) ?? string.Empty;

        var context = new ModContext(modRoot, generated);
        return _current.Initialize(context);
    }

    public static void ShutdownManaged()
    {
        _current?.Shutdown();
    }
}

public static class ManagedModBootstrapExports
{
    [UnmanagedCallersOnly(EntryPoint = "Initialize")]
    public static int Initialize(nint modRootUtf8, nint generatedOutputUtf8)
        => ManagedModBootstrap.InitializeManaged(modRootUtf8, generatedOutputUtf8);

    [UnmanagedCallersOnly(EntryPoint = "Shutdown")]
    public static void Shutdown()
        => ManagedModBootstrap.ShutdownManaged();
}
