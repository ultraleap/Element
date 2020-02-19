using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public interface IScope : IValue
    {
        IValue? this[Identifier id] { get; }
        IScope? Parent { get; }
    }

    public static class ScopeExtensions
    {
        /// <summary>
        /// Gets the IConstraint that this type refers to.
        /// </summary>
        public static IValue? IndexRecursively(this IScope scope, Identifier identifier) =>
            scope[identifier] ?? scope.Parent?.IndexRecursively(identifier);

        public static IValue? ResolveIndexExpressions(this IScope scope, Identifier identifier,
            List<IndexingExpression> indexingExpressions, CompilationContext compilationContext)
        {
            var value = scope.IndexRecursively(identifier);
            if (value == null)
            {
                compilationContext.LogError(7, $"Couldn't find '{identifier}'");
                return null;
            }

            foreach (var indexingExpr in indexingExpressions ?? Enumerable.Empty<IndexingExpression>())
            {
                value = indexingExpr.ResolveSubExpression(value, null, compilationContext);
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