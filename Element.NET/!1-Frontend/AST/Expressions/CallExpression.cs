using System.Linq;

namespace Element.AST
{
    // ReSharper disable once UnusedType.Global
    public class CallExpression : ListOf<Expression>, ISubExpression
    {
        void ISubExpression.Initialize(Declaration declaration)
        {
            foreach (var expr in List)
            {
                expr.Initialize(declaration);
            }
        }

        public bool Validate(SourceContext sourceContext) =>
            List.Aggregate(true, (current, expr) => current & expr.Validate(sourceContext));

        IValue ISubExpression.ResolveSubExpression(IValue previous, IScope scope, CompilationContext compilationContext) =>
            previous is IFunctionSignature function
                ? function.ResolveCall(List.Select(argExpr => argExpr.ResolveExpression(scope, compilationContext)).ToArray(), false, compilationContext)
                : compilationContext.LogError(16, $"{previous} cannot be called - it is not a function");
    }
}