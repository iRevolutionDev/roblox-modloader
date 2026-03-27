using RobloxModLoader.Managed.Api;

namespace RobloxModLoader.Managed.Modding;

public abstract class ModBase : IMod
{
    public abstract int Initialize(ModContext context);

    public virtual void Shutdown()
    {
    }
}
