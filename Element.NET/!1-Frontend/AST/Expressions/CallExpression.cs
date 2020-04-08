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

        IValue ISubExpression.ResolveSubExpression(IValue previous, CompilationContext compilationContext) =>
            previous is IFunctionSignature function
                ? function.ResolveCall(List.Select(argExpr => argExpr.ResolveExpression(compilationContext)).ToArray(), false,
                                       compilationContext)
                : compilationContext.LogError(16, $"{previous} cannot be called - it is not a function");
    }
}