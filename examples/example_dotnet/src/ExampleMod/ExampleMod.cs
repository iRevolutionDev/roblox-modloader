using System.Runtime.InteropServices;
using RML.Core.Api;
using RML.Core.Modding;
using RML.Logging;
using Roblox;

namespace ExampleMod;

[Mod(
    "dotnet-example",
    "1.0.0",
    Author = "Revolution",
    Description = "Example managed mod for RobloxModLoader"
)]
public sealed class ExampleMod : ModBase, IDataModelAware
{
    public static ILogger Logger { get; } = Log.CreateLogger("ExampleMod");

    public void OnDataModelLoaded(DataModel dataModel, DataModelType dataModelType)
    {
        Logger.Info($"[DOTNET]: DataModel loaded: {dataModelType}");
        var runService = dataModel.RunService;
        Logger.Info($"[DOTNET]: RunService name: {runService?.Name}");
        Logger.Info($"[DOTNET]: RunService instance: {runService?.FindFirstChild("Teste", true)}");

        var dmChildren = dataModel.GetChildren();
        Logger.Info($"[DOTNET]: DataModel name: {dataModel.Name}");
        Logger.Info($"[DOTNET]: DataModel.GetChildren count: {dmChildren.Count}");
        foreach (var service in dmChildren) Logger.Info($"[DOTNET]: Service: {service.Name}");

        Task.Run(() =>
        {
            Thread.Sleep(5000);
            var gameId = dataModel.GameId;
            var placeId = dataModel.PlaceId;

            Logger.Info($"[DOTNET]: GameId: {gameId}");
            Logger.Info($"[DOTNET]: PlaceId: {placeId}");
        });

        var workspace = dataModel.GetService<Workspace>().Cast<Workspace>();
        Logger.Info($"[DOTNET]: Workspace name: {workspace?.Name ?? "null"}");

        if (dataModelType == DataModelType.Edit)
        {
            workspace?.DescendantAdded += instance =>
            {
                Logger.Info(
                    $"[DOTNET]: Instance added: {instance.Name} {instance.Parent?.Name} ({instance.ClassName})");
            };

            workspace?.DescendantRemoving += instance =>
            {
                Logger.Info(
                    $"[DOTNET]: Instance removed: {instance.Name} {instance.Parent?.Name} ({instance.ClassName})");
            };

            Task.Run(() =>
            {
                while (true)
                {
                    Thread.Sleep(1000);
                    var camera = workspace?.CurrentCamera;
                    Logger.Info($"[DOTNET]: CurrentCamera CFrame: {camera?.CFrame}");
                }
            });
        }
    }

    public void OnDataModelUnloaded(DataModel dataModel, DataModelType dataModelType)
    {
    }

    public override int OnLoad()
    {
        Logger.Info("Hello from ExampleMod!");
        return 0;
    }

    public override void OnUnload()
    {
        Logger.Info("Goodbye from ExampleMod!");
    }

    [DllImport("user32.dll", SetLastError = true)]
    private static extern int MessageBoxA(IntPtr hWnd, string lpText, string lpCaption, uint uType);
}