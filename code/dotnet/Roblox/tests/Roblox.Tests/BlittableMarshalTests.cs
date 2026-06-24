using System.Runtime.InteropServices;

using RML.Interop;

using Xunit;

namespace Roblox.Tests;

public class BlittableMarshalTests
{
    private static unsafe T RoundTrip<T>(T value) where T : struct
    {
        var size = Marshal.SizeOf<T>();
        var buffer = Marshal.AllocHGlobal(size);
        try
        {
            Marshal.StructureToPtr(value, buffer, false);
            var variant = InteropVariant.FromBlittable((nuint)buffer);
            var result = Reflection.ConvertResult(variant, typeof(T), freeNativeResources: false);
            return Assert.IsType<T>(result);
        }
        finally
        {
            Marshal.FreeHGlobal(buffer);
        }
    }

    [Fact]
    public void Vector3_RoundTrips()
    {
        var v = new Vector3(1.5f, -2.5f, 3.25f);
        Assert.Equal(v, RoundTrip(v));
    }

    [Fact]
    public void Color3_RoundTrips()
    {
        var c = Color3.FromRGB(10, 128, 255);
        Assert.Equal(c, RoundTrip(c));
    }

    [Fact]
    public void UDim2_RoundTrips()
    {
        var u = new UDim2(0.5f, 12, 1f, -8);
        Assert.Equal(u, RoundTrip(u));
    }

    [Fact]
    public void CFrame_RoundTrips_All_12_Components()
    {
        var cf = new CFrame(new Vector3(5f, 6f, 7f),
            0.1f, 0.2f, 0.3f,
            0.4f, 0.5f, 0.6f,
            0.7f, 0.8f, 0.9f);
        var back = RoundTrip(cf);
        Assert.Equal(cf.GetComponents(), back.GetComponents());
    }

    [Fact]
    public void Region3_RoundTrips()
    {
        var r = new Region3(new Vector3(0f, 0f, 0f), new Vector3(4f, 6f, 8f));
        Assert.Equal(r, RoundTrip(r));
    }

    [Fact]
    public void Null_Pointer_Blittable_Is_Null()
    {
        var variant = InteropVariant.FromBlittable(0);
        Assert.Null(Reflection.ConvertResult(variant, typeof(Vector3), freeNativeResources: false));
    }
}
