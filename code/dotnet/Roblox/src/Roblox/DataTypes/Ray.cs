using System.Runtime.InteropServices;

namespace Roblox;

[StructLayout(LayoutKind.Sequential)]
public readonly struct Ray : IEquatable<Ray>, IRobloxDataType
{
    public Vector3 Origin { get; }
    public Vector3 Direction { get; }

    public Ray(Vector3 origin, Vector3 direction)
    {
        Origin = origin;
        Direction = direction;
    }

    /// <summary>The point on the line closest to <paramref name="point"/>.</summary>
    public Vector3 ClosestPoint(Vector3 point)
    {
        var unit = Direction.Unit;
        var t = (point - Origin).Dot(unit);
        return Origin + unit * t;
    }

    /// <summary>The distance from <paramref name="point"/> to the line.</summary>
    public float Distance(Vector3 point) => (point - ClosestPoint(point)).Magnitude;

    public bool Equals(Ray other) => Origin.Equals(other.Origin) && Direction.Equals(other.Direction);
    public override bool Equals(object? obj) => obj is Ray other && Equals(other);
    public override int GetHashCode() => HashCode.Combine(Origin, Direction);

    public static bool operator ==(Ray a, Ray b) => a.Equals(b);
    public static bool operator !=(Ray a, Ray b) => !a.Equals(b);

    public override string ToString() => $"{{{Origin}}}, {{{Direction}}}";
}
