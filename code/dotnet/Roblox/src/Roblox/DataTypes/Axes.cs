namespace Roblox;

public readonly struct Axes : IEquatable<Axes>
{
    public bool X { get; }
    public bool Y { get; }
    public bool Z { get; }

    public Axes(bool x = false, bool y = false, bool z = false)
    {
        X = x;
        Y = y;
        Z = z;
    }

    public bool Equals(Axes other) => X == other.X && Y == other.Y && Z == other.Z;
    public override bool Equals(object? obj) => obj is Axes other && Equals(other);
    public override int GetHashCode() => HashCode.Combine(X, Y, Z);

    public static bool operator ==(Axes a, Axes b) => a.Equals(b);
    public static bool operator !=(Axes a, Axes b) => !a.Equals(b);
}
