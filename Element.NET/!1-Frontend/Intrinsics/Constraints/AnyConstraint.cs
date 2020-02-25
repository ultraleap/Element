namespace Element.AST
{
    /// <summary>
    /// A type that accepts any other type, e.g. value, expression, function. The default type in Element.
    /// </summary>
    public sealed class AnyConstraint : IntrinsicConstraint
    {
        private AnyConstraint() {}
        public static AnyConstraint Instance { get; } = new AnyConstraint();
        public override string Location { get; } = "Any";
        public override string ToString() => "<any>";
        public override bool MatchesConstraint(IValue value, CompilationContext compilationContext) => true;
    }
}