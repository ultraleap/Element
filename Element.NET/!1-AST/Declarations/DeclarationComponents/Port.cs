using System;
using Lexico;

namespace Element.AST
{
    [WhitespaceSurrounded, MultiLine]
    public class Port : AstNode
    {
#pragma warning disable 649, 8618
        // ReSharper disable UnusedAutoPropertyAccessor.Local
        [Alternative(typeof(Identifier), typeof(Unidentifier))] private object _identifier;
        [field: Optional] public ReturnConstraint? ReturnConstraint { get; private set; }
        [field: Optional] public ExpressionBody? DefaultArgument { get; private set; }
        // ReSharper restore UnusedAutoPropertyAccessor.Local
#pragma warning restore 649, 8618

        public Identifier? Identifier => _identifier is Identifier id ? (Identifier?)id : null;
        public override string ToString() => $"{_identifier}{ReturnConstraint}";

        protected override void ValidateImpl(ResultBuilder builder, Context context)
        {
            if (_identifier is Identifier id) id.Validate(builder, Array.Empty<Identifier>(), Array.Empty<Identifier>()); // Don't validate identifier if this port has none
            ReturnConstraint?.Validate(builder, context);
            DefaultArgument?.Expression.Validate(builder, context);
        }

        public Result<ResolvedPort> Resolve(IScope scope, Context context)
        {
            Result<IValue> ResolveConstraint() => ReturnConstraint?.Expression.ResolveExpression(scope, context)
                                                  ?? AnyConstraint.Instance;
            Result<IValue> ResolveDefaultArgument() => DefaultArgument.Expression.ResolveExpression(scope, context);
            
            Result<ResolvedPort> WithDefaultArgument()
            {
                Result DefaultArgumentMatchesPortConstraint((IValue Constraint, IValue DefaultArgument) t) =>
                    t.Constraint.MatchesConstraint(t.DefaultArgument, context)
                     .Bind(matches => matches
                                          ? Result.Success
                                          : context.Trace(MessageCode.ConstraintNotSatisfied, $"Default argument value '{t.DefaultArgument}' does not match port constraint '{t.Constraint}'"));

                ResolvedPort ToResolvedPort((IValue Constraint, IValue DefaultArgument) t) => new ResolvedPort(Identifier, t.Constraint, t.DefaultArgument);

                return ResolveConstraint()
                       .Accumulate(ResolveDefaultArgument)
                       .Check(DefaultArgumentMatchesPortConstraint)
                       .Map(ToResolvedPort);
            }

            Result<ResolvedPort> WithoutDefaultArgument() => ResolveConstraint()
                .Map(constraint => new ResolvedPort(Identifier, constraint, null));

            return DefaultArgument != null
                       ? WithDefaultArgument()
                       : WithoutDefaultArgument();
        }
    }

    public class ResolvedPort
    {
        public ResolvedPort(Identifier? identifier, IValue resolvedConstraint, IValue? resolvedDefaultArgument)
        {
            Identifier = identifier;
            ResolvedConstraint = resolvedConstraint;
            ResolvedDefaultArgument = resolvedDefaultArgument;
        }

        public ResolvedPort(IValue constraint) => ResolvedConstraint = constraint;
        public static ResolvedPort VariadicPort { get; } = new ResolvedPort(VariadicPortMarker.Instance);

        public Identifier? Identifier { get; }
        public IValue ResolvedConstraint { get; }
        public IValue? ResolvedDefaultArgument { get; }
        public Result<IValue> DefaultValue(Context context) => ResolvedDefaultArgument != null ? new Result<IValue>(ResolvedDefaultArgument) : ResolvedConstraint.DefaultValue(context);
    }

    public class VariadicPortMarker : Value
    {
        public override string SummaryString => "<variadic port marker>";
        private VariadicPortMarker() {}
        public static VariadicPortMarker Instance { get; } = new VariadicPortMarker();
    }
}