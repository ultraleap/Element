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
            _identifier = new Identifier("Nothing");
        }
        public static NothingConstraint Instance { get; } = new NothingConstraint();
        public override Result<bool> MatchesConstraint(IValue value, CompilationContext context) => false;
        protected override Identifier _identifier { get; }
    }
}