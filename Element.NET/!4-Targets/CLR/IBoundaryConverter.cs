using System;
using System.Collections.Generic;
using Element.AST;
using LExpression = System.Linq.Expressions.Expression;

namespace Element.CLR
{
    public delegate Result<System.Linq.Expressions.Expression> ConvertFunction(IValue value, Type outputType, Context context);

    public interface IBoundaryConverter
    {
        Result<IValue> LinqToElement(LExpression parameter, Context context);
        Result<LExpression> ElementToLinq(IValue value, Type outputType, ConvertFunction convertFunction, Context context);
        Result SerializeClrInstance(object clrInstance, ICollection<float> floats, Context context);
    }
}