using RML.Core.Api;
using RML.Core.Modding;
using System.Runtime.InteropServices;

namespace ExampleMod;

[RmlMod("dotnet-example", "1.0.0", Author = "Revolution", Description = "Example managed mod for RobloxModLoader")]
public sealed class Mod : ModBase
{
    [DllImport("user32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
    private static extern int MessageBoxW(IntPtr hWnd, string lpText, string lpCaption, uint uType);

    public override int Initialize(ModContext context)
    {
        Directory.CreateDirectory(context.GeneratedOutputDirectory);
        File.AppendAllText(
            Path.Combine(context.GeneratedOutputDirectory, "example_mod.log"),
            $"[{DateTime.UtcNow:O}] ExampleManagedMod initialized at {context.ModRoot}{Environment.NewLine}");

        MessageBoxW(IntPtr.Zero, "Hello from ExampleManagedMod!", "ExampleManagedMod", 0);

        return 0;
    }

    public override void Shutdown()
    {
    }
}
