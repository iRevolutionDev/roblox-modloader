using Roblox;

namespace RML.Core.Api;

public interface IDataModelAware
{
    public void OnDataModelLoaded(DataModel dataModel, DataModelType dataModelType);

    public void OnDataModelUnloaded(DataModel dataModel, DataModelType dataModelType);
}