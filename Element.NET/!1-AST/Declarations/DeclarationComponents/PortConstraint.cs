using Lexico;

namespace Element.AST
{
    public class PortConstraint : AstNode
    {
#pragma warning disable 169, 8618
        [Literal(":"), WhitespaceSurrounded(ParserFlags = ParserFlags.IgnoreInTrace), MultiLine] private Unnamed _;

        // ReSharper disable once UnusedAutoPropertyAccessor.Local
        [field: Term] public Expression Expression { get; private set; }
#pragma warning restore 169, 8618
        
        public override string ToString() => $":{Expression}";

        protected override void ValidateImpl(ResultBuilder builder, Context context)
        {
            Expression.Validate(builder, context);
            // TODO: Disallow complex expressions e.g. calls
        }
    }
    
    public static class ReturnConstraintExtensions
    {
        public static Result<IValue> ResolvePortConstraint(this PortConstraint? portConstraint, IScope scope, Context context) =>
            portConstraint?.Expression.ResolveExpression(scope, context) ?? AnyConstraint.Instance;
    }
}