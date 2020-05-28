namespace Element.AST
{
    /// <summary>
    /// A type that accepts any function.
    /// </summary>
    public sealed class FunctionConstraint : IConstraint
    {
        private FunctionConstraint() {}
        public static FunctionConstraint Instance { get; } = new FunctionConstraint();
        public override string ToString() => "<function>";
        bool IConstraint.MatchesConstraint(IValue value, CompilationContext compilationContext) => value is IFunctionSignature;
    }
}