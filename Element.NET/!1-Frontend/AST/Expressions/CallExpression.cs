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
        public override void Validate(ResultBuilder resultBuilder)
        {
            foreach (var expr in Expressions.List)
            {
                expr.Validate(resultBuilder);
            }
        }

        protected override Result<IValue> SubExpressionImpl(IValue previous, IScope scope, CompilationContext compilationContext) =>
            Expressions.List
                       .Select(argExpr => argExpr.ResolveExpression(scope, compilationContext))
                       .BindEnumerable(args => previous.Call(args.ToArray(), compilationContext));
    }
}