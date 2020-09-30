using Lexico;

namespace Element.AST
{
    [WhitespaceSurrounded]
    public class ExpressionBody
    {
#pragma warning disable 169, 8618
        [Literal("=")] private Unnamed _;
        // ReSharper disable once UnusedAutoPropertyAccessor.Local
        [field: Term] public Expression Expression { get; private set; }
#pragma warning restore 169, 8618
    }
}