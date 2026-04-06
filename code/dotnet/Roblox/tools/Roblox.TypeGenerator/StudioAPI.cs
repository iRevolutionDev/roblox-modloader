using System.Net.Http;
using System.Text.Json;
using System.Threading.Tasks;
using TypeGenerator.APITypes;

namespace TypeGenerator;

internal static class StudioAPI
{
    private static readonly string _baseURL = "https://raw.githubusercontent.com/MaximumADHD/Roblox-Client-Tracker/roblox/";

    public static async Task<Dump> GetDump()
    {
        var body = await Request("Mini-API-Dump.json");
        return JsonSerializer.Deserialize<Dump>(body) ?? new Dump();
    }

    public static async Task<ReflectionMetadataReader> GetReflectionMetadata()
    {
        var body = await Request("ReflectionMetadata.xml");
        return new ReflectionMetadataReader(body);
    }

    public static async Task<ApiDocsReader> GetApiDocs()
    {
        var body = await Request("api-docs/en-us.json");
        return new ApiDocsReader(body);
    }

    private static async Task<string> Request(string endpoint)
    {
        using HttpClient client = new HttpClient();
        var response = await client.GetAsync(_baseURL + endpoint);
        response.EnsureSuccessStatusCode();
        return await response.Content.ReadAsStringAsync();
    }
}

