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
            "Vector2"           => $"global::Roblox.Vector2{suffix}",
            "Vector3"           => $"global::Roblox.Vector3{suffix}",
            "CFrame"            => $"global::Roblox.CFrame{suffix}",
            "Color3"            => $"global::Roblox.Color3{suffix}",
            "UDim"              => $"global::Roblox.UDim{suffix}",
            "UDim2"             => $"global::Roblox.UDim2{suffix}",
            "Rect"              => $"global::Roblox.Rect{suffix}",
            "Region3"           => $"global::Roblox.Region3{suffix}",
            "Ray"               => $"global::Roblox.Ray{suffix}",
            "NumberRange"       => $"global::Roblox.NumberRange{suffix}",
            "Faces"             => $"global::Roblox.Faces{suffix}",
            "Axes"              => $"global::Roblox.Axes{suffix}",
            "BrickColor"        => $"global::Roblox.BrickColor{suffix}",
            "NumberSequence"    => $"global::Roblox.NumberSequence{suffix}",
            "ColorSequence"     => $"global::Roblox.ColorSequence{suffix}",
            "Content"           => $"string{suffix}",
            "BinaryString"      => $"byte[]{suffix}",
            "SharedString"      => $"string{suffix}",
            "ProtectedString"   => $"string{suffix}",
            "QDir"              => $"string{suffix}",
            "QFont"             => $"string{suffix}",
            "Instances"         => $"IReadOnlyList<Instance>",
            _                   => "object" + suffix,
        };
    }

    private static string TypeSafeName(string name) => string.IsNullOrWhiteSpace(name) ? "object" : Utility.ToPascalIdentifier(name);
}
