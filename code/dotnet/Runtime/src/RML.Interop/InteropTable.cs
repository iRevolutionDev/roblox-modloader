using System.Runtime.InteropServices;

namespace RML.Interop;

[StructLayout(LayoutKind.Explicit, Size = 16)]
public struct InteropVariant
{
    [FieldOffset(0)] public byte Tag;

    [FieldOffset(8)] public ulong AsUInt64;
    [FieldOffset(8)] public long AsInt64;
    [FieldOffset(8)] public double AsDouble;
    [FieldOffset(8)] public float AsFloat;
    [FieldOffset(8)] public bool AsBool;
    [FieldOffset(8)] public nuint AsPointer;

    public static class Tags
    {
        public const byte Null = 0;
        public const byte Bool = 1;
        public const byte Int64 = 2;
        public const byte Double = 3;
        public const byte Float = 4;
        public const byte String = 5;
        public const byte Instance = 6;
        public const byte InstanceArray = 7;
        public const byte Blittable = 8;
    }

    public static InteropVariant FromBool(bool v) => new() { Tag = Tags.Bool, AsBool = v };
    public static InteropVariant FromInt64(long v) => new() { Tag = Tags.Int64, AsInt64 = v };
    public static InteropVariant FromDouble(double v) => new() { Tag = Tags.Double, AsDouble = v };
    public static InteropVariant FromFloat(float v) => new() { Tag = Tags.Float, AsFloat = v };
    public static InteropVariant FromPointer(nuint v) => new() { Tag = Tags.Instance, AsPointer = v };
    public static InteropVariant FromString(nuint ptr) => new() { Tag = Tags.String, AsPointer = ptr };
}

internal static unsafe class NativeInterop
{
    public const int InteropTableVersion = 3;

    [StructLayout(LayoutKind.Sequential)]
    public struct InteropTable
    {
        public uint Version;
        public uint Size;

        public delegate* unmanaged[Cdecl]<sbyte*, void*> GetProcAddress;

        public delegate* unmanaged[Cdecl]<void*, sbyte*, InteropVariant*, uint, InteropVariant*, void> ReflectionInvoke;
        public delegate* unmanaged[Cdecl]<void*, sbyte*, InteropVariant*, void> ReflectionGetProperty;
        public delegate* unmanaged[Cdecl]<void*, sbyte*, InteropVariant*, void> ReflectionSetProperty;
        
        public delegate* unmanaged[Cdecl]<void*, sbyte*, delegate* unmanaged[Cdecl]<void*, InteropVariant*, uint, void>, void*, nuint> ReflectionEventConnect;
        public delegate* unmanaged[Cdecl]<nuint, void> ReflectionEventDisconnect;

        public delegate* unmanaged[Cdecl]<void*, nuint> InstanceGetClassDescriptor;

        public delegate* unmanaged[Cdecl]<int, sbyte*, int, void> Log;
        public delegate* unmanaged[Cdecl]<sbyte*, void> FreeString;
        public delegate* unmanaged[Cdecl]<void*, void> FreeNativePtr;
    }
}