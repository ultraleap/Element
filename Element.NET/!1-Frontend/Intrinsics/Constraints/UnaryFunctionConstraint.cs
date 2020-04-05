namespace Element.AST
{
    /// <summary>
    /// A function type that accepts unary (1-arity) functions.
    /// </summary>
    public sealed class UnaryFunctionConstraint : IConstraint
    {
        private UnaryFunctionConstraint() { }

        public static UnaryFunctionConstraint Instance { get; } = new UnaryFunctionConstraint();
        public override string ToString() => "Unary";
        public bool MatchesConstraint(IValue value, CompilationContext compilationContext) =>
            value is IFunctionSignature fn && fn.Inputs.Length == 1;
        public IType Type => ConstraintType.Instance;
    }
}