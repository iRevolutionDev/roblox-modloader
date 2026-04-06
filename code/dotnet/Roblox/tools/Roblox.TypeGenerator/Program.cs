using System;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using TypeGenerator.Generators;

namespace TypeGenerator;

internal static class Program
{
    public static async Task<int> Main(string[] args)
    {
        string? output = null;
        for (var i = 0; i < args.Length; i++)
        {
            if (args[i] != "-o" && args[i] != "--output")
            {
                continue;
            }

            if (i + 1 < args.Length)
            {
                output = args[++i];
            }
        }

        if (string.IsNullOrEmpty(output))
        {
            Console.WriteLine("Usage: TypeGenerator -o <outputDirectory>");
            return 1;
        }

        var generatedDir = Path.Combine(output, "Generated");
        var classesDir   = Path.Combine(generatedDir, "Classes");
        Directory.CreateDirectory(generatedDir);
        Directory.CreateDirectory(classesDir);

        Console.WriteLine("Fetching Roblox API dump and reflection metadata…");

        var dump       = await StudioAPI.GetDump();
        var reflection = await StudioAPI.GetReflectionMetadata();
        var apiDocs    = await StudioAPI.GetApiDocs();
        
        var enumsPath    = Path.Combine(generatedDir, "Enums.cs");
        var enumGenerator = new EnumGenerator(enumsPath);
        enumGenerator.Generate(dump.Enums ?? Enumerable.Empty<APITypes.EnumType>());
        Console.WriteLine($"  Enums   → {enumsPath}");
        
        var classList = dump.Classes?.ToList() ?? [];

        var classGenerator = new ClassGenerator(classesDir, reflection, apiDocs);
        classGenerator.Generate(classList);
        Console.WriteLine($"  Classes → {classesDir}  ({classList.Count} entries processed)");

        Console.WriteLine("Type generation finished.");
        return 0;
    }
}
