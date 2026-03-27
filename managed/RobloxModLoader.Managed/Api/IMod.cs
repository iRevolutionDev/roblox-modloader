namespace RobloxModLoader.Managed.Api;

public interface IMod
{
    int Initialize(ModContext context);

    void Shutdown();
}
