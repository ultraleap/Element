namespace Element.CLR
{
    /// <summary>
    /// Something that can be compiled to a linq expression.
    /// </summary>
    public interface ICLRExpression
    {
        System.Linq.Expressions.Expression Compile();
    }
}