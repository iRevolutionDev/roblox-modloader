using System.Runtime.InteropServices;

namespace Roblox;

[StructLayout(LayoutKind.Sequential)]
public readonly struct Vector2 : IEquatable<Vector2>, IRobloxDataType
{
    public float X { get; }
    public float Y { get; }

    public Vector2(float x, float y)
    {
        X = x;
        Y = y;
    }

    public static readonly Vector2 Zero = new(0f, 0f);
    public static readonly Vector2 One = new(1f, 1f);
    public static readonly Vector2 XAxis = new(1f, 0f);
    public static readonly Vector2 YAxis = new(0f, 1f);

    public float Magnitude => MathF.Sqrt(X * X + Y * Y);

    public Vector2 Unit
    {
        get
        {
            var m = Magnitude;
            return m == 0f ? Zero : this / m;
        }
    }

    public float Dot(Vector2 other) => X * other.X + Y * other.Y;

    public Vector2 Lerp(Vector2 goal, float alpha) => this + (goal - this) * alpha;

    public Vector2 Abs() => new(MathF.Abs(X), MathF.Abs(Y));

    public Vector2 Min(Vector2 other) => new(MathF.Min(X, other.X), MathF.Min(Y, other.Y));

    public Vector2 Max(Vector2 other) => new(MathF.Max(X, other.X), MathF.Max(Y, other.Y));

    public static Vector2 operator +(Vector2 a, Vector2 b) => new(a.X + b.X, a.Y + b.Y);
    public static Vector2 operator -(Vector2 a, Vector2 b) => new(a.X - b.X, a.Y - b.Y);
    public static Vector2 operator -(Vector2 v) => new(-v.X, -v.Y);
    public static Vector2 operator *(Vector2 a, Vector2 b) => new(a.X * b.X, a.Y * b.Y);
    public static Vector2 operator *(Vector2 v, float s) => new(v.X * s, v.Y * s);
    public static Vector2 operator *(float s, Vector2 v) => new(v.X * s, v.Y * s);
    public static Vector2 operator /(Vector2 a, Vector2 b) => new(a.X / b.X, a.Y / b.Y);
    public static Vector2 operator /(Vector2 v, float s) => new(v.X / s, v.Y / s);

    public bool Equals(Vector2 other) => X.Equals(other.X) && Y.Equals(other.Y);
    public override bool Equals(object? obj) => obj is Vector2 other && Equals(other);
    public override int GetHashCode() => HashCode.Combine(X, Y);

    public static bool operator ==(Vector2 a, Vector2 b) => a.Equals(b);
    public static bool operator !=(Vector2 a, Vector2 b) => !a.Equals(b);

    public override string ToString() => $"{X}, {Y}";
}
