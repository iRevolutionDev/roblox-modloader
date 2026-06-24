using System.Runtime.InteropServices;

namespace Roblox;

[StructLayout(LayoutKind.Sequential)]
public readonly struct ColorSequenceKeypoint : IEquatable<ColorSequenceKeypoint>, IRobloxDataType
{
    /// <summary>Position along the sequence, 0..1.</summary>
    public float Time { get; }
    public Color3 Value { get; }
    
    public float Envelope { get; }

    public ColorSequenceKeypoint(float time, Color3 value)
        : this(time, value, 0f)
    {
    }

    public ColorSequenceKeypoint(float time, Color3 value, float envelope)
    {
        Time = time;
        Value = value;
        Envelope = envelope;
    }

    public bool Equals(ColorSequenceKeypoint other) =>
        Time.Equals(other.Time) && Value.Equals(other.Value) && Envelope.Equals(other.Envelope);
    public override bool Equals(object? obj) => obj is ColorSequenceKeypoint other && Equals(other);
    public override int GetHashCode() => HashCode.Combine(Time, Value, Envelope);

    public static bool operator ==(ColorSequenceKeypoint a, ColorSequenceKeypoint b) => a.Equals(b);
    public static bool operator !=(ColorSequenceKeypoint a, ColorSequenceKeypoint b) => !a.Equals(b);

    public override string ToString() => $"{Time} {Value} {Envelope}";
}
