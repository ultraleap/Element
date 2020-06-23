namespace Element.AST
{
    /// <summary>
    /// A constraint that accepts binary (2-arity) functions.
    /// </summary>
    public sealed class BinaryFunctionConstraint : IConstraint
    {
        private BinaryFunctionConstraint() {}
        public static BinaryFunctionConstraint Instance { get; } = new BinaryFunctionConstraint();
        Result<bool> IConstraint.MatchesConstraint(IValue value, CompilationContext compilationContext) =>
            value is IFunctionSignature fn && fn.Inputs.Length == 2;
    }
}