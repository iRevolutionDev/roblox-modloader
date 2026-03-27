namespace RML.Core.Api;

public interface IMod
{
    public int Initialize(ModContext context);

    public void Shutdown();
}
