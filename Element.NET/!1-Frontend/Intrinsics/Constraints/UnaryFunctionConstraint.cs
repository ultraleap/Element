namespace Element.AST
{
    /// <summary>
    /// A function type that accepts unary (1-arity) functions.
    /// </summary>
    public sealed class UnaryFunctionConstraint : IConstraint
    {
        private UnaryFunctionConstraint() { }

        public static UnaryFunctionConstraint Instance { get; } = new UnaryFunctionConstraint();

        bool IConstraint.MatchesConstraint(IValue value, CompilationContext compilationContext) =>
            value is IFunctionSignature fn && fn.Inputs.Length == 1;
    }
}