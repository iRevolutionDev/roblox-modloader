using RML.Logging;

using Roblox;

namespace GuiPixelNudge;

internal sealed class NudgeController : IDisposable
{
    private readonly ILogger _logger;
    private readonly KeyboardHook _hook;

    private Selection? _selection;

    public NudgeController(KeyboardHook hook, ILogger logger)
    {
        _hook = hook;
        _logger = logger;
        _hook.Handler = Nudge;
    }
    
    public void UseDataModel(DataModel game) => _selection = game.GetService<Selection>();

    public void ClearDataModel() => _selection = null;
    
    private bool Nudge(int dx, int dy, bool resize)
    {
        var selection = _selection;
        if (selection is null)
            return false;

        var count = 0;
        foreach (var instance in selection.Get())
        {
            if (instance.As<GuiObject>() is not { } gui)
                continue;

            if (resize)
                gui.Size = OffsetBy(gui.Size, dx, dy);
            else
                gui.Position = OffsetBy(gui.Position, dx, dy);

            count++;
        }

        if (count > 0)
            _logger.Debug($"Nudged {count} GuiObject(s) d=({dx},{dy}) resize={resize}");

        return count > 0;
    }

    private static UDim2 OffsetBy(UDim2 value, int dx, int dy)
        => new(new UDim(value.X.Scale, value.X.Offset + dx),
               new UDim(value.Y.Scale, value.Y.Offset + dy));

    public void Dispose()
    {
        if (_hook.Handler == Nudge)
            _hook.Handler = null;

        _selection = null;
    }
}
