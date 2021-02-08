namespace Element.CLR
{
    /// <summary>
    /// Something that can be implicitly compiled to a linq expression.
    /// </summary>
    public interface ILinqExpression
    {
        System.Linq.Expressions.Expression Expression { get; }
    }
}