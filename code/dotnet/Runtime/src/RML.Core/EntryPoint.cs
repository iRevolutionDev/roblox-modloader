namespace RML.Core;

public static class EntryPoint
{
    public static readonly string AssemblyName = "RML.Core";


    public static int Initialize(string modsRoot, nint tablePtr)
    {
        Interop.Interop.Initialize(tablePtr);
        
        ModLoader.Initialize(modsRoot);
        
        return 0;
    }

    public static void LoadMod(string dllPath)
    {
        ModLoader.LoadMod(dllPath);
    }

    public static void UnloadMod(string dllPath)
    {
        ModLoader.UnloadMod(dllPath);
    }

    public static void Shutdown() => ModLoader.Shutdown();
}