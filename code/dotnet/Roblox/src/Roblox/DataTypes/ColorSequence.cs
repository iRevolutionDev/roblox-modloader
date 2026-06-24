namespace Roblox;

public readonly struct ColorSequence : IEquatable<ColorSequence>
{
    private readonly ColorSequenceKeypoint[]? _keypoints;

    public IReadOnlyList<ColorSequenceKeypoint> Keypoints => _keypoints ?? [];

    public ColorSequence(Color3 color)
        : this([new ColorSequenceKeypoint(0f, color), new ColorSequenceKeypoint(1f, color)])
    {
    }

    public ColorSequence(Color3 color0, Color3 color1)
        : this([new ColorSequenceKeypoint(0f, color0), new ColorSequenceKeypoint(1f, color1)])
    {
    }

    public ColorSequence(ColorSequenceKeypoint[] keypoints)
    {
        ArgumentNullException.ThrowIfNull(keypoints);
        Validate(keypoints);
        _keypoints = (ColorSequenceKeypoint[])keypoints.Clone();
    }

    private ColorSequence(ColorSequenceKeypoint[] keypoints, bool _) => _keypoints = keypoints;
    
    internal static ColorSequence FromEngine(ColorSequenceKeypoint[] keypoints) => new(keypoints, false);

    private static void Validate(ColorSequenceKeypoint[] kp)
    {
        if (kp.Length < 2)
        {
            throw new ArgumentException("A ColorSequence needs at least 2 keypoints.", nameof(kp));
        }

        if (kp[0].Time != 0f || kp[^1].Time != 1f)
        {
            throw new ArgumentException("The first keypoint time must be 0 and the last must be 1.", nameof(kp));
        }

        for (var i = 1; i < kp.Length; i++)
        {
            if (kp[i].Time < kp[i - 1].Time)
            {
                throw new ArgumentException("Keypoint times must be non-decreasing.", nameof(kp));
            }
        }
    }

    public bool Equals(ColorSequence other)
    {
        var a = Keypoints;
        var b = other.Keypoints;
        if (a.Count != b.Count)
        {
            return false;
        }

        for (var i = 0; i < a.Count; i++)
        {
            if (!a[i].Equals(b[i]))
            {
                return false;
            }
        }

        return true;
    }

    public override bool Equals(object? obj) => obj is ColorSequence other && Equals(other);

    public override int GetHashCode()
    {
        var hash = new HashCode();
        foreach (var k in Keypoints)
        {
            hash.Add(k);
        }

        return hash.ToHashCode();
    }

    public static bool operator ==(ColorSequence a, ColorSequence b) => a.Equals(b);
    public static bool operator !=(ColorSequence a, ColorSequence b) => !a.Equals(b);
}
