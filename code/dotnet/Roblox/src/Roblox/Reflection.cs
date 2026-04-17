using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;

using RML.Interop;

namespace Roblox;

public static unsafe class Reflection
{
    public static T? Invoke<T>(Object @object, string methodName, params object[] args)
    {
        ArgumentNullException.ThrowIfNull(@object);

        return Invoke<T>(@object.Handle, methodName, args);
    }

    public static T? Invoke<T>(nuint handle, string methodName, params object[] args)
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

        try
        {
            InteropVariant variantValue;

            if (value is null)
            {
                variantValue = default;
            }
            else
            {
                switch (value)
                {
                    case string s:
                    {
                        var b = Encoding.UTF8.GetBytes(s);
                        var strPtr = Marshal.AllocHGlobal(b.Length + 1);
                        Marshal.Copy(b, 0, strPtr, b.Length);
                        Marshal.WriteByte(strPtr + b.Length, 0);
                        variantValue = InteropVariant.FromString((nuint)(ulong)strPtr);
                        Interop.Reflection.SetProperty((void*)handle, propertyName, variantValue);
                        Marshal.FreeHGlobal(strPtr);
                        return;
                    }
                    case Object inst:
                        variantValue = InteropVariant.FromPointer(inst.Handle);
                        break;
                    case bool bb:
                        variantValue = InteropVariant.FromBool(bb);
                        break;
                    case double dd:
                        variantValue = InteropVariant.FromDouble(dd);
                        break;
                    case float ff:
                        variantValue = InteropVariant.FromFloat(ff);
                        break;
                    case System.Enum:
                        variantValue = InteropVariant.FromInt64(Convert.ToInt64(value));
                        break;
                    case nuint nu:
                        variantValue = InteropVariant.FromPointer(nu);
                        break;
                    case nint ni:
                        variantValue = InteropVariant.FromPointer((nuint)ni);
                        break;
                    default:
                        long raw;
                        try
                        {
                            raw = Convert.ToInt64(value);
                        }
                        catch
                        {
                            raw = 0;
                        }

                        variantValue = InteropVariant.FromInt64(raw);
                        break;
                }
            }

            Interop.Reflection.SetProperty((void*)handle, propertyName, variantValue);
        }
        catch (Exception)
        {
        }
    }

    private static T? ConvertResult<T>(InteropVariant variant)
    {
        Type t = typeof(T);

        if (variant.Tag == InteropVariant.Tags.Null)
        {
            return default;
        }

        if (t == typeof(string))
        {
            if (variant.Tag != InteropVariant.Tags.String)
            {
                return default;
            }

            var ptr = new IntPtr((long)variant.AsPointer);
            if (ptr == IntPtr.Zero)
            {
                return default;
            }

            var s = Marshal.PtrToStringUTF8(ptr);
            Interop.FreeNativeString(ptr);
            return (T)((object?)s)!;
        }

        if (typeof(Object).IsAssignableFrom(t))
        {
            if (variant.Tag != InteropVariant.Tags.Instance)
            {
                return default;
            }

            var handle = variant.AsPointer;
            if (handle == 0)
            {
                return default;
            }

            if (t == typeof(Object))
            {
                return (T)(object)new Object(handle);
            }

            MethodInfo? fromHandle = t.GetMethod("FromHandle",
                BindingFlags.Public | BindingFlags.Static,
                null,
                new[] { typeof(nuint) },
                null);

            if (fromHandle != null)
            {
                var result = fromHandle.Invoke(null, new object[] { handle });
                return result is null ? default! : (T)result;
            }

            return (T)(object)new Object(handle);
        }

        if (t == typeof(bool))
        {
            return variant.Tag == InteropVariant.Tags.Bool
                ? (T)(object)variant.AsBool
                : (T)(object)(variant.AsUInt64 != 0);
        }

        if (t == typeof(double))
        {
            if (variant.Tag == InteropVariant.Tags.Double)
            {
                return (T)(object)variant.AsDouble;
            }

            if (variant.Tag == InteropVariant.Tags.Float)
            {
                return (T)(object)(double)variant.AsFloat;
            }

            return (T)(object)BitConverter.Int64BitsToDouble(variant.AsInt64);
        }

        if (t == typeof(float))
        {
            if (variant.Tag == InteropVariant.Tags.Float)
            {
                return (T)(object)variant.AsFloat;
            }

            return (T)(object)BitConverter.Int32BitsToSingle((int)(uint)variant.AsUInt64);
        }

        if (t.IsEnum)
        {
            var underlying = Convert.ChangeType(variant.AsUInt64, System.Enum.GetUnderlyingType(t));
            return (T)System.Enum.ToObject(t, underlying);
        }

        if (t == typeof(object))
        {
            return (T)(object)variant.AsUInt64;
        }

        if (t == typeof(byte) || t == typeof(sbyte) || t == typeof(short) || t == typeof(ushort) ||
            t == typeof(int) || t == typeof(uint) || t == typeof(long) || t == typeof(ulong) ||
            t == typeof(nint) || t == typeof(nuint))
        {
            object boxed = variant.AsUInt64;
            return (T)Convert.ChangeType(boxed, t);
        }

        Type? nullableUnderlying = Nullable.GetUnderlyingType(t);
        if (nullableUnderlying != null)
        {
            var boxed = Convert.ChangeType(variant.AsUInt64, nullableUnderlying);
            return (T)boxed;
        }

        try
        {
            return (T)Convert.ChangeType(variant.AsUInt64, t);
        }
        catch (Exception ex)
        {
            throw new InvalidCastException($"Cannot convert native result to {t}", ex);
        }
    }
}