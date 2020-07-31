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
            _identifier = new Identifier("Any");
        }
        public static AnyConstraint Instance { get; } = new AnyConstraint();
        public override Result<bool> MatchesConstraint(IValue value, Context context) => true;
        protected override Identifier _identifier { get; }
    }
}