using System.Collections.Concurrent;
using RML.Core.Api;
using RML.Core.Modding;
using RML.Logging;
using Roblox;
using ScriptEditorWebview.Editor;
using ScriptEditorWebview.Lsp;
using ScriptEditorWebview.Native;
using ScriptEditorWebview.Qt;
using ScriptEditorWebview.Threading;

namespace ScriptEditorWebview;

[Mod("script-editor-webview", "0.1.0", Author = "Revolution", Description = "Monaco Editor on Roblox")]
public sealed class ScriptEditorWebviewMod : ModBase, IDataModelAware
{
    private const int SourcemapDebounceMs = 1000;
    private const int SourcemapStepBudgetMs = 60;
    public new static readonly ILogger Logger = Log.CreateLogger("ScriptEditorWebview");

    private static readonly string[] ScriptEditorClassNames =
    [
        "StudioScriptEditor",
        "RBX::ScriptEditor::ScriptEditor",
        "ScriptTextEditorWidget"
    ];

    private readonly List<ScriptDocument> _pendingDocuments = [];
    private readonly Lock _pendingGate = new();

    private readonly ConcurrentDictionary<ScriptDocument, ScriptEditorSession> _sessions = new();
    private DataModel? _game;

    private GuiDispatcher? _gui;
    private IModsMenuAction? _modsAction;
    private Action<Instance>? _onDescendantAdded;
    private Action<Instance>? _onDescendantRemoving;
    private Action<ScriptDocument, object>? _onDocChange;
    private Action<ScriptDocument>? _onDocClose;
    private Action<ScriptDocument>? _onDocOpen;
    private Timer? _reconcileTimer;

    private ScriptEditorService? _scriptEditorService;
    private RobloxDataModelSourcemap? _sourcemap;
    private Timer? _sourcemapDebounce;
    private volatile string? _sourcemapJson;
    private bool _sourcemapWalkActive;

    public void OnDataModelLoaded(DataModel game, DataModelType dataModelType)
    {
        if (dataModelType != DataModelType.Edit) return;

        var service = game.GetService<ScriptEditorService>();

        _scriptEditorService = service;

        _onDocOpen = OnTextDocumentDidOpen;
        _onDocClose = OnTextDocumentDidClose;
        _onDocChange = OnTextDocumentDidChange;

        service.TextDocumentDidOpen += _onDocOpen;
        service.TextDocumentDidClose += _onDocClose;
        service.TextDocumentDidChange += _onDocChange;

        try
        {
            foreach (var instance in service.GetScriptDocuments())
                if (instance.As<ScriptDocument>() is { } doc)
                    OnTextDocumentDidOpen(doc);
        }
        catch (Exception ex)
        {
            Logger.Debug($"enumerating open documents failed: {ex.Message}");
        }

        SetupSourcemap(game);

        Logger.Info($"attached to ScriptEditorService for data model {dataModelType}");
    }

    public void OnDataModelUnloaded(DataModel game, DataModelType dataModelType)
    {
        UnsubscribeService();
        TeardownSourcemap();
        DisposeAllSessions();
        lock (_pendingGate)
        {
            _pendingDocuments.Clear();
        }
    }

    private void SetupSourcemap(DataModel game)
    {
        _game = game;
        _sourcemap = new RobloxDataModelSourcemap(game);
        _sourcemapDebounce = new Timer(_ => _gui?.Post(BeginSourcemapWalk), null, Timeout.Infinite, Timeout.Infinite);

        _onDescendantAdded = _ => ScheduleSourcemapRebuild();
        _onDescendantRemoving = _ => ScheduleSourcemapRebuild();
        try
        {
            game.DescendantAdded += _onDescendantAdded;
            game.DescendantRemoving += _onDescendantRemoving;
        }
        catch (Exception ex)
        {
            Logger.Debug($"hooking DataModel change events failed: {ex.Message}");
        }

        ScheduleSourcemapRebuild();
    }

