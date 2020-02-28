using System.Linq;

namespace Element.AST
{
    // ReSharper disable once UnusedType.Global
    public class CallExpression : ListOf<Expression>, ISubExpression
    {
        public IValue ResolveSubExpression(IValue previous, IScope containingScope, CompilationContext compilationContext)
        {
            if (!(previous is ICallable callable)) return compilationContext.LogError(16, $"{previous} is not callable");

            // Compile the arguments for this call expression
            var arguments = List.Select(argExpr => argExpr.ResolveExpression(containingScope, compilationContext)).ToArray();

            return callable.Call(arguments, compilationContext);
        }
    }

    public static class CallExtensions
    {
        public static bool ValidateArguments(this IValue[] arguments, int count, CompilationContext compilationContext)
        {
            if (arguments.Length != count)
            {
                compilationContext.LogError(6, $"Expected '{count}' arguments but got '{arguments.Length}'");
                return false;
            }

            return true;
        }

        public static bool ValidateArguments(this IValue[] arguments, Port[] ports, IScope scope, CompilationContext compilationContext) =>
            ValidateArguments(arguments, ports.Length, compilationContext)
            && ValidateConstraints(arguments, ports, scope, compilationContext);

        private static bool ValidateConstraints(IValue[] arguments, Port[] ports, IScope scope, CompilationContext compilationContext)
        {
            var success = true;
            for (var i = 0; i < ports.Length; i++)
            {
                var arg = arguments[i];
                var port = ports[i];
                var constraint = port.Type.Resolve(scope, compilationContext);
                if (!constraint.MatchesConstraint(arg, compilationContext))
                {
                    compilationContext.LogError(8, $"Value given for port '{port}' does not match '{constraint}' constraint");
                    success = false;
                }
            }

            return success;
        }
    }
}