using Xunit;

namespace Roblox.Tests;

public class RobloxClassManifestTests
{
    [Fact]
    public void Manifest_Is_Populated()
        => Assert.True(RobloxClassManifest.All().Length > 500);

    [Fact]
    public void Every_Entry_Is_Wellformed()
    {
        foreach (var entry in RobloxClassManifest.All())
        {
            Assert.False(string.IsNullOrEmpty(entry.ClassName));
            Assert.NotNull(entry.Type);
            Assert.NotNull(entry.Factory);

            Assert.True(typeof(Object).IsAssignableFrom(entry.Type));
            var attr = (RobloxClassAttribute?)Attribute.GetCustomAttribute(entry.Type, typeof(RobloxClassAttribute));
            Assert.Equal(entry.ClassName, attr?.ClassName);
        }
    }

    [Fact]
    public void Class_Names_Are_Unique()
    {
        var names = RobloxClassManifest.All().Select(e => e.ClassName).ToList();
        Assert.Equal(names.Count, names.Distinct().Count());
    }

    [Fact]
    public void Resolve_Maps_Known_Class_To_Type()
    {
        Assert.Equal(typeof(Part), RobloxTypeRegistry.Resolve("Part"));
        Assert.Equal(typeof(Workspace), RobloxTypeRegistry.Resolve("Workspace"));
        Assert.Null(RobloxTypeRegistry.Resolve("ThisClassDoesNotExist"));
    }
}
