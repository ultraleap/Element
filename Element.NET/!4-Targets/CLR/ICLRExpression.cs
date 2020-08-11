namespace Element.CLR
{
    using System;
    
    /// <summary>
    /// An expression that can be compiled to a linq expression.
    /// </summary>
    public interface ICLRExpression
    {
        System.Linq.Expressions.Expression Compile(Func<Expression, System.Linq.Expressions.Expression> compileOther);
    }
}