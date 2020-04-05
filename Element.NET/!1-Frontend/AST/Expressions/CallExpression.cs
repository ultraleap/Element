using System.Linq;

namespace Element.AST
{
    // ReSharper disable once UnusedType.Global
    public class CallExpression : ListOf<Expression>, ISubExpression
    {
        public IValue ResolveSubExpression(IValue previous, IScope resolutionScope, CompilationContext compilationContext)
        {
            if (!(previous is IFunctionSignature function)) return compilationContext.LogError(16, $"{previous} is not a function");

            // Compile the arguments for this call expression
            var arguments = List.Select(argExpr => argExpr.ResolveExpression(resolutionScope, compilationContext)).ToArray();

            return function.ResolveCall(arguments, resolutionScope, false, compilationContext);
        }
    }
}