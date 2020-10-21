using Lexico;

namespace Element.AST
{
    [WhitespaceSurrounded, MultiLine]
    // ReSharper disable once ClassNeverInstantiated.Global
    public abstract class Expression : AstNode
    {
        public Result<IValue> ResolveExpression(IScope parentScope, Context context)
        {
            context.TraceStack.Push(this.MakeTraceSite($"{GetType().Name} '{ToString()}'"));
            var resolveResult = (context.Aspect?.BeforeExpression(this, parentScope, context) ?? Result.Success)
                .Bind(() => ExpressionImpl(parentScope, context));
            var result = context.Aspect != null
                             ? resolveResult.Bind(resolvedValue => context.Aspect.Expression(this, parentScope, resolvedValue, context))
                             : resolveResult;
            context.TraceStack.Pop();
            return result;
        }

        protected abstract Result<IValue> ExpressionImpl(IScope parentScope, Context context);
    }
}