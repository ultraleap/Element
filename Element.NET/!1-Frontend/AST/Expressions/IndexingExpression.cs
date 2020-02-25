using Lexico;

namespace Element.AST
{
    public class IndexingExpression : ISubExpression
    {
        [Literal(".")] private Unnamed _;
        [field: Term] public Identifier Identifier { get; }

        public override string ToString() => $".{Identifier}";

        public IValue ResolveSubExpression(IValue previous, IScope containingScope, CompilationContext compilationContext) =>
            previous switch
            {
                Literal lit => DeclaredFunction.ResolveAsInstanceFunction(Identifier, lit, NumType.Instance.Declarer as Struct, compilationContext),
                IScope scope => scope[Identifier, compilationContext].ToValue(Identifier, scope, compilationContext),
                _ => compilationContext.LogError(16, $"'{previous}' is not indexable")
            };
    }
}