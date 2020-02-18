using System.Collections.Generic;
using Lexico;

namespace Element.AST
{
    public class Type
    {
        [Literal(":"), WhitespaceSurrounded] private Unnamed _;
        [field: Term] public Identifier Identifier { get; }
        [field: Optional] public List<IndexingExpression> IndexingExpressions { get; } = new List<IndexingExpression>();
    }
}
/*if (value is IConstraint constraint) return constraint;

compilationContext.LogError(16, $"'{value}' is not a type");*/