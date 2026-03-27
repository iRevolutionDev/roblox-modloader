using System.Reflection;
using System.Runtime.Loader;
using RML.Core.Api;

namespace RML.Core.Internal;

internal sealed class ManagedModLoadContext(string assemblyPath) :
    AssemblyLoadContext(name: Path.GetFileNameWithoutExtension(assemblyPath), isCollectible: true)
{
    private readonly AssemblyDependencyResolver _resolver = new(assemblyPath);

    protected override Assembly? Load(AssemblyName assemblyName)
    {
        if (assemblyName.Name is "RobloxModLoader.Managed")
        {
            return typeof(IMod).Assembly;
        }

        var existing = Default.Assemblies
            .FirstOrDefault(assembly => string.Equals(
                assembly.GetName().Name,
                assemblyName.Name,
                StringComparison.OrdinalIgnoreCase));

        if (existing is not null)
        {
            return existing;
        }

        var resolvedPath = _resolver.ResolveAssemblyToPath(assemblyName);
        return resolvedPath is not null ? LoadFromAssemblyPath(resolvedPath) : null;
    }
}
