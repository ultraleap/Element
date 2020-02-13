using System.Collections.Generic;
using Lexico;

namespace Element.AST
{
    public interface IExpressionListStart {}
    public class CallExpression : ListOf<Expression> { }
    
    [WhitespaceSurrounded]
    public class Expression
    {
        [field: Term] public IExpressionListStart LitOrId { get; }
        [field: Optional] public List<CallExpression> CallExpressions { get; } = new List<CallExpression>();

        public override string ToString() => $"{LitOrId}{(CallExpressions != null ? string.Concat(CallExpressions) : string.Empty)}";
    }
}