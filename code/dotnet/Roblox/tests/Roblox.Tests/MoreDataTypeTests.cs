using Xunit;

namespace Roblox.Tests;

public class RayTests
{
    [Fact]
    public void ClosestPoint_And_Distance()
    {
        var ray = new Ray(Vector3.Zero, Vector3.XAxis);
        var p = new Vector3(5f, 3f, 0f);
        Assert.True(ray.ClosestPoint(p).FuzzyEq(new Vector3(5f, 0f, 0f)));
        Assert.Equal(3f, ray.Distance(p), 4);
    }
}

public class Region3Tests
{
    [Fact]
    public void Center_And_Size()
    {
        var r = new Region3(new Vector3(0f, 0f, 0f), new Vector3(4f, 6f, 8f));
        Assert.True(r.CFrame.Position.FuzzyEq(new Vector3(2f, 3f, 4f)));
        Assert.Equal(new Vector3(4f, 6f, 8f), r.Size);
    }

    [Fact]
    public void ExpandToGrid_Snaps_Bounds()
    {
        var r = new Region3(new Vector3(1.2f, 1.2f, 1.2f), new Vector3(2.7f, 2.7f, 2.7f));
        var g = r.ExpandToGrid(1f);
        // min floors to 1, max ceils to 3 → size 2, center 2.
        Assert.True(g.Size.FuzzyEq(new Vector3(2f, 2f, 2f)), $"size {g.Size}");
        Assert.True(g.CFrame.Position.FuzzyEq(new Vector3(2f, 2f, 2f)), $"center {g.CFrame.Position}");
    }
}

public class NumberRangeTests
{
    [Fact]
    public void Single_Value_And_Range()
    {
        Assert.Equal(new NumberRange(5f, 5f), new NumberRange(5f));
        var r = new NumberRange(1f, 9f);
        Assert.Equal(1f, r.Min);
        Assert.Equal(9f, r.Max);
    }

    [Fact]
    public void Throws_When_Max_Below_Min()
    {
        Assert.Throws<ArgumentException>(() => new NumberRange(5f, 1f));
    }
}

public class RectTests
{
    [Fact]
    public void Width_And_Height()
    {
        var rect = new Rect(1f, 2f, 5f, 10f);
        Assert.Equal(4f, rect.Width);
        Assert.Equal(8f, rect.Height);
    }
}

public class FacesAxesTests
{
    [Fact]
    public void Faces_Equality()
    {
        Assert.Equal(new Faces(top: true, front: true), new Faces(top: true, front: true));
        Assert.NotEqual(new Faces(top: true), new Faces(bottom: true));
    }

    [Fact]
    public void Axes_Equality()
    {
        Assert.Equal(new Axes(x: true, z: true), new Axes(x: true, z: true));
        Assert.NotEqual(new Axes(x: true), new Axes(y: true));
    }
}
