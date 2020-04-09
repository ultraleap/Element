namespace Element.AST
{
    /// <summary>
    /// A type that accepts any other type, e.g. value, expression, function. The default type in Element.
    /// </summary>
    public sealed class AnyConstraint : IIntrinsic, IConstraint
    {
        private AnyConstraint() {}
        public static AnyConstraint Instance { get; } = new AnyConstraint();
        public override string ToString() => "<any>";
        string IIntrinsic.Location => "Any";
        bool IConstraint.MatchesConstraint(IValue value, CompilationContext compilationContext) => true;
        IType IValue.Type => ConstraintType.Instance;
    }
}