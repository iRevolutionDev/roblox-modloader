namespace RML.Logging;

public readonly struct LogEvent(LogLevel level, string source, string message, DateTime timestamp, Exception? exception)
{
    public LogLevel Level { get; } = level;
    public string Source { get; } = source;
    public string Message { get; } = message;

    public DateTime Timestamp { get; } = timestamp;
    public Exception? Exception { get; } = exception;
}