    private void TeardownSourcemap()
    {
        if (_game is not null)
            try
            {
                if (_onDescendantAdded is not null) _game.DescendantAdded -= _onDescendantAdded;
                if (_onDescendantRemoving is not null) _game.DescendantRemoving -= _onDescendantRemoving;
            }
            catch
            {
                // ignored
            }

        _sourcemapDebounce?.Dispose();
        _sourcemapDebounce = null;
        _onDescendantAdded = null;
        _onDescendantRemoving = null;
        _sourcemap = null;
        _sourcemapJson = null;
        _game = null;
    }

    private void ScheduleSourcemapRebuild()
    {
        _sourcemapDebounce?.Change(SourcemapDebounceMs, Timeout.Infinite);
    }

    private void BeginSourcemapWalk()
    {
        if (_sourcemap is null || _sessions.IsEmpty) return;

        _sourcemap.Restart();
        _sourcemapWalkActive = true;
    }

    private void StepSourcemap()
    {
        if (!_sourcemapWalkActive || _sourcemap is null) return;

        if (_sessions.IsEmpty)
        {
            _sourcemapWalkActive = false;
            return;
        }

        if (!_sourcemap.Step(SourcemapStepBudgetMs)) return;

        _sourcemapWalkActive = false;

        var json = _sourcemap.Serialize();
        if (json is null) return;

        _sourcemapJson = json;
        Logger.Info($"sourcemap ready: {_sourcemap.NodeCount} nodes");
        foreach (var session in _sessions.Values) session.PushSourcemap();
    }

    public override int OnLoad()
    {
        try
        {
            _modsAction = ModsMenu.AddAction("Script Editor Webview: Reattach", RequestReattach);
        }
        catch (Exception ex)
        {
            Logger.Error($"failed to register the Mods menu action: {ex.Message}");
        }

        _gui = new GuiDispatcher();
        _reconcileTimer = new Timer(_ => OnReconcileTick(), null, TimeSpan.FromMilliseconds(250),
            TimeSpan.FromMilliseconds(200));

        Logger.Info($"loaded from '{Context.Directory}'");
        return 0;
    }

    public override void OnUnload()
    {
        _reconcileTimer?.Dispose();
        _reconcileTimer = null;

        _modsAction?.Dispose();
        _modsAction = null;

        TeardownSourcemap();
        DisposeAllSessions();

        _gui?.Dispose();
        _gui = null;

        Logger.Info("unloaded");
    }

    private void OnTextDocumentDidOpen(ScriptDocument document)
    {
        try
        {
            if (document.IsCommandBar()) return;
        }
        catch
        {
            // ignored
        }

        lock (_pendingGate)
        {
            if (!_sessions.ContainsKey(document) && !_pendingDocuments.Contains(document))
                _pendingDocuments.Add(document);
        }
    }

    private void OnTextDocumentDidClose(ScriptDocument document)
    {
        lock (_pendingGate)
        {
            _pendingDocuments.Remove(document);
        }

        if (_sessions.TryRemove(document, out var session)) session.Dispose();
    }

    private void OnTextDocumentDidChange(ScriptDocument document, object changes)
    {
        if (_sessions.TryGetValue(document, out var session)) _gui?.Post(session.OnEngineDocumentChanged);
    }

    private void OnReconcileTick()
    {
        var gui = _gui;
        if (gui is null) return;

        if (!gui.IsReady)
        {
            gui.EnsureStarted(() => Logger.Info("GUI dispatcher online"));
            return;
        }

        gui.Post(Reconcile);
    }

    private void Reconcile()
    {
        if (_gui is null) return;

        AttachPendingDocuments();

        foreach (var session in _sessions.Values) session.SyncBounds();

        StepSourcemap();
    }

