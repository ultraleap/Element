using System;
using Lexico;

namespace Element.AST
{
    // ReSharper disable once ClassNeverInstantiated.Global
    public class Type
    {
#pragma warning disable 169
        [Literal(":"), WhitespaceSurrounded, MultiLine] private Unnamed _;
        // ReSharper disable once UnusedAutoPropertyAccessor.Local
        [field: Term] public Expression Expression { get; private set; }
#pragma warning restore 169

        public override string ToString() => $":{Expression}";
    }

    public static class TypeExtensions
    {
        public static IConstraint Resolve(this Type type, IScope startingScope, CompilationContext compilationContext)
        {
            if (type == null) return AnyConstraint.Instance;
            if (startingScope == null) throw new ArgumentNullException(nameof(startingScope));

            return type.Expression.ResolveExpression(startingScope, compilationContext) switch
            {
                IConstraint constraint => constraint,
                {} notConstraint => compilationContext.LogError(16, $"'{notConstraint}' is not a constraint"),
                null => throw new ArgumentOutOfRangeException()
            };
        }
    }
}