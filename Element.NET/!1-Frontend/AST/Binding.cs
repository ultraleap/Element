using Lexico;

namespace Element.AST
{
    [WhitespaceSurrounded]
    // ReSharper disable once ClassNeverInstantiated.Global
    public class Binding : ExpressionBody
    {
#pragma warning disable 169
        [Term] private Terminal _terminal;
#pragma warning restore 169

        public override string ToString() => $"= {Expression};";
    }
}