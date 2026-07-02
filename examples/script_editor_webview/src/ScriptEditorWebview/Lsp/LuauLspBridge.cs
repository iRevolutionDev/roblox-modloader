using System.Diagnostics;
using System.Text;
using System.Text.Json;
using System.Text.Json.Nodes;

namespace ScriptEditorWebview.Lsp;

internal sealed class LuauLspBridge(string lspExePath, string robloxTypesPath) : IDisposable
{
    private readonly Lock _writeLock = new();
    private volatile bool _disposed;

    private Process? _process;
    private Thread? _readerThread;

    public bool IsRunning => _process is { HasExited: false };

    public void Dispose()
    {
        if (_disposed) return;

        _disposed = true;

        try
        {
            if (_process is { HasExited: false }) _process.Kill(true);
        }
        catch
        {
            // ignore
        }

        try
        {
            _readerThread?.Join(TimeSpan.FromSeconds(1));
        }
        catch
        {
            // ignore
        }

        _process?.Dispose();
        _process = null;
    }

    public event Action<string>? ServerMessage;

    public event Action? Initialized;

    public void Start()
    {
        if (_process is not null) return;

        if (!File.Exists(lspExePath))
        {
            ScriptEditorWebviewMod.Logger.Error($"luau-lsp executable not found at '{lspExePath}'.");
            return;
        }

        var psi = new ProcessStartInfo
        {
            FileName = lspExePath,
            RedirectStandardInput = true,
            RedirectStandardOutput = true,
            RedirectStandardError = true,
            UseShellExecute = false,
            CreateNoWindow = true,
            WorkingDirectory = Path.GetDirectoryName(lspExePath) ?? Environment.CurrentDirectory
        };

        psi.ArgumentList.Add("lsp");
        psi.ArgumentList.Add("--stdio");
        if (File.Exists(robloxTypesPath))
        {
            psi.ArgumentList.Add("--definitions");
            psi.ArgumentList.Add($"@roblox={robloxTypesPath}");
        }
        else
        {
            ScriptEditorWebviewMod.Logger.Warn(
                $"Roblox type definitions not found at '{robloxTypesPath}'. Continuing without @roblox globals.");
        }

        try
        {
            _process = Process.Start(psi);
        }
        catch (Exception ex)
        {
            ScriptEditorWebviewMod.Logger.Error($"Failed to start luau-lsp: {ex}");
            return;
        }

        if (_process is null)
        {
            ScriptEditorWebviewMod.Logger.Error("Failed to start luau-lsp (null process).");
            return;
        }

        _process.ErrorDataReceived += (_, e) =>
        {
            if (!string.IsNullOrWhiteSpace(e.Data)) ScriptEditorWebviewMod.Logger.Debug($"[luau-lsp] {e.Data}");
        };
        _process.BeginErrorReadLine();

        _readerThread = new Thread(ReadLoop) { IsBackground = true, Name = "luau-lsp-reader" };
        _readerThread.Start();

        ScriptEditorWebviewMod.Logger.Info("luau-lsp started");
    }

    public void SendToServer(string jsonRpc)
    {
        if (_disposed || _process is null || _process.HasExited) return;

        WriteFrame(InjectRobloxSettings(jsonRpc));

        if (IsMethod(jsonRpc, "initialized")) Initialized?.Invoke();
    }

    public void SendNotification(string method, string paramsJson)
    {
        if (_disposed || _process is null || _process.HasExited) return;

        WriteFrame($"{{\"jsonrpc\":\"2.0\",\"method\":{JsonSerializer.Serialize(method)},\"params\":{paramsJson}}}");
    }

    private void WriteFrame(string message)
    {
        var payload = Encoding.UTF8.GetBytes(message);
        var header = Encoding.ASCII.GetBytes($"Content-Length: {payload.Length}\r\n\r\n");

        try
        {
            lock (_writeLock)
            {
                if (_process is null || _process.HasExited) return;

                var stdin = _process.StandardInput.BaseStream;
                stdin.Write(header, 0, header.Length);
                stdin.Write(payload, 0, payload.Length);
                stdin.Flush();
            }
        }
        catch (Exception ex)
        {
            ScriptEditorWebviewMod.Logger.Error($"Writing to luau-lsp failed: {ex}");
        }
    }

    private static bool IsMethod(string jsonRpc, string method)
    {
        try
        {
            return JsonNode.Parse(jsonRpc) is JsonObject root && root["method"]?.GetValue<string>() == method;
        }
        catch
        {
            return false;
        }
    }

