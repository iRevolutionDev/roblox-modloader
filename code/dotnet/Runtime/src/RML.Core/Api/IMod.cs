using RML.Logging;

using Roblox;

namespace RML.Core.Api;

public interface IMod
{
    public DataModel? Game { get; set; }

    public int OnLoad();

    public void OnUnload();
}

public abstract class Mod : IMod
{
    public DataModel? Game { get; set; }

    private ILogger? _logger;

    protected ILogger Logger => _logger ??= Log.CreateLogger(GetType().Name);

    public abstract int OnLoad();

    public abstract void OnUnload();
}