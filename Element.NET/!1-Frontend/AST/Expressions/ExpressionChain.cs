using System.Collections.Generic;
using System.Linq;
using Lexico;

namespace Element.AST
{
    // ReSharper disable once UnusedType.Global
    public class ExpressionChain : Expression
    {
        // ReSharper disable UnusedAutoPropertyAccessor.Local
        [field: Alternative(typeof(Identifier), typeof(Constant))] private object LitOrId { get; set; }
        [field: Optional] private List<SubExpression>? Expressions { get; set; }
        // ReSharper restore UnusedAutoPropertyAccessor.Local

        public override string ToString() => $"{LitOrId}{(Expressions != null ? string.Concat(Expressions) : string.Empty)}";
        
        protected override void InitializeImpl()
        {
            foreach (var expr in Expressions ?? Enumerable.Empty<SubExpression>())
            {
                expr.Initialize(Declarer);
            }
        }
        
        public override bool Validate(SourceContext sourceContext) =>
            Expressions?.Aggregate(true, (current, expr) => current & expr.Validate(sourceContext)) ?? true;

        protected override IValue ExpressionImpl(IScope scope, CompilationContext compilationContext)
        {
            var previous = LitOrId switch
            {
                // If the start of the list is an identifier, find the value that it identifies
                Identifier id => scope[id, true, compilationContext] is { } v
                                     ? v
                                     : compilationContext.LogError(7, $"Couldn't find '{id}' in local or outer scope"),
                Constant constant => constant,
                _ => throw new InternalCompilerException("Trying to compile expression that doesn't start with literal or identifier - should be impossible")
            };

            // Early out if something failed above
            if (previous is CompilationError err) return err;

            previous = previous.FullyResolveValue(compilationContext);
            // Evaluate all expressions for this chain if there are any, making sure that the result is fully resolved if it returns a nullary.
            previous = (Expressions?.Aggregate(previous, (current, expr) => expr.ResolveSubExpression(current.FullyResolveValue(compilationContext), scope, compilationContext))
                        ?? previous)
                .FullyResolveValue(compilationContext);

            return previous;
        }
    }
}