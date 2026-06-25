using System;
using System.Linq;
using System.Reflection;

using Xunit;

namespace Roblox.Tests;

public class DeprecationTests
{
    [Fact]
    public void Deprecated_Members_Are_Marked_Obsolete_With_A_Message()
    {
        var obsolete = typeof(Instance).Assembly.GetTypes()
            .SelectMany(t => t.GetMembers(BindingFlags.Public | BindingFlags.Instance | BindingFlags.DeclaredOnly))
            .Select(m => m.GetCustomAttribute<ObsoleteAttribute>())
            .Where(a => a is not null)
            .ToList();
            
        Assert.True(obsolete.Count > 100, $"expected many [Obsolete] members, found {obsolete.Count}");
        Assert.All(obsolete, a => Assert.False(string.IsNullOrWhiteSpace(a!.Message)));
    }
}
