namespace RML.Core.Api;

public interface IMenuNode : IDisposable
{
    IMenuNode AddSubmenu(string text);
    void AddAction(string text, Action onClick);
    void AddSeparator();
    void AddCheckable(string text, bool initial, Action<bool> onToggle);
    void SetIcon(string path);
}
