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

        var res = Interop.Reflection.Invoke((void*)handle, methodName, args);
        return ConvertResult<T>(res);
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

        var res = Interop.Reflection.GetProperty((void*)handle, propertyName);
        return ConvertResult<T>(res);
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

        var strPtr = IntPtr.Zero;
        try
        {
            ulong nativeVal;

            if (value is null)
            {
                nativeVal = 0;
            }
            else
            {
                switch (value)
                {
                    case string s:
                    {
                        var b = Encoding.UTF8.GetBytes(s);
                        strPtr = Marshal.AllocHGlobal(b.Length + 1);
                        Marshal.Copy(b, 0, strPtr, b.Length);
                        Marshal.WriteByte(strPtr + b.Length, 0);
                        nativeVal = (ulong)strPtr.ToInt64();
                        break;
                    }
                    case Object inst:
                        nativeVal = inst.Handle;
                        break;
                    case IntPtr ip:
                        nativeVal = (ulong)ip.ToInt64();
                        break;
                    case UIntPtr uip:
                        nativeVal = uip.ToUInt64();
                        break;
                    case bool bb:
                        nativeVal = bb ? 1UL : 0UL;
                        break;
                    case double dd:
                        nativeVal = (ulong)BitConverter.DoubleToInt64Bits(dd);
                        break;
                    case float ff:
                        nativeVal = BitConverter.SingleToUInt32Bits(ff);
                        break;
                    case System.Enum:
                        nativeVal = Convert.ToUInt64(value);
                        break;
                    default:
                        try
                        {
                            nativeVal = Convert.ToUInt64(value);
                        }
                        catch
                        {
                            nativeVal = value switch
                            {
                                nint ni => (ulong)ni,
                                nuint nui => nui,
                                _ => 0
                            };
                        }

                        break;
                }
            }

            Interop.Reflection.SetProperty((void*)handle, propertyName, nativeVal);
        }
        catch (Exception e)
        {
        }
    }

    private static T? ConvertResult<T>(ulong res)
    {
        Type t = typeof(T);

        if (t == typeof(string))
        {
            if (res == 0)
            {
                return default!;
            }

            var ptr = new IntPtr((long)res);
            var s = Marshal.PtrToStringUTF8(ptr);
            Interop.FreeNativeString(ptr);
            return (T)((object?)s)!;
        }

        if (typeof(Object).IsAssignableFrom(t))
        {
            if (res == 0)
            {
                return default!;
            }

            // If T is exactly Object, wrap it generically.
            // If T is a generated subclass with a static FromHandle(nuint) factory, call it.
            var handle = (nuint)res;
            if (t == typeof(Object))
            {
                var inst = new Object(handle);
                return (T)(object)inst;
            }

            // Try to call the generated static FromHandle(nuint) factory on the target type.
            var fromHandle = t.GetMethod("FromHandle",
                System.Reflection.BindingFlags.Public | System.Reflection.BindingFlags.Static,
                null,
                new[] { typeof(nuint) },
                null);

            if (fromHandle != null)
            {
                var result = fromHandle.Invoke(null, new object[] { handle });
                return result is null ? default! : (T)result;
            }

            // Fallback: wrap as generic Object and cast.
            return (T)(object)new Object(handle);
        }

        if (t == typeof(bool))
        {
            return (T)(object)(res != 0);
        }

        if (t == typeof(double))
        {
            var bits = (long)res;
            var d = BitConverter.Int64BitsToDouble(bits);
            return (T)(object)d;
        }

        if (t == typeof(float))
        {
            var u = (uint)res;
            var f = BitConverter.Int32BitsToSingle((int)u);
            return (T)(object)f;
        }

        if (t.IsEnum)
        {
            var underlying = Convert.ChangeType(res, System.Enum.GetUnderlyingType(t));
            return (T)System.Enum.ToObject(t, underlying);
        }

        if (t == typeof(object))
        {
            return (T)(object)res;
        }

        if (t == typeof(byte) || t == typeof(sbyte) || t == typeof(short) || t == typeof(ushort) ||
            t == typeof(int) || t == typeof(uint) || t == typeof(long) || t == typeof(ulong) ||
            t == typeof(nint) || t == typeof(nuint))
        {
            object boxed = res;
            return (T)Convert.ChangeType(boxed, t);
        }

        Type? nullableUnderlying = Nullable.GetUnderlyingType(t);
        if (nullableUnderlying != null)
        {
            var boxed = Convert.ChangeType(res, nullableUnderlying);
            return (T)boxed;
        }

        try
        {
            return (T)Convert.ChangeType(res, t);
        }
        catch (Exception ex)
        {
            throw new InvalidCastException($"Cannot convert native result {res} to {t}", ex);
        }
    }
}