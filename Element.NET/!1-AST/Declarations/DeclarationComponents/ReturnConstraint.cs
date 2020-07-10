using Lexico;

namespace Element.AST
{
    public class ReturnConstraint : AstNode
    {
#pragma warning disable 169, 8618
        [Literal(":"), WhitespaceSurrounded, MultiLine] private Unnamed _;

        // ReSharper disable once UnusedAutoPropertyAccessor.Local
        [field: Term] public Expression Expression { get; private set; }
#pragma warning restore 169, 8618
        
        public override string ToString() => $":{Expression}";

        protected override void ValidateImpl(ResultBuilder resultBuilder, CompilationContext context)
        {
            Expression.Validate(resultBuilder, context);
            // TODO: Disallow complex expressions e.g. calls
        }
    }
    
    public static class ReturnConstraintExtensions
    {
        public static Result<IValue> ResolveReturnConstraint(this ReturnConstraint? returnConstraint, IScope scope, CompilationContext context) =>
            returnConstraint?.Expression.ResolveExpression(scope, context)
            ?? scope.Lookup(AnyConstraint.Instance.Identifier, context);
    }
}