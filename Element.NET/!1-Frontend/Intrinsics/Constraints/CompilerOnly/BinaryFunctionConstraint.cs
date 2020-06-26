namespace Element.AST
{
    /// <summary>
    /// A constraint that accepts binary (2-arity) functions.
    /// </summary>
    public sealed class BinaryFunctionConstraint : IConstraint
    {
        private BinaryFunctionConstraint() {}
        public static BinaryFunctionConstraint Instance { get; } = new BinaryFunctionConstraint();
        Result<bool> IConstraint.MatchesConstraint(IValue value, CompilationContext context) =>
            value.FullyResolveValue(context).Map(v => v is IFunction fn && fn.Inputs.Count == 2);
    }
}