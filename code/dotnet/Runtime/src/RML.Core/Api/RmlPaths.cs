using System.Reflection;

namespace RML.Core.Api;

public static class RmlPaths
{
    public static string RuntimeDirectory { get; } = ResolveRuntimeDirectory();

    public static string RootDirectory { get; } =
        Directory.GetParent(RuntimeDirectory)?.FullName ?? RuntimeDirectory;

    public static string ModsDirectory => Path.Combine(RootDirectory, "mods");

    public static string LogsDirectory => Path.Combine(RootDirectory, "logs");

    public static string DataDirectory => Path.Combine(RootDirectory, "data");

    public static string CrashesDirectory => Path.Combine(RootDirectory, "crashes");

    public static string ConfigFilePath => Path.Combine(RootDirectory, "config.toml");

    private static string ResolveRuntimeDirectory()
    {
        var location = Assembly.GetExecutingAssembly().Location;
        if (!string.IsNullOrEmpty(location) && Path.GetDirectoryName(location) is { Length: > 0 } dir)
        {
            return dir;
        }

        return AppContext.BaseDirectory.TrimEnd(Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar);
    }
}