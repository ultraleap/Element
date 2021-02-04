namespace Element.AST
{
    /// <summary>
    /// A constraint that accepts any value, e.g. struct, constraint, function, namespace. The default constraint in Element.
    /// </summary>
    public sealed class AnyConstraint : IntrinsicValue, IIntrinsicConstraintImplementation
    {
        // Can be IntrinsicValue directly because it doesn't rely on anything declared in source to function
        private AnyConstraint()
        {
            Identifier = new Identifier("Any");
        }
        public static AnyConstraint Instance { get; } = new AnyConstraint();
        public override Result MatchesConstraint(IValue value, Context context) => Result.Success;
        public override Identifier Identifier { get; }
    }
}