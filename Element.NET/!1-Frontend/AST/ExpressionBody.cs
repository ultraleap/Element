using Lexico;

namespace Element.AST
{
    [WhitespaceSurrounded]
    // ReSharper disable once ClassNeverInstantiated.Global
    public class ExpressionBody
    {
        // ReSharper disable once MemberCanBeProtected.Global
#pragma warning disable 8618
        public ExpressionBody(){}
#pragma warning restore 8618
        public ExpressionBody(Expression expression) => Expression = expression;
        
#pragma warning disable 169
        [Literal("=")] private Unnamed _;
        // ReSharper disable once UnusedAutoPropertyAccessor.Local
        // ReSharper disable once AutoPropertyCanBeMadeGetOnly.Local
        [field: Term] public Expression Expression { get; private set; }
#pragma warning restore 169

        public override string ToString() => $"= {Expression}";
    }
}