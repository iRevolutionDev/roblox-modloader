using System.Runtime.InteropServices;

using RML.Interop;

using Xunit;

namespace Roblox.Tests;

public class NewDataTypeMarshalTests
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
    public void Faces_Is_FourBytes()
        => Assert.Equal(4, Marshal.SizeOf<Faces>());

    [Fact]
    public void Axes_Is_FourBytes()
        => Assert.Equal(4, Marshal.SizeOf<Axes>());

    [Fact]
    public void BrickColor_Is_FourBytes()
        => Assert.Equal(4, Marshal.SizeOf<BrickColor>());

    [Fact]
    public void Faces_RoundTrips()
    {
        var f = new Faces(top: true, right: true, front: true);
        var back = RoundTrip(f);
        Assert.Equal(f, back);
        Assert.True(back.Top && back.Right && back.Front);
        Assert.False(back.Bottom || back.Left || back.Back);
    }

    [Fact]
    public unsafe void Faces_Mask_Matches_Engine_NormalIdBits()
    {
        Assert.Equal(0x01, MaskOf(new Faces(right: true)));
        Assert.Equal(0x02, MaskOf(new Faces(top: true)));
        Assert.Equal(0x04, MaskOf(new Faces(back: true)));
        Assert.Equal(0x08, MaskOf(new Faces(left: true)));
        Assert.Equal(0x10, MaskOf(new Faces(bottom: true)));
        Assert.Equal(0x20, MaskOf(new Faces(front: true)));
        Assert.Equal(0x3F, MaskOf(new Faces(true, true, true, true, true, true)));

        static int MaskOf(Faces f)
        {
            var span = MemoryMarshal.Cast<Faces, int>(MemoryMarshal.CreateReadOnlySpan(ref f, 1));
            return span[0];
        }
    }

    [Fact]
    public void Axes_RoundTrips()
    {
        var a = new Axes(x: true, z: true);
        var back = RoundTrip(a);
        Assert.Equal(a, back);
        Assert.True(back.X && back.Z);
        Assert.False(back.Y);
    }

    [Fact]
    public void Axes_Mask_Matches_Engine_AxisBits()
    {
        Assert.Equal(0x01, MaskOf(new Axes(x: true)));
        Assert.Equal(0x02, MaskOf(new Axes(y: true)));
        Assert.Equal(0x04, MaskOf(new Axes(z: true)));

        static int MaskOf(Axes a)
        {
            var span = MemoryMarshal.Cast<Axes, int>(MemoryMarshal.CreateReadOnlySpan(ref a, 1));
            return span[0];
        }
    }

    [Fact]
    public void BrickColor_RoundTrips_And_Resolves_Palette()
    {
        var red = new BrickColor(21);
        var back = RoundTrip(red);
        Assert.Equal(red, back);
        Assert.Equal(21, back.Number);
        Assert.Equal("Bright red", back.Name);
        Assert.Equal(Color3.FromRGB(196, 40, 28), back.Color);
    }

    [Fact]
    public void BrickColor_Palette_Known_Entries()
    {
        Assert.Equal("White", new BrickColor(1).Name);
        Assert.Equal("Black", new BrickColor(26).Name);
        Assert.Equal("Medium stone grey", BrickColor.Default.Name);
        Assert.Equal(194, BrickColor.Default.Number);
        Assert.Equal(11, BrickColor.FromName("Pastel Blue").Number);
        Assert.Equal(Color3.FromRGB(128, 187, 219), new BrickColor(11).Color);
    }
    
    [Fact]
    public void NumberSequenceKeypoint_Is_TwelveBytes()
        => Assert.Equal(12, Marshal.SizeOf<NumberSequenceKeypoint>());

    [Fact]
    public void ColorSequenceKeypoint_Is_TwentyBytes()
        => Assert.Equal(20, Marshal.SizeOf<ColorSequenceKeypoint>());

    [Fact]
    public unsafe void NumberSequence_Reads_From_CountPrefixedBuffer()
    {
        var keys = new[]
        {
            new NumberSequenceKeypoint(0f, 1f, 0.1f),
            new NumberSequenceKeypoint(0.5f, 3f),
            new NumberSequenceKeypoint(1f, 2f),
        };

        var result = (NumberSequence)RoundTripSequence(keys, typeof(NumberSequence))!;
        Assert.Equal(keys.Length, result.Keypoints.Count);
        for (var i = 0; i < keys.Length; i++)
        {
            Assert.Equal(keys[i], result.Keypoints[i]);
        }
    }

    [Fact]
    public unsafe void ColorSequence_Reads_From_CountPrefixedBuffer()
    {
        var keys = new[]
        {
            new ColorSequenceKeypoint(0f, Color3.FromRGB(255, 0, 0)),
            new ColorSequenceKeypoint(1f, Color3.FromRGB(0, 0, 255)),
        };

        var result = (ColorSequence)RoundTripSequence(keys, typeof(ColorSequence))!;
        Assert.Equal(keys.Length, result.Keypoints.Count);
        for (var i = 0; i < keys.Length; i++)
        {
            Assert.Equal(keys[i], result.Keypoints[i]);
        }
    }

    [Fact]
    public void Empty_Sequence_Buffer_Reads_Empty()
    {
        var result = (NumberSequence)RoundTripSequence(Array.Empty<NumberSequenceKeypoint>(), typeof(NumberSequence))!;
        Assert.Empty(result.Keypoints);
    }

    private static unsafe object? RoundTripSequence<T>(T[] keys, Type target) where T : struct
    {
        var stride = Marshal.SizeOf<T>();
        var size = sizeof(int) + (keys.Length * stride);
        var buffer = Marshal.AllocHGlobal(size);
        try
        {
            Marshal.WriteInt32(buffer, keys.Length);
            for (var i = 0; i < keys.Length; i++)
            {
                Marshal.StructureToPtr(keys[i], buffer + sizeof(int) + (i * stride), false);
            }

            var variant = InteropVariant.FromBlittable((nuint)buffer);
            return Reflection.ConvertResult(variant, target, freeNativeResources: false);
        }
        finally
        {
            Marshal.FreeHGlobal(buffer);
        }
    }
}
