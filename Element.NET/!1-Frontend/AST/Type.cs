using System.Collections.Generic;
using Lexico;

namespace Element.AST
{
    // ReSharper disable once ClassNeverInstantiated.Global
    public class Type
    {
        [Literal(":"), WhitespaceSurrounded] private Unnamed _;
        // ReSharper disable once UnassignedGetOnlyAutoProperty
        [field: Term] public Identifier Identifier { get; }
        // ReSharper disable once UnassignedGetOnlyAutoProperty
        [field: Optional] public List<IndexingExpression> IndexingExpressions { get; }

        public IConstraint? FindConstraint(IScope scope, CompilationContext compilationContext)
        {
            var value = scope.ResolveIndexExpressions(Identifier, IndexingExpressions, compilationContext);
            if (value is IConstraint constraint) return constraint;
            compilationContext.LogError(16, $"'{value}' is not a type");
            return null;
        }
    }
}