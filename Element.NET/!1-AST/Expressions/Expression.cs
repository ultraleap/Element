using Lexico;
using ResultNET;

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
            context.Aspect?.Expression(this, parentScope, resolveResult);
            context.TraceStack.Pop();
            return resolveResult;
        }

        protected abstract Result<IValue> ExpressionImpl(IScope parentScope, Context context);
    }
}