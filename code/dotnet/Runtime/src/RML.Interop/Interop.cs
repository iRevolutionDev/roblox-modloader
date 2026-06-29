using System.Collections.Concurrent;
using System.Runtime.InteropServices;
using System.Text;

using RML.Logging;

namespace RML.Interop;

public static unsafe class Interop
{
    public static bool IsInitialized => Table != null;

    internal static NativeInterop.InteropTable* Table { get; private set; }

    public static void Log(LogLevel level, string message)
    {
        if (message is null)
        {
            return;
        }

        try
        {
            if (IsInitialized && Table != null && Table->Log != null)
            {
                var bytes = Encoding.UTF8.GetBytes(message);
                fixed (byte* p = bytes)
                {
                    Table->Log((int)level, (sbyte*)p, bytes.Length);
                }

                return;
            }
        }
        catch
        {
        }

        var writer = level >= LogLevel.Warn ? Console.Error : Console.Out;
        writer.WriteLine($"[RML/{level}] {message}");
    }

    public static void Initialize(nint tablePtr)
    {
        if (tablePtr == nint.Zero)
        {
            throw new ArgumentException("Invalid interop table pointer or size.");
        }

        if (sizeof(InteropVariant) != 16)
        {
            throw new InvalidOperationException(
                $"InteropVariant ABI mismatch: managed size is {sizeof(InteropVariant)}, expected 16.");
        }

        var table = (NativeInterop.InteropTable*)tablePtr;
        if (table->Version != NativeInterop.InteropTableVersion)
        {
            throw new InvalidOperationException(
                $"Unsupported interop table version: {table->Version} (expected {NativeInterop.InteropTableVersion}).");
        }

        if (table->Size != 0 && table->Size != (uint)sizeof(NativeInterop.InteropTable))
        {
            throw new InvalidOperationException(
                $"Interop table size mismatch: native {table->Size} != managed {sizeof(NativeInterop.InteropTable)}.");
        }

        Table = table;
    }

    public static void Uninitialize()
    {
        Reflection.ClearCaches();
        Table = null;
    }

    public static void FreeNativeString(nint ptr)
    {
        if (ptr == nint.Zero || !IsInitialized || Table == null || Table->FreeString == null)
        {
            return;
        }

        Table->FreeString((sbyte*)ptr);
    }

    public static void FreeNativeArray(nint ptr)
    {
        if (ptr == nint.Zero || !IsInitialized || Table == null || Table->FreeNativePtr == null)
        {
            return;
        }

        Table->FreeNativePtr((void*)ptr);
    }

    public class Reflection
    {
        private static readonly ConcurrentDictionary<string, nint> CachedMemberNames = new(StringComparer.Ordinal);

        internal static void ClearCaches()
        {
            foreach (var ptr in CachedMemberNames.Values)
            {
                if (ptr != nint.Zero)
                {
                    Marshal.FreeHGlobal(ptr);
                }
            }

            CachedMemberNames.Clear();
        }

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

        public static InteropVariant Invoke(void* instance, string methodName, params object?[]? args)
        {
            if (!IsInitialized || Table == null || Table->ReflectionInvoke == null)
            {
                throw new InvalidOperationException(
                    "Interop table is not initialized or ReflectionInvoke is unavailable.");
            }

            ArgumentNullException.ThrowIfNull(methodName);

            var nameS = GetCachedMemberName(methodName);
            var argCount = args?.Length ?? 0;
            InteropVariant result = default;

            if (argCount == 0)
            {
                Table->ReflectionInvoke(instance, nameS, null, 0, &result);
                return result;
            }

            var tempPtrs = stackalloc nint[argCount];
            var tempPtrCount = 0;
            var argVariants = stackalloc InteropVariant[argCount];

            try
            {
                for (var i = 0; i < argCount; i++)
                {
                    argVariants[i] = BuildVariant(args![i], tempPtrs, ref tempPtrCount);
                }

                Table->ReflectionInvoke(instance, nameS, argVariants, (uint)argCount, &result);
                return result;
            }
            finally
            {
                for (var i = 0; i < tempPtrCount; i++)
                {
                    if (tempPtrs[i] != 0)
                    {
                        Marshal.FreeHGlobal(tempPtrs[i]);
                    }
                }
            }
        }

