namespace RobloxModLoader.Managed.Api;

[AttributeUsage(AttributeTargets.Class, Inherited = false)]
public sealed class ModMetadataAttribute(string name, string author, string version, string description = "") : Attribute
{
    public string Name { get; } = name;

    public string Author { get; } = author;

    public string Version { get; } = version;

    public string Description { get; } = description;
}
