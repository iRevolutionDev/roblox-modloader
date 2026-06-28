using System.Collections.Concurrent;
using System.Text.Json;

namespace DiscordRpc;

internal sealed record PlaceInfo(string Name, string? ThumbnailUrl);

internal static class RobloxPlaceInfo
{
    private static readonly HttpClient Http = CreateClient();
    private static readonly ConcurrentDictionary<long, PlaceInfo> Cache = new();

    public static async Task<PlaceInfo?> FetchAsync(long placeId)
    {
        if (placeId <= 0) return null;

        if (Cache.TryGetValue(placeId, out var cached)) return cached;

        try
        {
            var universeId = await GetUniverseIdAsync(placeId).ConfigureAwait(false);
            if (universeId <= 0) return null;

            var name = await GetGameNameAsync(universeId).ConfigureAwait(false) ?? "an untitled place";
            var thumbnail = await GetThumbnailAsync(universeId).ConfigureAwait(false);

            var info = new PlaceInfo(name, thumbnail);
            Cache[placeId] = info;
            return info;
        }
        catch (Exception ex)
        {
            DiscordRpc.Logger.Error($"Failed to resolve place {placeId}: {ex.Message}");
            return null;
        }
    }

    private static async Task<long> GetUniverseIdAsync(long placeId)
    {
        await using var stream = await Http
            .GetStreamAsync($"https://apis.roblox.com/universes/v1/places/{placeId}/universe")
            .ConfigureAwait(false);
        using var doc = await JsonDocument.ParseAsync(stream).ConfigureAwait(false);

        return doc.RootElement.TryGetProperty("universeId", out var id) && id.TryGetInt64(out var value)
            ? value
            : 0;
    }

    private static async Task<string?> GetGameNameAsync(long universeId)
    {
        await using var stream = await Http
            .GetStreamAsync($"https://games.roblox.com/v1/games?universeIds={universeId}")
            .ConfigureAwait(false);
        using var doc = await JsonDocument.ParseAsync(stream).ConfigureAwait(false);

        return FirstDataItem(doc, out var item) && item.TryGetProperty("name", out var name)
            ? name.GetString()
            : null;
    }

    private static async Task<string?> GetThumbnailAsync(long universeId)
    {
        await using var stream = await Http
            .GetStreamAsync(
                $"https://thumbnails.roblox.com/v1/games/icons?universeIds={universeId}" +
                "&returnPolicy=PlaceHolder&size=512x512&format=Png&isCircular=false")
            .ConfigureAwait(false);
        using var doc = await JsonDocument.ParseAsync(stream).ConfigureAwait(false);

        return FirstDataItem(doc, out var item) && item.TryGetProperty("imageUrl", out var url)
            ? url.GetString()
            : null;
    }

    private static bool FirstDataItem(JsonDocument doc, out JsonElement item)
    {
        if (doc.RootElement.TryGetProperty("data", out var data)
            && data.ValueKind == JsonValueKind.Array
            && data.GetArrayLength() > 0)
        {
            item = data[0];
            return true;
        }

        item = default;
        return false;
    }

    private static HttpClient CreateClient()
    {
        var client = new HttpClient { Timeout = TimeSpan.FromSeconds(10) };
        client.DefaultRequestHeaders.UserAgent.ParseAdd("RML/1.0");
        return client;
    }
}