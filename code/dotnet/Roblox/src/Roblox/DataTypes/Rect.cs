using System.Runtime.InteropServices;

namespace Roblox;

[StructLayout(LayoutKind.Sequential)]
public readonly struct Rect : IEquatable<Rect>, IRobloxDataType
{
    public Vector2 Min { get; }
    public Vector2 Max { get; }

    public Rect(Vector2 min, Vector2 max)
    {
        Min = min;
        Max = max;
    }

    public Rect(float minX, float minY, float maxX, float maxY)
        : this(new Vector2(minX, minY), new Vector2(maxX, maxY))
    {
    }

    public float Width => Max.X - Min.X;
    public float Height => Max.Y - Min.Y;

    public bool Equals(Rect other) => Min.Equals(other.Min) && Max.Equals(other.Max);
    public override bool Equals(object? obj) => obj is Rect other && Equals(other);
    public override int GetHashCode() => HashCode.Combine(Min, Max);

    public static bool operator ==(Rect a, Rect b) => a.Equals(b);
    public static bool operator !=(Rect a, Rect b) => !a.Equals(b);

    public override string ToString() => $"{{{Min}}}, {{{Max}}}";
}
