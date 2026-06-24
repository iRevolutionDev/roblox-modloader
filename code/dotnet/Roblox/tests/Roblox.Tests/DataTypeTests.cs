using Xunit;

namespace Roblox.Tests;

public class Vector3Tests
{
    [Fact]
    public void Magnitude_And_Unit()
    {
        var v = new Vector3(3f, 4f, 0f);
        Assert.Equal(5f, v.Magnitude, 5);
        Assert.True(v.Unit.FuzzyEq(new Vector3(0.6f, 0.8f, 0f)));
        Assert.Equal(Vector3.Zero, Vector3.Zero.Unit);
    }

    [Fact]
    public void Dot_And_Cross()
    {
        Assert.Equal(0f, Vector3.XAxis.Dot(Vector3.YAxis));
        Assert.Equal(Vector3.ZAxis, Vector3.XAxis.Cross(Vector3.YAxis));
    }

    [Fact]
    public void Operators_Are_Componentwise_And_Scalar()
    {
        var a = new Vector3(1f, 2f, 3f);
        var b = new Vector3(4f, 5f, 6f);
        Assert.Equal(new Vector3(5f, 7f, 9f), a + b);
        Assert.Equal(new Vector3(4f, 10f, 18f), a * b);   // component-wise
        Assert.Equal(new Vector3(2f, 4f, 6f), a * 2f);    // scalar
        Assert.Equal(new Vector3(2f, 4f, 6f), 2f * a);
        Assert.Equal(new Vector3(-1f, -2f, -3f), -a);
    }

    [Fact]
    public void Lerp_Endpoints()
    {
        var a = Vector3.Zero;
        var b = new Vector3(10f, 20f, 30f);
        Assert.Equal(a, a.Lerp(b, 0f));
        Assert.Equal(b, a.Lerp(b, 1f));
        Assert.Equal(new Vector3(5f, 10f, 15f), a.Lerp(b, 0.5f));
    }
}

public class Color3Tests
{
    [Fact]
    public void FromRGB_Normalizes()
    {
        var c = Color3.FromRGB(255, 0, 128);
        Assert.Equal(1f, c.R, 5);
        Assert.Equal(0f, c.G, 5);
        Assert.Equal(128f / 255f, c.B, 5);
    }

    [Fact]
    public void Hex_RoundTrips()
    {
        Assert.Equal("FF007F", Color3.FromHex("#FF007F").ToHex());
        Assert.Equal(Color3.FromRGB(18, 52, 86), Color3.FromHex("123456"));
    }

    [Fact]
    public void FromHSV_PrimaryColors()
    {
        Assert.True(Color3.FromHSV(0f, 1f, 1f).Equals(new Color3(1f, 0f, 0f)));     // red
        Assert.True(Color3.FromHSV(1f / 3f, 1f, 1f).Equals(new Color3(0f, 1f, 0f))); // green
    }
}

public class UDim2Tests
{
    [Fact]
    public void FromScale_And_Offset()
    {
        Assert.Equal(new UDim2(0.5f, 0, 1f, 0), UDim2.FromScale(0.5f, 1f));
        Assert.Equal(new UDim2(0f, 10, 0f, 20), UDim2.FromOffset(10, 20));
    }

    [Fact]
    public void Addition_Is_Per_Axis()
    {
        var sum = new UDim2(0.1f, 5, 0.2f, 6) + new UDim2(0.3f, 7, 0.4f, 8);
        Assert.Equal(new UDim2(0.4f, 12, 0.6f, 14), sum);
    }
}

public class CFrameTests
{
    [Fact]
    public void Identity_Has_Standard_Basis()
    {
        Assert.Equal(Vector3.XAxis, CFrame.Identity.RightVector);
        Assert.Equal(Vector3.YAxis, CFrame.Identity.UpVector);
        // Roblox look is -Z.
        Assert.Equal(new Vector3(0f, 0f, -1f), CFrame.Identity.LookVector);
    }

    [Fact]
    public void LookAt_Faces_Target()
    {
        var cf = CFrame.LookAt(Vector3.Zero, new Vector3(0f, 0f, -10f));
        Assert.True(cf.LookVector.FuzzyEq(new Vector3(0f, 0f, -1f)), $"look was {cf.LookVector}");
    }

    [Fact]
    public void Inverse_Composes_To_Identity()
    {
        var cf = new CFrame(new Vector3(5f, 6f, 7f), new Vector3(1f, 2f, 3f));
        var result = cf * cf.Inverse();
        Assert.True(result.Position.FuzzyEq(Vector3.Zero), $"pos {result.Position}");
        Assert.True(result.RightVector.FuzzyEq(Vector3.XAxis));
        Assert.True(result.UpVector.FuzzyEq(Vector3.YAxis));
    }

    [Fact]
    public void Translation_Transforms_Points()
    {
        var cf = new CFrame(new Vector3(10f, 0f, 0f));
        Assert.Equal(new Vector3(11f, 2f, 3f), cf * new Vector3(1f, 2f, 3f));
        Assert.True(cf.PointToObjectSpace(new Vector3(11f, 2f, 3f)).FuzzyEq(new Vector3(1f, 2f, 3f)));
    }

    [Fact]
    public void Lerp_Endpoints_Match()
    {
        var a = new CFrame(Vector3.Zero);
        var b = new CFrame(new Vector3(10f, 0f, 0f), new Vector3(0f, 0f, -1f));
        Assert.True(a.Lerp(b, 0f).Position.FuzzyEq(a.Position));
        Assert.True(a.Lerp(b, 1f).Position.FuzzyEq(b.Position));
        Assert.True(a.Lerp(b, 1f).LookVector.FuzzyEq(b.LookVector), $"look {a.Lerp(b, 1f).LookVector}");
    }
}
