using System.Runtime.InteropServices;

using Xunit;

namespace RML.Interop.Tests;

public unsafe class InteropVariantTests
{
    [Fact]
    public void Variant_Is_16_Bytes()
    {
        Assert.Equal(16, sizeof(InteropVariant));
        Assert.Equal(16, Marshal.SizeOf<InteropVariant>());
    }

    [Fact]
    public void FromBool_RoundTrips()
    {
        var v = InteropVariant.FromBool(true);
        Assert.Equal(InteropVariant.Tags.Bool, v.Tag);
        Assert.True(v.AsBool);
    }

    [Fact]
    public void FromInt64_RoundTrips()
    {
        var v = InteropVariant.FromInt64(-1234567890123L);
        Assert.Equal(InteropVariant.Tags.Int64, v.Tag);
        Assert.Equal(-1234567890123L, v.AsInt64);
    }

    [Fact]
    public void FromDouble_RoundTrips()
    {
        var v = InteropVariant.FromDouble(3.14159);
        Assert.Equal(InteropVariant.Tags.Double, v.Tag);
        Assert.Equal(3.14159, v.AsDouble, 10);
    }

    [Fact]
    public void FromFloat_RoundTrips()
    {
        var v = InteropVariant.FromFloat(2.5f);
        Assert.Equal(InteropVariant.Tags.Float, v.Tag);
        Assert.Equal(2.5f, v.AsFloat);
    }

    [Fact]
    public void FromPointer_RoundTrips()
    {
        var v = InteropVariant.FromPointer((nuint)0xDEADBEEF);
        Assert.Equal(InteropVariant.Tags.Instance, v.Tag);
        Assert.Equal((nuint)0xDEADBEEF, v.AsPointer);
    }

    [Fact]
    public void FromString_TagsAsString()
    {
        var v = InteropVariant.FromString((nuint)0x1000);
        Assert.Equal(InteropVariant.Tags.String, v.Tag);
        Assert.Equal((nuint)0x1000, v.AsPointer);
    }

    [Fact]
    public void Default_Variant_Is_Null_Tag()
    {
        InteropVariant v = default;
        Assert.Equal(InteropVariant.Tags.Null, v.Tag);
    }

    [Fact]
    public void FromBlittable_TagsAsBlittable()
    {
        var v = InteropVariant.FromBlittable((nuint)0x2000);
        Assert.Equal(InteropVariant.Tags.Blittable, v.Tag);
        Assert.Equal((nuint)0x2000, v.AsPointer);
    }

    [Fact]
    public void Null_Factory_Matches_Default()
    {
        Assert.Equal(InteropVariant.Tags.Null, InteropVariant.Null.Tag);
        Assert.Equal(0ul, InteropVariant.Null.AsUInt64);
    }

    [Fact]
    public void Layout_Is_Sixteen_Bytes()
        => Assert.Equal(16, System.Runtime.InteropServices.Marshal.SizeOf<InteropVariant>());
}
