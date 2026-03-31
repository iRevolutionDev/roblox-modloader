using System.IO;
using System.Linq;
using System.Collections.Generic;
using TypeGenerator.APITypes;

namespace TypeGenerator.Generators;

internal sealed class ClassGenerator
{
    private readonly string _outputDirectory;
    private readonly ReflectionMetadataReader _metadata;

    public ClassGenerator(string outputDirectory, ReflectionMetadataReader metadata)
    {
        _outputDirectory = outputDirectory;
        _metadata = metadata;
    }

    public void Generate(List<Class> classes)
    {
        foreach (var c in classes)
        {
            var className = Utility.ToPascalIdentifier(c.Name);
            var filePath = Path.Combine(_outputDirectory, className + ".cs");
            using var sw = File.CreateText(filePath);
            sw.WriteLine("// Auto-generated class wrapper");
            sw.WriteLine("using RML.Core.Api;");
            sw.WriteLine("using System;");
            sw.WriteLine();
            sw.WriteLine("namespace Roblox");
            sw.WriteLine("{");

            var classDesc = _metadata.ReadClassDesc(c.Name);
            if (!string.IsNullOrEmpty(classDesc))
            {
                sw.WriteLine("    /// <summary>");
                sw.WriteLine($"    /// {classDesc}");
                sw.WriteLine("    /// </summary>");
            }

            sw.WriteLine($"    public sealed class {className}");
            sw.WriteLine("    {");
            sw.WriteLine("        public RmlInstance Instance { get; }");
            sw.WriteLine();
            sw.WriteLine($"        public {className}(RmlInstance instance) => Instance = instance;");
            sw.WriteLine();

            if (c.Members != null)
            {
                foreach (var member in c.Members)
                {
                    if (string.IsNullOrEmpty(member.Name)) continue;
                    var memberName = Utility.ToPascalIdentifier(member.Name);

                    switch (member)
                    {
                        case Property prop:
                        {
                            var desc = _metadata.ReadMemberDesc(c.Name, prop.Name ?? string.Empty);
                            if (!string.IsNullOrEmpty(desc))
                            {
                                sw.WriteLine("        /// <summary>");
                                sw.WriteLine($"        /// {desc}");
                                sw.WriteLine("        /// </summary>");
                            }

                            sw.WriteLine($"        public ulong {memberName}");
                            sw.WriteLine("        {");
                            sw.WriteLine($"            get => Instance.GetProperty(\"{member.Name}\");");
                            sw.WriteLine($"            set => Instance.SetProperty(\"{member.Name}\", value);");
                            sw.WriteLine("        }");
                            sw.WriteLine();
                            break;
                        }
                        case Function fn:
                        {
                            var desc = _metadata.ReadMemberDesc(c.Name, fn.Name ?? string.Empty);
                            if (!string.IsNullOrEmpty(desc))
                            {
                                sw.WriteLine("        /// <summary>");
                                sw.WriteLine($"        /// {desc}");
                                sw.WriteLine("        /// </summary>");
                            }

                            sw.WriteLine($"        public ulong {memberName}(params ulong[] args) => Instance.Invoke(\"{member.Name}\", args);");
                            sw.WriteLine();
                            break;
                        }
                        default:
                            // skip callbacks/events for now
                            break;
                    }
                }
            }

            sw.WriteLine("    }");
            sw.WriteLine("}");
        }
    }
}
