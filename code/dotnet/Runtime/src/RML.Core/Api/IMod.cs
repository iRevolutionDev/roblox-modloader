namespace RML.Core.Api;

public interface IMod
{
    public int OnLoad();

    public void OnUnload();
}

public abstract class ModBase : IMod
{
    public abstract int OnLoad();

    public abstract void OnUnload();
}