using System;

namespace Element.AST
{
    public interface IScope
    {
        IValue? this[Identifier id, bool recurse, CompilationContext compilationContext] { get; }
    }

    public static class ScopeExtensions
    {
        public static IValue CompileFunction(this IScope callScope, object body, CompilationContext compilationContext)  =>
            body switch
            {
                // If a function has expression body we just compile the single expression using the call scope
                ExpressionBody exprBody => exprBody.Expression.ResolveExpression(callScope, compilationContext),
                // If a function has a scope body we need to find the return identifier
                IScope scopeBody => scopeBody[Parser.ReturnIdentifier, false, compilationContext] switch
                {
                    ICompilableFunction nullaryReturn when nullaryReturn.IsNullary() => nullaryReturn.Compile(scopeBody, compilationContext),
                    ICompilableFunction functionReturn => functionReturn,
                    null => compilationContext.LogError(7, $"'{Parser.ReturnIdentifier}' not found in function scope"),
                    var nyi => throw new NotImplementedException(nyi.ToString())
                },
                _ => CompilationErr.Instance
            };
    }
}