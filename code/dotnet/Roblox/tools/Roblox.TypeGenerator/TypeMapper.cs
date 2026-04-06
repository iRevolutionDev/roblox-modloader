using ApiValueType = TypeGenerator.APITypes.RobloxValueType;

namespace TypeGenerator;

internal static class TypeMapper
{
    public static string ToCSharp(ApiValueType? vt, bool nullable = false)
    {
        if (vt is null)
        {
            return "void";
        }

        var suffix = nullable ? "?" : string.Empty;

        return vt.Category switch
        {
            "Primitive" => vt.Name switch
            {
                "bool"          => "bool",
                "int"           => "int",
                "int64"         => "long",
                "float"         => "float",
                "double"        => "double",
                "string"        => $"string{suffix}",
                "void"          => "void",
                _               => "object" + suffix,
            },

            "Class"     => $"{TypeSafeName(vt.Name)}{suffix}",
            "Enum"      => $"Enum.{TypeSafeName(vt.Name)}",
            "DataType"  => MapDataType(vt.Name, nullable),
            "Group"     => "object" + suffix,

            _           => "object" + suffix,
        };
    }

    public static string ReturnType(ApiValueType? returnType, bool nullable = true) => returnType is null ? "void" : ToCSharp(returnType, nullable);

    public static bool IsVoid(string csharpType) => csharpType == "void";
    
    private static string MapDataType(string name, bool nullable)
    {
        var suffix = nullable ? "?" : string.Empty;
        return name switch
        {
            "Vector2"           => $"global::System.Numerics.Vector2",
            "Vector3"           => $"global::System.Numerics.Vector3",
            "CFrame"            => "object" + suffix,
            "Color3"            => "object" + suffix,
            "BrickColor"        => "object" + suffix,
            "UDim"              => "object" + suffix,
            "UDim2"             => "object" + suffix,
            "Rect"              => "object" + suffix,
            "Region3"           => "object" + suffix,
            "Ray"               => "object" + suffix,
            "NumberRange"       => "object" + suffix,
            "NumberSequence"    => "object" + suffix,
            "ColorSequence"     => "object" + suffix,
            "Faces"             => "int",
            "Axes"              => "int",
            "Content"           => $"string{suffix}",
            "BinaryString"      => $"byte[]{suffix}",
            "SharedString"      => $"string{suffix}",
            "ProtectedString"   => $"string{suffix}",
            "QDir"              => $"string{suffix}",
            "QFont"             => $"string{suffix}",
            _                   => "object" + suffix,
        };
    }

    private static string TypeSafeName(string name) => string.IsNullOrWhiteSpace(name) ? "object" : Utility.ToPascalIdentifier(name);
}
