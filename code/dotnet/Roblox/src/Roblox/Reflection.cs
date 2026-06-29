using System.Runtime.InteropServices;
using System.Text;

using RML.Interop;

namespace Roblox;

public static unsafe class Reflection
{
    public static T? Invoke<T>(Object @object, string methodName, params object?[] args)
    {
        ArgumentNullException.ThrowIfNull(@object);

        return Invoke<T>(@object.Handle, methodName, args);
    }

    public static T? Invoke<T>(nuint handle, string methodName, params object?[] args)
    {
        ArgumentNullException.ThrowIfNull(methodName);

        if (handle == 0)
        {
            throw new ArgumentException("Instance handle is null", nameof(handle));
        }

        InteropVariant variant = Interop.Reflection.Invoke((void*)handle, methodName, args);
        return ConvertResult<T>(variant);
    }

    public static T? GetProperty<T>(Object @object, string propertyName)
    {
        ArgumentNullException.ThrowIfNull(@object);

        return GetProperty<T>(@object.Handle, propertyName);
    }

    public static T? GetProperty<T>(nuint handle, string propertyName)
    {
        ArgumentNullException.ThrowIfNull(propertyName);
        if (handle == 0)
        {
            throw new ArgumentException("Instance handle is null", nameof(handle));
        }

        InteropVariant variant = Interop.Reflection.GetProperty((void*)handle, propertyName);
        return ConvertResult<T>(variant);
    }

    public static void SetProperty<T>(Object @object, string propertyName, T value)
    {
        ArgumentNullException.ThrowIfNull(@object);

        SetProperty(@object.Handle, propertyName, value);
    }

    public static void SetProperty<T>(nuint handle, string propertyName, T value)
    {
        ArgumentNullException.ThrowIfNull(propertyName);

        if (handle == 0)
        {
            throw new ArgumentException("Instance handle is null", nameof(handle));
        }

        if (value is null)
        {
            Interop.Reflection.SetProperty((void*)handle, propertyName, default);
            return;
        }

        if (value is string s)
        {
            var b = Encoding.UTF8.GetBytes(s);
            var strPtr = Marshal.AllocHGlobal(b.Length + 1);
            try
            {
                Marshal.Copy(b, 0, strPtr, b.Length);
                Marshal.WriteByte(strPtr + b.Length, 0);
                Interop.Reflection.SetProperty((void*)handle, propertyName,
                    InteropVariant.FromString((nuint)(ulong)strPtr));
            }
            finally
            {
                Marshal.FreeHGlobal(strPtr);
            }

            return;
        }

        if (value is NumberSequence ns)
        {
            WriteSequence(handle, propertyName, ns.Keypoints);
            return;
        }

        if (value is ColorSequence cs)
        {
            WriteSequence(handle, propertyName, cs.Keypoints);
            return;
        }

        if (value is IRobloxDataType)
        {
            var size = Marshal.SizeOf((object)value);
            var buffer = Marshal.AllocHGlobal(size);
            try
            {
                Marshal.StructureToPtr((object)value, buffer, false);
                Interop.Reflection.SetProperty((void*)handle, propertyName,
                    InteropVariant.FromBlittable((nuint)buffer));
            }
            finally
            {
                Marshal.FreeHGlobal(buffer);
            }

            return;
        }

        InteropVariant variantValue = value switch
        {
            Object inst => InteropVariant.FromPointer(inst.Handle),
            bool bb => InteropVariant.FromBool(bb),
            double dd => InteropVariant.FromDouble(dd),
            float ff => InteropVariant.FromFloat(ff),
            System.Enum => InteropVariant.FromInt64(Convert.ToInt64(value)),
            nuint nu => InteropVariant.FromPointer(nu),
            nint ni => InteropVariant.FromPointer((nuint)ni),
            _ => InteropVariant.FromInt64(ToInt64OrThrow(value, propertyName))
        };

        Interop.Reflection.SetProperty((void*)handle, propertyName, variantValue);
    }

