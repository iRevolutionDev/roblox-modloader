namespace ScriptEditorWebview.Qt;

internal readonly struct QObject(IntPtr handle)
{
    public IntPtr Handle { get; } = handle;

    public bool IsNull => Handle == IntPtr.Zero;

    public string ClassName => QtAbi.ClassName(Handle);

    public bool Inherits(string className)
    {
        return QtAbi.Inherits(Handle, className);
    }
}

internal readonly struct QWidget(IntPtr handle)
{
    public IntPtr Handle { get; } = handle;

    public bool IsNull => Handle == IntPtr.Zero;

    public string ClassName => QtAbi.ClassName(Handle);

    public bool Inherits(string className)
    {
        return QtAbi.Inherits(Handle, className);
    }

    public IntPtr WinId()
    {
        return QtAbi.WinId(Handle);
    }

    public QWidget Parent => new(QtAbi.ParentWidget(Handle));

    public void SetVisible(bool visible)
    {
        QtAbi.SetVisible(Handle, visible);
    }
}

internal static class QApplication
{
    private static IReadOnlyList<QWidget> AllWidgets()
    {
        var handles = QtAbi.AllWidgets();
        var widgets = new List<QWidget>(handles.Count);
        foreach (var h in handles)
            if (h != IntPtr.Zero)
                widgets.Add(new QWidget(h));

        return widgets;
    }

    public static List<QWidget> FindWidgets(Func<QWidget, bool> predicate)
    {
        var matches = new List<QWidget>();
        foreach (var widget in AllWidgets())
            try
            {
                if (predicate(widget)) matches.Add(widget);
            }
            catch
            {
            }

        return matches;
    }

    public static List<QWidget> ChildWidgets(QWidget parent)
    {
        var children = new List<QWidget>();
        if (parent.IsNull) return children;

        children.AddRange(AllWidgets().Where(widget =>
            widget.Handle != parent.Handle && QtAbi.ParentWidget(widget.Handle) == parent.Handle));

        return children;
    }
}