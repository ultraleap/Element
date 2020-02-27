using Lexico;

namespace Element.AST
{
    [WhitespaceSurrounded]
    // ReSharper disable once ClassNeverInstantiated.Global
    public class Binding
    {
#pragma warning disable 169
        [Literal("=")] private Unnamed _bind;
        // ReSharper disable once UnusedAutoPropertyAccessor.Local
        [field: Term] public Expression Expression { get; private set; }
        [Term] private Terminal _terminal;
#pragma warning restore 169

        public override string ToString() => $"= {Expression};";
    }
}