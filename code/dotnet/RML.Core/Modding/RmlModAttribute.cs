namespace RobloxModLoader.Managed.Modding;

[AttributeUsage(AttributeTargets.Class, Inherited = false)]
public sealed class RmlModAttribute : Attribute
{
    public RmlModAttribute(string id, string version)
    {
        Id = id;
        Version = version;
    }

    public string Id { get; }

    public string Version { get; }

    public string Author { get; init; } = "Unknown";

    public string Description { get; init; } = string.Empty;
}
