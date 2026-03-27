using RML.Core.Api;

namespace RML.Core.Modding;

public abstract class ModBase : IMod
{
    public abstract int Initialize(ModContext context);

    public virtual void Shutdown()
    {
    }
}
