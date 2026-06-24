using System.Runtime.InteropServices;

namespace Roblox;

[StructLayout(LayoutKind.Sequential)]
public readonly struct Axes : IEquatable<Axes>, IRobloxDataType
{
    private const int XBit = 0x01;
    private const int YBit = 0x02;
    private const int ZBit = 0x04;

    private readonly int _mask;

    private Axes(int mask) => _mask = mask;

    public Axes(bool x = false, bool y = false, bool z = false)
    {
        var mask = 0;
        if (x) mask |= XBit;
        if (y) mask |= YBit;
        if (z) mask |= ZBit;
        _mask = mask;
    }

    public bool X => (_mask & XBit) != 0;
    public bool Y => (_mask & YBit) != 0;
    public bool Z => (_mask & ZBit) != 0;

    public bool Equals(Axes other) => _mask == other._mask;
    public override bool Equals(object? obj) => obj is Axes other && Equals(other);
    public override int GetHashCode() => _mask;

    public static bool operator ==(Axes a, Axes b) => a._mask == b._mask;
    public static bool operator !=(Axes a, Axes b) => a._mask != b._mask;

    public override string ToString()
    {
        var parts = new List<string>(3);
        if (X) parts.Add(nameof(X));
        if (Y) parts.Add(nameof(Y));
        if (Z) parts.Add(nameof(Z));
        return string.Join(", ", parts);
    }
}
