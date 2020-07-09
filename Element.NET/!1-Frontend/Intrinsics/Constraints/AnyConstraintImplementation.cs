namespace Element.AST
{
    /// <summary>
    /// A constraint that accepts anything, e.g. value, expression, function. The default constraint in Element.
    /// </summary>
    public sealed class AnyConstraintImplementation : IntrinsicConstraintImplementation
    {
        private AnyConstraintImplementation()
        {
            Identifier = new Identifier("Any");
        }
        public static AnyConstraintImplementation Instance { get; } = new AnyConstraintImplementation();
        public override Result<bool> MatchesConstraint(IValue value, CompilationContext context) => true;
        public override Identifier Identifier { get; }
    }
}