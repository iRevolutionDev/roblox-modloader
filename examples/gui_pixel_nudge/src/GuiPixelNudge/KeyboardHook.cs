using System.Runtime.InteropServices;

namespace GuiPixelNudge;

internal sealed class KeyboardHook : IDisposable
{
    public Func<int, int, bool, bool>? Handler { get; set; }

    private const int WhKeyboard = 2;
    private const int HcAction = 0;
    private const uint KeyUpFlag = 0x80000000;

    private const int VkShift = 0x10;
    private const int VkMenu = 0x12;
    private const int VkLeft = 0x25;
    private const int VkUp = 0x26;
    private const int VkRight = 0x27;
    private const int VkDown = 0x28;

    private readonly HookProc _proc;
    private nint _hook;

    public KeyboardHook() => _proc = OnKey;

    public bool IsInstalled => _hook != 0;
    
    public bool Install(uint guiThreadId)
    {
        if (_hook != 0 || guiThreadId == 0)
            return _hook != 0;

        _hook = SetWindowsHookEx(WhKeyboard, _proc, 0, guiThreadId);
        return _hook != 0;
    }

    private nint OnKey(int code, nint wParam, nint lParam)
    {
        try
        {
            if (code == HcAction && ((uint)lParam & KeyUpFlag) == 0 && TryNudge((int)wParam))
                return 1;
        }
        catch
        {
            // ignored
        }

        return CallNextHookEx(_hook, code, wParam, lParam);
    }

    private bool TryNudge(int vk)
    {
        var handler = Handler;
        if (handler is null)
            return false;

        var step = IsDown(VkShift) ? 10 : 1;

        var (dx, dy) = vk switch
        {
            VkLeft => (-step, 0),
            VkRight => (step, 0),
            VkUp => (0, -step),
            VkDown => (0, step),
            _ => (0, 0)
        };

        if (dx == 0 && dy == 0)
            return false;

        return handler(dx, dy, IsDown(VkMenu));
    }

    private static bool IsDown(int vk) => (GetKeyState(vk) & 0x8000) != 0;

    public void Dispose()
    {
        Handler = null;

        if (_hook == 0)
            return;

        UnhookWindowsHookEx(_hook);
        _hook = 0;
    }

    private delegate nint HookProc(int code, nint wParam, nint lParam);

    [DllImport("user32.dll", SetLastError = true)]
    private static extern nint SetWindowsHookEx(int idHook, HookProc lpfn, nint hMod, uint dwThreadId);

    [DllImport("user32.dll", SetLastError = true)]
    [return: MarshalAs(UnmanagedType.Bool)]
    private static extern bool UnhookWindowsHookEx(nint hhk);

    [DllImport("user32.dll")]
    private static extern nint CallNextHookEx(nint hhk, int nCode, nint wParam, nint lParam);

    [DllImport("user32.dll")]
    private static extern short GetKeyState(int nVirtKey);
}
