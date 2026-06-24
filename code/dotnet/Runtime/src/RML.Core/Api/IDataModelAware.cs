using Roblox;

namespace RML.Core.Api;

public interface IDataModelAware
{
    public void OnDataModelLoaded(DataModel game, DataModelType dataModelType);

    public void OnDataModelUnloaded(DataModel game, DataModelType dataModelType);
}