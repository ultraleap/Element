using System.Collections.Generic;

namespace Element.CLR
{
    public interface IBoundaryStructInfo
    {
        string ElementExpression { get; }
        Dictionary<string, string> FieldMap { get; }
        Result SerializeClrInstance(object clrInstance, ICollection<float> floats, BoundaryContext context);
    }
}