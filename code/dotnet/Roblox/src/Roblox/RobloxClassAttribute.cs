using System;

namespace Roblox;

[AttributeUsage(AttributeTargets.Class, Inherited = false)]
public sealed class RobloxClassAttribute : Attribute
{
    public string ClassName { get; }

    public RobloxClassAttribute(string className)
    {
        ClassName = className ?? throw new ArgumentNullException(nameof(className));
    }
}
