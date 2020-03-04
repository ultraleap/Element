using System;

namespace Element.AST
{
    public interface IScope
    {
        IValue? this[Identifier id, CompilationContext compilationContext] { get; }
    }

    public static class ScopeExtensions
    {
        public static IValue CompileFunction(this IScope parentScope, object body, CompilationContext compilationContext)  =>
            body switch
            {
                // If a function is a binding (has expression body) we just compile the single expression
                Binding binding => binding.Expression.ResolveExpression(parentScope, compilationContext),
                // If a function has a scope body we find the return value
                Scope scope => scope[Parser.ReturnIdentifier, compilationContext] switch
                {
                    // If the return value is a function, compile it
                    ICompilableFunction returnFunction => returnFunction.Compile(parentScope, compilationContext),
                    null => compilationContext.LogError(7, $"'{Parser.ReturnIdentifier}' not found in function scope"),
                    var nyi => throw new NotImplementedException(nyi.ToString())
                },
                _ => CompilationErr.Instance
            };
    }
}