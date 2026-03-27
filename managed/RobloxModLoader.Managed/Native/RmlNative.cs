using System.Runtime.InteropServices;

namespace RobloxModLoader.Managed.Native;

internal static class RmlNative
{
    private const string NativeLibrary = "roblox_modloader";

    [DllImport(NativeLibrary, CallingConvention = CallingConvention.Cdecl, EntryPoint = "rml_reflection_invoke")]
    internal static extern ulong ReflectionInvoke(
        nint instance,
        [MarshalAs(UnmanagedType.LPUTF8Str)] string functionName,
        ulong arg0,
        ulong arg1,
        ulong arg2,
        ulong arg3,
        uint argCount);

    [DllImport(NativeLibrary, CallingConvention = CallingConvention.Cdecl, EntryPoint = "rml_reflection_get_property")]
    internal static extern ulong ReflectionGetProperty(
        nint instance,
        [MarshalAs(UnmanagedType.LPUTF8Str)] string propertyName);

    [DllImport(NativeLibrary, CallingConvention = CallingConvention.Cdecl, EntryPoint = "rml_reflection_set_property")]
    internal static extern ulong ReflectionSetProperty(
        nint instance,
        [MarshalAs(UnmanagedType.LPUTF8Str)] string propertyName,
        ulong value);

    [DllImport(NativeLibrary, CallingConvention = CallingConvention.Cdecl, EntryPoint = "rml_instance_get_class_descriptor")]
    internal static extern ulong InstanceGetClassDescriptor(nint instance);
}
