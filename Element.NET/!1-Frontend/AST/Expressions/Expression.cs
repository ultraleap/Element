using Lexico;

namespace Element.AST
{
    [WhitespaceSurrounded, MultiLine]
    // ReSharper disable once ClassNeverInstantiated.Global
    public abstract class Expression : IDeclared
    {
        public abstract void Initialize(Declaration declaration);
        public abstract IValue ResolveExpression(IScope scope, CompilationContext compilationContext);
        public abstract bool Validate(SourceContext sourceContext);
        public Declaration Declarer { get; protected set; }
    }

    public static class ExpressionExtensions
    {
        public static void InitializeUsingStubDeclaration(this Expression expression, CompilationContext compilationContext) =>
            expression.Initialize(Declaration.MakeStubDeclaration(new Identifier("<stub declaration>"), new ExpressionBody(expression), compilationContext));
    }
}