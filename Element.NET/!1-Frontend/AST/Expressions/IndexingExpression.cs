using Lexico;

namespace Element.AST
{
    public class IndexingExpression : ISubExpression
    {
        [Literal(".")] private Unnamed _;
        [field:Term] public Identifier Identifier { get; }

        public override string ToString() => $".{Identifier}";

        public IValue ResolveSubExpression(IValue previous, IScope _, CompilationContext compilationContext) =>
            previous is IScope scope
                ? scope[Identifier].ToValue(Identifier, compilationContext)
                : compilationContext.LogError(16, $"'{previous}' is not indexable");
    }
}