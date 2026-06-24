using System.Runtime.InteropServices;

namespace Roblox;

[StructLayout(LayoutKind.Sequential)]
public readonly struct BrickColor : IEquatable<BrickColor>, IRobloxDataType
{
    private readonly int _number;

    /// <summary>Creates a <see cref="BrickColor"/> from a palette number (e.g. <c>1</c> = White).</summary>
    public BrickColor(int number) => _number = number;

    /// <summary>The palette number (the engine's <c>BrickColor::Number</c>).</summary>
    public int Number => _number == 0 ? BrickColorPalette.DefaultNumber : _number;

    /// <summary>The palette color name (e.g. "Bright red"). Unknown numbers fall back to the default.</summary>
    public string Name => BrickColorPalette.EntryOf(Number).Name;

    /// <summary>The color as a <see cref="Color3"/> (sRGB components scaled to 0..1).</summary>
    public Color3 Color
    {
        get
        {
            var e = BrickColorPalette.EntryOf(Number);
            return Color3.FromRGB(e.R, e.G, e.B);
        }
    }

    /// <summary>Red component, 0..1.</summary>
    public float R => Color.R;

    /// <summary>Green component, 0..1.</summary>
    public float G => Color.G;

    /// <summary>Blue component, 0..1.</summary>
    public float B => Color.B;

    /// <summary>The engine default, <c>brick_194</c> "Medium stone grey".</summary>
    public static BrickColor Default => new(BrickColorPalette.DefaultNumber);

    /// <summary>Looks up a <see cref="BrickColor"/> by palette number.</summary>
    public static BrickColor FromNumber(int number) => new(number);

    /// <summary>
    /// Looks up a <see cref="BrickColor"/> by palette name (e.g. "Really red"). Returns the engine
    /// default when the name is unknown.
    /// </summary>
    public static BrickColor FromName(string name)
        => BrickColorPalette.TryGetNumberByName(name, out var number) ? new BrickColor(number) : Default;

    public bool Equals(BrickColor other) => Number == other.Number;
    public override bool Equals(object? obj) => obj is BrickColor other && Equals(other);
    public override int GetHashCode() => Number;

    public static bool operator ==(BrickColor a, BrickColor b) => a.Number == b.Number;
    public static bool operator !=(BrickColor a, BrickColor b) => a.Number != b.Number;

    public override string ToString() => Name;
}
