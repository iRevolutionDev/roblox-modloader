using TypeGenerator.APITypes;

namespace TypeGenerator;

internal readonly record struct PropertyAccess(bool CanRead, bool CanWrite)
{
    public bool IsReadOnly => CanRead && !CanWrite;

    public bool IsWriteOnly => CanWrite && !CanRead;

    public static PropertyAccess Resolve(Property property)
    {
        var tags = property.Tags;
        var hasReadOnlyTag = tags?.Contains("ReadOnly", StringComparer.Ordinal) == true;
        var hasWriteOnlyTag = tags?.Contains("WriteOnly", StringComparer.Ordinal) == true;

        var writeBlocked = hasReadOnlyTag || IsNotAccessible(property.Security?.Write);
        var readBlocked = hasWriteOnlyTag || IsNotAccessible(property.Security?.Read);

        if (writeBlocked && readBlocked)
        {
            writeBlocked = false;
        }

        return new PropertyAccess(CanRead: !readBlocked, CanWrite: !writeBlocked);
    }

    private static bool IsNotAccessible(string? securityLevel)
        => string.Equals(securityLevel, "NotAccessibleSecurity", StringComparison.Ordinal);
}