    public static T CreateInstance<T>() where T : Instance
    {
        var className = RobloxTypeRegistry.ClassNameOf<T>();
        var handle = Interop.Reflection.CreateInstanceByName(className, 3);
        return handle == 0
            ? throw new InvalidOperationException($"Failed to create instance of type '{className}'")
            : RobloxTypeRegistry.CreateAs<T>(handle);
    }

    public static nuint CreateInstance(string className)
    {
        var handle = Interop.Reflection.CreateInstanceByName(className, 3);
        return handle == 0
            ? throw new InvalidOperationException($"Failed to create instance of type '{className}'")
            : handle;
    }

    private static long ToInt64OrThrow(object value, string propertyName)
    {
        try
        {
            return Convert.ToInt64(value);
        }
        catch (Exception ex)
        {
            throw new InvalidCastException(
                $"Cannot marshal value of type '{value.GetType()}' for property '{propertyName}'.", ex);
        }
    }

    private static T? ConvertResult<T>(InteropVariant variant)
    {
        var converted = ConvertResult(variant, typeof(T), true);
        return converted is null ? default : (T)converted;
    }

    // Variable-length datatype wire format: [int32 count][keypoint_0 ... keypoint_{count-1}], each
    // keypoint a fixed blittable struct. Mirrors the native std::vector<Keypoint> packing.
    private static NumberSequence ReadNumberSequence(nint ptr)
    {
        NumberSequenceKeypoint[] keys = ReadKeypoints<NumberSequenceKeypoint>(ptr);
        return NumberSequence.FromEngine(keys);
    }

    private static ColorSequence ReadColorSequence(nint ptr)
    {
        ColorSequenceKeypoint[] keys = ReadKeypoints<ColorSequenceKeypoint>(ptr);
        return ColorSequence.FromEngine(keys);
    }

    private static T[] ReadKeypoints<T>(nint ptr) where T : struct
    {
        var count = Marshal.ReadInt32(ptr);
        if (count <= 0)
        {
            return [];
        }

        var stride = Marshal.SizeOf<T>();
        var elems = ptr + sizeof(int);
        var keys = new T[count];
        for (var i = 0; i < count; i++)
        {
            keys[i] = Marshal.PtrToStructure<T>(elems + i * stride);
        }

        return keys;
    }

    private static void WriteSequence<T>(nuint handle, string propertyName, IReadOnlyList<T> keypoints)
        where T : struct
    {
        var stride = Marshal.SizeOf<T>();
        var size = sizeof(int) + keypoints.Count * stride;
        var buffer = Marshal.AllocHGlobal(size);
        try
        {
            Marshal.WriteInt32(buffer, keypoints.Count);
            var elems = buffer + sizeof(int);
            for (var i = 0; i < keypoints.Count; i++)
            {
                Marshal.StructureToPtr(keypoints[i], elems + i * stride, false);
            }

            Interop.Reflection.SetProperty((void*)handle, propertyName,
                InteropVariant.FromBlittable((nuint)buffer));
        }
        finally
        {
            Marshal.FreeHGlobal(buffer);
        }
    }

    internal static object? ConvertVariant(InteropVariant variant, Type targetType)
        => ConvertResult(variant, targetType, false);

