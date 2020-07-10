namespace Element.AST
{
    /// <summary>
    /// A constraint that accepts any value, e.g. struct, constraint, function, namespace. The default constraint in Element.
    /// </summary>
    public sealed class AnyConstraint : IntrinsicConstraintImplementation
    {
        private AnyConstraint()
        {
            Identifier = new Identifier("Any");
        }
        public static AnyConstraint Instance { get; } = new AnyConstraint();
        public override Result<bool> MatchesConstraint(IValue value, CompilationContext context) => true;
        public override Identifier Identifier { get; }
    }
}