    private string InjectRobloxSettings(string jsonRpc)
    {
        try
        {
            if (JsonNode.Parse(jsonRpc) is not JsonObject root) return jsonRpc;

            var method = root["method"]?.GetValue<string>();
            switch (method)
            {
                case "initialize":
                {
                    var prms = root["params"] as JsonObject ?? new JsonObject();
                    prms["initializationOptions"] = ApplyRobloxSettings(prms["initializationOptions"] as JsonObject);
                    root["params"] = prms;
                    return root.ToJsonString();
                }
                case "workspace/didChangeConfiguration":
                {
                    var prms = root["params"] as JsonObject ?? new JsonObject();
                    prms["settings"] = ApplyRobloxSettings(prms["settings"] as JsonObject);
                    root["params"] = prms;
                    return root.ToJsonString();
                }
            }
        }
        catch (Exception ex)
        {
            ScriptEditorWebviewMod.Logger.Debug($"Roblox settings injection skipped: {ex.Message}");
        }

        return jsonRpc;
    }

    private JsonObject ApplyRobloxSettings(JsonObject? existing)
    {
        var settings = existing ?? new JsonObject();

        var platform = settings["platform"] as JsonObject ?? new JsonObject();
        platform["type"] = "roblox";
        settings["platform"] = platform;

        var sourcemap = settings["sourcemap"] as JsonObject ?? new JsonObject();
        sourcemap["enabled"] = true;
        sourcemap["autogenerate"] = false;
        settings["sourcemap"] = sourcemap;

        var types = settings["types"] as JsonObject ?? new JsonObject();
        types["roblox"] = true;
        var definitionFiles = types["definitionFiles"] as JsonObject ?? new JsonObject();
        if (File.Exists(robloxTypesPath)) definitionFiles["@roblox"] = robloxTypesPath;

        types["definitionFiles"] = definitionFiles;
        settings["types"] = types;

        settings["luau-lsp.platform.type"] = "roblox";
        settings["luau-lsp.sourcemap.enabled"] = true;
        settings["luau-lsp.sourcemap.autogenerate"] = false;
        settings["luau-lsp.types.roblox"] = true;

        return settings;
    }

    private void ReadLoop()
    {
        var process = _process;
        if (process is null) return;

        var stream = process.StandardOutput.BaseStream;
        var buffer = new List<byte>(16 * 1024);
        var chunk = new byte[8 * 1024];

        try
        {
            while (!_disposed)
            {
                var read = stream.Read(chunk, 0, chunk.Length);
                if (read <= 0) break;

                buffer.AddRange(chunk.AsSpan(0, read));
                DrainFrames(buffer);
            }
        }
        catch (Exception ex) when (!_disposed)
        {
            ScriptEditorWebviewMod.Logger.Error($"luau-lsp reader loop failed: {ex}");
        }
    }

    private void DrainFrames(List<byte> buffer)
    {
        while (true)
        {
            var headerEnd = IndexOfHeaderEnd(buffer);
            if (headerEnd < 0) return;

            var headerText = Encoding.ASCII.GetString(buffer.GetRange(0, headerEnd).ToArray());
            var contentLength = ParseContentLength(headerText);
            if (contentLength < 0)
            {
                buffer.Clear();
                return;
            }

            var total = headerEnd + 4 + contentLength;
            if (buffer.Count < total) return;

            var payloadBytes = buffer.GetRange(headerEnd + 4, contentLength).ToArray();
            buffer.RemoveRange(0, total);

            var json = Encoding.UTF8.GetString(payloadBytes);
            try
            {
                ServerMessage?.Invoke(json);
            }
            catch (Exception ex)
            {
                ScriptEditorWebviewMod.Logger.Error($"LSP server-message handler threw: {ex}");
            }
        }
    }

    private static int IndexOfHeaderEnd(List<byte> buffer)
    {
        for (var i = 0; i + 3 < buffer.Count; i++)
            if (buffer[i] == (byte)'\r' && buffer[i + 1] == (byte)'\n' &&
                buffer[i + 2] == (byte)'\r' && buffer[i + 3] == (byte)'\n')
                return i;

        return -1;
    }

    private static int ParseContentLength(string header)
    {
        foreach (var line in header.Split("\r\n", StringSplitOptions.RemoveEmptyEntries))
        {
            var idx = line.IndexOf(':');
            if (idx <= 0) continue;

            if (line[..idx].Trim().Equals("Content-Length", StringComparison.OrdinalIgnoreCase) &&
                int.TryParse(line[(idx + 1)..].Trim(), out var length))
                return length;
        }

        return -1;
    }
}