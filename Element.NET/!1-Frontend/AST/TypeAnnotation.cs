using System;
using Lexico;

namespace Element.AST
{
    // ReSharper disable once ClassNeverInstantiated.Global
    public class TypeAnnotation
    {
        private IConstraint? _constraint;

#pragma warning disable 169
        [Literal(":"), WhitespaceSurrounded, MultiLine] private Unnamed _;

        // ReSharper disable once UnusedAutoPropertyAccessor.Local
        [field: Term] private Expression Expression { get; set; }
#pragma warning restore 169

        internal void Initialize(Declaration declarer) => Expression.Initialize(declarer);

        public override string ToString() => $":{Expression}";

        public IConstraint? ResolveConstraint(CompilationContext compilationContext) =>
            _constraint ?? Expression.ResolveExpression(compilationContext) switch
            {
                IConstraint constraint => _constraint = constraint,
                {} notConstraint => compilationContext.LogError(16, $"'{notConstraint}' is not a constraint"),
                _ => throw new ArgumentOutOfRangeException()
            };
    }
}