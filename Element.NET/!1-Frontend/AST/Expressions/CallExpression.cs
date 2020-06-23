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

        protected override void InitializeImpl(IIntrinsicCache? cache)
        {
            foreach (var expr in Expressions.List)
            {
                expr.Initialize(Declarer, cache);
            }
        }

        public override string ToString() => Expressions.ToString();
        public override void Validate(ResultBuilder resultBuilder)
        {
            foreach (var expr in Expressions.List)
            {
                expr.Validate(resultBuilder);
            }
        }

        protected override Result<IValue> SubExpressionImpl(IValue previous, IScope scope, CompilationContext compilationContext) =>
            previous is IFunction function
                ? Expressions.List
                             .Select(argExpr => argExpr.ResolveExpression(scope, compilationContext))
                             .BindEnumerable(args => function.Call(args.ToArray(), compilationContext))
                : compilationContext.Trace(MessageCode.InvalidExpression, $"{previous} cannot be called - it is not a function");
    }
}