using System.Collections.Generic;
using ResultNET;

namespace Element.CLR
{
    public interface IBoundaryStructInfo
    {
        string ElementExpression { get; }
        Dictionary<string, string> FieldMap { get; }
        Result SerializeClrInstance(object clrInstance, ICollection<float> floats, Context context);
    }
}