using System.Collections.Concurrent;
using System.Runtime.InteropServices;
using ScriptEditorWebview.Native;

namespace ScriptEditorWebview.Threading;

internal sealed unsafe class GuiDispatcher : IDisposable
{
    private const long HookFireTimeoutMs = 2000;

    private static GuiDispatcher? _sActive;
    private readonly string _className = "RmlSew_Dispatch_" + Guid.NewGuid().ToString("N");

    private readonly ConcurrentQueue<Action> _queue = new();
    private volatile bool _bootstrapStarted;
    private bool _diagLogged;
    private volatile bool _disposed;

    private IntPtr _guiHwnd;
    private uint _guiThreadId;
    private IntPtr _hook;
    private long _hookInstalledTick;
    private IntPtr _messageHwnd;
    private Action? _onReady;
    private volatile bool _ready;

    public bool IsReady => _ready;

    public bool IsOnGuiThread => _guiThreadId != 0 && Win32.GetCurrentThreadId() == _guiThreadId;

    public void Dispose()
    {
        if (_disposed) return;

        _disposed = true;
        _ready = false;

        if (_hook != IntPtr.Zero)
        {
            Win32.UnhookWindowsHookEx(_hook);
            _hook = IntPtr.Zero;
        }

        if (_messageHwnd == IntPtr.Zero)
        {
            if (ReferenceEquals(_sActive, this)) _sActive = null;

            return;
        }

        void Teardown()
        {
            if (_messageHwnd != IntPtr.Zero)
            {
                Win32.DestroyWindow(_messageHwnd);
                _messageHwnd = IntPtr.Zero;
            }

            var classNamePtr = Marshal.StringToHGlobalUni(_className);
            try
            {
                Win32.UnregisterClassW(classNamePtr, Win32.GetModuleHandleW(null));
            }
            finally
            {
                Marshal.FreeHGlobal(classNamePtr);
            }
        }

        if (!IsOnGuiThread)
        {
            var done = new ManualResetEventSlim(false);
            _queue.Enqueue(() =>
            {
                Teardown();
                done.Set();
            });
            Win32.PostMessageW(_messageHwnd, Win32.WM_RML_DISPATCH, IntPtr.Zero, IntPtr.Zero);
            done.Wait(TimeSpan.FromSeconds(2));
        }
        else
        {
            Teardown();
        }

        if (ReferenceEquals(_sActive, this)) _sActive = null;
    }

    public void EnsureStarted(Action onReady)
    {
        if (_ready || _disposed || _bootstrapStarted) return;

        if (_hook != IntPtr.Zero)
        {
            if (Environment.TickCount64 - _hookInstalledTick < HookFireTimeoutMs) return;

            Win32.UnhookWindowsHookEx(_hook);
            _hook = IntPtr.Zero;
            _guiHwnd = IntPtr.Zero;
            _diagLogged = false;
            ScriptEditorWebviewMod.Logger.Info(
                "GUI dispatcher: hook did not fire in time; re-arming on a freshly chosen window…");
        }

        _onReady ??= onReady;
        _sActive = this;

        var guiHwnd = Win32.FindProcessMainWindow();
        if (guiHwnd == IntPtr.Zero) return;

        var guiThreadId = Win32.GetWindowThreadProcessId(guiHwnd, out _);
        if (guiThreadId == 0) return;

        _guiHwnd = guiHwnd;

        if (guiThreadId == Win32.GetCurrentThreadId())
        {
            BootstrapOnGuiThread();
            return;
        }

        var hookProc = (IntPtr)(delegate* unmanaged<int, IntPtr, IntPtr, IntPtr>)&HookProc;
        var hook = Win32.SetWindowsHookExW(Win32.WH_GETMESSAGE, hookProc, IntPtr.Zero, guiThreadId);
        if (hook == IntPtr.Zero)
        {
            if (_diagLogged) return;

            _diagLogged = true;
            ScriptEditorWebviewMod.Logger.Error(
                $"GUI dispatcher: SetWindowsHookEx(WH_GETMESSAGE) failed on thread {guiThreadId} (err {Marshal.GetLastWin32Error()})");

            return;
        }

        if (!_diagLogged)
        {
            _diagLogged = true;
            ScriptEditorWebviewMod.Logger.Info(
                $"GUI dispatcher: WH_GETMESSAGE hook installed on Qt GUI thread {guiThreadId} (hwnd 0x{guiHwnd:X}); nudging…");
        }

        _hook = hook;
        _hookInstalledTick = Environment.TickCount64;

        if (_bootstrapStarted)
        {
            Win32.UnhookWindowsHookEx(hook);
            _hook = IntPtr.Zero;
        }
        else
        {
            Win32.PostThreadMessageW(guiThreadId, Win32.WM_NULL, IntPtr.Zero, IntPtr.Zero);
        }
    }

