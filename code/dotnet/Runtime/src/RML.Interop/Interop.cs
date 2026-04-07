using System.Collections.Concurrent;
using System.Runtime.InteropServices;
using System.Text;

namespace RML.Interop;

public static unsafe class Interop
{
    public static bool IsInitialized => Table != null;

    internal static NativeInterop.InteropTable* Table { get; private set; }

    public static void Initialize(nint tablePtr)
    {
        if (tablePtr == nint.Zero)
        {
            throw new ArgumentException("Invalid interop table pointer or size.");
        }

        var table = (NativeInterop.InteropTable*)tablePtr;
        if (table->Version != NativeInterop.InteropTableVersion)
        {
            throw new InvalidOperationException($"Unsupported interop table version: {table->Version}");
        }

        Table = table;
    }

    public static void FreeNativeString(nint ptr)
    {
        if (ptr == nint.Zero || !IsInitialized || Table == null || Table->FreeString == null)
            return;
        Table->FreeString((sbyte*)ptr);
    }

    public class Reflection
    {
        private static readonly ConcurrentDictionary<string, nint> CachedMemberNames = new(StringComparer.Ordinal);

        private static sbyte* GetCachedMemberName(string memberName)
        {
            var ptr = CachedMemberNames.GetOrAdd(memberName, static name =>
            {
                var bytes = Encoding.UTF8.GetBytes(name);
                var mem = Marshal.AllocHGlobal(bytes.Length + 1);
                Marshal.Copy(bytes, 0, mem, bytes.Length);
                Marshal.WriteByte(mem + bytes.Length, 0);
                return mem;
            });

            return (sbyte*)ptr;
        }

        public static ulong Invoke(void* instance, string methodName, params object[] args)
        {
            if (!IsInitialized || Table == null || Table->ReflectionInvoke == null)
            {
                throw new InvalidOperationException("Interop table is not initialized or reflection is unavailable.");
            }

            if (methodName is null)
            {
                throw new ArgumentNullException(nameof(methodName));
            }

            var allocated = new IntPtr[5];
            var allocCount = 0;

            try
            {
                var nameS = GetCachedMemberName(methodName);

                ulong a0 = 0, a1 = 0, a2 = 0, a3 = 0;
                uint argCount = 0;

                if (args != null)
                {
                    for (var i = 0; i < args.Length && i < 4; ++i)
                    {
                        var arg = args[i];
                        ulong val = 0;

                        if (arg == null)
                        {
                            val = 0;
                        }
                        else if (arg is string s)
                        {
                            var b = Encoding.UTF8.GetBytes(s);
                            var p = Marshal.AllocHGlobal(b.Length + 1);
                            Marshal.Copy(b, 0, p, b.Length);
                            Marshal.WriteByte(p + b.Length, 0);
                            allocated[allocCount++] = p;
                            val = (ulong)p.ToInt64();
                        }
                        else if (arg is IntPtr ip)
                        {
                            val = (ulong)ip.ToInt64();
                        }
                        else if (arg is UIntPtr uip)
                        {
                            val = uip.ToUInt64();
                        }
                        else if (arg is bool b)
                        {
                            val = b ? 1UL : 0UL;
                        }
                        else if (arg is double dd)
                        {
                            val = (ulong)BitConverter.DoubleToInt64Bits(dd);
                        }
                        else if (arg is float ff)
                        {
                            val = BitConverter.SingleToUInt32Bits(ff);
                        }
                        else if (arg is Enum)
                        {
                            val = Convert.ToUInt64(arg);
                        }
                        else
                        {
                            try
                            {
                                val = Convert.ToUInt64(arg);
                            }
                            catch
                            {
                                if (arg is nint ni)
                                {
                                    val = (ulong)ni;
                                }
                                else if (arg is nuint nui)
                                {
                                    val = nui;
                                }
                                else
                                {
                                    val = 0;
                                }
                            }
                        }

                        switch (i)
                        {
                            case 0: a0 = val; break;
                            case 1: a1 = val; break;
                            case 2: a2 = val; break;
                            case 3: a3 = val; break;
                        }

                        argCount++;
                    }
                }

                var fn = Table->ReflectionInvoke;
                if (fn == null)
                {
                    throw new InvalidOperationException("ReflectionInvoke function pointer is null.");
                }

                var res = fn(instance, nameS, a0, a1, a2, a3, argCount);
                return res;
            }
            finally
            {
                for (var i = 0; i < allocCount; ++i)
                {
                    if (allocated[i] != IntPtr.Zero)
                    {
                        Marshal.FreeHGlobal(allocated[i]);
                    }
                }
            }
        }

        public static ulong GetProperty(void* instance, string propertyName)
        {
            if (!IsInitialized || Table == null || Table->ReflectionGetProperty == null)
            {
                throw new InvalidOperationException("Interop table is not initialized or reflection is unavailable.");
            }

            if (propertyName is null)
            {
                throw new ArgumentNullException(nameof(propertyName));
            }

            var nameS = GetCachedMemberName(propertyName);

            var fn = Table->ReflectionGetProperty;
            if (fn == null)
            {
                throw new InvalidOperationException("ReflectionGetProperty function pointer is null.");
            }

            return fn(instance, nameS);
        }

        public static ulong SetProperty(void* instance, string propertyName, ulong value)
        {
            if (!IsInitialized || Table == null || Table->ReflectionSetProperty == null)
            {
                throw new InvalidOperationException("Interop table is not initialized or reflection is unavailable.");
            }

            if (propertyName is null)
            {
                throw new ArgumentNullException(nameof(propertyName));
            }

            var nameS = GetCachedMemberName(propertyName);

            var fn = Table->ReflectionSetProperty;
            if (fn == null)
            {
                throw new InvalidOperationException("ReflectionSetProperty function pointer is null.");
            }

            return fn(instance, nameS, value);
        }
    }
}