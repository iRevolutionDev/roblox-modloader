namespace Roblox;

/// <summary>
///     Root base class for all Roblox engine objects.
///     Wraps a native (C++) pointer and exposes it via the reflection system.
/// </summary>
/// <remarks>
///     API members declared in the Roblox API dump are generated into
///     <c>Generated/Classes/Object.Members.cs</c>.  Do not edit that file manually.
/// </remarks>
public partial class Object
{
    internal readonly nuint Handle;

    internal Object(nuint handle)
    {
        if (handle == 0)
        {
            throw new ArgumentException("Cannot wrap a null native handle", nameof(handle));
        }

        Handle = handle;
    }

    public override bool Equals(object? obj)
        => obj is Object other && other.Handle == Handle;

    public override int GetHashCode() => Handle.GetHashCode();

    protected internal void AddEventHandler(string eventName, Delegate handler)
        => EventManager.Add(Handle, eventName, handler);

    protected internal void RemoveEventHandler(string eventName, Delegate handler)
        => EventManager.Remove(Handle, eventName, handler);
}