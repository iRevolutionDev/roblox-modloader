namespace Roblox;

public readonly struct NumberSequence : IEquatable<NumberSequence>
{
    private readonly NumberSequenceKeypoint[]? _keypoints;

    public IReadOnlyList<NumberSequenceKeypoint> Keypoints => _keypoints ?? [];

    public NumberSequence(float value)
        : this([new NumberSequenceKeypoint(0f, value), new NumberSequenceKeypoint(1f, value)])
    {
    }

    public NumberSequence(float value0, float value1)
        : this([new NumberSequenceKeypoint(0f, value0), new NumberSequenceKeypoint(1f, value1)])
    {
    }

    public NumberSequence(NumberSequenceKeypoint[] keypoints)
    {
        ArgumentNullException.ThrowIfNull(keypoints);
        Validate(keypoints);
        _keypoints = (NumberSequenceKeypoint[])keypoints.Clone();
    }

    private NumberSequence(NumberSequenceKeypoint[] keypoints, bool _) => _keypoints = keypoints;
    
    internal static NumberSequence FromEngine(NumberSequenceKeypoint[] keypoints) => new(keypoints, false);

    private static void Validate(NumberSequenceKeypoint[] kp)
    {
        if (kp.Length < 2)
        {
            throw new ArgumentException("A NumberSequence needs at least 2 keypoints.", nameof(kp));
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

    public bool Equals(NumberSequence other)
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

    public override bool Equals(object? obj) => obj is NumberSequence other && Equals(other);

    public override int GetHashCode()
    {
        var hash = new HashCode();
        foreach (var k in Keypoints)
        {
            hash.Add(k);
        }

        return hash.ToHashCode();
    }

    public static bool operator ==(NumberSequence a, NumberSequence b) => a.Equals(b);
    public static bool operator !=(NumberSequence a, NumberSequence b) => !a.Equals(b);
}
