namespace RML.Core.Api;

public sealed class ModContext(string modRoot, string generatedOutputDirectory)
{
    public string ModRoot { get; } = modRoot;

    public string GeneratedOutputDirectory { get; } = generatedOutputDirectory;

    public static RmlInstance InstanceFromPointer(nint nativeInstance)
        => new(nativeInstance);
}