    private void AttachPendingDocuments()
    {
        List<ScriptDocument> pending;
        lock (_pendingGate)
        {
            if (_pendingDocuments.Count == 0) return;

            pending = new List<ScriptDocument>(_pendingDocuments);
        }

        var attachedHwnds = new HashSet<IntPtr>(_sessions.Values.Select(s => s.EditorHwnd));

        foreach (var document in pending)
        {
            var editorWidget = FindUnattachedEditorWidget(attachedHwnds);
            if (editorWidget.IsNull) continue;

            var editorHwnd = editorWidget.WinId();
            if (editorHwnd == IntPtr.Zero) continue;

            var session = new ScriptEditorSession(_gui!, document, editorHwnd, Context, () => _sourcemapJson);
            if (_sessions.TryAdd(document, session))
            {
                attachedHwnds.Add(editorHwnd);
                session.Start();
                ScheduleSourcemapRebuild();
                Logger.Info("attached editor overlay to a script editor widget");

                lock (_pendingGate)
                {
                    _pendingDocuments.Remove(document);
                }
            }
            else
            {
                session.Dispose();
            }
        }
    }

    private static QWidget FindUnattachedEditorWidget(HashSet<IntPtr> attached)
    {
        var candidates = QApplication.FindWidgets(IsScriptEditorWidget);

        QWidget best = default;
        long bestScore = -1;
        var bestClass = string.Empty;
        var bestHwnd = IntPtr.Zero;
        int bestW = 0, bestH = 0;

        foreach (var widget in candidates)
        {
            var hwnd = widget.WinId();
            if (hwnd == IntPtr.Zero || attached.Contains(hwnd) || !Win32.IsWindowVisible(hwnd)) continue;

            if (!Win32.GetClientRect(hwnd, out var rect)) continue;

            int w = rect.Width, h = rect.Height;

            if (w is < 200 or >= 32000 || h is < 120 or >= 32000) continue;

            var className = widget.ClassName;
            var score = (long)w * h + (className == "StudioScriptEditor" ? 1_000_000_000L : 0L);
            if (score <= bestScore) continue;

            bestScore = score;
            best = widget;
            bestClass = className;
            bestHwnd = hwnd;
            bestW = w;
            bestH = h;
        }

        if (!best.IsNull) Logger.Info($"overlay target: class='{bestClass}' hwnd=0x{bestHwnd:X} size={bestW}x{bestH}");

        return best;
    }

    private static bool IsScriptEditorWidget(QWidget widget)
    {
        var className = widget.ClassName;
        if (string.IsNullOrEmpty(className)) return false;

        return ScriptEditorClassNames.Any(name => className == name || widget.Inherits(name)) ||
               className.Contains("ScriptEditor", StringComparison.Ordinal);
    }

    private void UnsubscribeService()
    {
        if (_scriptEditorService is null) return;

        if (_onDocOpen is not null) _scriptEditorService.TextDocumentDidOpen -= _onDocOpen;

        if (_onDocClose is not null) _scriptEditorService.TextDocumentDidClose -= _onDocClose;

        if (_onDocChange is not null) _scriptEditorService.TextDocumentDidChange -= _onDocChange;

        _scriptEditorService = null;
        _onDocOpen = null;
        _onDocClose = null;
        _onDocChange = null;
    }

    private void RequestReattach()
    {
        _gui?.Post(() =>
        {
            DisposeAllSessions();

            if (_scriptEditorService is not null)
                try
                {
                    foreach (var instance in _scriptEditorService.GetScriptDocuments())
                        if (instance.As<ScriptDocument>() is { } doc)
                            OnTextDocumentDidOpen(doc);
                }
                catch (Exception ex)
                {
                    Logger.Debug($"reattach enumeration failed: {ex.Message}");
                }

            Logger.Info("reattach requested from the Mods menu");
        });
    }

    private void DisposeAllSessions()
    {
        foreach (var document in _sessions.Keys.ToArray())
            if (_sessions.TryRemove(document, out var session))
                session.Dispose();
    }
}