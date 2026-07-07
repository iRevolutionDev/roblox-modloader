using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

using RML.Interop;

using InteropApi = RML.Interop.Interop;

namespace RML.Core.Api;

public static unsafe class ModsMenu
{
    public static IModsMenuAction AddAction(string text, Action onClick)
    {
        ArgumentException.ThrowIfNullOrEmpty(text);
        ArgumentNullException.ThrowIfNull(onClick);

        var handle = GCHandle.Alloc(onClick);
        var callback = (nint)(delegate* unmanaged[Cdecl]<void*, InteropVariant*, uint, void>)&Trampoline;

        var id = InteropApi.ModsMenuAddAction(0, text, callback, GCHandle.ToIntPtr(handle));
        if (id == 0)
        {
            handle.Free();
            return NoOpAction.Instance;
        }

        return new ModsMenuActionHandle(id, handle);
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    private static void Trampoline(void* state, InteropVariant* args, uint argCount)
    {
        try
        {
            var handle = GCHandle.FromIntPtr((nint)state);
            if (handle.IsAllocated && handle.Target is Action action)
            {
                action();
            }
        }
        catch (Exception ex)
        {
            Console.Error.WriteLine($"[RML/Error] Mods menu action callback threw: {ex}");
        }
    }

    private sealed class ModsMenuActionHandle(nuint id, GCHandle handle) : IModsMenuAction
    {
        private nuint _id = id;
        private GCHandle _handle = handle;

        public void Dispose()
        {
            if (_id != 0)
            {
                InteropApi.ModsMenuRemove(_id);
                _id = 0;
            }

            if (_handle.IsAllocated)
            {
                _handle.Free();
            }
        }
    }

    private sealed class NoOpAction : IModsMenuAction
    {
        public static readonly NoOpAction Instance = new();

        public void Dispose()
        {
        }
    }
}

public interface IModsMenuAction : IDisposable
{
}
