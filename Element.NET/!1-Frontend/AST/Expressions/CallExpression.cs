using System.Linq;
using Lexico;

namespace Element.AST
{
    // ReSharper disable once UnusedType.Global
    public class CallExpression : SubExpression
    {
#pragma warning disable 169, 8618
        // ReSharper disable once UnusedAutoPropertyAccessor.Local
        [field: Term] private ListOf<Expression> Expressions { get; set; }
#pragma warning restore 169, 8618

        protected override void InitializeImpl()
        {
            foreach (var expr in Expressions.List)
            {
                expr.Initialize(Declarer);
            }
        }

        public override string ToString() => Expressions.ToString();

        public override bool Validate(SourceContext sourceContext) =>
            Expressions.List.Aggregate(true, (current, expr) => current & expr.Validate(sourceContext));

        protected override IValue SubExpressionImpl(IValue previous, IScope scope, CompilationContext compilationContext) =>
            previous is IFunctionSignature function
                ? function.ResolveCall(Expressions.List.Select(argExpr => argExpr.ResolveExpression(scope, compilationContext)).ToArray(), false, compilationContext)
                : compilationContext.LogError(16, $"{previous} cannot be called - it is not a function");
    }
}