using Lexico;

namespace Element.AST
{
    // ReSharper disable once ClassNeverInstantiated.Global
    public class IndexingExpression : ISubExpression
    {
#pragma warning disable 169
        [Literal(".")] private Unnamed _;
        // ReSharper disable once UnusedAutoPropertyAccessor.Local
        [field: Term] private Identifier Identifier { get; set; }
#pragma warning restore 169

        public override string ToString() => $".{Identifier}";

        void ISubExpression.Initialize(Declaration declaration) { } // No-op
        IValue ISubExpression.ResolveSubExpression(IValue previous, CompilationContext compilationContext) =>
            previous is IScope scope
                ? scope[Identifier, false, compilationContext]
                : compilationContext.LogError(16, $"'{previous}' is not indexable");
    }
}