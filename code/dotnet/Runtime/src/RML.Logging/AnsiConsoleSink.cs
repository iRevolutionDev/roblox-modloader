using System.Runtime.InteropServices;
using System.Text;

namespace RML.Logging;

public sealed class AnsiConsoleSink : ILogSink
{
    private const string Esc = "\x1b";
    private const string Reset = "\x1b[0m";
    private const string Dim = "\x1b[38;5;240m";
    private const char Separator = '│';

    private static readonly int[] SourcePalette =
        [39, 75, 114, 150, 179, 215, 210, 207, 141, 116, 108, 180, 81, 222];

    private readonly TextWriter _writer;
    private readonly bool _colors;

    public AnsiConsoleSink(TextWriter? writer = null, bool? enableColors = null)
    {
        _colors = enableColors ?? DetectColorSupport();

        if (writer is null && _colors)
        {
            TryEnableVirtualTerminal();
            TryUseUtf8Output();
        }

        _writer = writer ?? Console.Out;
    }

    public void Emit(in LogEvent e)
    {
        var timestamp = e.Timestamp.ToString("HH:mm:ss.fff");
        var level = LevelLabel(e.Level);
        var sb = new StringBuilder(96);

        if (_colors)
        {
            sb.Append(Dim).Append(timestamp).Append(' ').Append(Separator).Append(Reset).Append(' ')
              .Append(LevelColor(e.Level)).Append(level).Append(Reset).Append(' ')
              .Append(Dim).Append(Separator).Append(Reset).Append(' ')
              .Append(SourceColor(e.Source)).Append(e.Source).Append(Reset).Append(' ')
              .Append(Dim).Append(Separator).Append(Reset).Append(' ')
              .Append(MessageColor(e.Level)).Append(e.Message).Append(Reset);
        }
        else
        {
            sb.Append(timestamp).Append(' ').Append(Separator).Append(' ')
              .Append(level).Append(' ').Append(Separator).Append(' ')
              .Append(e.Source).Append(' ').Append(Separator).Append(' ')
              .Append(e.Message);
        }

        if (e.Exception is not null)
        {
            sb.Append(Environment.NewLine).Append(e.Exception);
        }

        _writer.WriteLine(sb.ToString());
    }

    private static string LevelLabel(LogLevel level) => level switch
    {
        LogLevel.Trace => "TRACE",
        LogLevel.Debug => "DEBUG",
        LogLevel.Info => "INFO ",
        LogLevel.Warn => "WARN ",
        LogLevel.Error => "ERROR",
        LogLevel.Fatal => "FATAL",
        _ => "?????",
    };

    private static string LevelColor(LogLevel level) => level switch
    {
        LogLevel.Trace => "\x1b[38;5;244m",
        LogLevel.Debug => "\x1b[38;5;39m",
        LogLevel.Info => "\x1b[38;5;42m",
        LogLevel.Warn => "\x1b[1;38;5;220m",
        LogLevel.Error => "\x1b[1;38;5;203m",
        LogLevel.Fatal => "\x1b[1;97;48;5;124m",
        _ => Reset,
    };

    private static string MessageColor(LogLevel level) => level switch
    {
        LogLevel.Trace => "\x1b[38;5;244m",
        LogLevel.Error => "\x1b[38;5;210m",
        LogLevel.Fatal => "\x1b[1;38;5;210m",
        _ => Reset,
    };

    private static string SourceColor(string source)
    {
        uint hash = 2166136261u;
        foreach (var c in source)
        {
            hash = (hash ^ c) * 16777619u;
        }

        var code = SourcePalette[hash % (uint)SourcePalette.Length];
        return $"{Esc}[1;38;5;{code}m";
    }

    private static bool DetectColorSupport()
    {
        if (Environment.GetEnvironmentVariable("NO_COLOR") is not null) return false;
        if (Environment.GetEnvironmentVariable("RML_NO_COLOR") is not null) return false;

        try
        {
            if (Console.IsOutputRedirected) return false;
        }
        catch
        {
            return false;
        }

        return true;
    }

    private static void TryEnableVirtualTerminal()
    {
        if (!OperatingSystem.IsWindows())
        {
            return;
        }

        try
        {
            var handle = GetStdHandle(StdOutputHandle);
            if (handle == nint.Zero || handle == new nint(-1))
            {
                return;
            }

            if (GetConsoleMode(handle, out var mode))
            {
                SetConsoleMode(handle, mode | EnableVirtualTerminalProcessing);
            }
        }
        catch
        {
        }
    }

    private static void TryUseUtf8Output()
    {
        if (!OperatingSystem.IsWindows())
        {
            return;
        }

        try
        {
            Console.OutputEncoding = Encoding.UTF8;
        }
        catch
        {
        }
    }

    private const int StdOutputHandle = -11;
    private const uint EnableVirtualTerminalProcessing = 0x0004;

    [DllImport("kernel32.dll", SetLastError = true)]
    private static extern nint GetStdHandle(int nStdHandle);

    [DllImport("kernel32.dll", SetLastError = true)]
    private static extern bool GetConsoleMode(nint hConsoleHandle, out uint lpMode);

    [DllImport("kernel32.dll", SetLastError = true)]
    private static extern bool SetConsoleMode(nint hConsoleHandle, uint dwMode);
}
