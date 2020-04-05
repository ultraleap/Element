namespace Element.AST
{
    /// <summary>
    /// A function type that accepts binary (2-arity) functions.
    /// </summary>
    public sealed class BinaryFunctionConstraint : IConstraint
    {
        private BinaryFunctionConstraint() {}
        public static BinaryFunctionConstraint Instance { get; } = new BinaryFunctionConstraint();
        public override string ToString() => "Binary";
        public bool MatchesConstraint(IValue value, CompilationContext compilationContext) =>
            value is IFunctionSignature fn && fn.Inputs.Length == 2;
        public IType Type => ConstraintType.Instance;
        public int? SerializableSize => null;
    }
}