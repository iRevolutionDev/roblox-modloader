using System.Diagnostics;
using System.Runtime.InteropServices;

using RML.Core.Api;
using RML.Core.Modding;
using RML.Logging;

using Roblox;

namespace GuiPixelNudge;

[Mod(
    "gui-pixel-nudge",
    "1.0.0",
    Author = "Revolution",
    Description = "Nudge selected GUI objects pixel-by-pixel with the arrow keys in Studio edit mode")]
public sealed class GuiPixelNudgeMod : ModBase, IDataModelAware
{
    private static readonly ILogger Log = RML.Logging.Log.CreateLogger("GuiPixelNudge");

    private readonly KeyboardHook _hook = new();
    private NudgeController? _controller;

    public override int OnLoad()
    {
        _controller = new NudgeController(_hook, Log);
        return 0;
    }
    
    public void OnDataModelLoaded(DataModel game, DataModelType dataModelType)
    {
        if (dataModelType != DataModelType.Edit)
            return;

        if (!_hook.IsInstalled)
        {
            var guiThreadId = ResolveGuiThreadId();
            if (!_hook.Install(guiThreadId))
                Log.Warn($"Failed to install keyboard hook on GUI thread {guiThreadId}.");
            else
                Log.Info("GUI pixel nudge armed.");
        }

        _controller?.UseDataModel(game);
    }

    public void OnDataModelUnloaded(DataModel game, DataModelType dataModelType)
    {
        if (dataModelType != DataModelType.Edit)
            return;

        _controller?.ClearDataModel();
    }

    public override void OnUnload()
    {
        _hook.Dispose();
        _controller?.Dispose();
        _controller = null;
    }

    private static uint ResolveGuiThreadId()
    {
        var hwnd = Process.GetCurrentProcess().MainWindowHandle;
        if (hwnd == 0) return GetCurrentThreadId();
        var tid = GetWindowThreadProcessId(hwnd, out _);
        return tid != 0 ? tid : GetCurrentThreadId();
    }

    [DllImport("user32.dll")]
    private static extern uint GetWindowThreadProcessId(nint hWnd, out uint lpdwProcessId);

    [DllImport("kernel32.dll")]
    private static extern uint GetCurrentThreadId();
}
