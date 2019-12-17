namespace Element.CLR
{
    using System;
    
    /// <summary>
    /// 
    /// </summary>
    public interface ICLRExpression
    {
        System.Linq.Expressions.Expression Compile(Func<Expression, System.Linq.Expressions.Expression> compileOther);
    }
}