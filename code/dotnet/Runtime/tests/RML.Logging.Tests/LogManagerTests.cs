using Xunit;

namespace RML.Logging.Tests;

[Collection("LogManager")]
public class LogManagerTests
{
    private sealed class CapturingSink : ILogSink
    {
        public readonly List<LogEvent> Events = [];
        public void Emit(in LogEvent logEvent) => Events.Add(logEvent);
    }

    [Fact]
    public void Logger_Dispatches_To_Sinks_With_Source()
    {
        var sink = new CapturingSink();
        var previousLevel = Log.MinimumLevel;
        Log.AddSink(sink);
        try
        {
            Log.MinimumLevel = LogLevel.Trace;
            var logger = Log.CreateLogger("Abc");
            logger.Info("hi");
            logger.Error("bad", new Exception("x"));

            Assert.Equal(2, sink.Events.Count);
            Assert.Equal("Abc", sink.Events[0].Source);
            Assert.Equal(LogLevel.Info, sink.Events[0].Level);
            Assert.Equal("hi", sink.Events[0].Message);
            Assert.NotNull(sink.Events[1].Exception);
        }
        finally
        {
            Log.RemoveSink(sink);
            Log.MinimumLevel = previousLevel;
        }
    }

    [Fact]
    public void Below_Minimum_Level_Is_Dropped()
    {
        var sink = new CapturingSink();
        var previousLevel = Log.MinimumLevel;
        Log.AddSink(sink);
        try
        {
            Log.MinimumLevel = LogLevel.Warn;
            var logger = Log.CreateLogger("Filter");

            logger.Info("dropped");
            logger.Debug("dropped");
            logger.Warn("kept");
            logger.Error("kept");

            Assert.Equal(2, sink.Events.Count);
            Assert.All(sink.Events, e => Assert.True(e.Level >= LogLevel.Warn));
        }
        finally
        {
            Log.RemoveSink(sink);
            Log.MinimumLevel = previousLevel;
        }
    }

    [Fact]
    public void CreateLogger_Rejects_Empty_Source()
    {
        Assert.Throws<ArgumentException>(() => Log.CreateLogger(""));
    }
}
