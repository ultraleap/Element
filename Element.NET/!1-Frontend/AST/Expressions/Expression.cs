using System.Collections.Generic;
using System.Linq;
using Lexico;

namespace Element.AST
{
    public interface IExpressionListStart {}

    public interface ISubExpression
    {
        IValue ResolveSubExpression(IValue previous, IScope containingScope, CompilationContext compilationContext);
    }

    [WhitespaceSurrounded, MultiLine]
    // ReSharper disable once ClassNeverInstantiated.Global
    public class Expression
    {
        // ReSharper disable once UnusedAutoPropertyAccessor.Local
        [field: Term] public IExpressionListStart LitOrId { get; private set; }
        // ReSharper disable once UnusedAutoPropertyAccessor.Local
        [field: Optional] public List<ISubExpression> Expressions { get; private set; }

        public override string ToString() => $"{LitOrId}{(Expressions != null ? string.Concat(Expressions) : string.Empty)}";

        public IValue ResolveExpression(IScope scope, CompilationContext compilationContext)
        {
            var previous = LitOrId switch
            {
                // If the start of the list is an identifier, find the value that it identifies
                Identifier id => scope.IndexRecursively(id, compilationContext, out var containingScope).ToValue(id, containingScope, compilationContext),
                Literal lit => lit,
                _ => throw new InternalCompilerException("Trying to compile expression that doesn't start with literal or identifier - should be impossible")
            };

            // Early out if something failed above
            if (previous is CompilationErr) return CompilationErr.Instance;

            compilationContext.Push(new TraceSite(previous.ToString(), null, 0, 0));

            previous = previous.ResolveNullaryFunction(compilationContext);
            
            // TODO: Handle lambdas

            // ReSharper disable once ConstantConditionalAccessQualifier
            previous = Expressions?.Aggregate(previous, (current, expr) => expr.ResolveSubExpression(current, scope, compilationContext));

            previous = previous.ResolveNullaryFunction(compilationContext);

            compilationContext.Pop();

            return previous;
        }
    }
}