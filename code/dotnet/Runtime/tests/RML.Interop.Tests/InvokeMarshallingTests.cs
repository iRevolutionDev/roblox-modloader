using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

using Xunit;

namespace RML.Interop.Tests;

public unsafe class InvokeMarshallingTests : IDisposable
{
    private readonly NativeInterop.InteropTable* _table;

    public InvokeMarshallingTests()
    {
        _table = (NativeInterop.InteropTable*)NativeMemory.AllocZeroed((nuint)sizeof(NativeInterop.InteropTable));
        _table->Version = NativeInterop.InteropTableVersion;
        _table->Size = 0;
        _table->ReflectionInvoke = &ReflectionInvokeStub;
        _table->ReflectionInvokeAsync = &ReflectionInvokeAsyncStub;

        Interop.Initialize((nint)_table);
    }

    public void Dispose()
    {
        Interop.Uninitialize();
        NativeMemory.Free(_table);
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    private static void ReflectionInvokeStub(void* instance, sbyte* name, InteropVariant* args, uint argCount, InteropVariant* result)
    {
        *result = InteropVariant.Null;
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    private static void ReflectionInvokeAsyncStub(
        void* instance,
        sbyte* name,
        InteropVariant* args,
        uint argCount,
        delegate* unmanaged[Cdecl]<void*, InteropVariant*, sbyte*, void> callback,
        void* state)
    {
    }

    private sealed class Unmarshalable
    {
    }

    [Fact]
    public void Invoke_Throws_NotSupportedException_For_Unmarshalable_Argument()
    {
        Assert.Throws<NotSupportedException>(() =>
            Interop.Reflection.Invoke(null, "Foo", new object?[] { new Unmarshalable() }));
    }

    [Fact]
    public void Invoke_Succeeds_With_Large_Argument_Count_Above_StackAlloc_Threshold()
    {
        var args = new object?[500];
        for (var i = 0; i < args.Length; i++)
        {
            args[i] = i;
        }

        var result = Interop.Reflection.Invoke(null, "Foo", args);
        Assert.Equal(InteropVariant.Tags.Null, result.Tag);
    }

    [Fact]
    public void Invoke_Throws_When_Argument_Count_Exceeds_Maximum()
    {
        var args = new object?[5000];
        for (var i = 0; i < args.Length; i++)
        {
            args[i] = i;
        }

        Assert.Throws<ArgumentException>(() => Interop.Reflection.Invoke(null, "Foo", args));
    }
}
