namespace Element.AST
{
    /// <summary>
    /// A function type that accepts unary (1-arity) functions.
    /// </summary>
    public sealed class UnaryFunctionConstraint : IConstraint
    {
        private UnaryFunctionConstraint() { }

        public static UnaryFunctionConstraint Instance { get; } = new UnaryFunctionConstraint();

        Result<bool> IConstraint.MatchesConstraint(IValue value, CompilationContext context) =>
            value.FullyResolveValue(context).Map(v => v is IFunction fn && fn.Inputs.Count == 1);
    }
}