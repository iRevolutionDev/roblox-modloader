namespace Roblox;

public readonly struct Faces : IEquatable<Faces>
{
    public bool Top { get; }
    public bool Bottom { get; }
    public bool Left { get; }
    public bool Right { get; }
    public bool Back { get; }
    public bool Front { get; }

    public Faces(
        bool top = false, bool bottom = false, bool left = false,
        bool right = false, bool back = false, bool front = false)
    {
        Top = top;
        Bottom = bottom;
        Left = left;
        Right = right;
        Back = back;
        Front = front;
    }

    public bool Equals(Faces other) =>
        Top == other.Top && Bottom == other.Bottom && Left == other.Left &&
        Right == other.Right && Back == other.Back && Front == other.Front;

    public override bool Equals(object? obj) => obj is Faces other && Equals(other);
    public override int GetHashCode() => HashCode.Combine(Top, Bottom, Left, Right, Back, Front);

    public static bool operator ==(Faces a, Faces b) => a.Equals(b);
    public static bool operator !=(Faces a, Faces b) => !a.Equals(b);
}
