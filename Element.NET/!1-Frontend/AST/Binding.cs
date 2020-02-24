using Lexico;

namespace Element.AST
{
    [WhitespaceSurrounded]
    // ReSharper disable once ClassNeverInstantiated.Global
    public class Binding
    {
        [Literal("=")] private Unnamed _bind;
        // ReSharper disable once UnassignedGetOnlyAutoProperty
        [field: Term] public Expression Expression { get; }
        [Term] private Terminal _terminal;

        public override string ToString() => $"= {Expression};";
    }
}