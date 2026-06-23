using System.Collections.Concurrent;
using System.Linq.Expressions;
using System.Reflection;

namespace Roblox;

internal static class RobloxTypeRegistry
{
    private static readonly Dictionary<string, Type> _byClassName = Build();
    private static readonly ConcurrentDictionary<Type, Func<nuint, object>> _factories = new();

    public static Type? Resolve(string className)
        => _byClassName.GetValueOrDefault(className);

    public static Object Create(nuint handle)
    {
        var className = TryGetClassName(handle);
        if (className is not null && _byClassName.TryGetValue(className, out Type? type))
        {
            return (Object)GetFactory(type)(handle);
        }

        return new Instance(handle);
    }

    public static object CreateAs(Type type, nuint handle) => GetFactory(type)(handle);

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

    private static Func<nuint, object> GetFactory(Type type)
        => _factories.GetOrAdd(type, static t =>
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

    private static Dictionary<string, Type> Build()
    {
        var map = new Dictionary<string, Type>(StringComparer.Ordinal);

        Type?[] types;
        try
        {
            types = typeof(RobloxTypeRegistry).Assembly.GetTypes();
        }
        catch (ReflectionTypeLoadException ex)
        {
            types = ex.Types;
        }

        foreach (Type? type in types)
        {
            RobloxClassAttribute? attr = type?.GetCustomAttribute<RobloxClassAttribute>();
            if (type is not null && attr is not null)
            {
                map[attr.ClassName] = type;
            }
        }

        return map;
    }
}