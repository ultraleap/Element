using System;
using System.Collections.Generic;
using Element.AST;
using LExpression = System.Linq.Expressions.Expression;

namespace Element.CLR
{
    public delegate Result<System.Linq.Expressions.Expression> ConvertFunction(IValue value, Type outputType, BoundaryContext context);

    public interface IBoundaryConverter
    {
        Result<IValue> LinqToElement(LExpression parameter, BoundaryContext context);
        Result<LExpression> ElementToLinq(IValue value, Type outputType, ConvertFunction convertFunction, BoundaryContext context);
        Result SerializeClrInstance(object clrInstance, ICollection<float> floats, BoundaryContext context);
    }
}