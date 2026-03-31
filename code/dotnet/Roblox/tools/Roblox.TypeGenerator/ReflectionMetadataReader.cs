using System.Text.RegularExpressions;
using System.Xml.Linq;
using System.Xml.XPath;

namespace TypeGenerator;

internal class ReflectionMetadataReader
{
    private readonly XElement _metadata;

    public ReflectionMetadataReader(string body)
    {
        _metadata = XElement.Parse(body);
    }

    public string? ReadClassDesc(string name)
    {
        var el = _metadata.XPathSelectElement($"//Item[@class='ReflectionMetadataClass']/Properties/string[@name='Name'][text()='{name}']/../../Properties/string[@name='summary']");
        return el?.Value;
    }

    public string? ReadMemberDesc(string className, string name)
    {
        // search member by name under class
        var basePath = $"//Item[@class='ReflectionMetadataClass']/Properties/string[@name='Name'][text()='{className}']/../../";
        var query = basePath + "Item/Item[@class='ReflectionMetadataMember']/Properties/string[@name='Name'][text()='" + name + "']/../string[@name='summary']";
        var el = _metadata.XPathSelectElement(query);
        return el?.Value;
    }
}
