using System;
using System.Collections.Generic;
using System.Linq;
using Lexico;

namespace Element.AST
{
    public interface IExpressionChainStart
    {
        string TraceString { get; }
    }
    
    // ReSharper disable once ClassNeverInstantiated.Global
    public class ExpressionChain : Expression
    {
        // ReSharper disable UnusedAutoPropertyAccessor.Local
#pragma warning disable 8618
        [field: Alternative(typeof(Identifier), typeof(Constant))] public IExpressionChainStart LitOrId { get; private set; }
        [field: Optional] public List<SubExpression>? Expressions { get; private set; }
#pragma warning restore 8618
        // ReSharper restore UnusedAutoPropertyAccessor.Local

        public override string ToString() => $"{LitOrId.TraceString}{(Expressions != null ? string.Concat(Expressions) : string.Empty)}";

        protected override void ValidateImpl(ResultBuilder builder, Context context)
        {
            foreach (var expr in Expressions ?? Enumerable.Empty<SubExpression>())
            {
                expr.Validate(builder, context);
            }
        }

        public static Result<IValue> ResolveExpressionChain(IValue start, IEnumerable<SubExpression> subExpressions, IScope parentScope, Context context)
        {
            Result<IValue> ResolveSubExpression(Result<IValue> previousResult, SubExpression subExpr) =>
                previousResult.Bind(previous => subExpr.ResolveSubExpression(previous, parentScope, context));
            
            return subExpressions.Aggregate(new Result<IValue>(start), ResolveSubExpression);
        }

        protected override Result<IValue> ExpressionImpl(IScope parentScope, Context context) =>
            (LitOrId switch
                {
                    // If the start of the list is an identifier, find the value that it identifies
                    Identifier id => parentScope.Lookup(id, context).Map(v => v),
                    Constant constant => constant,
                    _ => throw new InternalCompilerException("Trying to compile expression that doesn't start with literal or identifier - should be impossible")
                })
            .Bind(start => Expressions != null // Return the starting value if there's no subexpressions
                               ? ResolveExpressionChain(start, Expressions, parentScope, context)
                               : new Result<IValue>(start));

        public abstract class SubExpression : AstNode
        {
            public Result<IValue> ResolveSubExpression(IValue previous, IScope parentScope, Context context)
            {
                context.TraceStack.Push(this.MakeTraceSite($"{GetType().Name} '{ToString()}'"));
                var result = SubExpressionImpl(previous, parentScope, context);
                context.TraceStack.Pop();
                return result;
            }

            protected abstract Result<IValue> SubExpressionImpl(IValue previous, IScope scope, Context context);
        }
        
        // ReSharper disable once UnusedType.Global
        public class CallExpression : SubExpression
        {
#pragma warning disable 169, 8618
            // ReSharper disable once UnusedAutoPropertyAccessor.Local
            [field: Term] public ListOf<Expression> Expressions { get; private set; }
#pragma warning restore 169, 8618

            public override string ToString() => Expressions.ToString();

            protected override void ValidateImpl(ResultBuilder builder, Context context)
            {
                foreach (var expr in Expressions.List)
                {
                    expr.Validate(builder, context);
                }
            }

            protected override Result<IValue> SubExpressionImpl(IValue previous, IScope scope, Context context) =>
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
            protected override void ValidateImpl(ResultBuilder builder, Context context) => Identifier.Validate(builder, Array.Empty<Identifier>(), Array.Empty<Identifier>());
            protected override Result<IValue> SubExpressionImpl(IValue previous, IScope _, Context context) => previous.Index(Identifier, context);
        }
    }
}