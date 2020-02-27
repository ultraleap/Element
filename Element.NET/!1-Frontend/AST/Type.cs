using System;
using System.Collections.Generic;
using System.Linq;
using Lexico;

namespace Element.AST
{
    // ReSharper disable once ClassNeverInstantiated.Global
    public class Type
    {
#pragma warning disable 169
        [Literal(":"), WhitespaceSurrounded, MultiLine] private Unnamed _;
        // ReSharper disable once UnusedAutoPropertyAccessor.Local
        [field: Term] public Identifier Identifier { get; private set; }
        // ReSharper disable once UnusedAutoPropertyAccessor.Local
        [field: Optional] public List<IndexingExpression> IndexingExpressions { get; private set; }
#pragma warning restore 169

        public override string ToString() => $":{Identifier}{(IndexingExpressions?.Count > 0 ? string.Join(string.Empty, IndexingExpressions) : string.Empty)}";
    }

    public static class TypeExtensions
    {
        public static IConstraint? ResolveConstraint(this Type type, IScope startingScope, CompilationContext compilationContext)
        {
            if (type == null) return AnyConstraint.Instance;
            if (startingScope == null) throw new ArgumentNullException(nameof(startingScope));

            var previous = startingScope.IndexRecursively(type.Identifier, compilationContext, out var foundValueIn);
            var currentIdentifier = type.Identifier;

            // ReSharper disable once ConstantNullCoalescingCondition
            foreach (var indexingExpr in type.IndexingExpressions ?? Enumerable.Empty<IndexingExpression>())
            {
                currentIdentifier = indexingExpr.Identifier;
                var scopeToIndex = previous.ToValue(currentIdentifier, foundValueIn, compilationContext);
                foundValueIn = scopeToIndex as IScope;
                previous = indexingExpr.ResolveSubExpression(scopeToIndex, null, compilationContext);
            }

            return previous switch
            {
                IConstraint constraint => constraint,
                null => compilationContext.LogError(7, $"Couldn't find '{currentIdentifier}' in '{type}'"),
                _ => compilationContext.LogError(16, $"'{previous}' is not a constraint")
            };
        }
    }
}