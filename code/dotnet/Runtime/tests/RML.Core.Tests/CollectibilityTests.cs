using System.Reflection;
using System.Runtime.CompilerServices;

using RML.Core.Internal;

using Xunit;

namespace RML.Core.Tests;

public class CollectibilityTests
{
    [Fact]
    public void ModAssemblyContext_Is_Collected_After_Unload()
    {
        var weak = LoadAndUnload();

        for (var i = 0; weak.IsAlive && i < 10; i++)
        {
            GC.Collect();
            GC.WaitForPendingFinalizers();
        }

        Assert.False(weak.IsAlive,
            "ModAssemblyContext was not collected after Unload() — something is still rooting the ALC.");
    }

    [MethodImpl(MethodImplOptions.NoInlining)]
    private static WeakReference LoadAndUnload()
    {
        var assemblyPath = typeof(ModAssemblyContext).Assembly.Location;

        var context = new ModAssemblyContext(assemblyPath, Array.Empty<Assembly>());
        Assembly loaded = context.LoadFromAssemblyPath(assemblyPath);
        _ = loaded.GetName();

        var weak = new WeakReference(context);
        context.Unload();
        return weak;
    }
}
