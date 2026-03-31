using System.Globalization;
using System.Text;

namespace TypeGenerator;

internal static class Utility
{
    public static string ToPascalIdentifier(string name)
    {
        if (string.IsNullOrWhiteSpace(name)) return "_";

        var sb = new StringBuilder();
        var words = name.Split(new[] { ' ', '.', '-', ':' }, StringSplitOptions.RemoveEmptyEntries);
        foreach (var w in words)
        {
            var s = w;
            if (s.Length == 0) continue;
            var first = char.ToUpperInvariant(s[0]);
            var rest = s.Length > 1 ? s.Substring(1) : string.Empty;
            sb.Append(first);
            sb.Append(rest);
        }

        var result = sb.ToString();
        // remove invalid chars
        var filtered = new StringBuilder();
        foreach (var c in result)
        {
            if (char.IsLetterOrDigit(c) || c == '_') filtered.Append(c);
        }

        var outStr = filtered.ToString();
        if (string.IsNullOrEmpty(outStr)) outStr = "_";
        if (char.IsDigit(outStr[0])) outStr = "_" + outStr;
        return outStr;
    }
}
