using System;
using Lexico;

namespace Element.AST
{
    // ReSharper disable once ClassNeverInstantiated.Global
    public class TypeAnnotation : Declared
    {
        private IConstraint? _constraint;

#pragma warning disable 169, 8618
        [Literal(":"), WhitespaceSurrounded, MultiLine] private Unnamed _;

        // ReSharper disable once UnusedAutoPropertyAccessor.Local
#pragma warning disable 
        [field: Term] private Expression Expression { get; set; }
#pragma warning restore 169, 8618

        public override string ToString() => $":{Expression}";
        
        public Result<IConstraint> ResolveConstraint(IScope scope, CompilationContext context) =>
            _constraint != null
                ? new Result<IConstraint>(_constraint)
                : Expression.ResolveExpression(scope, context).Bind(value => value switch
                {
                    IConstraint constraint => new Result<IConstraint>(_constraint = constraint),
                    {} notConstraint => context.Trace(MessageCode.InvalidExpression, $"'{notConstraint}' is not a constraint"),
                    _ => throw new ArgumentOutOfRangeException()
                });

        protected override void InitializeImpl(IIntrinsicCache? cache) => Expression.Initialize(Declarer, cache);
        
        public override void Validate(ResultBuilder resultBuilder) {} // TODO: Disallow complex expressions e.g. calls
    }
}