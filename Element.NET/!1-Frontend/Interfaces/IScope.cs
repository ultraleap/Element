using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public interface IScopeItem { }
    
    public interface IScope
    {
        IScopeItem? this[Identifier id, CompilationContext compilationContext] { get; }
        IScope? Parent { get; }
        string Location { get; }
    }

    public static class ScopeExtensions
    {
        public static IValue ToValue(this IScopeItem item, Identifier idIfNull, IScope checkedScope, CompilationContext compilationContext) => item switch
        {
            IValue v => v,
            {} notValue => compilationContext.LogError(16, $"'{notValue}' cannot be referenced in an expression as it is not a first class value"),
            _ => compilationContext.LogError(7, $"Couldn't find '{idIfNull}' in '{checkedScope.Location}'")
        };
        
        public static IScopeItem? IndexRecursively(this IScope scope, Identifier identifier, CompilationContext compilationContext, out IScope? scopeFoundIn)
        {
            scopeFoundIn = scope;
            return scope[identifier, compilationContext] ?? scope.Parent?.IndexRecursively(identifier, compilationContext, out scopeFoundIn);
        }
    }
}