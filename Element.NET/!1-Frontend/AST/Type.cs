using System;
using System.Collections.Generic;
using System.Linq;
using Lexico;

namespace Element.AST
{
    // ReSharper disable once ClassNeverInstantiated.Global
    public class Type
    {
        [Literal(":"), WhitespaceSurrounded, MultiLine] private Unnamed _;
        // ReSharper disable once UnassignedGetOnlyAutoProperty
        [field: Term] public Identifier Identifier { get; }
        // ReSharper disable once UnassignedGetOnlyAutoProperty
        [field: Optional] public List<IndexingExpression> IndexingExpressions { get; }

        public override string ToString() => $":{Identifier}{(IndexingExpressions?.Count > 0 ? string.Join(string.Empty, IndexingExpressions) : string.Empty)}";
    }

    public static class TypeExtensions
    {
        public static IConstraint? ResolveConstraint(this Type type, IScope startingScope, CompilationContext compilationContext)
        {
            if (startingScope == null) throw new ArgumentNullException(nameof(startingScope));
            if (type == null) return AnyConstraint.Instance;
            var value = startingScope.IndexRecursively(type.Identifier);
            var currentIdentifier = type.Identifier;
            // ReSharper disable once ConstantNullCoalescingCondition
            foreach (var indexingExpr in type.IndexingExpressions ?? Enumerable.Empty<IndexingExpression>())
            {
                currentIdentifier = indexingExpr.Identifier;
                value = indexingExpr.ResolveSubExpression(value.ToValue(currentIdentifier, compilationContext), null, compilationContext);
            }

            return value switch
            {
                IConstraint constraint => constraint,
                null => compilationContext.LogError(7, $"Couldn't find '{currentIdentifier}' in '{type}'"),
                _ => compilationContext.LogError(16, $"'{value}' is not a constraint")
            };
        }
    }
}