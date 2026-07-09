using RML.Interop;

namespace Roblox;

internal static unsafe class EngineEvents
{
    public static void Fire(nuint handle, string eventName, object?[]? args)
    {
        if (handle != 0)
        {
            Interop.Reflection.EventFire((void*)handle, eventName, args);
        }
    }

    public static void DisconnectAll(nuint handle, string eventName)
    {
        if (handle != 0)
        {
            Interop.Reflection.EventDisconnectAll((void*)handle, eventName);
        }
    }

    public static nuint[] Slots(nuint handle, string eventName)
        => handle == 0 ? [] : Interop.Reflection.EventSlots((void*)handle, eventName);

    public static void SlotFire(nuint instance, string eventName, nuint slotHandle, object?[]? args)
        => Interop.Reflection.EventSlotFire((void*)instance, eventName, slotHandle, args);
}
