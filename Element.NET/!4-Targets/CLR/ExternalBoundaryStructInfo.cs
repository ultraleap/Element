using System;
using System.Collections.Generic;

namespace Element.CLR
{
    public class ExternalBoundaryStructInfo : IBoundaryStructInfo
    {
        public ExternalBoundaryStructInfo(string elementExpression, Dictionary<string, string> fieldMap, Func<object, ICollection<float>, BoundaryContext, Result> serializeFunc)
        {
            ElementExpression = elementExpression;
            FieldMap = fieldMap;
            _serializeFunc = serializeFunc;
        }

        private readonly Func<object, ICollection<float>, BoundaryContext, Result> _serializeFunc;

        public string ElementExpression { get; }
        public Dictionary<string, string> FieldMap { get; }
        public Result SerializeClrInstance(object clrInstance, ICollection<float> floats, BoundaryContext context) => _serializeFunc(clrInstance, floats, context);
    }
}