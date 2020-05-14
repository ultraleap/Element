using Lexico;

namespace Element.AST
{
    [WhitespaceSurrounded, MultiLine]
    // ReSharper disable once ClassNeverInstantiated.Global
    public abstract class Expression : Declared
    {
        public IValue ResolveExpression(IScope scope, CompilationContext compilationContext)
        {
            compilationContext.PushTrace(MakeTraceSite());
            var result = ExpressionImpl(scope, compilationContext);
            compilationContext.PopTrace();
            return result;
        }

        protected abstract IValue ExpressionImpl(IScope scope, CompilationContext context);
    }

    public static class ExpressionExtensions
    {
        public static void InitializeUsingStubDeclaration(this Expression expression, string expressionString, CompilationContext compilationContext) =>
            expression.Initialize(Declaration.MakeStubDeclaration(new Identifier("<stub declaration>"), new ExpressionBody(expression), expressionString, compilationContext));
    }
}