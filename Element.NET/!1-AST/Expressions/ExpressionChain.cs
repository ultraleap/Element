using System;
using System.Collections.Generic;
using System.Linq;
using Lexico;

namespace Element.AST
{
    // ReSharper disable once UnusedType.Global
    public class ExpressionChain : Expression
    {
        // ReSharper disable UnusedAutoPropertyAccessor.Local
#pragma warning disable 8618
        [field: Alternative(typeof(Identifier), typeof(Constant))] public object LitOrId { get; private set; }
        [field: Optional] public List<SubExpression>? Expressions { get; private set; }
#pragma warning restore 8618
        // ReSharper restore UnusedAutoPropertyAccessor.Local

        public override string ToString() => $"{LitOrId}{(Expressions != null ? string.Concat(Expressions) : string.Empty)}";

        protected override void ValidateImpl(ResultBuilder resultBuilder, CompilationContext context)
        {
            foreach (var expr in Expressions ?? Enumerable.Empty<SubExpression>())
            {
                expr.Validate(resultBuilder, context);
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

        public abstract class SubExpression : AstNode
        {
            public Result<IValue> ResolveSubExpression(IValue previous, IScope scope, CompilationContext compilationContext)
            {
                compilationContext.PushTrace(MakeTraceSite(ToString()));
                var result = SubExpressionImpl(previous, scope, compilationContext);
                compilationContext.PopTrace();
                return result;
            }

            protected abstract Result<IValue> SubExpressionImpl(IValue previous, IScope scope, CompilationContext context);
        }
        
        // ReSharper disable once UnusedType.Global
        public class CallExpression : SubExpression
        {
#pragma warning disable 169, 8618
            // ReSharper disable once UnusedAutoPropertyAccessor.Local
            [field: Term] public ListOf<Expression> Expressions { get; private set; }
#pragma warning restore 169, 8618

            public override string ToString() => Expressions.ToString();

            protected override void ValidateImpl(ResultBuilder resultBuilder, CompilationContext context)
            {
                foreach (var expr in Expressions.List)
                {
                    expr.Validate(resultBuilder, context);
                }
            }

            protected override Result<IValue> SubExpressionImpl(IValue previous, IScope scope, CompilationContext context) =>
                Expressions.List
                           .Select(argExpr => argExpr.ResolveExpression(scope, context))
                           .BindEnumerable(args => previous.Call(args.ToArray(), context));
        }

        // ReSharper disable once UnusedType.Global
        public class IndexingExpression : SubExpression
        {
#pragma warning disable 169
            // ReSharper disable once UnusedAutoPropertyAccessor.Local
            [field: Term, Prefix(".")] public Identifier Identifier { get; private set; }
#pragma warning restore 169

            public override string ToString() => $".{Identifier}";
            protected override void ValidateImpl(ResultBuilder resultBuilder, CompilationContext context) => Identifier.Validate(resultBuilder, Array.Empty<Identifier>(), Array.Empty<Identifier>());
            protected override Result<IValue> SubExpressionImpl(IValue previous, IScope _, CompilationContext compilationContext) => previous.Index(Identifier, compilationContext);
        }
    }
}