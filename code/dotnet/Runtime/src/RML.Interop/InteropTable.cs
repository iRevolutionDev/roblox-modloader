using System.Runtime.InteropServices;

namespace RML.Interop;

internal static unsafe class NativeInterop
{
    public const int InteropTableVersion = 1;

    [StructLayout(LayoutKind.Sequential)]
    public struct InteropTable
    {
        public uint Version;
        public uint Size;

        public delegate* unmanaged[Cdecl]<void*, sbyte*, ulong, ulong, ulong, ulong, uint, ulong> ReflectionInvoke;
        public delegate* unmanaged[Cdecl]<void*, sbyte*, ulong> ReflectionGetProperty;
        public delegate* unmanaged[Cdecl]<void*, sbyte*, ulong, ulong> ReflectionSetProperty;
        public delegate* unmanaged[Cdecl]<void*, nuint> InstanceGetClassDescriptor;

        public delegate* unmanaged[Cdecl]<int, sbyte*, int, void> Log;
    }
}