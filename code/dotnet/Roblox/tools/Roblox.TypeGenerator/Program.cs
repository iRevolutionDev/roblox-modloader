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
        for (int i = 0; i < args.Length; i++)
        {
            if (args[i] == "-o" || args[i] == "--output")
            {
                if (i + 1 < args.Length) output = args[++i];
            }
        }

        if (string.IsNullOrEmpty(output))
        {
            Console.WriteLine("Usage: TypeGenerator -o <outputDirectory>");
            return 1;
        }

        Directory.CreateDirectory(Path.Combine(output, "Generated"));
        Directory.CreateDirectory(Path.Combine(output, "Generated", "Classes"));

        var dump = await StudioAPI.GetDump();
        var reflection = await StudioAPI.GetReflectionMetadata();

        var enumsPath = Path.Combine(output, "Generated", "Enums.cs");
        var enumGenerator = new EnumGenerator(enumsPath);
        enumGenerator.Generate(dump.Enums ?? Enumerable.Empty<TypeGenerator.APITypes.EnumType>());

        var classesDir = Path.Combine(output, "Generated", "Classes");
        var classGenerator = new ClassGenerator(classesDir, reflection);
        classGenerator.Generate(dump.Classes?.ToList() ?? new System.Collections.Generic.List<TypeGenerator.APITypes.Class>());

        Console.WriteLine("Type generation finished.");
        return 0;
    }
}
