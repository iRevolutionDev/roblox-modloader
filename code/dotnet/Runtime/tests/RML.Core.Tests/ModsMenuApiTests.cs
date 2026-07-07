using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

using RML.Core.Api;
using RML.Interop;

using Xunit;

using InteropApi = RML.Interop.Interop;

namespace RML.Core.Tests;

public unsafe class ModsMenuApiTests : IDisposable
{
    private readonly NativeInterop.InteropTable* _table;

    private static readonly List<(nuint Parent, string Text)> s_submenuCalls = [];
    private static readonly List<(nuint Parent, string Text)> s_actionCalls = [];
    private static readonly List<nuint> s_removeCalls = [];
    private static nuint s_nextId;

    public ModsMenuApiTests()
    {
        s_submenuCalls.Clear();
        s_actionCalls.Clear();
        s_removeCalls.Clear();
        s_nextId = 100;

        _table = (NativeInterop.InteropTable*)NativeMemory.AllocZeroed((nuint)sizeof(NativeInterop.InteropTable));
        _table->Version = NativeInterop.InteropTableVersion;
        _table->Size = (uint)sizeof(NativeInterop.InteropTable);
        _table->ModsMenuAddAction = &ModsMenuAddActionStub;
        _table->ModsMenuAddSubmenu = &ModsMenuAddSubmenuStub;
        _table->ModsMenuAddSeparator = &ModsMenuAddSeparatorStub;
        _table->ModsMenuAddCheckable = &ModsMenuAddCheckableStub;
        _table->ModsMenuSetItemIcon = &ModsMenuSetItemIconStub;
        _table->ModsMenuRemove = &ModsMenuRemoveStub;

        InteropApi.Initialize((nint)_table);
    }

    public void Dispose()
    {
        InteropApi.Uninitialize();
        NativeMemory.Free(_table);
    }

    private static string ReadUtf8(sbyte* text) => Marshal.PtrToStringUTF8((nint)text) ?? string.Empty;

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    private static nuint ModsMenuAddSubmenuStub(nuint parentId, sbyte* text)
    {
        s_submenuCalls.Add((parentId, ReadUtf8(text)));
        return s_nextId++;
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    private static nuint ModsMenuAddActionStub(
        nuint parentId, sbyte* text, delegate* unmanaged[Cdecl]<void*, InteropVariant*, uint, void> callback, void* state)
    {
        s_actionCalls.Add((parentId, ReadUtf8(text)));
        return s_nextId++;
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    private static nuint ModsMenuAddSeparatorStub(nuint parentId) => s_nextId++;

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    private static nuint ModsMenuAddCheckableStub(
        nuint parentId, sbyte* text, int initial,
        delegate* unmanaged[Cdecl]<void*, InteropVariant*, uint, void> callback, void* state)
    {
        var arg = InteropVariant.FromBool(initial != 0);
        callback(state, &arg, 1);
        return s_nextId++;
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    private static void ModsMenuSetItemIconStub(nuint id, sbyte* path)
    {
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    private static void ModsMenuRemoveStub(nuint id)
    {
        s_removeCalls.Add(id);
    }

    [Fact]
    public void AddSubmenu_Then_AddAction_Routes_Through_The_Submenu_Id_Then_Dispose_Removes_It()
    {
        var node = ModsMenu.AddSubmenu("X");
        node.AddAction("Y", () => { });

        Assert.Single(s_submenuCalls);
        Assert.Equal(((nuint)0, "X"), s_submenuCalls[0]);

        const nuint subId = 100;

        Assert.Single(s_actionCalls);
        Assert.Equal((subId, "Y"), s_actionCalls[0]);

        node.Dispose();

        Assert.Single(s_removeCalls);
        Assert.Equal(subId, s_removeCalls[0]);
    }

    [Fact]
    public void AddCheckable_Delivers_The_Initial_Value_Through_The_Checkable_Trampoline()
    {
        bool? received = null;
        using var node = ModsMenu.AddSubmenu("Root");

        node.AddCheckable("Toggle", true, v => received = v);

        Assert.True(received);
    }

    [Fact]
    public void AddSeparator_And_SetIcon_Do_Not_Throw()
    {
        using var node = ModsMenu.AddSubmenu("Root");

        node.AddSeparator();
        node.SetIcon("icon.png");
    }
}
