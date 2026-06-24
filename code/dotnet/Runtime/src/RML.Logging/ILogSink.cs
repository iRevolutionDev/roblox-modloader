namespace RML.Logging;

public interface ILogSink
{
    void Emit(in LogEvent logEvent);
}
