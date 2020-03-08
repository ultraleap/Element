using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public interface IValue
    {
        IType Type { get; }
    }

    public static class ValueExtensions
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
                var constraint = port.ResolveConstraint(scope, compilationContext);
                if (!constraint.MatchesConstraint(arg, compilationContext))
                {
                    compilationContext.LogError(8, $"Value given for port '{port}' does not match '{constraint}' constraint");
                    success = false;
                }
            }

            return success;
        }

        public static IEnumerable<(Identifier Identifier, IValue Value)> WithoutDiscardedArguments(this IValue[] arguments, Port[] ports) =>
            ports.Where(port => port.Identifier.HasValue)
                 .Select((port, idx) => (port.Identifier.Value, arguments[idx]));
    }
}