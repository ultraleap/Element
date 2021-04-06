using System;
using System.Collections.Generic;

namespace Element.CLR
{
    public class ExternalBoundaryStructInfo : IBoundaryStructInfo
    {
        public ExternalBoundaryStructInfo(string elementExpression, Dictionary<string, string> fieldMap, Func<object, ICollection<float>, Context, Result> serializeFunc)
        {
            ElementExpression = elementExpression;
            FieldMap = fieldMap;
            _serializeFunc = serializeFunc;
        }

        private readonly Func<object, ICollection<float>, Context, Result> _serializeFunc;

        public string ElementExpression { get; }
        public Dictionary<string, string> FieldMap { get; }
        public Result SerializeClrInstance(object clrInstance, ICollection<float> floats, Context context) => _serializeFunc(clrInstance, floats, context);
    }
}