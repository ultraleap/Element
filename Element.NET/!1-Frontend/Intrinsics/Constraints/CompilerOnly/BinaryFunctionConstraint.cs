namespace Element.AST
{
    /// <summary>
    /// A constraint that accepts binary (2-arity) functions.
    /// </summary>
    public sealed class BinaryFunctionConstraint : Value
    {
        private BinaryFunctionConstraint() {}
        public static BinaryFunctionConstraint Instance { get; } = new BinaryFunctionConstraint();
        public override string ToString() => "<binary function constraint>";
        public override Result<bool> MatchesConstraint(IValue value, CompilationContext context) =>
            value.FullyResolveValue(context).Map(v => v is IFunctionSignature fn && fn.Inputs.Count == 2);
    }
}