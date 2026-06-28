using DiscordRPC;

namespace DiscordRpc;

internal sealed class DiscordPresenceService : IDisposable
{
    private const string ApplicationId = "1396335710755098757";

    private readonly DiscordRpcClient _client;
    private readonly DateTime _sessionStart = DateTime.UtcNow;

    public DiscordPresenceService()
    {
        _client = new DiscordRpcClient(ApplicationId);
        _client.OnReady += (_, e) => DiscordRpc.Logger.Info($"Connected to Discord as {e.User.Username}");
        _client.OnError += (_, e) => DiscordRpc.Logger.Error($"Discord error: {e.Message}");
        _client.Initialize();
    }

    public void Dispose()
    {
        try
        {
            if (_client.IsDisposed) return;

            _client.ClearPresence();
            _client.Dispose();
        }
        catch (Exception ex)
        {
            DiscordRpc.Logger.Error($"Error during shutdown: {ex.Message}");
        }
    }

    public void SetEditing(string placeName, string? thumbnailUrl)
    {
        Set($"Editing {placeName}", PresenceState.Editing, thumbnailUrl);
    }

    public void SetPlayTesting(string placeName, string? thumbnailUrl)
    {
        Set($"Play testing {placeName}", PresenceState.PlayTesting, thumbnailUrl);
    }

    public void SetIdle()
    {
        Set("In Roblox Studio", PresenceState.Idle, null);
    }

    private void Set(string details, PresenceState state, string? thumbnailUrl)
    {
        if (_client.IsDisposed) return;

        _client.SetPresence(new RichPresence
        {
            Details = details,
            State = state.ToString(),
            Timestamps = new Timestamps(_sessionStart),
            Assets = new Assets
            {
                LargeImageKey = string.IsNullOrEmpty(thumbnailUrl) ? "studio" : thumbnailUrl,
                LargeImageText = "Roblox Studio",
                SmallImageKey = state switch
                {
                    PresenceState.Idle => "idle",
                    PresenceState.Editing => "editing",
                    PresenceState.PlayTesting => "play",
                    _ => "studio"
                },
                SmallImageText = "RobloxModLoader"
            }
        });
    }

    private enum PresenceState
    {
        Idle,
        Editing,
        PlayTesting
    }
}