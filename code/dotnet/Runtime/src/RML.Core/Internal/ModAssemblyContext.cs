using System.Reflection;
using System.Runtime.Loader;

namespace RML.Core.Internal;

internal sealed class ModAssemblyContext : AssemblyLoadContext
{
    private readonly AssemblyDependencyResolver _resolver;
    private readonly Dictionary<string, Assembly> _sharedAssemblies;

    public ModAssemblyContext(string modPath, IEnumerable<Assembly> sharedAssemblies)
        : base(Path.GetFileNameWithoutExtension(modPath), true)
    {
        _resolver = new AssemblyDependencyResolver(modPath);
        _sharedAssemblies = sharedAssemblies
            .Where(a => a.GetName().Name is not null)
            .GroupBy(a => a.GetName().Name!, StringComparer.OrdinalIgnoreCase)
            .ToDictionary(g => g.Key, g => g.First(), StringComparer.OrdinalIgnoreCase);
    }

    protected override Assembly? Load(AssemblyName assemblyName)
    {
        if (assemblyName.Name is not null &&
            _sharedAssemblies.TryGetValue(assemblyName.Name, out var shared))
        {
            return shared;
        }

        var path = _resolver.ResolveAssemblyToPath(assemblyName);
        return path is not null ? LoadFromAssemblyPath(path) : null;
    }
}