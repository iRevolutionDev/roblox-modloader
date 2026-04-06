namespace RML.Core.Api;

public interface IModLogger
{
    public void Info(string msg);
    public void Warn(string msg);
    public void Error(string msg);
}