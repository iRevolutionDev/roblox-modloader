using System;
using System.Collections.Generic;
using System.Text.Json;
using System.Text.Json.Serialization;

namespace TypeGenerator.APITypes;

#pragma warning disable CS8618
public sealed class Dump
{
    public List<Class>? Classes { get; set; }
    public List<EnumType>? Enums { get; set; }
    public float Version { get; set; }
}

public sealed class EnumType
{
    public string Name { get; set; }
    public List<EnumItem> Items { get; set; }
}

public sealed class EnumItem
{
    public string[]? LegacyNames { get; set; }
    public string Name { get; set; }
    public int Value { get; set; }
}

[JsonConverter(typeof(MemberConverter))]
public abstract class MemberBase
{
    public string MemberType { get; set; }
    public string? Name { get; set; }
    public string? Description { get; set; }
}

public class Callback : MemberBase
{
    public List<Parameter>? Parameters { get; set; }
}

public sealed class Event : Callback { }

public sealed class Function : Callback
{
    public List<ValueType>? ReturnType { get; set; }
}

public sealed class Property : MemberBase
{
    public string? Category { get; set; }
    public string? Default { get; set; }
    public ValueType? ValueType { get; set; }
}

public sealed class Class
{
    public List<MemberBase>? Members { get; set; }
    public string Name { get; set; }
    public string? Superclass { get; set; }
    public HashSet<string>? Subclasses { get; set; }
    public string? Description { get; set; }
}

public sealed class ValueType
{
    public string Category { get; set; }
    public string Name { get; set; }
}

public sealed class Parameter
{
    public string Name { get; set; }
    public ValueType Type { get; set; }
}

public class MemberConverter : JsonConverter<MemberBase>
{
    public override MemberBase Read(ref Utf8JsonReader reader, Type typeToConvert, JsonSerializerOptions options)
    {
        using var doc = JsonDocument.ParseValue(ref reader);
        var root = doc.RootElement;
        var memberType = root.GetProperty("MemberType").GetString();

        return memberType switch
        {
            "Callback" => JsonSerializer.Deserialize<Callback>(root.GetRawText(), options)!,
            "Event" => JsonSerializer.Deserialize<Event>(root.GetRawText(), options)!,
            "Function" => JsonSerializer.Deserialize<Function>(root.GetRawText(), options)!,
            "Property" => JsonSerializer.Deserialize<Property>(root.GetRawText(), options)!,
            _ => throw new NotSupportedException($"MemberType '{memberType}' is not supported")
        };
    }

    public override void Write(Utf8JsonWriter writer, MemberBase value, JsonSerializerOptions options)
    {
        JsonSerializer.Serialize(writer, value, value.GetType(), options);
    }
}
#pragma warning restore CS8618
