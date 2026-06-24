using System.Runtime.InteropServices;

namespace Roblox;

[StructLayout(LayoutKind.Sequential)]
public readonly struct Faces : IEquatable<Faces>, IRobloxDataType
{
    private const int RightBit = 0x01;   // NORM_X      (+X)
    private const int TopBit = 0x02;     // NORM_Y      (+Y)
    private const int BackBit = 0x04;    // NORM_Z      (+Z)
    private const int LeftBit = 0x08;    // NORM_X_NEG  (-X)
    private const int BottomBit = 0x10;  // NORM_Y_NEG  (-Y)
    private const int FrontBit = 0x20;   // NORM_Z_NEG  (-Z)

    private readonly int _mask;

    private Faces(int mask) => _mask = mask;

    public Faces(
        bool top = false, bool bottom = false, bool left = false,
        bool right = false, bool back = false, bool front = false)
    {
        var mask = 0;
        if (top) mask |= TopBit;
        if (bottom) mask |= BottomBit;
        if (left) mask |= LeftBit;
        if (right) mask |= RightBit;
        if (back) mask |= BackBit;
        if (front) mask |= FrontBit;
        _mask = mask;
    }

    public bool Top => (_mask & TopBit) != 0;
    public bool Bottom => (_mask & BottomBit) != 0;
    public bool Left => (_mask & LeftBit) != 0;
    public bool Right => (_mask & RightBit) != 0;
    public bool Back => (_mask & BackBit) != 0;
    public bool Front => (_mask & FrontBit) != 0;

    public bool Equals(Faces other) => _mask == other._mask;
    public override bool Equals(object? obj) => obj is Faces other && Equals(other);
    public override int GetHashCode() => _mask;

    public static bool operator ==(Faces a, Faces b) => a._mask == b._mask;
    public static bool operator !=(Faces a, Faces b) => a._mask != b._mask;

    public override string ToString()
    {
        var parts = new List<string>(6);
        if (Top) parts.Add(nameof(Top));
        if (Bottom) parts.Add(nameof(Bottom));
        if (Left) parts.Add(nameof(Left));
        if (Right) parts.Add(nameof(Right));
        if (Back) parts.Add(nameof(Back));
        if (Front) parts.Add(nameof(Front));
        return string.Join(", ", parts);
    }
}
