using System.Linq;

using Xunit;

namespace Roblox.Tests;

public class CustomDefinitionsTests
{
    [Fact]
    public void ClassNameOf_Reads_RobloxClass_Attribute()
    {
        Assert.Equal("Workspace", RobloxTypeRegistry.ClassNameOf<Workspace>());
        Assert.Equal("Players", RobloxTypeRegistry.ClassNameOf<Players>());
        Assert.Equal("Humanoid", RobloxTypeRegistry.ClassNameOf(typeof(Humanoid)));
    }

    [Fact]
    public void GetService_Generic_Overload_Returns_The_Type_Argument()
    {
        var getService = typeof(ServiceProvider).GetMethods()
            .Single(m => m.Name == "GetService" && m.IsGenericMethodDefinition && m.GetParameters().Length == 0);

        Assert.Equal(getService.GetGenericArguments()[0], getService.ReturnType);
    }

    [Fact]
    public void FindFirstChild_Generic_Overload_Returns_The_Type_Argument()
    {
        var find = typeof(Instance).GetMethods()
            .Single(m => m.Name == "FindFirstChild" && m.IsGenericMethodDefinition);

        Assert.Equal(find.GetGenericArguments()[0], find.ReturnType);
    }
    
    private static Workspace UseGetServiceByType(ServiceProvider p) => p.GetService<Workspace>();
    private static Workspace UseGetServiceByName(ServiceProvider p) => p.GetService<Workspace>("Workspace");
    private static BasePart? UseFindFirstChild(Instance i) => i.FindFirstChild<BasePart>("Handle");
    private static Humanoid? UseFindFirstChildOfClass(Instance i) => i.FindFirstChildOfClass<Humanoid>();
    private static BasePart? UseFindFirstChildWhichIsA(Instance i) => i.FindFirstChildWhichIsA<BasePart>(recursive: true);
    private static BasePart? UseClone(BasePart p) => p.Clone<BasePart>();
    private static bool UseIsA(Instance i) => i.IsA<BasePart>();
    private static BasePart? UseAs(Instance i) => i.As<BasePart>();
    private static BasePart UseCast(Instance i) => i.Cast<BasePart>();
}
