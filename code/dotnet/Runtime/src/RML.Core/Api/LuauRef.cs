using InteropApi = RML.Interop.Interop;

namespace RML.Core.Api;

public sealed class LuauRef : IDisposable
{
    private readonly nuint _handle;
    private int _released;

    internal LuauRef(nuint handle)
    {
        _handle = handle;
    }

    public nuint Handle => _handle;

    public bool IsReleased => Volatile.Read(ref _released) != 0;

    public Task<LuauValue> InvokeAsync(params object?[] args) => LuauScriptManager.CallRefAsync(this, args);

    public Task<LuauValue> IndexAsync(string key) => LuauScriptManager.IndexRefAsync(this, key);

    public void Dispose()
    {
        Release();
        GC.SuppressFinalize(this);
    }

    ~LuauRef()
    {
        Release();
    }

    private void Release()
    {
        if (Interlocked.Exchange(ref _released, 1) != 0)
        {
            return;
        }

        if (_handle == 0)
        {
            return;
        }

        try
        {
            InteropApi.LuauRefRelease(_handle);
        }
        catch (Exception ex)
        {
            Console.Error.WriteLine($"[RML/Error] Luau reference release threw: {ex}");
        }
    }
}
