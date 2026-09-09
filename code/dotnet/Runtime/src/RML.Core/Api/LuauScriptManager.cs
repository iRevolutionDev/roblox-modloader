using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

using RML.Interop;

using Roblox;

using InteropApi = RML.Interop.Interop;

namespace RML.Core.Api;

public static unsafe class LuauScriptManager
{
    public static bool IsReady(DataModelType context) => InteropApi.LuauHostReady((int)context);

    public static Task ScheduleAsync(DataModelType context, string source, string? chunkName = null)
    {
        ArgumentNullException.ThrowIfNull(source);

        var tcs = new TaskCompletionSource<LuauValue>(TaskCreationOptions.RunContinuationsAsynchronously);
        var gcHandle = GCHandle.Alloc(new AsyncCall(tcs));

        try
        {
            var dispatched = InteropApi.LuauSchedule(
                (int)context, source, chunkName, TrampolinePointer, GCHandle.ToIntPtr(gcHandle));

            if (!dispatched)
            {
                gcHandle.Free();
                return Task.FromException(Unavailable());
            }
        }
        catch
        {
            gcHandle.Free();
            throw;
        }

        return tcs.Task;
    }

    public static Task<LuauValue> EvaluateAsync(DataModelType context, string source, string? chunkName = null)
    {
        ArgumentNullException.ThrowIfNull(source);

        var tcs = new TaskCompletionSource<LuauValue>(TaskCreationOptions.RunContinuationsAsynchronously);
        var gcHandle = GCHandle.Alloc(new AsyncCall(tcs));

        try
        {
            var dispatched = InteropApi.LuauEvaluate(
                (int)context, source, chunkName, TrampolinePointer, GCHandle.ToIntPtr(gcHandle));

            if (!dispatched)
            {
                gcHandle.Free();
                return Task.FromException<LuauValue>(Unavailable());
            }
        }
        catch
        {
            gcHandle.Free();
            throw;
        }

        return tcs.Task;
    }

    internal static Task<LuauValue> CallRefAsync(LuauRef reference, object?[]? args)
    {
        ArgumentNullException.ThrowIfNull(reference);

        if (reference.IsReleased)
        {
            return Task.FromException<LuauValue>(new ObjectDisposedException(nameof(LuauRef)));
        }

        var tcs = new TaskCompletionSource<LuauValue>(TaskCreationOptions.RunContinuationsAsynchronously);
        var gcHandle = GCHandle.Alloc(new AsyncCall(tcs));

        try
        {
            if (!InteropApi.LuauRefCall(reference.Handle, args, TrampolinePointer, GCHandle.ToIntPtr(gcHandle)))
            {
                gcHandle.Free();
                return Task.FromException<LuauValue>(Unavailable());
            }
        }
        catch
        {
            gcHandle.Free();
            throw;
        }

        return tcs.Task;
    }

    internal static Task<LuauValue> IndexRefAsync(LuauRef reference, string key)
    {
        ArgumentNullException.ThrowIfNull(reference);
        ArgumentException.ThrowIfNullOrEmpty(key);

        if (reference.IsReleased)
        {
            return Task.FromException<LuauValue>(new ObjectDisposedException(nameof(LuauRef)));
        }

        var tcs = new TaskCompletionSource<LuauValue>(TaskCreationOptions.RunContinuationsAsynchronously);
        var gcHandle = GCHandle.Alloc(new AsyncCall(tcs));

        try
        {
            if (!InteropApi.LuauRefIndex(reference.Handle, key, TrampolinePointer, GCHandle.ToIntPtr(gcHandle)))
            {
                gcHandle.Free();
                return Task.FromException<LuauValue>(Unavailable());
            }
        }
        catch
        {
            gcHandle.Free();
            throw;
        }

        return tcs.Task;
    }

    private static nint TrampolinePointer =>
        (nint)(delegate* unmanaged[Cdecl]<void*, InteropVariant*, sbyte*, void>)&Complete;

    private static InvalidOperationException Unavailable() =>
        new("The Luau interop slots are unavailable; the loader is not initialized or was built without Luau support.");

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    private static void Complete(void* state, InteropVariant* result, sbyte* error)
    {
        var handle = GCHandle.FromIntPtr((nint)state);
        try
        {
            var call = (AsyncCall)handle.Target!;
            var message = error == null ? null : Marshal.PtrToStringUTF8((nint)error);
            call.Complete(result == null ? InteropVariant.Null : *result, message);
        }
        catch (Exception ex)
        {
            Console.Error.WriteLine($"[RML/Error] Luau completion callback threw: {ex}");
        }
        finally
        {
            handle.Free();
        }
    }

    private sealed class AsyncCall(TaskCompletionSource<LuauValue> source)
    {
        public void Complete(InteropVariant variant, string? error)
        {
            if (error is not null)
            {
                source.TrySetException(new InvalidOperationException(error));
                return;
            }

            try
            {
                source.TrySetResult(LuauValue.FromVariant(variant));
            }
            catch (Exception ex)
            {
                source.TrySetException(ex);
            }
        }
    }
}