        public static InteropVariant GetProperty(void* instance, string propertyName)
        {
            if (!IsInitialized || Table == null || Table->ReflectionGetProperty == null)
            {
                throw new InvalidOperationException(
                    "Interop table is not initialized or ReflectionGetProperty is unavailable.");
            }

            ArgumentNullException.ThrowIfNull(propertyName);

            var nameS = GetCachedMemberName(propertyName);
            InteropVariant result = default;
            Table->ReflectionGetProperty(instance, nameS, &result);
            return result;
        }

        public static void SetProperty(void* instance, string propertyName, InteropVariant value)
        {
            if (!IsInitialized || Table == null || Table->ReflectionSetProperty == null)
            {
                throw new InvalidOperationException(
                    "Interop table is not initialized or ReflectionSetProperty is unavailable.");
            }

            ArgumentNullException.ThrowIfNull(propertyName);

            var nameS = GetCachedMemberName(propertyName);
            Table->ReflectionSetProperty(instance, nameS, &value);
        }

        public static nuint EventConnect(
            void* instance,
            string eventName,
            delegate* unmanaged[Cdecl]<void*, InteropVariant*, uint, void> callback,
            void* state)
        {
            if (!IsInitialized || Table == null || Table->ReflectionEventConnect == null)
            {
                throw new InvalidOperationException(
                    "Interop table is not initialized or ReflectionEventConnect is unavailable.");
            }

            ArgumentNullException.ThrowIfNull(eventName);

            var nameS = GetCachedMemberName(eventName);
            return Table->ReflectionEventConnect(instance, nameS, callback, state);
        }

        public static void EventDisconnect(nuint connectionHandle)
        {
            if (connectionHandle == 0 || !IsInitialized || Table == null || Table->ReflectionEventDisconnect == null)
            {
                return;
            }

            Table->ReflectionEventDisconnect(connectionHandle);
        }

        public static nuint CreateInstanceByName(string className, int creatorRole)
        {
            if (!IsInitialized || Table == null || Table->CreateInstanceByName == null)
            {
                throw new InvalidOperationException(
                    "Interop table is not initialized or CreateInstanceByName is unavailable.");
            }

            ArgumentNullException.ThrowIfNull(className);

            var bytes = Encoding.UTF8.GetBytes(className);
            fixed (byte* p = bytes)
            {
                return Table->CreateInstanceByName((sbyte*)p, creatorRole);
            }
        }

        private static InteropVariant BuildVariant(object? arg, nint* tempPtrs, ref int tempPtrCount)
        {
            if (arg is null)
            {
                return default;
            }

            return arg switch
            {
                string s => BuildStringVariant(s, tempPtrs, ref tempPtrCount),
                bool b => InteropVariant.FromBool(b),
                double d => InteropVariant.FromDouble(d),
                float f => InteropVariant.FromFloat(f),
                Enum e => InteropVariant.FromInt64(Convert.ToInt64(e)),
                nuint nu => InteropVariant.FromPointer(nu),
                nint ni => InteropVariant.FromPointer((nuint)ni),
                _ => BuildFallbackVariant(arg, tempPtrs, ref tempPtrCount)
            };
        }

        private static InteropVariant BuildFallbackVariant(object arg, nint* tempPtrs, ref int tempPtrCount)
        {
            var type = arg.GetType();
            if (type.IsValueType && !type.IsPrimitive && !type.IsEnum)
            {
                try
                {
                    var size = Marshal.SizeOf(arg);
                    var p = Marshal.AllocHGlobal(size);
                    Marshal.StructureToPtr(arg, p, false);
                    tempPtrs[tempPtrCount++] = p;
                    return InteropVariant.FromBlittable((nuint)p);
                }
                catch (ArgumentException)
                {
                }
            }

            return InteropVariant.FromInt64(ToInt64Fallback(arg));
        }

        private static InteropVariant BuildStringVariant(string s, nint* tempPtrs, ref int tempPtrCount)
        {
            var bytes = Encoding.UTF8.GetBytes(s);
            var p = Marshal.AllocHGlobal(bytes.Length + 1);
            Marshal.Copy(bytes, 0, p, bytes.Length);
            Marshal.WriteByte(p + bytes.Length, 0);
            tempPtrs[tempPtrCount++] = p;
            return InteropVariant.FromString((nuint)(ulong)p);
        }

        private static long ToInt64Fallback(object arg)
        {
            try
            {
                return Convert.ToInt64(arg);
            }
            catch
            {
                return 0;
            }
        }
    }
}