using System.Runtime.InteropServices;

using Xunit;

namespace Roblox.Tests;

public class DataTypeLayoutTests
{
    [Theory]
    [InlineData(typeof(Vector2), 8)]
    [InlineData(typeof(Vector3), 12)]
    [InlineData(typeof(Color3), 12)]
    [InlineData(typeof(UDim), 8)]
    [InlineData(typeof(UDim2), 16)]
    [InlineData(typeof(Rect), 16)]
    [InlineData(typeof(NumberRange), 8)]
    [InlineData(typeof(Ray), 24)]
    [InlineData(typeof(CFrame), 48)]
    [InlineData(typeof(Region3), 60)]
    public void Has_Expected_Blittable_Size(Type type, int expectedSize)
    {
        Assert.Equal(expectedSize, Marshal.SizeOf(type));
    }

    [Fact]
    public void CFrame_Memory_Is_Rotation_Then_Translation()
    {
        var cf = new CFrame(new Vector3(1f, 2f, 3f),
            10f, 11f, 12f,
            13f, 14f, 15f,
            16f, 17f, 18f);

        Span<byte> bytes = stackalloc byte[Marshal.SizeOf<CFrame>()];
        MemoryMarshal.Write(bytes, in cf);
        var floats = MemoryMarshal.Cast<byte, float>(bytes);

        Assert.Equal(10f, floats[0]);
        Assert.Equal(14f, floats[4]);
        Assert.Equal(18f, floats[8]);
        Assert.Equal(1f, floats[9]);
        Assert.Equal(2f, floats[10]);
        Assert.Equal(3f, floats[11]);
    }
}
