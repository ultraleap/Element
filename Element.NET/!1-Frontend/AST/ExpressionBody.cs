using Lexico;

namespace Element.AST
{
    [WhitespaceSurrounded]
    // ReSharper disable once ClassNeverInstantiated.Global
    public class ExpressionBody : IDeclared
    {
#pragma warning disable 169
        [Literal("=")] private Unnamed _;
        // ReSharper disable once UnusedAutoPropertyAccessor.Local
        [field: Term] public Expression Expression { get; private set; }
#pragma warning restore 169

        public override string ToString() => $"= {Expression};";
        public Declaration Declarer => Expression.Declarer;
    }
}