using Lexico;

namespace Element.AST
{
    [WhitespaceSurrounded]
    public class ExpressionBody
    {
#pragma warning disable 8618
        // ReSharper disable once MemberCanBeProtected.Global
        public ExpressionBody(){}
#pragma warning restore 8618
        public ExpressionBody(Expression expression) => Expression = expression;
        
#pragma warning disable 169
        [Literal("=")] private Unnamed _;
        // ReSharper disable once AutoPropertyCanBeMadeGetOnly.Local
        [field: Term] public Expression Expression { get; private set; }
#pragma warning restore 169
    }
}