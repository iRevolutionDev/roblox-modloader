using System.Runtime.InteropServices;

using Xunit;

namespace Roblox.Tests;

public class NumberSequenceTests
{
    [Fact]
    public void Single_Value_Expands_To_Two_Keypoints()
    {
        var seq = new NumberSequence(0.5f);
        Assert.Equal(2, seq.Keypoints.Count);
        Assert.Equal(new NumberSequenceKeypoint(0f, 0.5f), seq.Keypoints[0]);
        Assert.Equal(new NumberSequenceKeypoint(1f, 0.5f), seq.Keypoints[1]);
    }

    [Fact]
    public void Two_Values_Endpoints()
    {
        var seq = new NumberSequence(0f, 1f);
        Assert.Equal(0f, seq.Keypoints[0].Value);
        Assert.Equal(1f, seq.Keypoints[1].Value);
    }

    [Theory]
    [InlineData(new[] { 0f })]                 // too few
    [InlineData(new[] { 0.1f, 1f })]           // first not 0
    [InlineData(new[] { 0f, 0.9f })]           // last not 1
    [InlineData(new[] { 0f, 0.8f, 0.5f, 1f })] // decreasing
    public void Invalid_Keypoints_Throw(float[] times)
    {
        var kps = Array.ConvertAll(times, t => new NumberSequenceKeypoint(t, 0f));
        Assert.Throws<ArgumentException>(() => new NumberSequence(kps));
    }

    [Fact]
    public void Equality_Compares_Keypoints()
    {
        Assert.Equal(new NumberSequence(1f), new NumberSequence(1f));
        Assert.NotEqual(new NumberSequence(1f), new NumberSequence(0f));
    }

    [Fact]
    public void Default_Has_No_Keypoints()
    {
        Assert.Empty(default(NumberSequence).Keypoints);
    }
}

public class ColorSequenceTests
{
    [Fact]
    public void Single_Color_Expands_To_Two_Keypoints()
    {
        var red = new Color3(1f, 0f, 0f);
        var seq = new ColorSequence(red);
        Assert.Equal(2, seq.Keypoints.Count);
        Assert.Equal(red, seq.Keypoints[0].Value);
        Assert.Equal(1f, seq.Keypoints[1].Time);
    }

    [Fact]
    public void Two_Colors_Endpoints()
    {
        var a = new Color3(1f, 0f, 0f);
        var b = new Color3(0f, 0f, 1f);
        var seq = new ColorSequence(a, b);
        Assert.Equal(a, seq.Keypoints[0].Value);
        Assert.Equal(b, seq.Keypoints[1].Value);
    }
}

public class SequenceLayoutTests
{
    [Fact]
    public void Keypoint_Sizes_Are_Blittable()
    {
        Assert.Equal(12, Marshal.SizeOf<NumberSequenceKeypoint>());
        Assert.Equal(20, Marshal.SizeOf<ColorSequenceKeypoint>());
    }
}
