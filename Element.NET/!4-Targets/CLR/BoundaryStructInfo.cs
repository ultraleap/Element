using System;
using System.Collections.Generic;
using System.Linq;
using System.Linq.Expressions;

namespace Element.CLR
{
    public class BoundaryStructInfo : IBoundaryStructInfo
    {
        private readonly Type _structType;
        private readonly Func<object, IEnumerable<object>> _getFieldsFunc;

        public BoundaryStructInfo(Type structType, string elementExpression, Dictionary<string, string>? fieldMap)
        {
            _structType = structType;
            ElementExpression = elementExpression;
            FieldMap = fieldMap ?? structType
                                  .GetFields()
                                  .ToDictionary(f => f.Name,
                                       f => $"{char.ToLower(f.Name[0])}{f.Name.Substring(1)}");
            
            // Below LinqExpression is equivalent to this reflection-based implementation of a getter
            // IEnumerable<object> getFields(object input) => FieldMap.Select(f => (object)structType.GetField(f).GetFloat(input)).ToArray();
            
            var inputExpr = Expression.Parameter(typeof(object));
            var parameterAsStructType = Expression.Convert(inputExpr, structType);
            var getFields = FieldMap.Keys.Select(f => Expression.Convert(Expression.Field(parameterAsStructType, structType.GetField(f)), typeof(object)));
            var fieldArrayExpression = Expression.NewArrayInit(typeof(object), getFields);
            _getFieldsFunc = Expression.Lambda<Func<object, IEnumerable<object>>>(fieldArrayExpression, false, inputExpr).Compile();
        }

        public string ElementExpression { get; }
        public Dictionary<string, string> FieldMap { get; }
        public Result SerializeClrInstance(object clrInstance, ICollection<float> floats, Context context) =>
            _structType.IsInstanceOfType(clrInstance)
                ? _getFieldsFunc(clrInstance).Select(f => context.SerializeClrInstance(f, floats)).Fold()
                : throw new InvalidOperationException($"Expected '{nameof(clrInstance)}' to be of type '{_structType}'");
    }
}