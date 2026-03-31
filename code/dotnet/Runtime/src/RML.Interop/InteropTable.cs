using System.Runtime.InteropServices;

namespace RML.Interop;

internal static unsafe class NativeInterop
{
    public const int RML_INTEROP_TABLE_VERSION = 1;

    [StructLayout(LayoutKind.Sequential)]
    public struct InteropTable
    {
        public uint Version;
        public uint Size;

        // Reflection
        public delegate* unmanaged[Cdecl]<void*, sbyte*, ulong, ulong, ulong, ulong, uint, ulong> ReflectionInvoke;
        public delegate* unmanaged[Cdecl]<void*, sbyte*, ulong> ReflectionGetProperty;
        public delegate* unmanaged[Cdecl]<void*, sbyte*, ulong, ulong> ReflectionSetProperty;

        // Instance
        public delegate* unmanaged[Cdecl]<void*, ulong> InstanceGetClassDescriptor;
        public delegate* unmanaged[Cdecl]<void*, sbyte*> InstanceGetName;
        public delegate* unmanaged[Cdecl]<void*, sbyte*> InstanceGetClassName;

        // Logging
        public delegate* unmanaged[Cdecl]<int, sbyte*, void> Log;
    }
}