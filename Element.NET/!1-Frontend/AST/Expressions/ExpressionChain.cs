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

        public override void Validate(ResultBuilder resultBuilder)
        {
            foreach (var expr in Expressions ?? Enumerable.Empty<SubExpression>())
            {
                expr.Validate(resultBuilder);
            }
        }

        protected override Result<IValue> ExpressionImpl(IScope scope, CompilationContext compilationContext) =>
            (LitOrId switch
                {
                    // If the start of the list is an identifier, find the value that it identifies
                    Identifier id => scope.Lookup(id, compilationContext).Map(v => v),
                    Constant constant => constant,
                    _ => throw new InternalCompilerException("Trying to compile expression that doesn't start with literal or identifier - should be impossible")
                })
            .Bind(previous =>
            {
                var fullyResolved = previous.FullyResolveValue(compilationContext);
                // Evaluate all expressions for this chain if there are any
                return Expressions?.Aggregate(fullyResolved, ResolveSubExpression)
                                  .Bind(result => result.FullyResolveValue(compilationContext)) // Make sure to fully resolve the result of the chain
                       ?? fullyResolved; // If there's no subexpressions just return what we found

                Result<IValue> ResolveSubExpression(Result<IValue> current, SubExpression subExpr) =>
                    current.Bind(v => v.FullyResolveValue(compilationContext))
                           .Bind(fullyResolvedSubExpr => subExpr.ResolveSubExpression(fullyResolvedSubExpr, scope, compilationContext));
            });
    }
}