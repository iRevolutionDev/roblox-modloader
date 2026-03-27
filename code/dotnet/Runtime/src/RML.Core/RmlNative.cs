using System.Runtime.InteropServices;

namespace RML.Core;

internal static class RmlNative
{
    private const string NativeLibrary = "roblox_modloader";

    internal static class Reflection
    {
        [DllImport(NativeLibrary, CallingConvention = CallingConvention.Cdecl, EntryPoint = "rml_reflection_invoke")]
        internal static extern ulong Invoke(
        nint instance,
        [MarshalAs(UnmanagedType.LPUTF8Str)] string functionName,
        ulong arg0,
        ulong arg1,
        ulong arg2,
        ulong arg3,
        uint argCount);

        [DllImport(NativeLibrary, CallingConvention = CallingConvention.Cdecl, EntryPoint = "rml_reflection_get_property")]
        internal static extern ulong GetProperty(
            nint instance,
            [MarshalAs(UnmanagedType.LPUTF8Str)] string propertyName);

        [DllImport(NativeLibrary, CallingConvention = CallingConvention.Cdecl, EntryPoint = "rml_reflection_set_property")]
        internal static extern ulong SetProperty(
            nint instance,
            [MarshalAs(UnmanagedType.LPUTF8Str)] string propertyName,
            ulong value);
    }

    internal static class Instance
    {
        [DllImport(NativeLibrary, CallingConvention = CallingConvention.Cdecl, EntryPoint = "rml_instance_get_class_descriptor")]
        internal static extern ulong GetClassDescriptor(nint instance);
    }
}
