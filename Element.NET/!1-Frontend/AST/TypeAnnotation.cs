using Lexico;

namespace Element.AST
{
    // ReSharper disable once ClassNeverInstantiated.Global
    public class TypeAnnotation : Declared
    {
        private IValue? _constraint;

#pragma warning disable 169, 8618
        [Literal(":"), WhitespaceSurrounded, MultiLine] private Unnamed _;

        // ReSharper disable once UnusedAutoPropertyAccessor.Local
#pragma warning disable 
        [field: Term] private Expression Expression { get; set; }
#pragma warning restore 169, 8618

        public override string ToString() => $":{Expression}";
        
        public Result<IValue> ResolveConstraint(IScope scope, CompilationContext context) =>
            _constraint != null
                ? new Result<IValue>(_constraint)
                : Expression.ResolveExpression(scope, context).Map(value => _constraint = value);

        protected override void InitializeImpl() => Expression.Initialize(Declarer);
        
        public override void Validate(ResultBuilder resultBuilder) {} // TODO: Disallow complex expressions e.g. calls
    }
}