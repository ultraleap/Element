using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public interface IIndexable : IValue
    {
        IValue? this[Identifier id] { get; }
        IIndexable? Parent { get; }
    }

    public static class IndexableExtensions
    {
        /// <summary>
        /// Gets the IConstraint that this type refers to.
        /// </summary>
        public static IValue? IndexRecursively(this IIndexable indexable, Identifier identifier) =>
            indexable[identifier] ?? indexable.Parent.IndexRecursively(identifier);

        public static IValue? ResolveIndexExpressions(this IIndexable indexable, Identifier identifier,
            List<IndexingExpression> indexingExpressions, CompilationContext compilationContext)
        {
            var value = indexable.IndexRecursively(identifier);
            if (value == null)
            {
                compilationContext.LogError(7, $"Couldn't find '{identifier}'");
                return null;
            }

            foreach (var indexingExpr in indexingExpressions)
            {
                value = indexingExpr.ResolveSubExpression(value, null, compilationContext);
            }

            return value;
        }

        public static IIndexable PushTemporaryScope(this IIndexable indexable, IEnumerable<(Identifier, IValue)> pairs) => new TemporaryScope(indexable, pairs);

        private sealed class TemporaryScope : ScopeBase
        {
            public TemporaryScope(IIndexable parent, IEnumerable<(Identifier Identifier, IValue Value)> members) :
                base(parent) =>
                AddRangeToCache(members);

            protected override IEnumerable<Item> ItemsToCacheOnValidate  => Enumerable.Empty<Item>();
        }
    }
}