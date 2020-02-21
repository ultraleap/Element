using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public interface IScopeItem { }
    
    public interface IScope
    {
        IScopeItem? this[Identifier id] { get; }
        IScope? Parent { get; }
    }

    public static class ScopeExtensions
    {
        public static IValue ToValue(this IScopeItem item, Identifier idIfNull, CompilationContext compilationContext) => item switch
        {
            IValue v => v,
            {} notValue => compilationContext.LogError(16, $"'{notValue}' cannot be referenced in an expression as it is not a first class value"),
            _ => compilationContext.LogError(7, $"Couldn't find '{idIfNull}' in a local or outer scope")
        };
        
        public static IScopeItem? IndexRecursively(this IScope scope, Identifier identifier) =>
            scope[identifier] ?? scope.Parent?.IndexRecursively(identifier);

        public static IScopeItem? ResolveIndexExpressions(this IScope scope, Identifier identifier,
                                                          List<IndexingExpression> indexingExpressions, CompilationContext compilationContext)
        {
            var value = scope.IndexRecursively(identifier);
            foreach (var indexingExpr in indexingExpressions ?? Enumerable.Empty<IndexingExpression>())
            {
                value = indexingExpr.ResolveSubExpression(value.ToValue(identifier, compilationContext), null, compilationContext);
            }
            return value;
        }

        public static IScope PushTemporaryScope(this IScope scope, IEnumerable<(Identifier, IValue)> pairs) => new TemporaryScope(scope, pairs);

        private sealed class TemporaryScope : TransientScope
        {
            public TemporaryScope(IScope parent, IEnumerable<(Identifier Identifier, IValue Value)> members) :
                base(parent) =>
                AddRangeToCache(members);
        }
    }
}