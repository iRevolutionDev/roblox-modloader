using System.Runtime.InteropServices;
using RML.Core.Api;
using RML.Core.Modding;

namespace ExampleMod;

[RmlMod("dotnet-example", "1.0.0", Author = "Revolution", Description = "Example managed mod for RobloxModLoader")]
public sealed class Mod : IMod
{
    public int OnLoad()
    {
        MessageBoxW(IntPtr.Zero, "Hello from the example mod!", "Example Mod", 0);
        Console.WriteLine("Hello from the example mod!");
        return 0;
    }

    public void OnUnload()
    {
        Console.WriteLine("Goodbye from the example mod!");
    }

    [DllImport("user32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
    private static extern int MessageBoxW(IntPtr hWnd, string lpText, string lpCaption, uint uType);
}