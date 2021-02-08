namespace Element.AST
{
    /// <summary>
    /// A constraint that accepts nothing.
    /// </summary>
    public sealed class NothingConstraint : IntrinsicValue, IIntrinsicConstraintImplementation
    {
        // Can be IntrinsicValue directly because it doesn't rely on anything declared in source to function
        private NothingConstraint()
        {
            Identifier = new Identifier("Nothing");
        }
        public static NothingConstraint Instance { get; } = new NothingConstraint();
        public override Result MatchesConstraint(IValue value, Context context) => context.Trace(EleMessageCode.ConstraintNotSatisfied, "Nothing constraint cannot be matched");
        public override Identifier Identifier { get; }
    }
}