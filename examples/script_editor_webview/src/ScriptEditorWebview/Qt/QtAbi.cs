using System.Collections.Concurrent;
using System.Runtime.InteropServices;

namespace ScriptEditorWebview.Qt;

internal static unsafe class QtAbi
{
    private static readonly string[] CoreModules = ["Qt5Core.dll", "Qt5Cored.dll", "QtCore"];
    private static readonly string[] WidgetsModules = ["Qt5Widgets.dll", "Qt5Widgetsd.dll", "QtWidgets"];

    private static readonly ConcurrentDictionary<string, IntPtr> ExportCache = new(StringComparer.Ordinal);

    private static IntPtr Resolve(string[] modules, params string[] mangledCandidates)
    {
        var key = modules[0] + "::" + mangledCandidates[0];
        if (ExportCache.TryGetValue(key, out var cached)) return cached;

        var found = IntPtr.Zero;
        foreach (var module in modules)
        {
            if (!NativeLibrary.TryLoad(module, out var handle)) continue;

            foreach (var mangled in mangledCandidates)
                if (NativeLibrary.TryGetExport(handle, mangled, out var export) && export != IntPtr.Zero)
                {
                    found = export;
                    break;
                }

            if (found != IntPtr.Zero) break;
        }

        ExportCache[key] = found;
        return found;
    }

    public static List<IntPtr> AllWidgets()
    {
        var result = new List<IntPtr>();

        var ptr = Resolve(WidgetsModules, "?allWidgets@QApplication@@SA?AV?$QList@PEAVQWidget@@@@XZ");
        if (ptr == IntPtr.Zero) return result;

        var fn = (delegate* unmanaged[Cdecl]<void*, void>)ptr;
        IntPtr listData;
        fn(&listData);
        if (listData == IntPtr.Zero) return result;

        var basePtr = (byte*)listData;
        var begin = *(int*)(basePtr + 8);
        var end = *(int*)(basePtr + 12);
        var array = (void**)(basePtr + 16);

        if (end > begin)
        {
            result.Capacity = end - begin;
            for (var i = begin; i < end; i++) result.Add((IntPtr)array[i]);
        }

        return result;
    }

    public static IntPtr MetaObject(IntPtr obj)
    {
        if (obj == IntPtr.Zero) return IntPtr.Zero;

        var vtable = *(void***)obj;
        var fn = (delegate* unmanaged[Cdecl]<IntPtr, IntPtr>)vtable[0];
        return fn(obj);
    }

    public static string ClassName(IntPtr obj)
    {
        var meta = MetaObject(obj);
        if (meta == IntPtr.Zero) return string.Empty;

        var ptr = Resolve(CoreModules, "?className@QMetaObject@@QEBAPEBDXZ");
        if (ptr == IntPtr.Zero) return string.Empty;

        var fn = (delegate* unmanaged[Cdecl]<IntPtr, IntPtr>)ptr;
        var namePtr = fn(meta);
        return namePtr == IntPtr.Zero ? string.Empty : Marshal.PtrToStringAnsi(namePtr) ?? string.Empty;
    }

    public static bool Inherits(IntPtr obj, string className)
    {
        if (obj == IntPtr.Zero) return false;

        var ptr = Resolve(CoreModules, "?inherits@QObject@@QEBA_NPEBD@Z");
        if (ptr == IntPtr.Zero) return false;

        var bytes = Marshal.StringToHGlobalAnsi(className);
        try
        {
            var fn = (delegate* unmanaged[Cdecl]<IntPtr, IntPtr, byte>)ptr;
            return fn(obj, bytes) != 0;
        }
        finally
        {
            Marshal.FreeHGlobal(bytes);
        }
    }

    public static IntPtr WinId(IntPtr widget)
    {
        for (var cursor = widget; cursor != IntPtr.Zero; cursor = ParentWidget(cursor))
        {
            var direct = CallWinId(cursor,
                "?winId@QWidget@@QEAA_JXZ", "?winId@QWidget@@QEAA_KXZ",
                "?winId@QWidget@@QEBA_JXZ", "?winId@QWidget@@QEBA_KXZ");
            if (direct != IntPtr.Zero) return direct;

            var effective = CallWinId(cursor,
                "?effectiveWinId@QWidget@@QEBA_JXZ", "?effectiveWinId@QWidget@@QEBA_KXZ");
            if (effective != IntPtr.Zero) return effective;
        }

        return IntPtr.Zero;
    }

    private static IntPtr CallWinId(IntPtr widget, params string[] mangledCandidates)
    {
        var ptr = Resolve(WidgetsModules, mangledCandidates);
        if (ptr == IntPtr.Zero) return IntPtr.Zero;

        var fn = (delegate* unmanaged[Cdecl]<IntPtr, ulong>)ptr;
        return (IntPtr)fn(widget);
    }

    public static IntPtr ParentWidget(IntPtr widget)
    {
        if (widget == IntPtr.Zero) return IntPtr.Zero;

        var ptr = Resolve(WidgetsModules, "?parentWidget@QWidget@@QEBAPEAV1@XZ");
        if (ptr == IntPtr.Zero) return IntPtr.Zero;

        var fn = (delegate* unmanaged[Cdecl]<IntPtr, IntPtr>)ptr;
        return fn(widget);
    }

    public static void SetVisible(IntPtr widget, bool visible)
    {
        if (widget == IntPtr.Zero) return;

        var ptr = Resolve(WidgetsModules, "?setVisible@QWidget@@UEAAX_N@Z");
        if (ptr == IntPtr.Zero) return;

        var fn = (delegate* unmanaged[Cdecl]<IntPtr, byte, void>)ptr;
        fn(widget, visible ? (byte)1 : (byte)0);
    }
}