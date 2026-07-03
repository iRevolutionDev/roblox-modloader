using System.Text.Json.Nodes;
using RML.Core.Api;
using Roblox;
using ScriptEditorWebview.Lsp;
using ScriptEditorWebview.Native;
using ScriptEditorWebview.Threading;
using ScriptEditorWebview.WebView;

namespace ScriptEditorWebview.Editor;

internal sealed class ScriptEditorSession : IDisposable
{
    private static readonly TimeSpan WritebackDebounce = TimeSpan.FromMilliseconds(300);

    private readonly GuiDispatcher _gui;
    private readonly LuauLspBridge _lsp;
    private readonly string _webRoot;
    private readonly Func<string?> _sourcemapProvider;

    private readonly WebViewHost _webView;
    private readonly Timer _writebackTimer;
    private readonly Lock _writeGate = new();
    private bool _disposed;
    private bool _editorReady;

    private string _lastSyncedText = string.Empty;
    private volatile bool _lspReady;
    private string? _pendingWriteText;

    public ScriptEditorSession(GuiDispatcher gui, ScriptDocument document, IntPtr editorHwnd, ModContext context,
        Func<string?> sourcemapProvider)
    {
        _gui = gui;
        Document = document;
        EditorHwnd = editorHwnd;
        _webRoot = context.GetPath("web");
        _sourcemapProvider = sourcemapProvider;

        _webView = new WebViewHost(gui);
        _lsp = new LuauLspBridge(
            context.GetPath("tools", "bin", "luau-lsp.exe"),
            context.GetPath("tools", "cache", "globalTypes.PluginSecurity.d.luau"));
        _writebackTimer = new Timer(_ => FlushWriteback(), null, Timeout.Infinite, Timeout.Infinite);
    }

    private ScriptDocument Document { get; }

    public IntPtr EditorHwnd { get; }

    public void Dispose()
    {
        if (_disposed) return;

        _disposed = true;

        _webView.MessageReceived -= OnWebMessage;
        _webView.Ready -= OnWebViewReady;
        _lsp.ServerMessage -= OnLspServerMessage;
        _lsp.Initialized -= OnLspInitialized;

        _writebackTimer.Dispose();
        _lsp.Dispose();
        _webView.Dispose();
    }

    public void Start()
    {
        _webView.MessageReceived += OnWebMessage;
        _webView.Ready += OnWebViewReady;

        _lsp.ServerMessage += OnLspServerMessage;
        _lsp.Initialized += OnLspInitialized;
        _lsp.Start();

        Win32.EnsureClipChildren(EditorHwnd);

        _webView.Initialize(EditorHwnd, _webRoot);
    }

    public void SyncBounds()
    {
        _webView.SyncBounds();
    }

    private void OnLspInitialized()
    {
        _lspReady = true;
        PushSourcemap();
    }

    public void PushSourcemap()
    {
        if (_disposed || !_lspReady) return;

        try
        {
            if (_sourcemapProvider() is { Length: > 0 } tree)
                _lsp.SendNotification("$/plugin/full", tree);
        }
        catch (Exception ex)
        {
            ScriptEditorWebviewMod.Logger.Debug($"pushing sourcemap failed: {ex.Message}");
        }
    }

    private void OnWebViewReady()
    {
    }

    private void OnWebMessage(string raw)
    {
        JsonObject? message;
        try
        {
            message = JsonNode.Parse(raw) as JsonObject;
        }
        catch
        {
            return;
        }

        var type = message?["type"]?.GetValue<string>();
        switch (type)
        {
            case "editor.ready":
                _editorReady = true;
                _gui.Post(PushEngineTextToEditor);
                break;

            case "editor.changed":
                var text = message?["text"]?.GetValue<string>();
                if (text is not null) QueueWriteback(text);

                break;

            case "lsp.message":
                var payload = message?["data"];
                if (payload is not null) _lsp.SendToServer(payload.ToJsonString());

                break;
        }
    }

    private void OnLspServerMessage(string json)
    {
        var envelope = $"{{\"type\":\"lsp.message\",\"data\":{json}}}";
        _webView.PostMessage(envelope);
    }

    public void OnEngineDocumentChanged()
    {
        if (_disposed || !_editorReady) return;

        PushEngineTextToEditor();
    }

    private void PushEngineTextToEditor()
    {
        if (_disposed || !_editorReady) return;

        string text;
        try
        {
            text = Document.GetText();
        }
        catch (Exception ex)
        {
            ScriptEditorWebviewMod.Logger.Error($"Reading ScriptDocument text failed: {ex}");
            return;
        }

        if (text == _lastSyncedText) return;

        _lastSyncedText = text;

        var envelope = new JsonObject
        {
            ["type"] = "editor.setText",
            ["text"] = text,
            ["uri"] = SafeInternalUri(),
            ["remote"] = true
        };
        _webView.PostMessage(envelope.ToJsonString());
    }

    private string SafeInternalUri()
    {
        try
        {
            return Document.GetInternalUri();
        }
        catch
        {
            return "file:///rml/main.luau";
        }
    }

    private void QueueWriteback(string text)
    {
        lock (_writeGate)
        {
            _pendingWriteText = text;
        }

        _writebackTimer.Change(WritebackDebounce, Timeout.InfiniteTimeSpan);
    }

    private void FlushWriteback()
    {
        string? text;
        lock (_writeGate)
        {
            text = _pendingWriteText;
            _pendingWriteText = null;
        }

        if (text is null || _disposed) return;

        _gui.Post(() => WriteToEngine(text));
    }

    private void WriteToEngine(string text)
    {
        if (_disposed || text == _lastSyncedText) return;

        try
        {
            ReplaceWholeDocument(Document, text);
            _lastSyncedText = text;
        }
        catch (Exception ex)
        {
            ScriptEditorWebviewMod.Logger.Error($"Writing ScriptDocument text failed: {ex}");
        }
    }

    private static void ReplaceWholeDocument(ScriptDocument document, string newText)
    {
        var lineCount = Math.Max(1, document.GetLineCount());
        var lastLine = document.GetLine(lineCount);
        document.EditTextAsync(newText, 1, 1, lineCount, lastLine.Length + 1);
    }
}