using System;

namespace Element.CLR
{
    [AttributeUsage(AttributeTargets.Struct)]
    public class ElementBoundaryStructAttribute : Attribute
    {
        public string ElementTypeExpression { get; }
        public ElementBoundaryStructAttribute(string elementTypeExpression) => ElementTypeExpression = elementTypeExpression;
    }
}