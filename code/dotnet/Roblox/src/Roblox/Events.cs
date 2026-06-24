using System.Collections.Concurrent;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Runtime.Loader;

using RML.Interop;

namespace Roblox;

internal static unsafe class EventManager
{
    private static readonly ConcurrentDictionary<(nuint Instance, string Event), Subscription> Subscriptions = new();

    private static readonly object SyncRoot = new();

    private static readonly HashSet<AssemblyLoadContext> HookedContexts = new();

    public static void Add(nuint instanceHandle, string eventName, Delegate handler)
    {
        ArgumentNullException.ThrowIfNull(eventName);
        ArgumentNullException.ThrowIfNull(handler);

        if (instanceHandle == 0)
        {
            throw new ArgumentException("Instance handle is null", nameof(instanceHandle));
        }

        lock (SyncRoot)
        {
            (UIntPtr instanceHandle, string eventName) key = (instanceHandle, eventName);
            if (!Subscriptions.TryGetValue(key, out Subscription? subscription))
            {
                subscription = new Subscription(instanceHandle, eventName);
                subscription.Self = GCHandle.Alloc(subscription);

                var state = (void*)GCHandle.ToIntPtr(subscription.Self);
                subscription.NativeHandle = Interop.Reflection.EventConnect(
                    (void*)instanceHandle, eventName, &OnNativeEvent, state);

                if (subscription.NativeHandle == 0)
                {
                    if (subscription.Self.IsAllocated)
                    {
                        subscription.Self.Free();
                    }

                    throw new InvalidOperationException(
                        $"Failed to connect to native event '{eventName}'.");
                }

                Subscriptions[key] = subscription;
            }

            subscription.Handlers.Add(handler);
            TrackHandlerContext(handler);
        }
    }

    public static void Remove(nuint instanceHandle, string eventName, Delegate handler)
    {
        if (handler is null || eventName is null || instanceHandle == 0)
        {
            return;
        }

        lock (SyncRoot)
        {
            (UIntPtr instanceHandle, string eventName) key = (instanceHandle, eventName);
            if (!Subscriptions.TryGetValue(key, out Subscription? subscription))
            {
                return;
            }

            subscription.Handlers.Remove(handler);
            if (subscription.Handlers.Count != 0)
            {
                return;
            }

            Subscriptions.TryRemove(key, out _);
            Interop.Reflection.EventDisconnect(subscription.NativeHandle);
            if (subscription.Self.IsAllocated)
            {
                subscription.Self.Free();
            }
        }
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    private static void OnNativeEvent(void* state, InteropVariant* args, uint argCount)
    {
        try
        {
            var target = GCHandle.FromIntPtr((nint)state).Target;
            if (target is not Subscription subscription)
            {
                return;
            }

            Delegate[] handlers;
            lock (SyncRoot)
            {
                if (subscription.Handlers.Count == 0)
                {
                    return;
                }

                handlers = [.. subscription.Handlers];
            }

            foreach (Delegate handler in handlers)
            {
                Dispatch(handler, args, argCount);
            }
        }
        catch (Exception ex)
        {
            Console.Error.WriteLine($"[Roblox.EventManager] Event dispatch failed: {ex}");
        }
    }

    private static readonly ConcurrentDictionary<MethodInfo, Type[]> ParameterTypeCache = new();

    private static Type[] GetParameterTypes(MethodInfo method)
        => ParameterTypeCache.GetOrAdd(method, static m =>
        {
            ParameterInfo[] ps = m.GetParameters();
            var types = new Type[ps.Length];
            for (var i = 0; i < ps.Length; i++)
            {
                types[i] = ps[i].ParameterType;
            }

            return types;
        });

    private static void Dispatch(Delegate handler, InteropVariant* args, uint argCount)
    {
        try
        {
            Type[] parameterTypes = GetParameterTypes(handler.Method);
            var managedArgs = new object?[parameterTypes.Length];

            for (var i = 0; i < parameterTypes.Length; i++)
            {
                managedArgs[i] = i < argCount
                    ? Reflection.ConvertVariant(args[i], parameterTypes[i])
                    : DefaultValue(parameterTypes[i]);
            }

            handler.DynamicInvoke(managedArgs);
        }
        catch (Exception ex)
        {
            Console.Error.WriteLine($"[Roblox.EventManager] Handler invocation failed: {ex}");
        }
    }

    private static object? DefaultValue(Type type)
        => type.IsValueType ? RuntimeHelpers.GetUninitializedObject(type) : null;

    private static void TrackHandlerContext(Delegate handler)
    {
        Assembly? assembly = handler.Method.DeclaringType?.Assembly;
        if (assembly is null)
        {
            return;
        }

        AssemblyLoadContext? context = AssemblyLoadContext.GetLoadContext(assembly);

        if (context is null || !context.IsCollectible)
        {
            return;
        }

        if (HookedContexts.Add(context))
        {
            context.Unloading += OnContextUnloading;
        }
    }

    private static void OnContextUnloading(AssemblyLoadContext context)
    {
        lock (SyncRoot)
        {
            RemoveForContext(context);
            HookedContexts.Remove(context);
        }
    }

    private static void RemoveForContext(AssemblyLoadContext context)
    {
        foreach ((var key, Subscription subscription) in Subscriptions.ToArray())
        {
            subscription.Handlers.RemoveAll(h =>
            {
                Assembly? assembly = h.Method.DeclaringType?.Assembly;
                return assembly is not null && AssemblyLoadContext.GetLoadContext(assembly) == context;
            });

            if (subscription.Handlers.Count != 0)
            {
                continue;
            }

            Subscriptions.TryRemove(key, out _);
            Interop.Reflection.EventDisconnect(subscription.NativeHandle);
            if (subscription.Self.IsAllocated)
            {
                subscription.Self.Free();
            }
        }
    }

    private sealed class Subscription(nuint instanceHandle, string eventName)
    {
        public readonly string EventName = eventName;
        public readonly List<Delegate> Handlers = [];
        public readonly nuint InstanceHandle = instanceHandle;

        public nuint NativeHandle;
        public GCHandle Self;
    }
}