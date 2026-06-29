namespace Roblox;

[AttributeUsage(AttributeTargets.Class, Inherited = false)]
public sealed class RobloxClassAttribute(string className) : Attribute
{
    public string ClassName { get; } = className ?? throw new ArgumentNullException(nameof(className));
}