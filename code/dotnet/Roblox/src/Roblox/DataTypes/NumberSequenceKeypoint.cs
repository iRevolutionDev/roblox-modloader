using System.Runtime.InteropServices;

namespace Roblox;

[StructLayout(LayoutKind.Sequential)]
public readonly struct NumberSequenceKeypoint : IEquatable<NumberSequenceKeypoint>, IRobloxDataType
{
    /// <summary>Position along the sequence, 0..1.</summary>
    public float Time { get; }
    public float Value { get; }
    /// <summary>Allowed random variance around <see cref="Value"/>.</summary>
    public float Envelope { get; }

    public NumberSequenceKeypoint(float time, float value, float envelope = 0f)
    {
        Time = time;
        Value = value;
        Envelope = envelope;
    }

    public bool Equals(NumberSequenceKeypoint other) =>
        Time.Equals(other.Time) && Value.Equals(other.Value) && Envelope.Equals(other.Envelope);

    public override bool Equals(object? obj) => obj is NumberSequenceKeypoint other && Equals(other);
    public override int GetHashCode() => HashCode.Combine(Time, Value, Envelope);

    public static bool operator ==(NumberSequenceKeypoint a, NumberSequenceKeypoint b) => a.Equals(b);
    public static bool operator !=(NumberSequenceKeypoint a, NumberSequenceKeypoint b) => !a.Equals(b);

    public override string ToString() => $"{Time} {Value} {Envelope}";
}
