namespace Element.AST
{
    /// <summary>
    /// A type that accepts any other type, e.g. value, expression, function. The default constraint in Element.
    /// </summary>
    public sealed class AnyConstraint : IntrinsicConstraint
    {
        private AnyConstraint() {}
        public static AnyConstraint Instance { get; } = new AnyConstraint();
        public override Result<bool> MatchesConstraint(IValue value, CompilationContext context) => true;
        public override Identifier Identifier => new Identifier("Any");
    }
}