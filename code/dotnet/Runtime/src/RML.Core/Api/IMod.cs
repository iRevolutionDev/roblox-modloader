namespace RML.Core.Api;

public interface IMod
{
    public int OnLoad();

    public void OnUnload();
}