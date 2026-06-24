using System.Runtime.InteropServices;

namespace Roblox;

[StructLayout(LayoutKind.Sequential)]
public readonly struct Vector3 : IEquatable<Vector3>, IRobloxDataType
{
    public float X { get; }
    public float Y { get; }
    public float Z { get; }

    public Vector3(float x, float y, float z)
    {
        X = x;
        Y = y;
        Z = z;
    }

    public Vector3(float x, float y) : this(x, y, 0f)
    {
    }

    public static readonly Vector3 Zero = new(0f, 0f, 0f);
    public static readonly Vector3 One = new(1f, 1f, 1f);
    public static readonly Vector3 XAxis = new(1f, 0f, 0f);
    public static readonly Vector3 YAxis = new(0f, 1f, 0f);
    public static readonly Vector3 ZAxis = new(0f, 0f, 1f);

    /// <summary>The length of the vector.</summary>
    public float Magnitude => MathF.Sqrt(X * X + Y * Y + Z * Z);

    /// <summary>The unit vector (direction) with magnitude 1; <see cref="Zero"/> stays zero.</summary>
    public Vector3 Unit
    {
        get
        {
            var m = Magnitude;
            return m == 0f ? Zero : this / m;
        }
    }

    public float Dot(Vector3 other) => X * other.X + Y * other.Y + Z * other.Z;

    public Vector3 Cross(Vector3 other) => new(
        Y * other.Z - Z * other.Y,
        Z * other.X - X * other.Z,
        X * other.Y - Y * other.X);

    public Vector3 Lerp(Vector3 goal, float alpha) => this + (goal - this) * alpha;

    public Vector3 Abs() => new(MathF.Abs(X), MathF.Abs(Y), MathF.Abs(Z));

    public Vector3 Min(Vector3 other) => new(MathF.Min(X, other.X), MathF.Min(Y, other.Y), MathF.Min(Z, other.Z));

    public Vector3 Max(Vector3 other) => new(MathF.Max(X, other.X), MathF.Max(Y, other.Y), MathF.Max(Z, other.Z));

    public bool FuzzyEq(Vector3 other, float epsilon = 1e-5f) =>
        MathF.Abs(X - other.X) <= epsilon &&
        MathF.Abs(Y - other.Y) <= epsilon &&
        MathF.Abs(Z - other.Z) <= epsilon;

    public static Vector3 operator +(Vector3 a, Vector3 b) => new(a.X + b.X, a.Y + b.Y, a.Z + b.Z);
    public static Vector3 operator -(Vector3 a, Vector3 b) => new(a.X - b.X, a.Y - b.Y, a.Z - b.Z);
    public static Vector3 operator -(Vector3 v) => new(-v.X, -v.Y, -v.Z);

    // Roblox multiplies/divides component-wise for Vector3 operands, and by scalar for numbers.
    public static Vector3 operator *(Vector3 a, Vector3 b) => new(a.X * b.X, a.Y * b.Y, a.Z * b.Z);
    public static Vector3 operator *(Vector3 v, float s) => new(v.X * s, v.Y * s, v.Z * s);
    public static Vector3 operator *(float s, Vector3 v) => new(v.X * s, v.Y * s, v.Z * s);
    public static Vector3 operator /(Vector3 a, Vector3 b) => new(a.X / b.X, a.Y / b.Y, a.Z / b.Z);
    public static Vector3 operator /(Vector3 v, float s) => new(v.X / s, v.Y / s, v.Z / s);

    public bool Equals(Vector3 other) => X.Equals(other.X) && Y.Equals(other.Y) && Z.Equals(other.Z);
    public override bool Equals(object? obj) => obj is Vector3 other && Equals(other);
    public override int GetHashCode() => HashCode.Combine(X, Y, Z);

    public static bool operator ==(Vector3 a, Vector3 b) => a.Equals(b);
    public static bool operator !=(Vector3 a, Vector3 b) => !a.Equals(b);

    public override string ToString() => $"{X}, {Y}, {Z}";
}
