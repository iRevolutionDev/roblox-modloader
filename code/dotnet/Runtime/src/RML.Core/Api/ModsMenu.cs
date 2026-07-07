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

    public static IMenuNode AddSubmenu(string text)
    {
        ArgumentException.ThrowIfNullOrEmpty(text);

        var id = InteropApi.ModsMenuAddSubmenu(0, text);
        return id == 0 ? NoOpNode.Instance : new MenuNodeHandle(id);
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

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    private static void CheckableTrampoline(void* state, InteropVariant* args, uint argCount)
    {
        try
        {
            var handle = GCHandle.FromIntPtr((nint)state);
            if (handle.IsAllocated && handle.Target is Action<bool> onToggle && argCount > 0)
            {
                onToggle(args[0].AsBool);
            }
        }
        catch (Exception ex)
        {
            Console.Error.WriteLine($"[RML/Error] Mods menu checkable callback threw: {ex}");
        }
    }

    private sealed unsafe class MenuNodeHandle(nuint id) : IMenuNode
    {
        private nuint _id = id;
        private readonly List<GCHandle> _childHandles = [];

        public IMenuNode AddSubmenu(string text)
        {
            ArgumentException.ThrowIfNullOrEmpty(text);

            var subId = InteropApi.ModsMenuAddSubmenu(_id, text);
            return subId == 0 ? NoOpNode.Instance : new MenuNodeHandle(subId);
        }

        public void AddAction(string text, Action onClick)
        {
            ArgumentException.ThrowIfNullOrEmpty(text);
            ArgumentNullException.ThrowIfNull(onClick);

            var handle = GCHandle.Alloc(onClick);
            var callback = (nint)(delegate* unmanaged[Cdecl]<void*, InteropVariant*, uint, void>)&Trampoline;

            var actionId = InteropApi.ModsMenuAddAction(_id, text, callback, GCHandle.ToIntPtr(handle));
            if (actionId == 0)
            {
                handle.Free();
                return;
            }

            _childHandles.Add(handle);
        }

        public void AddSeparator()
        {
            InteropApi.ModsMenuAddSeparator(_id);
        }

        public void AddCheckable(string text, bool initial, Action<bool> onToggle)
        {
            ArgumentException.ThrowIfNullOrEmpty(text);
            ArgumentNullException.ThrowIfNull(onToggle);

            var handle = GCHandle.Alloc(onToggle);
            var callback = (nint)(delegate* unmanaged[Cdecl]<void*, InteropVariant*, uint, void>)&CheckableTrampoline;

            var actionId = InteropApi.ModsMenuAddCheckable(_id, text, initial, callback, GCHandle.ToIntPtr(handle));
            if (actionId == 0)
            {
                handle.Free();
                return;
            }

            _childHandles.Add(handle);
        }

        public void SetIcon(string path)
        {
            ArgumentException.ThrowIfNullOrEmpty(path);
            InteropApi.ModsMenuSetItemIcon(_id, path);
        }

        public void Dispose()
        {
            if (_id != 0)
            {
                InteropApi.ModsMenuRemove(_id);
                _id = 0;
            }

            foreach (var childHandle in _childHandles)
            {
                if (childHandle.IsAllocated)
                {
                    childHandle.Free();
                }
            }

            _childHandles.Clear();
        }
    }

    private sealed class NoOpNode : IMenuNode
    {
        public static readonly NoOpNode Instance = new();

        public IMenuNode AddSubmenu(string text) => Instance;

        public void AddAction(string text, Action onClick)
        {
        }

        public void AddSeparator()
        {
        }

        public void AddCheckable(string text, bool initial, Action<bool> onToggle)
        {
        }

        public void SetIcon(string path)
        {
        }

        public void Dispose()
        {
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
