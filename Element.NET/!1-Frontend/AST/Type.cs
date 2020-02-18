using System.Collections.Generic;
using Lexico;

namespace Element.AST
{
    public class Type
    {
        [Literal(":"), WhitespaceSurrounded] private Unnamed _;
        [field: Term] public Identifier Identifier { get; }
        [field: Optional] public List<IndexingExpression> IndexingExpressions { get; } = new List<IndexingExpression>();

        /// <summary>
        /// Gets the IConstraint that this type refers to.
        /// </summary>
        public IConstraint? GetConstraint(CompilationFrame frame, CompilationContext compilationContext)
        {
            if (!frame.Get(Identifier, compilationContext, out var value))
            {
                compilationContext.LogError(7, $"Couldn't find '{Identifier}'");
                return null;
            }

            foreach (var indexingExpr in IndexingExpressions)
            {
                value = indexingExpr.Resolve(value, null, compilationContext);
            }

            if (value is IConstraint constraint) return constraint;

            compilationContext.LogError(16, $"'{value}' is not a type");
            return null;
        }
    }
}