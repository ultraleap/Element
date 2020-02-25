using System.Collections.Generic;
using Lexico;

namespace Element.AST
{
    public interface IExpressionListStart {}

    public interface ISubExpression
    {
        IValue ResolveSubExpression(IValue previous, IScope containingScope, CompilationContext compilationContext);
    }

    [WhitespaceSurrounded, MultiLine]
    public class Expression
    {
        [field: Term] public IExpressionListStart LitOrId { get; }
        [field: Optional] public List<ISubExpression> Expressions { get; } = new List<ISubExpression>();

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

            DeclaredFunction.ResolveNullary(ref previous, compilationContext);
            
            // TODO: Handle lambdas

            foreach (var expr in Expressions)
            {
                previous =  expr.ResolveSubExpression(previous, scope, compilationContext);
            }

            DeclaredFunction.ResolveNullary(ref previous, compilationContext);

            compilationContext.Pop();

            return previous;
        }
    }
}