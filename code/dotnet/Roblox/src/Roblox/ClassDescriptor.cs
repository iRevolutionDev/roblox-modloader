namespace Roblox;

public sealed class ClassDescriptor
{
    public ClassDescriptor(nint nativeHandle, string className)
    {
        NativeHandle = nativeHandle;
        ClassName = className ?? throw new ArgumentNullException(nameof(className));
    }

    public nint NativeHandle { get; }

    public string ClassName { get; }
}