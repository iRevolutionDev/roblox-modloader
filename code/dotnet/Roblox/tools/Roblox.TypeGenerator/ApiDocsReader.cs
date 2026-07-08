using System;
using System.Collections.Generic;
using System.Text.Json;
using System.Text.RegularExpressions;

namespace TypeGenerator;

internal sealed record ApiDoc(string Documentation, string LearnMoreLink);

internal sealed class ApiDocsReader
{
    private readonly Dictionary<string, ApiDoc> _entries;

    public ApiDocsReader(string json)
    {
        _entries = new Dictionary<string, ApiDoc>(StringComparer.Ordinal);

        using var doc = JsonDocument.Parse(json);
        foreach (JsonProperty prop in doc.RootElement.EnumerateObject())
        {
            var key = prop.Name;
            if (!prop.Value.TryGetProperty("documentation", out JsonElement docEl))
            {
                continue;
            }

            var documentation = docEl.GetString() ?? string.Empty;

            var learnMore = string.Empty;
            if (prop.Value.TryGetProperty("learn_more_link", out JsonElement linkEl))
            {
                learnMore = linkEl.GetString() ?? string.Empty;
            }

            _entries[key] = new ApiDoc(documentation, learnMore);
        }
    }
    
    public ApiDoc? GetClass(string className)
    {
        _entries.TryGetValue($"@roblox/globaltype/{className}", out var doc);
        return doc;
    }
    
    public ApiDoc? GetMember(string className, string memberName)
    {
        _entries.TryGetValue($"@roblox/globaltype/{className}.{memberName}", out var doc);
        return doc;
    }

    private static readonly Regex _sHtmlTag = new(@"<[^>]+>", RegexOptions.Compiled);

    public static string HtmlToXmlDoc(string? html)
    {
        if (string.IsNullOrWhiteSpace(html))
        {
            return string.Empty;
        }

        var s = html;
        s = Regex.Replace(s, @"<code>(.*?)</code>",  m => $"<c>{EscapeXml(m.Groups[1].Value)}</c>",  RegexOptions.Singleline | RegexOptions.IgnoreCase);
        s = Regex.Replace(s, @"<em>(.*?)</em>",      m => $"<c>{EscapeXml(m.Groups[1].Value)}</c>",  RegexOptions.Singleline | RegexOptions.IgnoreCase);
        s = Regex.Replace(s, @"<strong>(.*?)</strong>", m => $"<b>{EscapeXml(m.Groups[1].Value)}</b>", RegexOptions.Singleline | RegexOptions.IgnoreCase);
        s = Regex.Replace(s, @"<a\s[^>]*href=[""']([^""']+)[""'][^>]*>(.*?)</a>",
            m => $"<see href=\"{m.Groups[1].Value}\">{EscapeXml(m.Groups[2].Value)}</see>",
            RegexOptions.Singleline | RegexOptions.IgnoreCase);
        
        s = _sHtmlTag.Replace(s, string.Empty);
        
        s = s.Replace("&amp;",  "&")
              .Replace("&lt;",   "<")
              .Replace("&gt;",   ">")
              .Replace("&quot;", "\"")
              .Replace("&apos;", "'")
              .Replace("&#160;", " ")
              .Replace("&nbsp;", " ");
        
        s = EscapeOutsideTags(s);
        
        s = Regex.Replace(s, @"\s+", " ").Trim();

        return s;
    }

    private static string EscapeXml(string s) =>
        s.Replace("&", "&amp;").Replace("<", "&lt;").Replace(">", "&gt;");
    
    private static string EscapeOutsideTags(string s)
    {
        var result = new System.Text.StringBuilder(s.Length);
        var i = 0;
        while (i < s.Length)
        {
            switch (s[i])
            {
                case '<':
                {
                    int end = s.IndexOf('>', i);
                    if (end < 0)
                    {
                        result.Append("&lt;");
                        i++;
                    }
                    else
                    {
                        result.Append(s, i, end - i + 1);
                        i = end + 1;
                    }

                    break;
                }
                case '&':
                    result.Append("&amp;");
                    i++;
                    break;
                default:
                    result.Append(s[i++]);
                    break;
            }
        }
        return result.ToString();
    }
}
