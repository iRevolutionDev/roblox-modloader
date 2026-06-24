using System.Runtime.InteropServices;

namespace Roblox;

[StructLayout(LayoutKind.Sequential)]
public readonly struct ColorSequenceKeypoint : IEquatable<ColorSequenceKeypoint>
{
    /// <summary>Position along the sequence, 0..1.</summary>
    public float Time { get; }
    public Color3 Value { get; }

    public ColorSequenceKeypoint(float time, Color3 value)
    {
        Time = time;
        Value = value;
    }

    public bool Equals(ColorSequenceKeypoint other) => Time.Equals(other.Time) && Value.Equals(other.Value);
    public override bool Equals(object? obj) => obj is ColorSequenceKeypoint other && Equals(other);
    public override int GetHashCode() => HashCode.Combine(Time, Value);

    public static bool operator ==(ColorSequenceKeypoint a, ColorSequenceKeypoint b) => a.Equals(b);
    public static bool operator !=(ColorSequenceKeypoint a, ColorSequenceKeypoint b) => !a.Equals(b);

    public override string ToString() => $"{Time} {Value}";
}
