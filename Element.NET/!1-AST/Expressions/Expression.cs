using Lexico;

namespace Element.AST
{
    [WhitespaceSurrounded, MultiLine]
    // ReSharper disable once ClassNeverInstantiated.Global
    public abstract class Expression : AstNode
    {
        public Result<IValue> ResolveExpression(IScope parentScope, CompilationContext compilationContext)
        {
            compilationContext.PushTrace(this.MakeTraceSite($"{GetType().Name} '{ToString()}'"));
            var result = ExpressionImpl(parentScope, compilationContext).Bind(v => v.FullyResolveValue(compilationContext));
            compilationContext.PopTrace();
            return result;
        }

        protected abstract Result<IValue> ExpressionImpl(IScope parentScope, CompilationContext context);
    }
}