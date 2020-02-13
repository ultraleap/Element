using Lexico;

namespace Element.AST
{
    [WhitespaceSurrounded]
    public class Binding : IFunctionBody
    {
        [Literal("=")] private Unnamed _bind;
        [field: Term] public Expression Expression { get; }
        [Term] private Terminal _terminal;

        public override string ToString() => $"= {Expression};";
    }
}