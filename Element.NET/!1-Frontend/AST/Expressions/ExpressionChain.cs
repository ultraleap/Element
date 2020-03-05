using System.Collections.Generic;
using System.Linq;
using Lexico;

namespace Element.AST
{
    public interface ISubExpression
    {
        IValue ResolveSubExpression(IValue previous, IScope resolutionScope, CompilationContext compilationContext);
    }

    // ReSharper disable once UnusedType.Global
    public class ExpressionChain : Expression
    {
        // ReSharper disable UnusedAutoPropertyAccessor.Local
        [field: Alternative(typeof(Identifier), typeof(Literal))] private object LitOrId { get; set; }
        [field: Optional] private List<ISubExpression>? Expressions { get; set; }
        // ReSharper restore UnusedAutoPropertyAccessor.Local

        public override string ToString() => $"{LitOrId}{(Expressions != null ? string.Concat(Expressions) : string.Empty)}";

        public override IValue ResolveExpression(IScope scope, CompilationContext compilationContext)
        {
            var previous = LitOrId switch
            {
                // If the start of the list is an identifier, find the value that it identifies
                Identifier id => scope[id, true, compilationContext] is {} v
                    ? v
                    : compilationContext.LogError(7, $"Couldn't find '{id}' in local or outer scope"),
                Literal lit => lit,
                _ => throw new InternalCompilerException("Trying to compile expression that doesn't start with literal or identifier - should be impossible")
            };

            // Early out if something failed above
            if (previous is CompilationErr err) return err;

            compilationContext.Push(new TraceSite(previous.ToString(), null, 0, 0));

            // Evaluate all expressions for this chain if there are any, making sure that the result is fully resolved if it returns a nullary.
            previous = previous.ResolveNullaryFunction(compilationContext);
            previous = (Expressions?.Aggregate(previous, (current, expr) => expr.ResolveSubExpression(current, scope, compilationContext)) ?? previous)
                .ResolveNullaryFunction(compilationContext);

            compilationContext.Pop();

            return previous;
        }
    }
}