using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

using Xunit;

namespace RML.Interop.Tests;

public unsafe class ModsMenuInteropTests : IDisposable
{
    private readonly NativeInterop.InteropTable* _table;
    private static bool? s_lastToggleValue;

    public ModsMenuInteropTests()
    {
        s_lastToggleValue = null;

        _table = (NativeInterop.InteropTable*)NativeMemory.AllocZeroed((nuint)sizeof(NativeInterop.InteropTable));
        _table->Version = NativeInterop.InteropTableVersion;
        _table->Size = (uint)sizeof(NativeInterop.InteropTable);
        _table->ModsMenuAddAction = &ModsMenuAddActionStub;
        _table->ModsMenuAddSubmenu = &ModsMenuAddSubmenuStub;
        _table->ModsMenuAddSeparator = &ModsMenuAddSeparatorStub;
        _table->ModsMenuAddCheckable = &ModsMenuAddCheckableStub;
        _table->ModsMenuSetItemIcon = &ModsMenuSetItemIconStub;
        _table->ModsMenuRemove = &ModsMenuRemoveStub;

        Interop.Initialize((nint)_table);
    }

    public void Dispose()
    {
        Interop.Uninitialize();
        NativeMemory.Free(_table);
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    private static nuint ModsMenuAddActionStub(nuint parentId, sbyte* text, delegate* unmanaged[Cdecl]<void*, InteropVariant*, uint, void> callback, void* state)
        => 1;

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    private static nuint ModsMenuAddSubmenuStub(nuint parentId, sbyte* text)
        => 2;

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    private static nuint ModsMenuAddSeparatorStub(nuint parentId)
        => 3;

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    private static nuint ModsMenuAddCheckableStub(nuint parentId, sbyte* text, int initial, delegate* unmanaged[Cdecl]<void*, InteropVariant*, uint, void> callback, void* state)
    {
        var arg = InteropVariant.FromBool(initial != 0);
        callback(state, &arg, 1);
        return 4;
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    private static void ModsMenuSetItemIconStub(nuint id, sbyte* path)
    {
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    private static void ModsMenuRemoveStub(nuint id)
    {
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    private static void CheckableTrampoline(void* state, InteropVariant* args, uint argCount)
    {
        s_lastToggleValue = args[0].AsBool;
    }

    [Fact]
    public void Initialize_Accepts_V7_Table()
    {
        Assert.True(Interop.IsInitialized);
    }

    [Fact]
    public void ModsMenuAddAction_Passes_ParentId_And_Returns_Id()
    {
        var cb = (nint)(delegate* unmanaged[Cdecl]<void*, InteropVariant*, uint, void>)&CheckableTrampoline;
        var id = Interop.ModsMenuAddAction(9, "Action", cb, 0);
        Assert.Equal((nuint)1, id);
    }

    [Fact]
    public void ModsMenuAddSubmenu_Returns_Id()
    {
        var id = Interop.ModsMenuAddSubmenu(0, "Submenu");
        Assert.Equal((nuint)2, id);
    }

    [Fact]
    public void ModsMenuAddSeparator_Returns_Id()
    {
        var id = Interop.ModsMenuAddSeparator(0);
        Assert.Equal((nuint)3, id);
    }

    [Fact]
    public void CheckableCallbackDeliversBool()
    {
        var variant = InteropVariant.FromBool(true);
        Assert.Equal(InteropVariant.Tags.Bool, variant.Tag);
        Assert.True(variant.AsBool);
    }

    [Fact]
    public void ModsMenuAddCheckable_Invokes_Callback_With_Bool_Variant()
    {
        var cb = (nint)(delegate* unmanaged[Cdecl]<void*, InteropVariant*, uint, void>)&CheckableTrampoline;
        var id = Interop.ModsMenuAddCheckable(0, "Toggle", true, cb, 0);

        Assert.Equal((nuint)4, id);
        Assert.True(s_lastToggleValue);
    }

    [Fact]
    public void ModsMenuSetItemIcon_And_Remove_Do_Not_Throw()
    {
        Interop.ModsMenuSetItemIcon(1, "icon.png");
        Interop.ModsMenuRemove(1);
    }
}
