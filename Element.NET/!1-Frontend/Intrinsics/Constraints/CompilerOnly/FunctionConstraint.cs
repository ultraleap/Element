namespace Element.AST
{
    /// <summary>
    /// A constraint that accepts any function.
    /// </summary>
    public sealed class FunctionConstraint : IConstraint
    {
        private FunctionConstraint() {}
        public static FunctionConstraint Instance { get; } = new FunctionConstraint();
        public override string ToString() => "<function>";
        Result<bool> IConstraint.MatchesConstraint(IValue value, CompilationContext compilationContext) => value is IFunctionSignature;
    }
}