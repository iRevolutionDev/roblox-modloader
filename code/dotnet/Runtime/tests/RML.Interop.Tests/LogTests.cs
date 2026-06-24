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
}
