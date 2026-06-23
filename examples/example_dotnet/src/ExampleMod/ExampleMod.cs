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
        var runService = dataModel.RunService;
        Console.WriteLine($"[DOTNET]: RunService name: {runService?.Name}");
        Console.WriteLine($"[DOTNET]: RunService instance: {runService?.FindFirstChild("Teste", true)}");

        var dmChildren = dataModel.GetChildren();
        Console.WriteLine($"[DOTNET]: DataModel name: {dataModel.Name}");
        Console.WriteLine($"[DOTNET]: DataModel.GetChildren count: {dmChildren.Count}");
        foreach (var service in dmChildren) Console.WriteLine($"[DOTNET]: Service: {service.Name}");

        Task.Run(() =>
        {
            Thread.Sleep(5000);
            var gameId = dataModel.GameId;
            var placeId = dataModel.PlaceId;

            Console.WriteLine($"[DOTNET]: GameId: {gameId}");
            Console.WriteLine($"[DOTNET]: PlaceId: {placeId}");
        });

        var workspace = dataModel.GetService("Workspace");
        Console.WriteLine($"[DOTNET]: Workspace name: {workspace?.Name ?? "null"}");

        if (dataModelType == DataModelType.Edit)
        {
            dataModel.Workspace.DescendantAdded += instance =>
            {
                Console.WriteLine(
                    $"[DOTNET]: Instance added: {instance.Name} {instance.Parent?.Name} ({instance.ClassName})");
            };

            dataModel.Workspace.DescendantRemoving += instance =>
            {
                Console.WriteLine(
                    $"[DOTNET]: Instance removed: {instance.Name} {instance.Parent?.Name} ({instance.ClassName})");
            };
        }
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