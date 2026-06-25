using System.Collections.Concurrent;
using System.Linq.Expressions;
using System.Reflection;

namespace Roblox;

internal static class RobloxTypeRegistry
{
    private static readonly RobloxClassManifest.Entry[] _entries = RobloxClassManifest.All();

    private static readonly Dictionary<string, RobloxClassManifest.Entry> _byClassName =
        _entries.ToDictionary(e => e.ClassName, StringComparer.Ordinal);

    private static readonly Dictionary<Type, Func<nuint, Object>> _factoriesByType =
        _entries.ToDictionary(e => e.Type, e => e.Factory);

    private static readonly ConcurrentDictionary<Type, Func<nuint, object>> _reflectionFactories = new();

    public static Type? Resolve(string className)
        => _byClassName.TryGetValue(className, out var entry) ? entry.Type : null;

    public static Object Create(nuint handle)
    {
        var className = TryGetClassName(handle);
        if (className is not null && _byClassName.TryGetValue(className, out var entry))
        {
            return entry.Factory(handle);
        }

        return new Instance(handle);
    }
    
    public static Type? ActualType(nuint handle)
    {
        var className = TryGetClassName(handle);
        return className is not null ? Resolve(className) : null;
    }

    public static object CreateAs(Type type, nuint handle)
        => _factoriesByType.TryGetValue(type, out var factory)
            ? factory(handle)
            : ReflectionFactory(type)(handle);
            
    public static T CreateAs<T>(nuint handle) where T : Object => (T)CreateAs(typeof(T), handle);

    public static T? CreateAsOrNull<T>(Instance? instance) where T : Instance
        => instance is null ? null : CreateAs<T>(instance.Handle);

    public static string ClassNameOf<T>() where T : Object => ClassNameOf(typeof(T));

    public static string ClassNameOf(Type type)
        => type.GetCustomAttribute<RobloxClassAttribute>()?.ClassName
           ?? throw new InvalidOperationException(
               $"Type '{type}' is not a generated Roblox class (missing [RobloxClass]).");

    private static string? TryGetClassName(nuint handle)
    {
        try
        {
            return Reflection.GetProperty<string>(handle, "ClassName");
        }
        catch
        {
            return null;
        }
    }

    private static Func<nuint, object> ReflectionFactory(Type type)
        => _reflectionFactories.GetOrAdd(type, static t =>
        {
            MethodInfo? fromHandle = t.GetMethod(
                "FromHandle",
                BindingFlags.Public | BindingFlags.Static,
                null,
                [typeof(nuint)],
                null);

            if (fromHandle is null)
            {
                return handle => Activator.CreateInstance(
                    t,
                    BindingFlags.Instance | BindingFlags.NonPublic | BindingFlags.Public,
                    null,
                    [handle],
                    null)!;
            }

            ParameterExpression handleParam = Expression.Parameter(typeof(nuint), "handle");
            return Expression.Lambda<Func<nuint, object>>(
                    Expression.Convert(Expression.Call(fromHandle, handleParam), typeof(object)),
                    handleParam)
                .Compile();
        });
}
