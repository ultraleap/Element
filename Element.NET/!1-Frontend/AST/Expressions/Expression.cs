using Lexico;

namespace Element.AST
{
    [WhitespaceSurrounded, MultiLine]
    // ReSharper disable once ClassNeverInstantiated.Global
    public abstract class Expression : IDeclared
    {
        public abstract void Initialize(Declaration declaration);
        public abstract IValue ResolveExpression(IScope scope, CompilationContext compilationContext);
        public Declaration Declarer { get; protected set; }
    }
}