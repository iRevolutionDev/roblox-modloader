using RML.Interop;

namespace Roblox;

public sealed class EventConnection : IDisposable
{
    private readonly nuint _instance;
    private readonly string _eventName;
    private nuint _handle;

    internal EventConnection(nuint instance, string eventName, nuint handle)
    {
        _instance = instance;
        _eventName = eventName;
        _handle = handle;
    }

    public void Disconnect()
    {
        if (_handle != 0)
        {
            Interop.Reflection.EventSlotDisconnect(_handle);
        }
    }

    public void Fire(params object?[] args)
    {
        if (_handle != 0)
        {
            EngineEvents.SlotFire(_instance, _eventName, _handle, args);
        }
    }

    public void Dispose()
    {
        if (_handle != 0)
        {
            Interop.Reflection.EventSlotRelease(_handle);
            _handle = 0;
        }

        GC.SuppressFinalize(this);
    }

    ~EventConnection()
    {
        if (_handle != 0)
        {
            Interop.Reflection.EventSlotRelease(_handle);
        }
    }
}
