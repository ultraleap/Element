using Lexico;

namespace Element.AST
{
    [WhitespaceSurrounded(ParserFlags = ParserFlags.IgnoreInTrace), MultiLine]
    // ReSharper disable once ClassNeverInstantiated.Global
    public abstract class Expression : AstNode
    {
        public Result<IValue> ResolveExpression(IScope parentScope, Context context)
        {
            context.TraceStack.Push(this.MakeTraceSite($"{GetType().Name} '{ToString()}'"));
            context.Aspect?.BeforeExpression(this, parentScope);
            var resolveResult = ExpressionImpl(parentScope, context);
            var result = context.Aspect?.Expression(this, parentScope, resolveResult) ?? resolveResult;
            context.TraceStack.Pop();
            return result;
        }

        protected abstract Result<IValue> ExpressionImpl(IScope parentScope, Context context);
    }
}