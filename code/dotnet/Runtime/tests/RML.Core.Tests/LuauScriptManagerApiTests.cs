using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;

using RML.Core.Api;
using RML.Interop;

using Roblox;

using Xunit;

using InteropApi = RML.Interop.Interop;

namespace RML.Core.Tests;

public class LuauScriptManagerApiTests : IDisposable
{
    private enum ResultMode
    {
        Nil,
        Number,
        Text,
        Reference,
        Error
    }

    private readonly unsafe NativeInterop.InteropTable* _table;

    private static readonly List<nuint> s_refReleases = [];
    private static readonly List<(nuint Handle, uint ArgCount)> s_refCalls = [];
    private static readonly List<(nuint Handle, string Key)> s_refIndexCalls = [];
    private static readonly List<nint> s_freedStrings = [];
    private static ResultMode s_mode;
    private static int s_hostReady;
    private static nuint s_refHandle;

    public unsafe LuauScriptManagerApiTests()
    {
        s_refReleases.Clear();
        s_refCalls.Clear();
        s_refIndexCalls.Clear();
        s_freedStrings.Clear();
        s_mode = ResultMode.Nil;
        s_hostReady = 1;
        s_refHandle = 77;

        _table = (NativeInterop.InteropTable*)NativeMemory.AllocZeroed((nuint)sizeof(NativeInterop.InteropTable));
        _table->Version = NativeInterop.InteropTableVersion;
        _table->Size = (uint)sizeof(NativeInterop.InteropTable);
        _table->FreeString = &FreeStringStub;
        _table->LuauHostReady = &LuauHostReadyStub;
        _table->LuauSchedule = &LuauScheduleStub;
        _table->LuauEvaluate = &LuauEvaluateStub;
        _table->LuauRefCall = &LuauRefCallStub;
        _table->LuauRefIndex = &LuauRefIndexStub;
        _table->LuauRefRelease = &LuauRefReleaseStub;

        InteropApi.Initialize((nint)_table);
    }

    public unsafe void Dispose()
    {
        InteropApi.Uninitialize();
        NativeMemory.Free(_table);
    }

    private unsafe void ClearHostReadySlot() => _table->LuauHostReady = null;

    private unsafe void ClearScheduleSlot() => _table->LuauSchedule = null;

