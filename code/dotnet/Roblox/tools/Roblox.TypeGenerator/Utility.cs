using System.Globalization;
using System.Text;

namespace TypeGenerator;

internal static class Utility
{
    public static string ToPascalIdentifier(string name)
    {
        if (string.IsNullOrWhiteSpace(name))
        {
            return "_";
        }

        var sb = new StringBuilder();
        var words = name.Split([' ', '.', '-', ':'], StringSplitOptions.RemoveEmptyEntries);
        foreach (var w in words)
        {
            if (w.Length == 0)
            {
                continue;
            }

            var first = char.ToUpperInvariant(w[0]);
            var rest = w.Length > 1 ? w.Substring(1) : string.Empty;
            sb.Append(first);
            sb.Append(rest);
        }

        var result = sb.ToString();

        var filtered = new StringBuilder();
        foreach (var c in result.Where(c => char.IsLetterOrDigit(c) || c == '_'))
        {
            filtered.Append(c);
        }

        var outStr = filtered.ToString();
        if (string.IsNullOrEmpty(outStr))
        {
            outStr = "_";
        }

        if (char.IsDigit(outStr[0]))
        {
            outStr = "_" + outStr;
        }

        return outStr;
    }
}
