using Xunit;

namespace RML.Logging.Tests;

public class AnsiConsoleSinkTests
{
    private static LogEvent Event(LogLevel level, string source, string message)
        => new(level, source, message, new DateTime(2026, 1, 2, 13, 4, 5, 678), null);

    [Fact]
    public void Plain_Format_Is_Deterministic()
    {
        var sw = new StringWriter();
        var sink = new AnsiConsoleSink(sw, enableColors: false);

        sink.Emit(Event(LogLevel.Info, "MyMod", "hello"));

        Assert.Equal("13:04:05.678 │ INFO  │ MyMod │ hello", sw.ToString().TrimEnd());
    }

    [Fact]
    public void Plain_Format_Has_No_Escape_Codes()
    {
        var sw = new StringWriter();
        var sink = new AnsiConsoleSink(sw, enableColors: false);

        sink.Emit(Event(LogLevel.Error, "X", "boom"));

        Assert.DoesNotContain('\x1b', sw.ToString());
    }

    [Fact]
    public void Colored_Output_Contains_Ansi_And_Resets()
    {
        var sw = new StringWriter();
        var sink = new AnsiConsoleSink(sw, enableColors: true);

        sink.Emit(Event(LogLevel.Warn, "MyMod", "careful"));
        var output = sw.ToString();

        Assert.Contains('\x1b', output);
        Assert.Contains("\x1b[0m", output);   // reset present
        Assert.Contains("WARN", output);
        Assert.Contains("MyMod", output);
        Assert.Contains("careful", output);
    }

    [Fact]
    public void Exception_Is_Appended()
    {
        var sw = new StringWriter();
        var sink = new AnsiConsoleSink(sw, enableColors: false);

        sink.Emit(new LogEvent(LogLevel.Error, "X", "failed", DateTime.Now, new InvalidOperationException("nope")));

        Assert.Contains("InvalidOperationException", sw.ToString());
        Assert.Contains("nope", sw.ToString());
    }

    [Fact]
    public void Source_Color_Is_Stable_Per_Source()
    {
        var a = new StringWriter();
        var b = new StringWriter();
        new AnsiConsoleSink(a, enableColors: true).Emit(Event(LogLevel.Info, "SameName", "1"));
        new AnsiConsoleSink(b, enableColors: true).Emit(Event(LogLevel.Info, "SameName", "2"));

        Assert.Equal(ColorPrefixOf(a.ToString()), ColorPrefixOf(b.ToString()));
    }

    private static string ColorPrefixOf(string line)
    {
        var idx = line.IndexOf("SameName", StringComparison.Ordinal);
        return line[..idx];
    }
}
