using Lexico;

namespace Element.AST
{
    [WhitespaceSurrounded, MultiLine]
    // ReSharper disable once ClassNeverInstantiated.Global
    public abstract class Expression : AstNode
    {
        public Result<IValue> ResolveExpression(IScope scope, CompilationContext compilationContext)
        {
            compilationContext.PushTrace(MakeTraceSite(ToString()));
            var result = ExpressionImpl(scope, compilationContext);
            compilationContext.PopTrace();
            return result;
        }

        protected abstract Result<IValue> ExpressionImpl(IScope scope, CompilationContext context);
    }
}