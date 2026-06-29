using RML.Core.Api;
using RML.Core.Modding;
using RML.Logging;
using Roblox;

namespace DiscordRpc;

[Mod(
    "discord-rpc-dotnet",
    "1.0.0",
    Author = "Revolution",
    Description = "Discord Rich Presence for Roblox Studio")]
public sealed class DiscordRpc : ModBase, IDataModelAware
{
    private DiscordPresenceService? _presence;

    public static ILogger Logger { get; } = Log.CreateLogger("DiscordRPC");

    public void OnDataModelLoaded(DataModel game, DataModelType dataModelType)
    {
        if (_presence is null) return;

        Task.Run(async () =>
        {
            await Task.Delay(1000);

            var playTesting = dataModelType is DataModelType.Client or DataModelType.Server;
            var placeId = game.PlaceId;

            await UpdatePresenceAsync(placeId, playTesting);
        });
    }

    public void OnDataModelUnloaded(DataModel game, DataModelType dataModelType)
    {
        _presence?.SetIdle();
    }

    public override int OnLoad()
    {
        _presence = new DiscordPresenceService();
        _presence.SetIdle();
        return 0;
    }

    public override void OnUnload()
    {
        _presence?.Dispose();
        _presence = null;
    }

    private async Task UpdatePresenceAsync(long placeId, bool playTesting)
    {
        try
        {
            var info = await RobloxPlaceInfo.FetchAsync(placeId).ConfigureAwait(false);
            var name = info?.Name ?? "an untitled place";
            var thumbnail = info?.ThumbnailUrl;

            if (_presence is null) return;

            if (playTesting)
                _presence.SetPlayTesting(name, thumbnail);
            else
                _presence.SetEditing(name, thumbnail);
        }
        catch (Exception ex)
        {
            Logger.Error($"Failed to update presence: {ex.Message}");
        }
    }
}