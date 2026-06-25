namespace Roblox;

public partial class ServiceProvider
{
    public T GetService<T>() where T : Instance
        => GetService<T>(RobloxTypeRegistry.ClassNameOf<T>());

    public T GetService<T>(string className) where T : Instance
        => RobloxTypeRegistry.CreateAs<T>(GetService(className)!.Handle);
        
    public T? FindService<T>() where T : Instance
        => RobloxTypeRegistry.CreateAsOrNull<T>(FindService(RobloxTypeRegistry.ClassNameOf<T>()));
}
