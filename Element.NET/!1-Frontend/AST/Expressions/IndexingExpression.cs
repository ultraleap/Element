using Lexico;

namespace Element.AST
{
    // ReSharper disable once ClassNeverInstantiated.Global
    public class IndexingExpression : ISubExpression
    {
#pragma warning disable 169
        [Literal(".")] private Unnamed _;
        // ReSharper disable once UnusedAutoPropertyAccessor.Local
        [field: Term] public Identifier Identifier { get; private set; }
#pragma warning restore 169

        public override string ToString() => $".{Identifier}";

        public IValue ResolveSubExpression(IValue previous, IScope containingScope, CompilationContext compilationContext) =>
            previous switch
            {
                IScope scope => scope[Identifier, compilationContext],
                _ => compilationContext.LogError(16, $"'{previous}' is not indexable")
            };
    }
}