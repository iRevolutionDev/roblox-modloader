namespace RML.Logging;

public static class Log
{
    private static readonly Lock Gate = new();
    private static readonly List<ILogSink> Sinks = [new AnsiConsoleSink()];

    public static volatile LogLevel MinimumLevel = LogLevel.Info;

    public static ILogger CreateLogger(string source)
    {
        ArgumentException.ThrowIfNullOrEmpty(source);
        return new Logger(source);
    }

    public static void AddSink(ILogSink sink)
    {
        ArgumentNullException.ThrowIfNull(sink);
        lock (Gate)
        {
            Sinks.Add(sink);
        }
    }

    public static bool RemoveSink(ILogSink sink)
    {
        lock (Gate)
        {
            return Sinks.Remove(sink);
        }
    }

    public static void SetSinks(params ILogSink[] sinks)
    {
        ArgumentNullException.ThrowIfNull(sinks);
        lock (Gate)
        {
            Sinks.Clear();
            Sinks.AddRange(sinks);
        }
    }

    internal static void Dispatch(in LogEvent logEvent)
    {
        ILogSink[] snapshot;
        lock (Gate)
        {
            snapshot = Sinks.ToArray();
        }

        foreach (var sink in snapshot)
        {
            try
            {
                sink.Emit(logEvent);
            }
            catch
            {
            }
        }
    }
}
