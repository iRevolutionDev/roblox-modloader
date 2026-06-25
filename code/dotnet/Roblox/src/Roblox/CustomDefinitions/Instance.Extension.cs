namespace Roblox;

public partial class Instance
{
    public T? FindFirstChild<T>(string name, bool? recursive = null) where T : Instance
        => RobloxTypeRegistry.CreateAsOrNull<T>(FindFirstChild(name, recursive));

    public T? FindFirstChildOfClass<T>() where T : Instance
        => RobloxTypeRegistry.CreateAsOrNull<T>(FindFirstChildOfClass(RobloxTypeRegistry.ClassNameOf<T>()));

    public T? FindFirstChildWhichIsA<T>(bool? recursive = null) where T : Instance
        => RobloxTypeRegistry.CreateAsOrNull<T>(FindFirstChildWhichIsA(RobloxTypeRegistry.ClassNameOf<T>(), recursive));

    public T? FindFirstAncestorOfClass<T>() where T : Instance
        => RobloxTypeRegistry.CreateAsOrNull<T>(FindFirstAncestorOfClass(RobloxTypeRegistry.ClassNameOf<T>()));

    public T? FindFirstAncestorWhichIsA<T>() where T : Instance
        => RobloxTypeRegistry.CreateAsOrNull<T>(FindFirstAncestorWhichIsA(RobloxTypeRegistry.ClassNameOf<T>()));
        
    public T? WaitForChild<T>(string childName, double timeOut) where T : Instance
        => RobloxTypeRegistry.CreateAsOrNull<T>(WaitForChild(childName, timeOut));

    public T? Clone<T>() where T : Instance
        => RobloxTypeRegistry.CreateAsOrNull<T>(Clone());
}
