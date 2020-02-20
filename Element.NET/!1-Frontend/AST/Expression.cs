using System;
using System.Collections.Generic;
using System.Linq;
using Lexico;

namespace Element.AST
{
    public interface IExpressionListStart {}

    public interface ISubExpression
    {
        IValue ResolveSubExpression(IValue previous, IScope expressionScope, CompilationContext compilationContext);
    }

    public class CallExpression : ListOf<Expression>, ISubExpression
    {
        public IValue ResolveSubExpression(IValue previous, IScope expressionScope, CompilationContext compilationContext)
        {
            if (!(previous is ICallable callable)) return compilationContext.LogError(16, $"{previous} is not callable");

            // Compile the arguments for this call expression
            var arguments = List.Select(argExpr => argExpr.ResolveExpression(expressionScope, compilationContext)).ToArray();

            return callable.Call(arguments, compilationContext);
        }
    }

    public static class CallExtensions
    {
        public static bool ValidateArgumentCount(this IValue[] arguments, int expectedArgCount, CompilationContext compilationContext)
        {
            if (arguments.Length != expectedArgCount)
            {
                compilationContext.LogError(6, $"Expected '{expectedArgCount}' arguments but got '{arguments.Length}'");
                return false;
            }

            return true;
        }

        public static bool ValidateArgumentConstraints(this IValue[] arguments, Port[] ports, Func<Type, CompilationContext, IConstraint?> findConstraint, CompilationContext compilationContext)
        {
            var success = true;
            for (var i = 0; i < ports.Length; i++)
            {
                var arg = arguments[i];
                var port = ports[i];
                var constraint = findConstraint(port.Type, compilationContext);
                if (constraint != null && !constraint.MatchesConstraint(arg, compilationContext))
                {
                    compilationContext.LogError(8, $"Value given for port '{port.Identifier}' does not match '{constraint}' constraint");
                    success = false;
                }
            }

            return success;
        }
    }

    public class IndexingExpression : ISubExpression
    {
        [Literal(".")] private Unnamed _;
        [field:Term] private Identifier Identifier { get; }

        public override string ToString() => $".{Identifier}";

        public IValue ResolveSubExpression(IValue previous, IScope _, CompilationContext compilationContext) =>
            previous is IScope indexable
                ? indexable[Identifier] switch
                {
                    { } v => v,
                    _ => compilationContext.LogError(7, $"Couldn't find '{Identifier}' in '{previous}'")
                }
                : compilationContext.LogError(16, $"'{previous}' is not indexable");
    }
    
    [WhitespaceSurrounded]
    public class Expression
    {
        [field: Term] public IExpressionListStart LitOrId { get; }
        [field: Optional] public List<ISubExpression> Expressions { get; } = new List<ISubExpression>();

        public override string ToString() => $"{LitOrId}{(Expressions != null ? string.Concat(Expressions) : string.Empty)}";

        public IValue ResolveExpression(IScope scope, CompilationContext compilationContext)
        {
            var previous = LitOrId switch
            {
                // If the start of the list is an identifier, find the value that it identifies
                Identifier id => scope.IndexRecursively(id) is { } value ? value : compilationContext.LogError(7, $"Couldn't find '{id}' in a local or outer scope"),
                Literal lit => lit,
                _ => throw new InternalCompilerException("Trying to compile expression that doesn't start with literal or identifier - should be impossible")
            };

            // Early out if something failed above
            if (previous is CompilationErr) return CompilationErr.Instance;

            compilationContext.Push(new TraceSite(previous.ToString(), null, 0, 0));

            if (previous is Function function)
            {
                previous = function.HandleNullary(compilationContext);
            }

            // TODO: Handle lambdas

            foreach (var expr in Expressions)
            {
                previous =  expr.ResolveSubExpression(previous, scope, compilationContext);
            }

            compilationContext.Pop();

            return previous;
        }
    }
}