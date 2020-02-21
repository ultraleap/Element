using System;
using System.Linq;

namespace Element.AST
{
    public class CallExpression : ListOf<Expression>, ISubExpression
    {
        public IValue ResolveSubExpression(IValue previous, IScope callSite, CompilationContext compilationContext)
        {
            if (!(previous is ICallable callable)) return compilationContext.LogError(16, $"{previous} is not callable");

            // Compile the arguments for this call expression
            var arguments = List.Select(argExpr => argExpr.ResolveExpression(callSite, compilationContext)).ToArray();

            return callable.Call(arguments, compilationContext);
        }
    }

    public static class CallExtensions
    {
        public static bool ValidateArgumentCount(this IValue[] arguments, int expectedArgCount,
                                                 CompilationContext compilationContext)
        {
            if (arguments.Length != expectedArgCount)
            {
                compilationContext.LogError(6, $"Expected '{expectedArgCount}' arguments but got '{arguments.Length}'");
                return false;
            }

            return true;
        }

        // TODO: Move validate functions to DeclaredCallable?
        public static bool ValidateArgumentConstraints(this IValue[] arguments, Port[] ports,
                                                       Func<Type, CompilationContext, IConstraint?> findConstraint,
                                                       CompilationContext compilationContext)
        {
            var success = true;
            for (var i = 0; i < ports.Length; i++)
            {
                var arg = arguments[i];
                var port = ports[i];
                var constraint = findConstraint(port.Type, compilationContext);
                if (constraint != null && !constraint.MatchesConstraint(arg, compilationContext))
                {
                    compilationContext.LogError(
                        8, $"Value given for port '{port.Identifier}' does not match '{constraint}' constraint");
                    success = false;
                }
            }

            return success;
        }
    }
}