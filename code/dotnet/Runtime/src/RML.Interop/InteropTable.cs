using System.Runtime.InteropServices;

namespace RML.Interop;

[StructLayout(LayoutKind.Explicit, Size = 16)]
public readonly struct InteropVariant
{
    [FieldOffset(0)] public readonly byte Tag;

    [FieldOffset(8)] public readonly ulong AsUInt64;
    [FieldOffset(8)] public readonly long AsInt64;
    [FieldOffset(8)] public readonly double AsDouble;
    [FieldOffset(8)] public readonly float AsFloat;
    [FieldOffset(8)] public readonly bool AsBool;
    [FieldOffset(8)] public readonly nuint AsPointer;

    private InteropVariant(byte tag, ulong payload) : this()
    {
        Tag = tag;
        AsUInt64 = payload;
    }

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

    public static InteropVariant Null => new(Tags.Null, 0);
    public static InteropVariant FromBool(bool v) => new(Tags.Bool, v ? 1ul : 0ul);
    public static InteropVariant FromInt64(long v) => new(Tags.Int64, unchecked((ulong)v));
    public static InteropVariant FromDouble(double v) => new(Tags.Double, BitConverter.DoubleToUInt64Bits(v));
    public static InteropVariant FromFloat(float v) => new(Tags.Float, BitConverter.SingleToUInt32Bits(v));
    public static InteropVariant FromPointer(nuint v) => new(Tags.Instance, v);
    public static InteropVariant FromString(nuint ptr) => new(Tags.String, ptr);
    public static InteropVariant FromBlittable(nuint ptr) => new(Tags.Blittable, ptr);
}

internal static unsafe class NativeInterop
{
    public const int InteropTableVersion = 4;

    [StructLayout(LayoutKind.Sequential)]
    public struct InteropTable
    {
        public uint Version;
        public uint Size;

        public delegate* unmanaged[Cdecl]<sbyte*, void*> GetProcAddress;

        public delegate* unmanaged[Cdecl]<void*, sbyte*, InteropVariant*, uint, InteropVariant*, void> ReflectionInvoke;
        public delegate* unmanaged[Cdecl]<void*, sbyte*, InteropVariant*, void> ReflectionGetProperty;
        public delegate* unmanaged[Cdecl]<void*, sbyte*, InteropVariant*, void> ReflectionSetProperty;

        public delegate* unmanaged[Cdecl]<void*, sbyte*, delegate* unmanaged[Cdecl]<void*, InteropVariant*, uint, void>,
            void*, nuint> ReflectionEventConnect;

        public delegate* unmanaged[Cdecl]<nuint, void> ReflectionEventDisconnect;

        public delegate* unmanaged[Cdecl]<void*, nuint> InstanceGetClassDescriptor;

        public delegate* unmanaged[Cdecl]<sbyte*, int, nuint> CreateInstanceByName;

        public delegate* unmanaged[Cdecl]<int, sbyte*, int, void> Log;
        public delegate* unmanaged[Cdecl]<sbyte*, void> FreeString;
        public delegate* unmanaged[Cdecl]<void*, void> FreeNativePtr;

        public delegate* unmanaged[Cdecl]<sbyte*, delegate* unmanaged[Cdecl]<void*, InteropVariant*, uint, void>,
            void*, nuint> ModsMenuAddAction;
        public delegate* unmanaged[Cdecl]<nuint, void> ModsMenuRemoveAction;
    }
}