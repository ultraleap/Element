using System;
using System.Collections.Generic;
using System.Linq;
using Lexico;

namespace Element.AST
{
    public interface IExpressionChainStart
    {
        Result<IValue> Resolve(ExpressionChain expressionChain, IScope scope, Context context);
        string TraceString { get; }
    }
    
    // ReSharper disable once ClassNeverInstantiated.Global
    public class ExpressionChain : Expression
    {
        // ReSharper disable UnusedAutoPropertyAccessor.Local
#pragma warning disable 8618
        [field: Alternative(typeof(Identifier), typeof(Constant))] public IExpressionChainStart ExpressionChainStart { get; private set; }
        [field: Optional] public List<SubExpression>? SubExpressions { get; private set; }
#pragma warning restore 8618
        // ReSharper restore UnusedAutoPropertyAccessor.Local

        public override string ToString() => $"{ExpressionChainStart.TraceString}{(SubExpressions != null ? string.Concat(SubExpressions) : string.Empty)}";

        protected override void ValidateImpl(ResultBuilder builder, Context context)
        {
            foreach (var expr in SubExpressions ?? Enumerable.Empty<SubExpression>())
            {
                expr.Validate(builder, context);
            }
        }

        private Result<IValue> ResolveExpressionChain(IValue start, IEnumerable<SubExpression> subExpressions, IScope parentScope, Context context)
        {
            Result<IValue> ResolveSubExpression(Result<IValue> previousResult, SubExpression subExpr) =>
                previousResult.Bind(previous => subExpr.ResolveSubExpression(this, previous, parentScope, context));
            
            return subExpressions.Aggregate(new Result<IValue>(start), ResolveSubExpression);
        }

        protected override Result<IValue> ExpressionImpl(IScope parentScope, Context context) =>
            ExpressionChainStart.Resolve(this, parentScope, context)
            .Bind(start => SubExpressions != null // Return the starting value if there's no subexpressions
                               ? ResolveExpressionChain(start, SubExpressions, parentScope, context)
                               : new Result<IValue>(start));

        public abstract class SubExpression : AstNode
        {
            public Result<IValue> ResolveSubExpression(ExpressionChain chain, IValue previous, IScope parentScope, Context context)
            {
                context.TraceStack.Push(this.MakeTraceSite($"{GetType().Name} '{ToString()}'"));
                var result = SubExpressionImpl(chain, previous, parentScope, context);
                context.TraceStack.Pop();
                return result;
            }

            protected abstract Result<IValue> SubExpressionImpl(ExpressionChain chain, IValue previous, IScope scope, Context context);
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

            protected override Result<IValue> SubExpressionImpl(ExpressionChain chain, IValue previous, IScope scope, Context context) =>
                Expressions.List
                           .Zip(previous.InputPorts, (argExpr, port) =>
                           {
                               context.Aspect?.BeforeCallArgument(previous, argExpr, port, scope);
                               var argExpressionResult = argExpr.ResolveExpression(scope, context);
                               return context.Aspect?.CallArgument(previous, argExpr, port, scope, argExpressionResult) ?? argExpressionResult;
                           })
                           .ToResultReadOnlyList()
                           .Bind(args =>
                           {
                               context.Aspect?.BeforeCall(chain, previous, this, args);
                               var callResult = previous.Call(args.ToArray(), context);
                               return context.Aspect?.Call(chain, previous, this, args, callResult) ?? callResult;
                           });
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
            protected override Result<IValue> SubExpressionImpl(ExpressionChain chain, IValue previous, IScope _, Context context)
            {
                context.Aspect?.BeforeIndex(chain, previous, this);
                var indexResult = previous.Index(Identifier, context);
                return context.Aspect?.Index(chain, previous, this, indexResult) ?? indexResult;
            }
        }
    }
}