    internal static object? ConvertResult(InteropVariant variant, Type t, bool freeNativeResources)
    {
        while (true)
        {
            if (variant.Tag == InteropVariant.Tags.Null)
            {
                return null;
            }

            if (variant.Tag == InteropVariant.Tags.Blittable)
            {
                if (variant.AsPointer == 0)
                {
                    return null;
                }

                var blittablePtr = (nint)variant.AsPointer;
                Type underlying = Nullable.GetUnderlyingType(t) ?? t;

                // Variable-length datatypes arrive as [int32 count][keypoint...]; fixed blittable
                // value types are a straight struct copy.
                object? value;
                if (underlying == typeof(NumberSequence))
                {
                    value = ReadNumberSequence(blittablePtr);
                }
                else if (underlying == typeof(ColorSequence))
                {
                    value = ReadColorSequence(blittablePtr);
                }
                else
                {
                    value = Marshal.PtrToStructure(blittablePtr, underlying);
                }

                if (freeNativeResources)
                {
                    Interop.FreeNativeArray(blittablePtr);
                }

                return value;
            }

            if (t == typeof(string))
            {
                if (variant.Tag != InteropVariant.Tags.String)
                {
                    return null;
                }

                var ptr = new IntPtr((long)variant.AsPointer);
                if (ptr == IntPtr.Zero)
                {
                    return null;
                }

                var s = Marshal.PtrToStringUTF8(ptr);
                if (freeNativeResources)
                {
                    Interop.FreeNativeString(ptr);
                }

                return s;
            }

            if (variant.Tag == InteropVariant.Tags.InstanceArray)
            {
                if (variant.AsPointer == 0)
                {
                    return null;
                }

                var buf = (byte*)variant.AsPointer;
                var count = *(uint*)buf;
                var handles = (nuint*)(buf + sizeof(ulong));

                var list = new List<Instance>((int)count);
                for (var i = 0u; i < count; i++)
                {
                    var handle = handles[i];
                    if (handle == 0)
                    {
                        continue;
                    }

                    list.Add((Instance)RobloxTypeRegistry.Create(handle));
                }

                if (freeNativeResources)
                {
                    Interop.FreeNativeArray((nint)variant.AsPointer);
                }

                if (t == typeof(Instance[]) || (t.IsArray && t.GetElementType() == typeof(Instance)))
                {
                    return list.ToArray();
                }

                if (t.IsAssignableFrom(typeof(List<Instance>)))
                {
                    return list;
                }

                return null;
            }

            if (typeof(Object).IsAssignableFrom(t))
            {
                if (variant.Tag != InteropVariant.Tags.Instance)
                {
                    return null;
                }

                var handle = variant.AsPointer;
                if (handle == 0)
                {
                    return null;
                }

                Object instance = RobloxTypeRegistry.Create(handle);
                return t.IsInstanceOfType(instance) ? instance : RobloxTypeRegistry.CreateAs(t, handle);
            }

            if (t == typeof(bool))
            {
                return variant.Tag == InteropVariant.Tags.Bool
                    ? variant.AsBool
                    : variant.AsUInt64 != 0;
            }

            if (t == typeof(double))
            {
                if (variant.Tag == InteropVariant.Tags.Double)
                {
                    return variant.AsDouble;
                }

                if (variant.Tag == InteropVariant.Tags.Float)
                {
                    return (double)variant.AsFloat;
                }

                return BitConverter.Int64BitsToDouble(variant.AsInt64);
            }

            if (t == typeof(float))
            {
                if (variant.Tag == InteropVariant.Tags.Float)
                {
                    return variant.AsFloat;
                }

                return BitConverter.Int32BitsToSingle((int)(uint)variant.AsUInt64);
            }

            if (t.IsEnum)
            {
                var underlying = Convert.ChangeType(variant.AsUInt64, System.Enum.GetUnderlyingType(t));
                return System.Enum.ToObject(t, underlying);
            }

            if (t == typeof(object))
            {
                return variant.AsUInt64;
            }

            if (t == typeof(byte) || t == typeof(sbyte) || t == typeof(short) || t == typeof(ushort) ||
                t == typeof(int) || t == typeof(uint) || t == typeof(long) || t == typeof(ulong) || t == typeof(nint) ||
                t == typeof(nuint))
            {
                return Convert.ChangeType(variant.AsUInt64, t);
            }

            Type? nullableUnderlying = Nullable.GetUnderlyingType(t);
            if (nullableUnderlying != null)
            {
                t = nullableUnderlying;
                continue;
            }

            try
            {
                return Convert.ChangeType(variant.AsUInt64, t);
            }
            catch (Exception ex)
            {
                throw new InvalidCastException($"Cannot convert native result to {t}", ex);
            }
        }
    }
}