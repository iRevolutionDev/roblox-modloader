using Xunit;

namespace RML.Interop.Tests;

public class InteropLifecycleTests
{
    [Fact]
    public void Initialize_Throws_On_Null_Table()
    {
        Assert.Throws<ArgumentException>(() => Interop.Initialize(nint.Zero));
    }

    [Fact]
    public void Uninitialize_Is_Safe_Without_Init()
    {
        Interop.Uninitialize();
        Interop.Uninitialize();

        Assert.False(Interop.IsInitialized);
    }
}
