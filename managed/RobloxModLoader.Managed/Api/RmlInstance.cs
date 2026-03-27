using RobloxModLoader.Managed.Native;

namespace RobloxModLoader.Managed.Api;

public readonly struct RmlInstance(nint nativeHandle)
{
    public nint NativeHandle { get; } = nativeHandle;

    public bool IsNull => NativeHandle == nint.Zero;

    public ulong Invoke(string functionName, params ulong[] args)
    {
        if (args.Length > 4)
        {
            throw new ArgumentOutOfRangeException(nameof(args), "Only up to 4 arguments are currently supported by the native bridge.");
        }

        var arg0 = args.Length > 0 ? args[0] : 0UL;
        var arg1 = args.Length > 1 ? args[1] : 0UL;
        var arg2 = args.Length > 2 ? args[2] : 0UL;
        var arg3 = args.Length > 3 ? args[3] : 0UL;

        return RmlNative.ReflectionInvoke(NativeHandle, functionName, arg0, arg1, arg2, arg3, (uint)args.Length);
    }

    public ulong GetProperty(string propertyName)
        => RmlNative.ReflectionGetProperty(NativeHandle, propertyName);

    public ulong SetProperty(string propertyName, ulong value)
        => RmlNative.ReflectionSetProperty(NativeHandle, propertyName, value);

    public ulong GetClassDescriptorAddress()
        => RmlNative.InstanceGetClassDescriptor(NativeHandle);
}