    public void Post(Action action)
    {
        if (_disposed) return;

        _queue.Enqueue(action);

        if (_ready && _messageHwnd != IntPtr.Zero)
            Win32.PostMessageW(_messageHwnd, Win32.WM_RML_DISPATCH, IntPtr.Zero, IntPtr.Zero);
    }

    private void Drain()
    {
        while (_queue.TryDequeue(out var action))
            try
            {
                action();
            }
            catch (Exception ex)
            {
                ScriptEditorWebviewMod.Logger.Error($"GUI dispatch action threw: {ex}");
            }
    }

    [UnmanagedCallersOnly]
    private static IntPtr HookProc(int nCode, IntPtr wParam, IntPtr lParam)
    {
        var self = _sActive;
        if (self is not null && nCode == Win32.HC_ACTION && !self._disposed && !self._bootstrapStarted)
            self.BootstrapOnGuiThread();

        return Win32.CallNextHookEx(IntPtr.Zero, nCode, wParam, lParam);
    }

    private void BootstrapOnGuiThread()
    {
        if (_bootstrapStarted) return;

        _bootstrapStarted = true;
        _guiThreadId = Win32.GetCurrentThreadId();
        ScriptEditorWebviewMod.Logger.Info(
            $"GUI dispatcher: hook fired on thread {_guiThreadId}; creating message window…");

        if (_hook != IntPtr.Zero)
        {
            Win32.UnhookWindowsHookEx(_hook);
            _hook = IntPtr.Zero;
        }

        try
        {
            CreateMessageWindow();
            _ready = true;

            var onReady = _onReady;
            _onReady = null;
            if (onReady is not null) _queue.Enqueue(onReady);

            if (_messageHwnd != IntPtr.Zero)
                Win32.PostMessageW(_messageHwnd, Win32.WM_RML_DISPATCH, IntPtr.Zero, IntPtr.Zero);
        }
        catch (Exception ex)
        {
            ScriptEditorWebviewMod.Logger.Error($"GUI dispatcher bootstrap failed: {ex}");
        }
    }

    private void CreateMessageWindow()
    {
        var classNamePtr = Marshal.StringToHGlobalUni(_className);
        try
        {
            var wndProc = (IntPtr)(delegate* unmanaged<IntPtr, uint, IntPtr, IntPtr, IntPtr>)&WndProc;
            var wc = new Win32.WNDCLASSEXW
            {
                cbSize = (uint)sizeof(Win32.WNDCLASSEXW),
                lpfnWndProc = wndProc,
                hInstance = Win32.GetModuleHandleW(null),
                lpszClassName = classNamePtr
            };

            Win32.RegisterClassExW(ref wc);

            _messageHwnd = Win32.CreateWindowExW(
                0, classNamePtr, null, 0, 0, 0, 0, 0,
                Win32.HWND_MESSAGE, IntPtr.Zero, wc.hInstance, IntPtr.Zero);

            if (_messageHwnd == IntPtr.Zero)
                throw new InvalidOperationException(
                    $"CreateWindowEx (message-only) failed: {Marshal.GetLastWin32Error()}");
        }
        finally
        {
            Marshal.FreeHGlobal(classNamePtr);
        }
    }

    [UnmanagedCallersOnly]
    private static IntPtr WndProc(IntPtr hWnd, uint msg, IntPtr wParam, IntPtr lParam)
    {
        if (msg != Win32.WM_RML_DISPATCH) return Win32.DefWindowProcW(hWnd, msg, wParam, lParam);
        _sActive?.Drain();
        return IntPtr.Zero;
    }
}