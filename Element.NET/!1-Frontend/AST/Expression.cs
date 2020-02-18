using System;
using System.Collections.Generic;
using System.Linq;
using Lexico;

namespace Element.AST
{
    public interface IExpressionListStart {}

    public interface ISubExpression
    {
        IValue ResolveSubExpression(IValue previous, IIndexable expressionScope, CompilationContext compilationContext);
    }

    public class CallExpression : ListOf<Expression>, ISubExpression
    {
        public IValue ResolveSubExpression(IValue previous, IIndexable expressionScope, CompilationContext compilationContext)
        {
            if (!(previous is ICallable callable)) return compilationContext.LogError(16, $"{previous} is not callable");

            // Compile the arguments for this call expression
            var arguments = List.Select(argExpr => argExpr.ResolveExpression(expressionScope, compilationContext)).ToArray();

            // Check argument count is correct
            var expectedArgCount = callable.Inputs?.Length ?? 0; // No inputs means no arguments required (nullary function)
            if (arguments.Length != expectedArgCount)
            {
                return compilationContext.LogError(6, $"Expected '{expectedArgCount}' arguments but got '{arguments.Length}'");
            }

            // TODO: Argument type checking

            return callable switch
            {
                Function intrinsic when intrinsic.IsIntrinsic => compilationContext.GlobalScope.GetIntrinsic(intrinsic.Identifier) switch
                {
                    { } intrinsicImpl => intrinsicImpl.Call(arguments, compilationContext),
                    _ => compilationContext.LogError(4, $"No intrinsic named '{intrinsic.Identifier}' is implemented")
                },
                _ => callable.Call(arguments, compilationContext)
            };
        }
    }

    public class IndexingExpression : ISubExpression
    {
        [Literal(".")] private Unnamed _;
        [field:Term] private Identifier Identifier { get; }

        public override string ToString() => $".{Identifier}";

        public IValue ResolveSubExpression(IValue previous, IIndexable _, CompilationContext compilationContext) =>
            previous is IIndexable indexable
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

        public IValue ResolveExpression(IIndexable scope, CompilationContext compilationContext)
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

            // Resolve Nullary (0-argument) functions
            previous = previous switch
            {
                Function func when func.Inputs.Length == 0 => func.Call(Array.Empty<IValue>(), compilationContext),
                _ => previous
            };

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