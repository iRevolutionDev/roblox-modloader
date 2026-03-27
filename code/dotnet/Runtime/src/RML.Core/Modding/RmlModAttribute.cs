namespace RML.Core.Modding;

[AttributeUsage(AttributeTargets.Class, Inherited = false)]
public sealed class RmlModAttribute(string id, string version) : Attribute
{
    public string Id { get; } = id;

    public string Version { get; } = version;

    public string Author { get; init; } = "Unknown";

    public string Description { get; init; } = string.Empty;
}
