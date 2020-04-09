using Lexico;

namespace Element.AST
{
    // ReSharper disable once ClassNeverInstantiated.Global
    public class IndexingExpression : ISubExpression
    {
#pragma warning disable 169
        // ReSharper disable once UnusedAutoPropertyAccessor.Local
        [field: Term, Prefix(".")] private Identifier Identifier { get; set; }
#pragma warning restore 169

        public override string ToString() => $".{Identifier}";

        IValue ISubExpression.ResolveSubExpression(IValue previous, IScope _, CompilationContext compilationContext) =>
            previous is IScope scope
                ? scope[Identifier, false, compilationContext]
                : compilationContext.LogError(16, $"'{previous}' is not indexable");
    }
}