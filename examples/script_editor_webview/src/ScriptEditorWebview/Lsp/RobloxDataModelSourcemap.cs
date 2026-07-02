using System.Diagnostics;
using System.Text;
using System.Text.Json;
using Roblox;

namespace ScriptEditorWebview.Lsp;

internal sealed class RobloxDataModelSourcemap(DataModel game)
{
    private const int MaxNodes = 200_000;

    private readonly Queue<Node> _frontier = new();
    private Node _root = new("game", "DataModel", game);
    private bool _seeded;

    public int NodeCount { get; private set; }

    public void Restart()
    {
        _root = new Node("game", "DataModel", game);
        _frontier.Clear();
        _frontier.Enqueue(_root);
        NodeCount = 1;
        _seeded = true;
    }

    public bool Step(long budgetMs)
    {
        if (!_seeded) Restart();

        var stopwatch = Stopwatch.StartNew();

        while (_frontier.Count > 0)
        {
            if (NodeCount >= MaxNodes)
            {
                _frontier.Clear();
                break;
            }

            var node = _frontier.Peek();
            node.Pending ??= node.Instance.GetChildren();

            while (node.Cursor < node.Pending.Count)
            {
                if (stopwatch.ElapsedMilliseconds >= budgetMs)
                    return false;

                if (NodeCount >= MaxNodes)
                    break;

                var child = node.Pending[node.Cursor++];

                var childNode = new Node(child.Name, child.ClassName, child);
                (node.Children ??= []).Add(childNode);
                _frontier.Enqueue(childNode);
                NodeCount++;
            }

            _frontier.Dequeue();
            node.Pending = null;
        }

        return true;
    }

    public string? Serialize()
    {
        try
        {
            using var buffer = new MemoryStream(256 * 1024);
            using (var writer = new Utf8JsonWriter(buffer))
            {
                WriteNode(writer, _root);
            }

            var json = Encoding.UTF8.GetString(buffer.GetBuffer(), 0, (int)buffer.Length);
            return json.Length > 0 ? json : null;
        }
        catch (Exception ex)
        {
            ScriptEditorWebviewMod.Logger.Debug($"sourcemap serialize failed: {ex.Message}");
            return null;
        }
    }

    private static void WriteNode(Utf8JsonWriter writer, Node node)
    {
        writer.WriteStartObject();
        writer.WriteString("Name", node.Name);
        writer.WriteString("ClassName", node.ClassName);

        writer.WriteStartArray("FilePaths");
        writer.WriteEndArray();

        writer.WriteStartArray("Children");
        if (node.Children is not null)
            foreach (var child in node.Children)
                WriteNode(writer, child);

        writer.WriteEndArray();
        writer.WriteEndObject();
    }

    private sealed class Node(string name, string className, Instance instance)
    {
        public readonly string ClassName = className;
        public readonly Instance Instance = instance;
        public readonly string Name = name;
        public List<Node>? Children;
        public int Cursor;

        public IReadOnlyList<Instance>? Pending;
    }
}