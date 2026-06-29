using System.Runtime.InteropServices;

namespace Roblox;

[StructLayout(LayoutKind.Sequential)]
public readonly struct CFrame : IEquatable<CFrame>, IRobloxDataType
{
    private readonly float _r00, _r01, _r02;
    private readonly float _r10, _r11, _r12;
    private readonly float _r20, _r21, _r22;
    private readonly float _tx, _ty, _tz;

    /// <summary>The translation (position) component.</summary>
    public Vector3 Position => new(_tx, _ty, _tz);

    public CFrame(
        Vector3 position,
        float r00, float r01, float r02,
        float r10, float r11, float r12,
        float r20, float r21, float r22)
    {
        _r00 = r00;
        _r01 = r01;
        _r02 = r02;
        _r10 = r10;
        _r11 = r11;
        _r12 = r12;
        _r20 = r20;
        _r21 = r21;
        _r22 = r22;
        _tx = position.X;
        _ty = position.Y;
        _tz = position.Z;
    }

    public CFrame(float x, float y, float z,
        float r00, float r01, float r02,
        float r10, float r11, float r12,
        float r20, float r21, float r22)
        : this(new Vector3(x, y, z), r00, r01, r02, r10, r11, r12, r20, r21, r22)
    {
    }

    public CFrame(Vector3 position) : this(position, 1, 0, 0, 0, 1, 0, 0, 0, 1)
    {
    }

    public CFrame(float x, float y, float z) : this(new Vector3(x, y, z))
    {
    }

    /// <summary>A CFrame positioned at <paramref name="position" /> oriented to look at <paramref name="lookAt" />.</summary>
    public CFrame(Vector3 position, Vector3 lookAt) : this(LookAt(position, lookAt))
    {
    }

    private CFrame(in CFrame other)
    {
        _r00 = other._r00;
        _r01 = other._r01;
        _r02 = other._r02;
        _r10 = other._r10;
        _r11 = other._r11;
        _r12 = other._r12;
        _r20 = other._r20;
        _r21 = other._r21;
        _r22 = other._r22;
        _tx = other._tx;
        _ty = other._ty;
        _tz = other._tz;
    }

    public static readonly CFrame Identity = new(Vector3.Zero, 1, 0, 0, 0, 1, 0, 0, 0, 1);

    public float X => Position.X;
    public float Y => Position.Y;
    public float Z => Position.Z;

    /// <summary>Column 0 of the rotation matrix.</summary>
    public Vector3 RightVector => new(_r00, _r10, _r20);

    /// <summary>Column 1 of the rotation matrix.</summary>
    public Vector3 UpVector => new(_r01, _r11, _r21);

    /// <summary>Negative column 2 — the direction the CFrame faces, as in Roblox.</summary>
    public Vector3 LookVector => new(-_r02, -_r12, -_r22);

    /// <summary>The 12 components in engine order: x, y, z, R00..R22.</summary>
    public (float, float, float, float, float, float, float, float, float, float, float, float) GetComponents()
        => (_tx, _ty, _tz, _r00, _r01, _r02, _r10, _r11, _r12, _r20, _r21, _r22);

    /// <summary>Rotates a direction by the rotation part (no translation).</summary>
    public Vector3 VectorToWorldSpace(Vector3 v) => new(
        _r00 * v.X + _r01 * v.Y + _r02 * v.Z,
        _r10 * v.X + _r11 * v.Y + _r12 * v.Z,
        _r20 * v.X + _r21 * v.Y + _r22 * v.Z);

    /// <summary>Rotates a direction by the inverse rotation (transpose).</summary>
    public Vector3 VectorToObjectSpace(Vector3 v) => new(
        _r00 * v.X + _r10 * v.Y + _r20 * v.Z,
        _r01 * v.X + _r11 * v.Y + _r21 * v.Z,
        _r02 * v.X + _r12 * v.Y + _r22 * v.Z);

    public Vector3 PointToWorldSpace(Vector3 v) => Position + VectorToWorldSpace(v);

    public Vector3 PointToObjectSpace(Vector3 v) => VectorToObjectSpace(v - Position);

    public CFrame Inverse()
    {
        // Rigid inverse: rotation = transpose, translation = -transpose * position.
        var invPos = new Vector3(
            -(_r00 * Position.X + _r10 * Position.Y + _r20 * Position.Z),
            -(_r01 * Position.X + _r11 * Position.Y + _r21 * Position.Z),
            -(_r02 * Position.X + _r12 * Position.Y + _r22 * Position.Z));

        return new CFrame(invPos,
            _r00, _r10, _r20,
            _r01, _r11, _r21,
            _r02, _r12, _r22);
    }

    public CFrame ToWorldSpace(CFrame cf) => this * cf;

    public CFrame ToObjectSpace(CFrame cf) => Inverse() * cf;

    public CFrame Lerp(CFrame goal, float alpha)
    {
        Vector3 pos = Position + (goal.Position - Position) * alpha;
        var (qx, qy, qz, qw) = Slerp(ToQuaternion(this), ToQuaternion(goal), alpha);
        return FromQuaternion(pos, qx, qy, qz, qw);
    }

    public static CFrame LookAt(Vector3 eye, Vector3 target, Vector3? up = null)
    {
        Vector3 upVec = up ?? Vector3.YAxis;
        Vector3 zaxis = (eye - target).Unit; // engine look is -Z, so +Z points back toward the eye
        Vector3 xaxis = upVec.Cross(zaxis).Unit; // right
        Vector3 yaxis = zaxis.Cross(xaxis); // orthonormal up

        return new CFrame(eye,
            xaxis.X, yaxis.X, zaxis.X,
            xaxis.Y, yaxis.Y, zaxis.Y,
            xaxis.Z, yaxis.Z, zaxis.Z);
    }

    public static CFrame operator *(CFrame a, CFrame b)
    {
        // position = a.position + a.R * b.position ; rotation = a.R * b.R
        Vector3 pos = a.Position + a.VectorToWorldSpace(b.Position);

        return new CFrame(pos,
            a._r00 * b._r00 + a._r01 * b._r10 + a._r02 * b._r20,
            a._r00 * b._r01 + a._r01 * b._r11 + a._r02 * b._r21,
            a._r00 * b._r02 + a._r01 * b._r12 + a._r02 * b._r22,
            a._r10 * b._r00 + a._r11 * b._r10 + a._r12 * b._r20,
            a._r10 * b._r01 + a._r11 * b._r11 + a._r12 * b._r21,
            a._r10 * b._r02 + a._r11 * b._r12 + a._r12 * b._r22,
            a._r20 * b._r00 + a._r21 * b._r10 + a._r22 * b._r20,
            a._r20 * b._r01 + a._r21 * b._r11 + a._r22 * b._r21,
            a._r20 * b._r02 + a._r21 * b._r12 + a._r22 * b._r22);
    }

    public static Vector3 operator *(CFrame cf, Vector3 v) => cf.PointToWorldSpace(v);

    public static CFrame operator +(CFrame cf, Vector3 v) => new(cf.Position + v,
        cf._r00, cf._r01, cf._r02, cf._r10, cf._r11, cf._r12, cf._r20, cf._r21, cf._r22);

    public static CFrame operator -(CFrame cf, Vector3 v) => new(cf.Position - v,
        cf._r00, cf._r01, cf._r02, cf._r10, cf._r11, cf._r12, cf._r20, cf._r21, cf._r22);

    private static (float x, float y, float z, float w) ToQuaternion(CFrame c)
    {
        var trace = c._r00 + c._r11 + c._r22;
        float x, y, z, w;
        if (trace > 0f)
        {
            var s = MathF.Sqrt(trace + 1f) * 2f;
            w = 0.25f * s;
            x = (c._r21 - c._r12) / s;
            y = (c._r02 - c._r20) / s;
            z = (c._r10 - c._r01) / s;
        }
        else if (c._r00 > c._r11 && c._r00 > c._r22)
        {
            var s = MathF.Sqrt(1f + c._r00 - c._r11 - c._r22) * 2f;
            w = (c._r21 - c._r12) / s;
            x = 0.25f * s;
            y = (c._r01 + c._r10) / s;
            z = (c._r02 + c._r20) / s;
        }
        else if (c._r11 > c._r22)
        {
            var s = MathF.Sqrt(1f + c._r11 - c._r00 - c._r22) * 2f;
            w = (c._r02 - c._r20) / s;
            x = (c._r01 + c._r10) / s;
            y = 0.25f * s;
            z = (c._r12 + c._r21) / s;
        }
        else
        {
            var s = MathF.Sqrt(1f + c._r22 - c._r00 - c._r11) * 2f;
            w = (c._r10 - c._r01) / s;
            x = (c._r02 + c._r20) / s;
            y = (c._r12 + c._r21) / s;
            z = 0.25f * s;
        }

        return (x, y, z, w);
    }

    private static (float x, float y, float z, float w) Slerp(
        (float x, float y, float z, float w) a, (float x, float y, float z, float w) b, float t)
    {
        var dot = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
        if (dot < 0f)
        {
            b = (-b.x, -b.y, -b.z, -b.w);
            dot = -dot;
        }

        if (dot > 0.9995f)
        {
            // Nearly parallel — linearly interpolate and renormalize.
            var lx = a.x + (b.x - a.x) * t;
            var ly = a.y + (b.y - a.y) * t;
            var lz = a.z + (b.z - a.z) * t;
            var lw = a.w + (b.w - a.w) * t;
            var len = MathF.Sqrt(lx * lx + ly * ly + lz * lz + lw * lw);
            return (lx / len, ly / len, lz / len, lw / len);
        }

        var theta0 = MathF.Acos(dot);
        var theta = theta0 * t;
        var sinTheta = MathF.Sin(theta);
        var sinTheta0 = MathF.Sin(theta0);

        var s0 = MathF.Cos(theta) - dot * sinTheta / sinTheta0;
        var s1 = sinTheta / sinTheta0;

        return (
            a.x * s0 + b.x * s1,
            a.y * s0 + b.y * s1,
            a.z * s0 + b.z * s1,
            a.w * s0 + b.w * s1);
    }

    private static CFrame FromQuaternion(Vector3 pos, float x, float y, float z, float w)
    {
        var xx = x * x;
        var yy = y * y;
        var zz = z * z;
        var xy = x * y;
        var xz = x * z;
        var yz = y * z;
        var wx = w * x;
        var wy = w * y;
        var wz = w * z;

        return new CFrame(pos,
            1f - 2f * (yy + zz), 2f * (xy - wz), 2f * (xz + wy),
            2f * (xy + wz), 1f - 2f * (xx + zz), 2f * (yz - wx),
            2f * (xz - wy), 2f * (yz + wx), 1f - 2f * (xx + yy));
    }

    public static CFrame Angles(float x, float y, float z)
    {
        var cx = MathF.Cos(x);
        var sx = MathF.Sin(x);
        var cy = MathF.Cos(y);
        var sy = MathF.Sin(y);
        var cz = MathF.Cos(z);
        var sz = MathF.Sin(z);

        return new CFrame(Vector3.Zero,
            cy * cz, -cy * sz, sy,
            sx * sy * cz + cx * sz, -sx * sy * sz + cx * cz, -sx * cy,
            -cx * sy * cz + sx * sz, cx * sy * sz + sx * cz, cx * cy);
    }

    public bool Equals(CFrame other) =>
        Position.Equals(other.Position) &&
        _r00.Equals(other._r00) && _r01.Equals(other._r01) && _r02.Equals(other._r02) &&
        _r10.Equals(other._r10) && _r11.Equals(other._r11) && _r12.Equals(other._r12) &&
        _r20.Equals(other._r20) && _r21.Equals(other._r21) && _r22.Equals(other._r22);

    public override bool Equals(object? obj) => obj is CFrame other && Equals(other);

    public override int GetHashCode() => HashCode.Combine(Position, _r00, _r11, _r22, _r01, _r02, _r10);

    public static bool operator ==(CFrame a, CFrame b) => a.Equals(b);
    public static bool operator !=(CFrame a, CFrame b) => !a.Equals(b);

    public override string ToString()
        =>
            $"{Position.X}, {Position.Y}, {Position.Z}, {_r00}, {_r01}, {_r02}, {_r10}, {_r11}, {_r12}, {_r20}, {_r21}, {_r22}";
}