namespace Element.AST
{
    /// <summary>
    /// A type that accepts any other type, e.g. value, expression, function. The default type in Element.
    /// </summary>
    public sealed class AnyConstraint : IIntrinsic, IConstraint
    {
        private AnyConstraint() {}
        public static AnyConstraint Instance { get; } = new AnyConstraint();
        public string Location => "Any";
        public override string ToString() => "<any>";
        public bool MatchesConstraint(IValue value, CompilationContext compilationContext) => true;
        public IType Type { get; } = null; // TODO: Constraint type
    }
}