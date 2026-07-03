using System.Collections.Concurrent;
using System.Reflection;
using System.Text.Json;

using RML.Logging;

namespace RML.Core.Api;

public sealed class ModContext
{
    private static readonly ConcurrentDictionary<Assembly, ModContext> _registry = new();

    private static readonly JsonSerializerOptions _jsonOptions = new()
    {
        WriteIndented = true,
        PropertyNameCaseInsensitive = true
    };

    internal ModContext(ModDescriptor descriptor, string assemblyPath, Assembly assembly)
    {
        Id = descriptor.Id;
        Version = descriptor.Version;
        Author = descriptor.Author;
        Description = descriptor.Description;

        AssemblyPath = assemblyPath;
        AssemblyDirectory = Path.GetDirectoryName(assemblyPath) ?? AppContext.BaseDirectory;
        Directory = ResolveModRoot(AssemblyDirectory);
        Assembly = assembly;
        Logger = Log.CreateLogger(Id);
    }

    public string Id { get; }

    public string Version { get; }

    public string Author { get; }

    public string Description { get; }

    public string AssemblyPath { get; }

    public string AssemblyDirectory { get; }

    public string Directory { get; }

    public Assembly Assembly { get; }

    public ILogger Logger { get; }

    public string DataDirectory
    {
        get
        {
            var directory = Path.Combine(RmlPaths.DataDirectory, SanitizeId(Id));
            System.IO.Directory.CreateDirectory(directory);
            return directory;
        }
    }

    public string GetPath(params string[] relativeParts) => Combine(Directory, relativeParts);

    public string GetDataPath(params string[] relativeParts) => Combine(DataDirectory, relativeParts);

    public Stream? OpenResource(string resourceName) => Assembly.GetManifestResourceStream(resourceName);

    public string? ReadResourceText(string resourceName)
    {
        using var stream = OpenResource(resourceName);
        if (stream is null)
        {
            return null;
        }

        using var reader = new StreamReader(stream);
        return reader.ReadToEnd();
    }

    public T LoadConfig<T>(string fileName = "config.json")
        where T : new()
    {
        var path = GetDataPath(fileName);
        try
        {
            if (File.Exists(path))
            {
                return JsonSerializer.Deserialize<T>(File.ReadAllText(path), _jsonOptions) ?? new T();
            }
        }
        catch (Exception ex)
        {
            Logger.Warn($"failed to load config '{fileName}': {ex.Message}");
        }

        return new T();
    }

    public void SaveConfig<T>(T config, string fileName = "config.json")
    {
        var path = GetDataPath(fileName);
        try
        {
            var json = JsonSerializer.Serialize(config, _jsonOptions);
            var temp = path + ".tmp";
            File.WriteAllText(temp, json);
            File.Move(temp, path, true);
        }
        catch (Exception ex)
        {
            Logger.Error($"failed to save config '{fileName}': {ex.Message}");
        }
    }

    public static ModContext? ForAssembly(Assembly assembly) => _registry.GetValueOrDefault(assembly);

    public static ModContext? ForType(Type type) => ForAssembly(type.Assembly);

    internal static void Register(ModContext context) => _registry[context.Assembly] = context;

    internal static void Unregister(Assembly assembly) => _registry.TryRemove(assembly, out _);

    private static string ResolveModRoot(string assemblyDirectory)
    {
        if (string.Equals(new DirectoryInfo(assemblyDirectory).Name, "dotnet", StringComparison.OrdinalIgnoreCase) &&
            System.IO.Directory.GetParent(assemblyDirectory) is { FullName: var parent })
        {
            return parent;
        }

        return assemblyDirectory;
    }

    private static string Combine(string baseDirectory, string[]? relativeParts)
    {
        if (relativeParts is null || relativeParts.Length == 0)
        {
            return baseDirectory;
        }

        var segments = new string[relativeParts.Length + 1];
        segments[0] = baseDirectory;
        Array.Copy(relativeParts, 0, segments, 1, relativeParts.Length);
        return Path.Combine(segments);
    }

    private static string SanitizeId(string id)
    {
        var invalid = Path.GetInvalidFileNameChars();
        var safe = new string(id.Select(c => invalid.Contains(c) ? '_' : c).ToArray()).Trim();
        return string.IsNullOrEmpty(safe) ? "mod" : safe;
    }
}

internal readonly record struct ModDescriptor(string Id, string Version, string Author, string Description);