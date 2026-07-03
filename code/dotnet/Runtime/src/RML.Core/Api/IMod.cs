using RML.Logging;

namespace RML.Core.Api;

public interface IMod
{
    public int OnLoad();

    public void OnUnload();
}

public abstract class ModBase : IMod
{
    public ModContext Context { get; internal set; } = null!;

    protected ILogger Logger => Context.Logger;

    protected string ModDirectory => Context.Directory;

    public abstract int OnLoad();

    public abstract void OnUnload();
}