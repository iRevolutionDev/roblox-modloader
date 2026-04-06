using Roblox;

namespace RML.Core.Api;

public interface IMod
{
    public DataModel? Game { get; set; }

    public int OnLoad();

    public void OnUnload();
}

public abstract class Mod
{
    public DataModel? Game { get; set; }
    public abstract int OnLoad();
    public abstract void OnUnload();
}