using ApiValueType = TypeGenerator.APITypes.RobloxValueType;

namespace TypeGenerator;

internal static class TypeMapper
{
    private static HashSet<string> _knownClasses = new(StringComparer.Ordinal);
    private static readonly SortedSet<string> _unmappedTypes = new(StringComparer.Ordinal);

    public static IReadOnlyCollection<string> UnmappedTypes => _unmappedTypes;

    public static void SetKnownClasses(IEnumerable<string> names)
    {
        _knownClasses = new HashSet<string>(names, StringComparer.Ordinal);
        _unmappedTypes.Clear();
    }

    public static string ToCSharp(ApiValueType? vt)
    {
        if (vt is null)
        {
            return "void";
        }

        if (vt.Name.EndsWith('?'))
        {
            return MakeNullable(ToCSharp(new ApiValueType { Category = vt.Category, Name = vt.Name[..^1] }));
        }

        return vt.Category switch
        {
            "Primitive" => vt.Name switch
            {
                "bool"          => "bool",
                "int"           => "int",
                "int64"         => "long",
                "float"         => "float",
                "double"        => "double",
                "string"        => "string",
                "void"          => "void",
                _               => RecordUnmapped("Primitive", vt.Name),
            },

            "Class"     => ResolveClassName(vt.Name),
            "Enum"      => $"Enum.{TypeSafeName(vt.Name)}",
            "DataType"  => MapDataType(vt.Name),
            "Group"     => RecordUnmapped("Group", vt.Name),

            _           => RecordUnmapped(vt.Category, vt.Name),
        };
    }

    private static string RecordUnmapped(string category, string name)
    {
        var label = string.IsNullOrWhiteSpace(name) ? "<unnamed>" : name;
        _unmappedTypes.Add($"{category}:{label}");
        return "object";
    }

    public static string ReturnType(ApiValueType? returnType)
    {
        if (returnType is null)
        {
            return "void";
        }
        
        if (IsTuple(returnType))
        {
            return "object?[]";
        }

        var t = ToCSharp(returnType);

        if (returnType.Category == "Class" && returnType.Name == "Instance")
        {
            t = MakeNullable(t);
        }

        return t;
    }

    public static bool IsTuple(ApiValueType? vt)
        => vt is { Category: "Group", Name: "Tuple" };

    public static bool IsVoid(string csharpType) => csharpType == "void";
    
    public static string MakeNullable(string csType)
        => csType.EndsWith('?') || csType == "void" ? csType : csType + "?";
    
    private static string MapDataType(string name) => name switch
    {
        "Vector2"                  => "global::Roblox.Vector2",
        "Vector3"                  => "global::Roblox.Vector3",
        "CFrame"                   => "global::Roblox.CFrame",
        "CoordinateFrame"          => "global::Roblox.CFrame",
        "OptionalCoordinateFrame"  => "global::Roblox.CFrame?",
        "Color3"                   => "global::Roblox.Color3",
        "UDim"                     => "global::Roblox.UDim",
        "UDim2"                    => "global::Roblox.UDim2",
        "Rect"                     => "global::Roblox.Rect",
        "Region3"                  => "global::Roblox.Region3",
        "Ray"                      => "global::Roblox.Ray",
        "NumberRange"              => "global::Roblox.NumberRange",
        "Faces"                    => "global::Roblox.Faces",
        "Axes"                     => "global::Roblox.Axes",
        "BrickColor"               => "global::Roblox.BrickColor",
        "NumberSequence"           => "global::Roblox.NumberSequence",
        "ColorSequence"            => "global::Roblox.ColorSequence",
        "Content"                  => "string",
        "BinaryString"             => "byte[]",
        "SharedString"             => "string",
        "ProtectedString"          => "string",
        "QDir"                     => "string",
        "QFont"                    => "string",
        "Instances"                => "IReadOnlyList<Instance>",
        _                          => RecordUnmapped("DataType", name),
    };

    private static string TypeSafeName(string name) => string.IsNullOrWhiteSpace(name) ? "object" : Utility.ToPascalIdentifier(name);

    private static string ResolveClassName(string name)
    {
        if (string.IsNullOrWhiteSpace(name))
            return "object";

        return _knownClasses.Count == 0 || _knownClasses.Contains(name)
            ? Utility.ToPascalIdentifier(name)
            : "Instance";
    }
}
