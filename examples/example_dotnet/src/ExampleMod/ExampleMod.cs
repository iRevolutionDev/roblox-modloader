using System.Runtime.InteropServices;
using RML.Core.Api;
using RML.Core.Modding;
using Roblox;

namespace ExampleMod;

[Mod(
    "dotnet-example",
    "1.0.0",
    Author = "Revolution",
    Description = "Example managed mod for RobloxModLoader"
)]
public sealed class ExampleMod : IMod, IDataModelAware
{
    public void OnDataModelLoaded(DataModel dataModel, DataModelType dataModelType)
    {
        Console.WriteLine($"[DOTNET]: DataModel loaded: {dataModelType}");
        Console.WriteLine($"[DOTNET]: DataModel root name: {dataModel.RunService?.Name}");
        // Console.WriteLine($"[DOTNET]: DataModel root name: {dataModel.ClassName}");
    }

    public void OnDataModelUnloaded(DataModel dataModel, DataModelType dataModelType)
    {
    }

    public DataModel? Game { get; set; }

    public int OnLoad()
    {
        Console.WriteLine("Hello from ExampleMod!");
        return 0;
    }

    public void OnUnload()
    {
        Console.WriteLine("Goodbye from ExampleMod!");
    }

    [DllImport("user32.dll", SetLastError = true)]
    private static extern int MessageBoxA(IntPtr hWnd, string lpText, string lpCaption, uint uType);
}