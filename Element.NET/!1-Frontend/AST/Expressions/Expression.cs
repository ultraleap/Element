using Lexico;

namespace Element.AST
{
    public interface IExpression
    {
        Result<IValue> ResolveExpression(IScope scope, CompilationContext compilationContext);
    }
    
    [WhitespaceSurrounded, MultiLine]
    // ReSharper disable once ClassNeverInstantiated.Global
    public abstract class Expression : Declared
    {
        public Result<IValue> ResolveExpression(IScope scope, CompilationContext compilationContext)
        {
            compilationContext.PushTrace(this.MakeTraceSite(ToString()));
            var result = ExpressionImpl(scope, compilationContext);
            compilationContext.PopTrace();
            return result;
        }

        protected abstract Result<IValue> ExpressionImpl(IScope scope, CompilationContext context);
    }

    public static class ExpressionExtensions
    {
        public static void InitializeUsingStubDeclaration(this Expression expression, string expressionString, IScope scope, SourceContext sourceContext) =>
            expression.Initialize(Declaration.MakeStubDeclaration(new Identifier("<stub declaration>"), new ExpressionBody(expression), expressionString, scope), sourceContext);
    }
}