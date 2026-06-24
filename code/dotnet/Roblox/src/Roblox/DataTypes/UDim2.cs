using System.Runtime.InteropServices;

namespace Roblox;

[StructLayout(LayoutKind.Sequential)]
public readonly struct UDim2 : IEquatable<UDim2>, IRobloxDataType
{
    public UDim X { get; }
    public UDim Y { get; }

    public UDim2(UDim x, UDim y)
    {
        X = x;
        Y = y;
    }

    public UDim2(float xScale, int xOffset, float yScale, int yOffset)
        : this(new UDim(xScale, xOffset), new UDim(yScale, yOffset))
    {
    }

    public static readonly UDim2 Zero = new(UDim.Zero, UDim.Zero);

    public static UDim2 FromScale(float xScale, float yScale) => new(new UDim(xScale, 0), new UDim(yScale, 0));

    public static UDim2 FromOffset(int xOffset, int yOffset) => new(new UDim(0f, xOffset), new UDim(0f, yOffset));

    public Vector2 Width => new(X.Scale, Y.Scale);

    public UDim2 Lerp(UDim2 goal, float alpha) => new(
        new UDim(
            X.Scale + (goal.X.Scale - X.Scale) * alpha,
            (int)MathF.Round(X.Offset + (goal.X.Offset - X.Offset) * alpha)),
        new UDim(
            Y.Scale + (goal.Y.Scale - Y.Scale) * alpha,
            (int)MathF.Round(Y.Offset + (goal.Y.Offset - Y.Offset) * alpha)));

    public static UDim2 operator +(UDim2 a, UDim2 b) => new(a.X + b.X, a.Y + b.Y);
    public static UDim2 operator -(UDim2 a, UDim2 b) => new(a.X - b.X, a.Y - b.Y);

    public bool Equals(UDim2 other) => X.Equals(other.X) && Y.Equals(other.Y);
    public override bool Equals(object? obj) => obj is UDim2 other && Equals(other);
    public override int GetHashCode() => HashCode.Combine(X, Y);

    public static bool operator ==(UDim2 a, UDim2 b) => a.Equals(b);
    public static bool operator !=(UDim2 a, UDim2 b) => !a.Equals(b);

    public override string ToString() => $"{{{X}}}, {{{Y}}}";
}
