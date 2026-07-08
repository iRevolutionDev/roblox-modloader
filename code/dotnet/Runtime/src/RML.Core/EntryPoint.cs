using RML.Logging;

namespace RML.Core;

public static class EntryPoint
{
    public static readonly string AssemblyName = "RML.Core";


    public static int Initialize(string modsRoot, nint tablePtr)
    {
        Interop.Interop.Initialize(tablePtr);

        Log.AddSink(new Interop.InteropLogSink());

        ModLoader.Initialize(modsRoot);

        return 0;
    }

    public static void NotifyDataModelChanged(ulong oldDataModelPtr, ulong newDataModelPtr, int dataModelType)
    {
        ModLoader.OnDataModelChanged(oldDataModelPtr, newDataModelPtr, (Roblox.DataModelType)dataModelType);
    }

    public static void LoadMod(string dllPath)
    {
        ModLoader.LoadMod(dllPath);
    }

    public static void UnloadMod(string dllPath)
    {
        ModLoader.UnloadMod(dllPath);
    }

    public static void Shutdown()
    {
        ModLoader.Shutdown();
        Interop.Interop.Uninitialize();
    }
}