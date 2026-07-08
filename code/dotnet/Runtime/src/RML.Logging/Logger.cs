namespace RML.Logging;

internal sealed class Logger(string source) : ILogger
{
    public string Source { get; } = source;

    public bool IsEnabled(LogLevel level) => level >= RML.Logging.Log.MinimumLevel;

    public void Log(LogLevel level, string message, Exception? exception = null)
    {
        if (!IsEnabled(level))
        {
            return;
        }

        RML.Logging.Log.Dispatch(new LogEvent(level, Source, message ?? string.Empty, DateTimeOffset.UtcNow, exception));
    }

    public void Trace(string message) => Log(LogLevel.Trace, message);
    public void Debug(string message) => Log(LogLevel.Debug, message);
    public void Info(string message) => Log(LogLevel.Info, message);
    public void Warn(string message) => Log(LogLevel.Warn, message);
    public void Error(string message, Exception? exception = null) => Log(LogLevel.Error, message, exception);
    public void Fatal(string message, Exception? exception = null) => Log(LogLevel.Fatal, message, exception);
}
