using System.Runtime.InteropServices;

namespace Roblox;

[StructLayout(LayoutKind.Sequential)]
public readonly struct Color3 : IEquatable<Color3>, IRobloxDataType
{
    public float R { get; }
    public float G { get; }
    public float B { get; }

    public Color3(float r, float g, float b)
    {
        R = r;
        G = g;
        B = b;
    }

    /// <summary>Equivalent to <c>Color3.fromRGB</c>: components in 0..255.</summary>
    public static Color3 FromRGB(int r, int g, int b) => new(r / 255f, g / 255f, b / 255f);

    /// <summary>Equivalent to <c>Color3.fromHSV</c>: hue/saturation/value in 0..1.</summary>
    public static Color3 FromHSV(float h, float s, float v)
    {
        if (s <= 0f)
        {
            return new Color3(v, v, v);
        }

        h = (h - MathF.Floor(h)) * 6f;
        var i = (int)h;
        var f = h - i;
        var p = v * (1f - s);
        var q = v * (1f - s * f);
        var t = v * (1f - s * (1f - f));

        return i switch
        {
            0 => new Color3(v, t, p),
            1 => new Color3(q, v, p),
            2 => new Color3(p, v, t),
            3 => new Color3(p, q, v),
            4 => new Color3(t, p, v),
            _ => new Color3(v, p, q),
        };
    }

    /// <summary>Equivalent to <c>Color3.fromHex</c>: "#RRGGBB" or "RRGGBB".</summary>
    public static Color3 FromHex(string hex)
    {
        ArgumentNullException.ThrowIfNull(hex);
        var s = hex.StartsWith('#') ? hex[1..] : hex;
        if (s.Length != 6)
        {
            throw new ArgumentException($"Expected a 6-digit hex color, got '{hex}'.", nameof(hex));
        }

        var r = Convert.ToInt32(s[..2], 16);
        var g = Convert.ToInt32(s.Substring(2, 2), 16);
        var b = Convert.ToInt32(s.Substring(4, 2), 16);
        return FromRGB(r, g, b);
    }

    public Color3 Lerp(Color3 goal, float alpha) => new(
        R + (goal.R - R) * alpha,
        G + (goal.G - G) * alpha,
        B + (goal.B - B) * alpha);

    public string ToHex()
    {
        static int Clamp255(float c) => Math.Clamp((int)MathF.Round(c * 255f), 0, 255);
        return $"{Clamp255(R):X2}{Clamp255(G):X2}{Clamp255(B):X2}";
    }

    public bool Equals(Color3 other) => R.Equals(other.R) && G.Equals(other.G) && B.Equals(other.B);
    public override bool Equals(object? obj) => obj is Color3 other && Equals(other);
    public override int GetHashCode() => HashCode.Combine(R, G, B);

    public static bool operator ==(Color3 a, Color3 b) => a.Equals(b);
    public static bool operator !=(Color3 a, Color3 b) => !a.Equals(b);

    public override string ToString() => $"{R}, {G}, {B}";
}
