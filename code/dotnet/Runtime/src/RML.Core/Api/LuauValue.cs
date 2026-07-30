using System.Runtime.InteropServices;

using RML.Interop;

using InteropApi = RML.Interop.Interop;

namespace RML.Core.Api;

public readonly struct LuauValue
{
    private readonly byte _tag;
    private readonly ulong _payload;
    private readonly object? _boxed;

    private LuauValue(byte tag, ulong payload, object? boxed)
    {
        _tag = tag;
        _payload = payload;
        _boxed = boxed;
    }

    public static LuauValue Nil => default;

    public bool IsNil => _tag == InteropVariant.Tags.Null;

    public bool AsBoolean() => _tag switch
    {
        InteropVariant.Tags.Null => false,
        InteropVariant.Tags.Bool => _payload != 0,
        _ => true
    };

    public double AsNumber() => _tag switch
    {
        InteropVariant.Tags.Double => BitConverter.UInt64BitsToDouble(_payload),
        InteropVariant.Tags.Int64 => unchecked((long)_payload),
        InteropVariant.Tags.Bool => _payload != 0 ? 1d : 0d,
        _ => 0d
    };

    public string? AsString() => _boxed as string;

    public LuauRef? AsRef() => _boxed as LuauRef;

    public nuint AsInstanceHandle() => _tag == InteropVariant.Tags.Instance ? (nuint)_payload : 0;

    public T? As<T>()
    {
        var type = typeof(T);

        if (type == typeof(bool))
        {
            return (T)(object)AsBoolean();
        }

        if (type == typeof(string))
        {
            return (T?)(object?)AsString();
        }

        if (type == typeof(LuauRef))
        {
            return (T?)(object?)AsRef();
        }

        if (IsNil)
        {
            return default;
        }

        if (type == typeof(double))
        {
            return (T)(object)AsNumber();
        }

        if (type == typeof(float))
        {
            return (T)(object)(float)AsNumber();
        }

        if (type == typeof(int))
        {
            return (T)(object)(int)AsNumber();
        }

        if (type == typeof(long))
        {
            return (T)(object)(long)AsNumber();
        }

        if (type == typeof(nuint))
        {
            return (T)(object)AsInstanceHandle();
        }

        throw new InvalidCastException($"Cannot convert a Luau value with tag '{_tag}' to {type}.");
    }

    internal static LuauValue FromVariant(InteropVariant variant)
    {
        switch (variant.Tag)
        {
            case InteropVariant.Tags.Bool:
                return new LuauValue(InteropVariant.Tags.Bool, variant.AsBool ? 1ul : 0ul, null);

            case InteropVariant.Tags.Int64:
                return new LuauValue(InteropVariant.Tags.Int64, unchecked((ulong)variant.AsInt64), null);

            case InteropVariant.Tags.Double:
                return new LuauValue(InteropVariant.Tags.Double, BitConverter.DoubleToUInt64Bits(variant.AsDouble), null);

            case InteropVariant.Tags.Float:
                return new LuauValue(InteropVariant.Tags.Double, BitConverter.DoubleToUInt64Bits(variant.AsFloat), null);

            case InteropVariant.Tags.String:
                return new LuauValue(InteropVariant.Tags.String, 0, ReadOwnedString(variant));

            case InteropVariant.Tags.Instance:
                return new LuauValue(InteropVariant.Tags.Instance, variant.AsUInt64, null);

            case InteropVariant.Tags.LuauRef:
                return variant.AsUInt64 == 0
                    ? default
                    : new LuauValue(InteropVariant.Tags.LuauRef, variant.AsUInt64, new LuauRef((nuint)variant.AsUInt64));

            default:
                return default;
        }
    }

    private static string ReadOwnedString(InteropVariant variant)
    {
        var ptr = (nint)variant.AsPointer;
        if (ptr == 0)
        {
            return string.Empty;
        }

        var text = Marshal.PtrToStringUTF8(ptr) ?? string.Empty;
        InteropApi.FreeNativeString(ptr);
        return text;
    }
}
