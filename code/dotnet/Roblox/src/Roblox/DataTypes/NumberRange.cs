using System.Runtime.InteropServices;

namespace Roblox;

[StructLayout(LayoutKind.Sequential)]
public readonly struct NumberRange : IEquatable<NumberRange>, IRobloxDataType
{
    public float Min { get; }
    public float Max { get; }

    public NumberRange(float min, float max)
    {
        if (max < min)
        {
            throw new ArgumentException($"NumberRange max ({max}) cannot be less than min ({min}).");
        }

        Min = min;
        Max = max;
    }

    public NumberRange(float value) : this(value, value)
    {
    }

    public bool Equals(NumberRange other) => Min.Equals(other.Min) && Max.Equals(other.Max);
    public override bool Equals(object? obj) => obj is NumberRange other && Equals(other);
    public override int GetHashCode() => HashCode.Combine(Min, Max);

    public static bool operator ==(NumberRange a, NumberRange b) => a.Equals(b);
    public static bool operator !=(NumberRange a, NumberRange b) => !a.Equals(b);

    public override string ToString() => $"{Min} {Max}";
}
