using RML.Logging;

using Xunit;

namespace RML.Interop.Tests;

public class LogTests
{
    [Fact]
    public void Log_Does_Not_Throw_When_Uninitialized()
    {
        Assert.Null(Record.Exception(() => Interop.Log(LogLevel.Info, "hello")));
        Assert.Null(Record.Exception(() => Interop.Log(LogLevel.Error, "boom")));
    }

    [Fact]
    public void Log_Ignores_Null_Message()
    {
        Assert.Null(Record.Exception(() => Interop.Log(LogLevel.Warn, null!)));
    }

    [Fact]
    public void InteropLogSink_Forwards_Formatted_Message_To_Interop_Log()
    {
        var originalOut = Console.Out;
        var sw = new StringWriter();
        Console.SetOut(sw);

        try
        {
            var sink = new InteropLogSink();
            sink.Emit(new LogEvent(LogLevel.Info, "MyMod", "hello world", DateTimeOffset.UtcNow, null));
        }
        finally
        {
            Console.SetOut(originalOut);
        }

        var output = sw.ToString();
        Assert.Contains("MyMod", output);
        Assert.Contains("hello world", output);
    }

    [Fact]
    public void InteropLogSink_Appends_Exception_Details()
    {
        var originalError = Console.Error;
        var sw = new StringWriter();
        Console.SetError(sw);

        try
        {
            var sink = new InteropLogSink();
            sink.Emit(new LogEvent(LogLevel.Error, "MyMod", "boom", DateTimeOffset.UtcNow, new InvalidOperationException("nope")));
        }
        finally
        {
            Console.SetError(originalError);
        }

        var output = sw.ToString();
        Assert.Contains("boom", output);
        Assert.Contains("InvalidOperationException", output);
        Assert.Contains("nope", output);
    }
}
