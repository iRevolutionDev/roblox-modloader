using System.Runtime.InteropServices;

namespace Roblox;

[StructLayout(LayoutKind.Sequential)]
public readonly struct Region3 : IEquatable<Region3>, IRobloxDataType
{
    public CFrame CFrame { get; }
    public Vector3 Size { get; }

    public Region3(Vector3 min, Vector3 max)
    {
        CFrame = new CFrame((min + max) * 0.5f);
        Size = max - min;
    }

    private Region3(CFrame cframe, Vector3 size)
    {
        CFrame = cframe;
        Size = size;
    }

    /// <summary>Grows the region so its bounds align to a grid of the given resolution.</summary>
    public Region3 ExpandToGrid(float resolution)
    {
        var half = Size * 0.5f;
        var min = CFrame.Position - half;
        var max = CFrame.Position + half;

        Vector3 Floor(Vector3 v) => new(
            MathF.Floor(v.X / resolution) * resolution,
            MathF.Floor(v.Y / resolution) * resolution,
            MathF.Floor(v.Z / resolution) * resolution);

        Vector3 Ceil(Vector3 v) => new(
            MathF.Ceiling(v.X / resolution) * resolution,
            MathF.Ceiling(v.Y / resolution) * resolution,
            MathF.Ceiling(v.Z / resolution) * resolution);

        return new Region3(Floor(min), Ceil(max));
    }

    public bool Equals(Region3 other) => CFrame.Equals(other.CFrame) && Size.Equals(other.Size);
    public override bool Equals(object? obj) => obj is Region3 other && Equals(other);
    public override int GetHashCode() => HashCode.Combine(CFrame, Size);

    public static bool operator ==(Region3 a, Region3 b) => a.Equals(b);
    public static bool operator !=(Region3 a, Region3 b) => !a.Equals(b);

    public override string ToString() => $"{Size} @ {CFrame.Position}";
}