    private static nint AllocUtf8(string text)
    {
        var bytes = Encoding.UTF8.GetBytes(text);
        var ptr = Marshal.AllocHGlobal(bytes.Length + 1);
        Marshal.Copy(bytes, 0, ptr, bytes.Length);
        Marshal.WriteByte(ptr + bytes.Length, 0);
        return ptr;
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    private static unsafe void FreeStringStub(sbyte* str)
    {
        s_freedStrings.Add((nint)str);
        Marshal.FreeHGlobal((nint)str);
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    private static int LuauHostReadyStub(int dataModelType) => s_hostReady;

    private static unsafe void Settle(delegate* unmanaged[Cdecl]<void*, InteropVariant*, sbyte*, void> callback, void* state)
    {
        switch (s_mode)
        {
            case ResultMode.Number:
            {
                var result = InteropVariant.FromDouble(21.5);
                callback(state, &result, null);
                return;
            }

            case ResultMode.Text:
            {
                var result = InteropVariant.FromString((nuint)AllocUtf8("hello luau"));
                callback(state, &result, null);
                return;
            }

            case ResultMode.Reference:
            {
                var result = InteropVariant.FromLuauRef(s_refHandle);
                callback(state, &result, null);
                return;
            }

            case ResultMode.Error:
            {
                var message = AllocUtf8("chunk failed to compile");
                try
                {
                    callback(state, null, (sbyte*)message);
                }
                finally
                {
                    Marshal.FreeHGlobal(message);
                }

                return;
            }

            default:
                callback(state, null, null);
                return;
        }
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    private static unsafe void LuauScheduleStub(
        int dataModelType, sbyte* chunkName, sbyte* source,
        delegate* unmanaged[Cdecl]<void*, InteropVariant*, sbyte*, void> callback, void* state)
        => Settle(callback, state);

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    private static unsafe void LuauEvaluateStub(
        int dataModelType, sbyte* chunkName, sbyte* source,
        delegate* unmanaged[Cdecl]<void*, InteropVariant*, sbyte*, void> callback, void* state)
        => Settle(callback, state);

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    private static unsafe void LuauRefCallStub(
        nuint refHandle, InteropVariant* args, uint argCount,
        delegate* unmanaged[Cdecl]<void*, InteropVariant*, sbyte*, void> callback, void* state)
    {
        s_refCalls.Add((refHandle, argCount));
        Settle(callback, state);
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    private static unsafe void LuauRefIndexStub(
        nuint refHandle, sbyte* key,
        delegate* unmanaged[Cdecl]<void*, InteropVariant*, sbyte*, void> callback, void* state)
    {
        s_refIndexCalls.Add((refHandle, Marshal.PtrToStringUTF8((nint)key) ?? string.Empty));
        Settle(callback, state);
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    private static void LuauRefReleaseStub(nuint refHandle) => s_refReleases.Add(refHandle);

    [Fact]
    public void IsReady_Returns_True_When_The_Native_Host_Reports_Ready()
    {
        Assert.True(LuauScriptManager.IsReady(DataModelType.Edit));
    }

    [Fact]
    public void IsReady_Returns_False_When_The_Slot_Is_Null()
    {
        ClearHostReadySlot();
        Assert.False(LuauScriptManager.IsReady(DataModelType.Edit));
    }

    [Fact]
    public void IsReady_Returns_False_When_The_Host_Reports_Not_Ready()
    {
        s_hostReady = 0;
        Assert.False(LuauScriptManager.IsReady(DataModelType.Client));
    }

    [Fact]
    public async Task ScheduleAsync_Completes_When_The_Native_Side_Fires_The_Callback()
    {
        await LuauScriptManager.ScheduleAsync(DataModelType.Edit, "print('hi')");
    }

    [Fact]
    public async Task ScheduleAsync_Faults_When_The_Slot_Is_Null()
    {
        ClearScheduleSlot();

        await Assert.ThrowsAsync<InvalidOperationException>(
            () => LuauScriptManager.ScheduleAsync(DataModelType.Edit, "print('hi')"));
    }

    [Fact]
    public async Task EvaluateAsync_Completes_With_A_Number_Value()
    {
        s_mode = ResultMode.Number;

        var value = await LuauScriptManager.EvaluateAsync(DataModelType.Edit, "return 21.5");

        Assert.False(value.IsNil);
        Assert.Equal(21.5, value.AsNumber());
        Assert.Equal(21.5, value.As<double>());
    }

    [Fact]
    public async Task EvaluateAsync_Completes_With_A_Nil_Value_When_No_Result_Is_Returned()
    {
        var value = await LuauScriptManager.EvaluateAsync(DataModelType.Edit, "return");

        Assert.True(value.IsNil);
        Assert.False(value.AsBoolean());
        Assert.Null(value.AsString());
        Assert.Null(value.AsRef());
    }

    [Fact]
    public async Task EvaluateAsync_Reads_And_Frees_A_Native_String_Result()
    {
        s_mode = ResultMode.Text;

        var value = await LuauScriptManager.EvaluateAsync(DataModelType.Edit, "return 'hello luau'");

        Assert.Equal("hello luau", value.AsString());
        Assert.Single(s_freedStrings);
    }

    [Fact]
    public async Task EvaluateAsync_Faults_When_The_Native_Side_Reports_An_Error()
    {
        s_mode = ResultMode.Error;

        var ex = await Assert.ThrowsAsync<InvalidOperationException>(
            () => LuauScriptManager.EvaluateAsync(DataModelType.Edit, "return ("));

        Assert.Equal("chunk failed to compile", ex.Message);
    }

    [Fact]
    public async Task EvaluateAsync_Throws_For_A_Null_Source()
    {
        await Assert.ThrowsAsync<ArgumentNullException>(
            () => LuauScriptManager.EvaluateAsync(DataModelType.Edit, null!));
    }

    [Fact]
    public async Task EvaluateAsync_Yields_A_LuauRef_For_A_Reference_Result()
    {
        s_mode = ResultMode.Reference;

        var value = await LuauScriptManager.EvaluateAsync(DataModelType.Edit, "return function() end");
        using var reference = value.AsRef();

        Assert.NotNull(reference);
        Assert.Equal((nuint)77, reference.Handle);
        Assert.Same(reference, value.As<LuauRef>());
    }

    [Fact]
    public async Task LuauRef_Dispose_Releases_The_Handle_Exactly_Once()
    {
        s_mode = ResultMode.Reference;

        var value = await LuauScriptManager.EvaluateAsync(DataModelType.Edit, "return function() end");
        var reference = value.AsRef()!;

        reference.Dispose();
        reference.Dispose();

        Assert.Single(s_refReleases);
        Assert.Equal((nuint)77, s_refReleases[0]);
        Assert.True(reference.IsReleased);
    }

    [Fact]
    public async Task LuauRef_InvokeAsync_Forwards_The_Handle_And_Arguments()
    {
        s_mode = ResultMode.Reference;

        var value = await LuauScriptManager.EvaluateAsync(DataModelType.Edit, "return function() end");
        using var reference = value.AsRef()!;

        s_mode = ResultMode.Number;
        var result = await reference.InvokeAsync("a", 2.0);

        Assert.Equal(21.5, result.AsNumber());
        Assert.Single(s_refCalls);
        Assert.Equal(((nuint)77, 2u), s_refCalls[0]);
    }

    [Fact]
    public async Task LuauRef_IndexAsync_Forwards_The_Key()
    {
        s_mode = ResultMode.Reference;

        var value = await LuauScriptManager.EvaluateAsync(DataModelType.Edit, "return {}");
        using var reference = value.AsRef()!;

        s_mode = ResultMode.Text;
        var result = await reference.IndexAsync("Name");

        Assert.Equal("hello luau", result.AsString());
        Assert.Single(s_refIndexCalls);
        Assert.Equal(((nuint)77, "Name"), s_refIndexCalls[0]);
    }

    [Fact]
    public async Task LuauRef_InvokeAsync_Faults_After_Dispose()
    {
        s_mode = ResultMode.Reference;

        var value = await LuauScriptManager.EvaluateAsync(DataModelType.Edit, "return function() end");
        var reference = value.AsRef()!;
        reference.Dispose();

        await Assert.ThrowsAsync<ObjectDisposedException>(() => reference.InvokeAsync());
    }
}
