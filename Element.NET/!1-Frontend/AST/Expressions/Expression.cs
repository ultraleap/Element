using Lexico;

namespace Element.AST
{
    [WhitespaceSurrounded, MultiLine]
    // ReSharper disable once ClassNeverInstantiated.Global
    public abstract class Expression
    {
        public abstract IValue ResolveExpression(IScope scope, CompilationContext compilationContext);
    }
}