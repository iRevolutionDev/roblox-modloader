using Roblox;

namespace RML.Core.Modding;

[AttributeUsage(AttributeTargets.Class, Inherited = false)]
public sealed class ModAttribute(string id, string version) : Attribute
{
    public string Id { get; } = id;

    public string Version { get; } = version;

    public string Author { get; init; } = "Unknown";

    public string Description { get; init; } = string.Empty;

    public DataModelType[]? LoadInDataModels { get; init; }

    public bool DeferInitializeUntilDataModel { get; init; } = false;
}