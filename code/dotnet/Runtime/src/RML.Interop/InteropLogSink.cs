using RML.Logging;

namespace RML.Interop;

public sealed class InteropLogSink : ILogSink
{
    public void Emit(in LogEvent logEvent)
    {
        var message = logEvent.Exception is null
            ? $"[{logEvent.Source}] {logEvent.Message}"
            : $"[{logEvent.Source}] {logEvent.Message}{Environment.NewLine}{logEvent.Exception}";

        Interop.Log(logEvent.Level, message);
    }
}
