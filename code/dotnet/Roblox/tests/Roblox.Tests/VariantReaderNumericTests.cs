using RML.Interop;

using Xunit;

namespace Roblox.Tests;

public class VariantReaderNumericTests
{
    private enum SignedEnum
    {
        NegativeOne = -1,
        Zero = 0,
    }

    [Fact]
    public void NegativeInt64_ReadAsInt_RoundTrips()
    {
        var variant = InteropVariant.FromInt64(-1);

        Assert.Equal(-1, Reflection.ConvertResult(variant, typeof(int), freeNativeResources: false));
    }

    [Fact]
    public void NegativeInt64_ReadAsLong_RoundTrips()
    {
        var variant = InteropVariant.FromInt64(-42);

        Assert.Equal(-42L, Reflection.ConvertResult(variant, typeof(long), freeNativeResources: false));
    }

    [Fact]
    public void NegativeEnumMember_RoundTrips()
    {
        var variant = InteropVariant.FromInt64(-1);

        Assert.Equal(SignedEnum.NegativeOne, Reflection.ConvertResult(variant, typeof(SignedEnum), freeNativeResources: false));
    }

    [Fact]
    public void MaxUInt64_ReadAsUlong_RoundTrips()
    {
        var variant = InteropVariant.FromInt64(-1);

        Assert.Equal(ulong.MaxValue, Reflection.ConvertResult(variant, typeof(ulong), freeNativeResources: false));
    }
}
