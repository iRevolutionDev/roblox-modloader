using System.IO;
using System.Text;
using System.Collections.Generic;
using TypeGenerator.APITypes;

namespace TypeGenerator.Generators;

internal sealed class EnumGenerator
{
    private readonly string _outputPath;

    public EnumGenerator(string outputPath)
    {
        _outputPath = outputPath;
    }

    public void Generate(IEnumerable<EnumType> enums)
    {
        using var sw = File.CreateText(_outputPath);
        sw.WriteLine("// Auto-generated enums");
        sw.WriteLine("using System;");
        sw.WriteLine();
        sw.WriteLine("namespace Roblox");
        sw.WriteLine("{");

        foreach (var e in enums)
        {
            var name = Utility.ToPascalIdentifier(e.Name);
            sw.WriteLine($"    public enum {name}");
            sw.WriteLine("    {");
            foreach (var item in e.Items)
            {
                var itemName = Utility.ToPascalIdentifier(item.Name);
                sw.WriteLine($"        {itemName} = {item.Value},");
            }
            sw.WriteLine("    }");
            sw.WriteLine();
        }

        sw.WriteLine("}");
    }
}
