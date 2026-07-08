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

    [JsonConverter(typeof(StringTagsConverter))]
    public List<string>? Tags { get; set; }

    public bool IsDeprecated => Tags?.Contains("Deprecated", StringComparer.Ordinal) == true;
}

public class Callback : MemberBase
{
    public List<Parameter>? Parameters { get; set; }
}

public sealed class Event : Callback { }

public sealed class Function : Callback
{
    [JsonConverter(typeof(RobloxValueTypeFlexConverter))]
    public RobloxValueType? ReturnType { get; set; }
}

public sealed class Property : MemberBase
{
    public string? Category { get; set; }
    public string? Default { get; set; }
    [JsonConverter(typeof(RobloxValueTypeFlexConverter))]
    public RobloxValueType? ValueType { get; set; }
    [JsonConverter(typeof(PropertySecurityFlexConverter))]
    public PropertySecurity? Security { get; set; }
}

public sealed class PropertySecurity
{
    public string? Read { get; set; }
    public string? Write { get; set; }
}

public sealed class Class
{
    public List<MemberBase>? Members { get; set; }
    public string Name { get; set; }
    public string? Superclass { get; set; }
    public HashSet<string>? Subclasses { get; set; }
    public string? Description { get; set; }
}

public sealed class RobloxValueType
{
    public string Category { get; set; }
    public string Name { get; set; }
}

public sealed class Parameter
{
    public string Name { get; set; }
    public RobloxValueType Type { get; set; }
    public string? Default { get; set; }
}

internal sealed class RobloxValueTypeFlexConverter : JsonConverter<RobloxValueType?>
{
    public override RobloxValueType? Read(ref Utf8JsonReader reader, Type typeToConvert, JsonSerializerOptions options)
    {
        if (reader.TokenType == JsonTokenType.Null)
        {
            return null;
        }

        if (reader.TokenType == JsonTokenType.StartArray)
        {
            RobloxValueType? result = null;
            while (reader.Read() && reader.TokenType != JsonTokenType.EndArray)
            {
                if (result is null && reader.TokenType == JsonTokenType.StartObject)
                {
                    result = ReadObject(ref reader, options);
                }
                else
                {
                    reader.Skip();
                }
            }
            return result;
        }

        if (reader.TokenType == JsonTokenType.StartObject)
        {
            return ReadObject(ref reader, options);
        }

        reader.Skip();
        return null;
    }

    private static RobloxValueType ReadObject(ref Utf8JsonReader reader, JsonSerializerOptions options)
    {
        string? category = null;
        string? name = null;

        while (reader.Read() && reader.TokenType != JsonTokenType.EndObject)
        {
            if (reader.TokenType != JsonTokenType.PropertyName)
            {
                continue;
            }

            var propName = reader.GetString();
            reader.Read();

            if (string.Equals(propName, "Category", StringComparison.OrdinalIgnoreCase))
            {
                category = reader.GetString();
            }
            else if (string.Equals(propName, "Name", StringComparison.OrdinalIgnoreCase))
            {
                name = reader.GetString();
            }
            else
            {
                reader.Skip();
            }
        }

        return new RobloxValueType { Category = category ?? string.Empty, Name = name ?? string.Empty };
    }

    public override void Write(Utf8JsonWriter writer, RobloxValueType? value, JsonSerializerOptions options)
    {
        if (value is null) { writer.WriteNullValue(); return; }
        writer.WriteStartObject();
        writer.WriteString("Category", value.Category);
        writer.WriteString("Name", value.Name);
        writer.WriteEndObject();
    }
}

internal sealed class PropertySecurityFlexConverter : JsonConverter<PropertySecurity?>
{
    public override PropertySecurity? Read(ref Utf8JsonReader reader, Type typeToConvert, JsonSerializerOptions options)
    {
        if (reader.TokenType == JsonTokenType.Null)
        {
            return null;
        }

        if (reader.TokenType == JsonTokenType.String)
        {
            var level = reader.GetString();
            return new PropertySecurity { Read = level, Write = level };
        }

        if (reader.TokenType != JsonTokenType.StartObject)
        {
            reader.Skip();
            return null;
        }

        string? read = null;
        string? write = null;

        while (reader.Read() && reader.TokenType != JsonTokenType.EndObject)
        {
            if (reader.TokenType != JsonTokenType.PropertyName)
            {
                continue;
            }

            var propName = reader.GetString();
            reader.Read();

            if (string.Equals(propName, "Read", StringComparison.OrdinalIgnoreCase))
            {
                read = reader.GetString();
            }
            else if (string.Equals(propName, "Write", StringComparison.OrdinalIgnoreCase))
            {
                write = reader.GetString();
            }
            else
            {
                reader.Skip();
            }
        }

        return new PropertySecurity { Read = read, Write = write };
    }

    public override void Write(Utf8JsonWriter writer, PropertySecurity? value, JsonSerializerOptions options)
    {
        if (value is null) { writer.WriteNullValue(); return; }
        writer.WriteStartObject();
        writer.WriteString("Read", value.Read);
        writer.WriteString("Write", value.Write);
        writer.WriteEndObject();
    }
}

internal sealed class StringTagsConverter : JsonConverter<List<string>?>
{
    public override List<string>? Read(ref Utf8JsonReader reader, Type typeToConvert, JsonSerializerOptions options)
    {
        if (reader.TokenType == JsonTokenType.Null)
        {
            return null;
        }

        if (reader.TokenType != JsonTokenType.StartArray) { reader.Skip(); return null; }

        var result = new List<string>();
        while (reader.Read() && reader.TokenType != JsonTokenType.EndArray)
        {
            if (reader.TokenType == JsonTokenType.String)
            {
                result.Add(reader.GetString()!);
            }
            else
            {
                reader.Skip();
            }
        }
        return result;
    }

    public override void Write(Utf8JsonWriter writer, List<string>? value, JsonSerializerOptions options)
    {
        if (value is null) { writer.WriteNullValue(); return; }
        writer.WriteStartArray();
        foreach (var s in value)
        {
            writer.WriteStringValue(s);
        }

        writer.WriteEndArray();
    }
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

    public override void Write(Utf8JsonWriter writer, MemberBase value, JsonSerializerOptions options) => JsonSerializer.Serialize(writer, value, value.GetType(), options);
}
#pragma warning restore CS8618
