using System;
using Lexico;

namespace Element.AST
{
    // ReSharper disable once ClassNeverInstantiated.Global
    public class Type
    {
        private IConstraint? _constraint;

        // ReSharper disable once UnusedMember.Global - Used by Lexico when parsing
        public Type() {}
        public Type(IConstraint constraint) => _constraint = constraint;

#pragma warning disable 169
        [Literal(":"), WhitespaceSurrounded, MultiLine] private Unnamed _;
        // ReSharper disable once UnusedAutoPropertyAccessor.Local
        [field: Term] private Expression Expression { get; set; }
#pragma warning restore 169

        public override string ToString() => $":{Expression}";

        public IConstraint? ResolveConstraint(IScope startingScope, CompilationContext? compilationContext) =>
            _constraint ?? Expression.ResolveExpression(startingScope, compilationContext) switch
            {
                IConstraint constraint => _constraint = constraint,
                {} notConstraint => compilationContext?.LogError(16, $"'{notConstraint}' is not a constraint"),
                _ => throw new ArgumentOutOfRangeException()
            };
    }
}