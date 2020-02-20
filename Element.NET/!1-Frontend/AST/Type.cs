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
    }
}